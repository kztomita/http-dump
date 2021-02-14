#include "h2.h"
#include <algorithm>
#include <locale>
#include <vector>
#include <utility>
#include <stdexcept>
#include <string_view>
#include <boost/asio/ssl.hpp>
#include "debug.h"
#include "http2_frame.h"
#include "http2_socket.h"
#include "http_header.h"
#include "hpack_decoder.h"
#include "socket.h"

namespace asio = boost::asio;

namespace {

std::string convert_header_name(const std::string& name) {
  std::string converted;

  std::size_t i = 0;
  if (name[i] == ':') {
    i = 1;
  }

  bool upper = true;
  while (i < name.size()) {
    auto ch = name[i];
    if (upper && ch >= 'a' && ch <= 'z') {
      converted += std::toupper(ch);
      upper = false;
    } else {
      converted += std::tolower(ch);
    }
    if (ch == '-') {
      upper = true;
    }
    i++;
  }

  return converted;
}

} // unnamed namespace


http_response h2::get(const boost::asio::ip::address& ip, uint16_t port, const std::string& host, const std::string& path, const http_header_list& req_headers) {
  asio::io_context io_context;

  asio::ssl::context ctx(asio::ssl::context::tlsv12_client);

  auto native_ctx = ctx.native_handle();
  SSL_CTX_set_alpn_protos(native_ctx, (const unsigned char *)"\x02h2", 3);

  asio::ssl::stream<asio::ip::tcp::socket> stream(io_context, ctx);

  // TODO 証明書

  stream.lowest_layer().connect(asio::ip::tcp::endpoint(ip, port));
  stream.lowest_layer().set_option(asio::ip::tcp::no_delay(true));

  stream.handshake(boost::asio::ssl::stream_base::client);

  const unsigned char* alpn_data = nullptr;
  unsigned int alpn_len = 0;
  SSL_get0_alpn_selected(stream.native_handle(), &alpn_data, &alpn_len);
  if (alpn_len == 0) {
    throw std::runtime_error("http/2 is not selected.");
  }
  if (std::string_view(reinterpret_cast<const char*>(alpn_data), alpn_len) != "h2") {
    throw std::runtime_error("http/2 is not selected.");
  }

  std::ostringstream request_stream;
  request_stream << "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
  asio::write(stream, asio::buffer(request_stream.str()));

  // SETTINGS送信
#if 1
  send_http2_settings(stream);
#else
  // パラメータを指定する場合
  std::vector<settings_parameter> params{
    {SETTINGS_MAX_CONCURRENT_STREAMS, 10},
    {SETTINGS_MAX_FRAME_SIZE, 16384},
  };
  send_http2_settings(stream, params);
#endif

  // SETTINGS受信
  bool rcv_settings = false;
  bool rcv_settings_ack = false;
  while (!rcv_settings || !rcv_settings_ack) {
    auto frame = read_http2_frame(stream);
    if (frame.type() == http2_frame_header::TYPE_SETTINGS) {
      if (frame.flags() & 0x01) {  // ACK
        rcv_settings_ack = true;
      } else {
        rcv_settings = true;
        // SETTINGS ACK送信
        send_http2_settings_ack(stream);
      }
    }
  }

  uint32_t stream_id = 1;

  // HEADERS送信
  send_http2_headers(stream, stream_id, host, path, req_headers);

  // HEADERS受信
  // Header Block FragmentからHeader Blockを構築
  http2_frame::payload_type header_block;
  auto headers_frame = read_http2_frame(stream);
  if (headers_frame.type() != http2_frame_header::TYPE_HEADERS) {
    throw std::runtime_error("The frame which isn't a HEADERS frame was received.");
  }
  std::size_t fragment_offset = 0;
  std::size_t pad_length = 0;
  if (headers_frame.flags() & 0x08) {        // PADDED flag
    pad_length = headers_frame.payload()[fragment_offset];
    fragment_offset += 1;
  }
  if (headers_frame.flags() & 0x20) {        // PRIORITY flag
    fragment_offset += 5;
  }
  header_block.insert(header_block.end(),
                      headers_frame.payload().begin() + fragment_offset,
                      headers_frame.payload().end() - pad_length);

  if (!(headers_frame.flags() & 0x04)) {  // !END_HEADERS
    // CONTINUATIONが続く
    while (true) {
      auto continuation_frame = read_http2_frame(stream);
      if (continuation_frame.type() != http2_frame_header::TYPE_CONTINUATION) {
        throw std::runtime_error("The frame which isn't a CONTINUATION frame was received.");
      }

      header_block.insert(header_block.end(),
                          continuation_frame.payload().begin(),
                          continuation_frame.payload().end());

      if (continuation_frame.flags() & 0x04) {  // END_HEADERS
        break;
      }
    }
  }

  hpack::hpack_decoder decoder(&header_block[0], header_block.size());
  hpack::decoding_context context;
  auto header_list = decoder.decode(context);

  debug_message("HPACK Dynamic Table");
  debug_dump(*context.dynamic_table);
  debug_message("");

  debug_message("HPACK Decoded header list");
  for (const auto& h : header_list) {
    debug_message(h.first + " " + h.second);
  }
  debug_message("");

  // DATA受信
  http_response::payload_type response_body;

  while (true) {
    auto data_frame = read_http2_frame(stream);
    if (data_frame.type() != http2_frame_header::TYPE_DATA) {
      throw std::runtime_error("The frame which isn't a DATA frame was received.");
    }
    if (data_frame.flags() & 0x08) {
      throw std::runtime_error("padding flag not supported.");
    }
    response_body.insert(response_body.end(), data_frame.payload().begin(), data_frame.payload().end());
    if (data_frame.flags() & 0x01) { // END_STREAM
      break;
    }

    // WINDOW UPDATE送信
    send_http2_window_update(stream, stream_id, data_frame.length());
    send_http2_window_update(stream, 0, data_frame.length());         // connection level
  }

  // GOAWAY送信
  send_http2_goaway(stream, stream_id);

  http_response r;

  auto status_header = std::find_if(header_list.begin(), header_list.end(),
                                    [] (const hpack::header_type& h) {
                                      return h.first == ":status";
                                    });
  if (status_header != header_list.end()) {
    r.code_ = std::stoi(status_header->second);
  } else {
    std::runtime_error("status header not found.");
  }

  for (const auto& h : header_list) {
    if (h.first == ":status") {
      continue;
    }
    r.headers_.push_back(std::make_unique<http_header>(convert_header_name(h.first), h.second));
  }

  swap(r.payload_, response_body);

  return r;
}

