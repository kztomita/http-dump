#include "hpack_decoder.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include "hpack_table.h"
#include "hpack_huffman.h"

namespace hpack {

namespace {

std::pair<int, std::size_t>
parse_integer_representation(const buffer_view<unsigned char>& buffer, std::size_t offset, std::size_t n) {
  // 2 ^ n - 1
  int value_mask = (1 << n) - 1;

  int value = buffer.at(offset) & value_mask;
  if (value < value_mask) {
    return std::pair(value, 1);
  }

  // 全bitが立っている

  std::size_t next = offset + 1;

  int m = 0;
  while (true) {
    int octet = buffer.at(next);
    next++;

    value += (octet & 0x7f) << m;
    m += 7;

    if (!(octet & 0x80)) {
      break;
    }
  }

  return std::pair(value, next - offset);
}

std::pair<std::string, std::size_t>
parse_string_literal_representation(const buffer_view<unsigned char>& buffer, std::size_t offset) {
  unsigned int top_byte = buffer.at(offset);

  // String Length
  auto integer_len = parse_integer_representation(buffer, offset, 7);
  std::size_t len = integer_len.first;

  std::size_t representation_size = integer_len.second + len;

  auto string_data_buffer = buffer.subview(offset + integer_len.second, len);

  if (top_byte & 0x80) {
    auto decoded = decode_huffman(string_data_buffer.address(0), string_data_buffer.size());
    return std::pair(std::string(reinterpret_cast<const char*>(&decoded[0]),
                                 decoded.size()),
                     representation_size);
  } else {
    return std::pair(std::string(reinterpret_cast<const char*>(string_data_buffer.address(0)),
                                 string_data_buffer.size()),
                     representation_size);
  }
}

}  // unnamed namespace


// 前方宣言した不完全型hpack_dynamic_tableのunique_ptrを保持するのに必要
decoding_context::decoding_context()
  : dynamic_table(std::make_unique<hpack_dynamic_table>()) {}
decoding_context::~decoding_context() = default;


hpack_decoder::hpack_decoder(const unsigned char* buffer, std::size_t length)
  : buffer_view_(buffer, length) {
}

hpack_decoder::header_list_type hpack_decoder::decode(decoding_context& context) {
  decode_pos_ = 0;

  header_list_type headers;
  while (true) {
    auto result = decode_representation(context);
    // Dynamic Table Size Updateの場合はヘッダはないが
    // result.endはfalseなので処理を続ける。
    if (result.header) {
      headers.push_back(result.header.value());
    }
    if (result.end) {
      break;
    }
  }

  return headers;
}

int hpack_decoder::fetch_integer(std::size_t n) {
  auto integer = parse_integer_representation(buffer_view_, decode_pos_, n);
  decode_pos_ += integer.second;
  return integer.first;
}

std::string hpack_decoder::fetch_string() {
  auto string_literal = parse_string_literal_representation(buffer_view_, decode_pos_);
  decode_pos_ += string_literal.second;
  return string_literal.first;
}

header_type hpack_decoder::fetch_header(const decoding_context& context, std::size_t index) {
  if (index) {
    // parse value string literal

    auto h = hpack_table_at(*context.dynamic_table, index);
    if (!h) {
      throw std::runtime_error("header not found in tables.");
    }

    // Value String
    std::string value = fetch_string();

    return header_type(h.value().first, value);
  } else {
    // parse name string literal and value string literal

    // Name String
    std::string name = fetch_string();

    // Value String
    std::string value = fetch_string();

    return header_type(name, value);
  }
}

hpack_decoder::decoded_result hpack_decoder::decode_representation(decoding_context& context) {

  if (!buffer_view_.test_range(decode_pos_, 1)) {
    return decoded_result(std::nullopt, true);
  }

  int top_byte = buffer_view_.at(decode_pos_);

  if (top_byte & 0x80) {
    // 0b1...
    // Indexed Header Field Representation
    auto index = fetch_integer(7);
    return decoded_result(hpack_table_at(*context.dynamic_table, index),
                          false);
  } else if ((top_byte & 0xc0) == 0x40) {
    // 0b01...
    // Literal Header Field with Incremental Indexing
    auto index = fetch_integer(6);
    header_type header = fetch_header(context, index);
    context.dynamic_table->add(header);
    return decoded_result(std::move(header),
                          false);
  } else if ((top_byte & 0xf0) == 0x00) {
    // 0b0000...
    // Literal Header Field without Indexing
    auto index = fetch_integer(4);
    return decoded_result(fetch_header(context, index),
                          false);
  } else if ((top_byte & 0xf0) == 0x10) {
    // 0b0001...
    // Literal Header Field Never Indexed
    auto index = fetch_integer(4);
    return decoded_result(fetch_header(context, index),
                          false);
  } else if ((top_byte & 0xe0) == 0x20) {
    // 0b001...
    // Dynamic Table Size Update
    // このエントリはコマンドなのでヘッダはない
    auto max_size = fetch_integer(5);
    context.dynamic_table->set_maximum_table_size(max_size);
    return decoded_result(std::nullopt, false);
  }

  throw std::runtime_error("Unknown representation.");
}

} // namespace hpack

