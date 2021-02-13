#ifndef SOCKET_H
#define SOCKET_H

#include <optional>
#include <sstream>
#include <string>
#include <boost/asio.hpp>
#include "http_response.h"
#include "http2_frame.h"
#include "string_util_split.h"
#include "string_util_tolower.h"
#include "debug.h"

// SyncReadStream:  boost::asio::ip::tcp::socket,
//                  asio::ssl::stream<asio::ip::tcp::socket>, and so on.
// read(),read_until()へ渡す型。
//
// Ref.
// https://www.boost.org/doc/libs/1_64_0/doc/html/boost_asio/reference/SyncReadStream.html

template<typename SyncReadStream>
bool read_line(SyncReadStream& socket, boost::asio::streambuf& receive_buffer, std::string& line) {
  namespace asio = boost::asio;

  boost::system::error_code error;

  // receive_bufferにdelimiterより先のデータもまとめて読み込まれ場合がある。
  // この場合、sizeとreceive_buffer.size()が異ることがあるのでsizeの方を
  // 参照する必要がある。
  std::size_t size = asio::read_until(socket, receive_buffer, "\r\n", error);
  if (error && error != asio::error::eof) {
    throw std::runtime_error(error.message());
  }
  if (error == asio::error::eof) {
    return false;
  }

  const char* data = asio::buffer_cast<const char*>(receive_buffer.data());
  line.assign(data, size - 2);

  receive_buffer.consume(size);

  return true;
}

template<typename SyncReadStream>
void fill_streambuf(SyncReadStream& socket, boost::asio::streambuf& receive_buffer, std::size_t size) {
  namespace asio = boost::asio;

  if (receive_buffer.size() < size) {
    asio::read(socket, receive_buffer, boost::asio::transfer_at_least(size - receive_buffer.size()));
  }
}

template<typename SyncReadStream>
http_response read_response(SyncReadStream& socket) {
  namespace asio = boost::asio;

  std::string line;
  asio::streambuf receive_buffer;

  http_response r;

  // Status Line読み込み
  read_line(socket, receive_buffer, line);
  auto elements = string_util::split(line, " ");
  r.code_ = std::atoi(elements[1].c_str());

  // ヘッダー読み込み
  std::optional<std::string> transfer_encoding;
  std::optional<std::size_t> content_length;

  while (true) {
    if (!read_line(socket, receive_buffer, line)) {
      break;
    }
    if (line.size() == 0) {
      // ヘッダー終了
      break;
    }
    auto h = create_http_header_from_header_line(line);

    if (string_util::tolower(h->key) == "transfer-encoding") {
      transfer_encoding = h->value;
    } else if (string_util::tolower(h->key) == "content-length") {
      content_length = std::atoi(h->value.c_str());
    }

    r.headers_.push_back(std::move(h));
  }

  // payload読み込み

  if (!transfer_encoding ||
      transfer_encoding == "identity") {
    if (!content_length) {
      throw std::runtime_error("Content-Length not found.");
    }
    std::size_t length = content_length.value();

    fill_streambuf(socket, receive_buffer, length);
    const char* data = asio::buffer_cast<const char*>(receive_buffer.data());
    http_response::payload_type payload(data, data + length);
    receive_buffer.consume(length);

    using std::swap;
    swap(r.payload_, payload);
  } else if (transfer_encoding == "chunked") {
    http_response::payload_type payload;
    std::string line;
    while (read_line(socket, receive_buffer, line)) {
      if (line.size() == 0) {
        break;
      }
      std::size_t pos;
      std::size_t chunk_size = std::stoi(line.c_str(), &pos, 16);
      if (pos != line.size()) {
        throw std::runtime_error("Can't parse chunk size.");
      }
      if (chunk_size == 0) {
        // \r\n読み捨て
        std::string l;
        read_line(socket, receive_buffer, l);
        break;
      }

      fill_streambuf(socket, receive_buffer, chunk_size);
      const char* data = asio::buffer_cast<const char*>(receive_buffer.data());
      payload.resize(payload.size() + chunk_size);
      std::copy(data, data + chunk_size, payload.end() - chunk_size);
      receive_buffer.consume(chunk_size);

      // \r\n読み捨て
      fill_streambuf(socket, receive_buffer, 2);
      receive_buffer.consume(2);
    }

    using std::swap;
    swap(r.payload_, payload);
  } else {
    std::ostringstream msg;
    msg << "Unsupported Transfer-Encoding type ("
        << transfer_encoding.value()
        << ").";
    throw std::runtime_error(msg.str());
  }

  return r;
}

template<typename SyncReadStream>
http2_frame read_http2_frame(SyncReadStream& socket) {
  namespace asio = boost::asio;

  asio::streambuf receive_buffer;

  asio::read(socket, receive_buffer, boost::asio::transfer_exactly(9));

  const char* headerp = asio::buffer_cast<const char*>(receive_buffer.data());

  http2_frame frame;

  std::memcpy(&frame.header(), headerp, 9);
  receive_buffer.consume(9);

  uint payload_size = frame.header().length();

  if (payload_size) {
    frame.payload().resize(payload_size);

    asio::read(socket, receive_buffer, boost::asio::transfer_exactly(payload_size));

    const char* payloadp = asio::buffer_cast<const char*>(receive_buffer.data());
    std::memcpy(frame.payload_buffer(), payloadp, payload_size);
  }

  debug_message("RECV");
  debug_dump(frame);

  return frame;
}

template<typename SyncReadStream>
void write_http2_frame(SyncReadStream& socket, const http2_frame& frame) {
  namespace asio = boost::asio;

  if (frame.header().length() != frame.payload().size()) {
    std::cerr << "payload length mismatch." << std::endl;
  }

  asio::write(socket, asio::buffer(reinterpret_cast<const char*>(&frame.header()), sizeof(http2_frame_header)));

  if (frame.payload().size()) {
    asio::write(socket, asio::buffer(reinterpret_cast<const char*>(frame.payload_buffer()), frame.payload().size()));
  }

  debug_message("SEND");
  debug_dump(frame);
}

#endif
