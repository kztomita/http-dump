#ifndef HTTP2_SOCKET_H
#define HTTP2_SOCKET_H

#include <algorithm>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include <boost/beast/core.hpp>
#include "hpack_encoder.h"
#include "http_header_list.h"
#include "http2_frame.h"
#include "string_util_tolower.h"

template<typename SyncReadStream>
http2_frame read_http2_frame(SyncReadStream& socket) {
  static_assert(boost::beast::is_sync_read_stream<SyncReadStream>::value,
                "SyncReadStream requirements not met");

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

template<typename SyncWriteStream>
void write_http2_frame(SyncWriteStream& socket, const http2_frame& frame) {
  static_assert(boost::beast::is_sync_write_stream<SyncWriteStream>::value,
                "SyncWriteStream requirements not met");

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


using settings_parameter = std::pair<uint16_t, uint32_t>;

template<typename SyncWriteStream, typename T>
void send_http2_settings(SyncWriteStream& stream, const T& parameters) {
  static_assert(boost::beast::is_sync_write_stream<SyncWriteStream>::value,
                "SyncWriteStream requirements not met");
  static_assert(std::is_same<typename T::value_type, settings_parameter>::value,
                "T::value_type is not settings_parameter");

  http2_frame settings;

  auto& header = settings.header();
  header.type = http2_frame_header::TYPE_SETTINGS;
  header.flags = 0x00;
  header.set_stream_id(0);
  header.flags = 0x00;

  auto& payload = settings.payload();
  for (const auto&p : parameters) {
    auto id = htons(p.first);
    auto value = htonl(p.second);
    payload.resize(payload.size() + 6);

    std::memcpy(&payload[payload.size() - 6], &id, 2);
    std::memcpy(&payload[payload.size() - 4], &value, 4);
  }

  header.set_length(payload.size());

  write_http2_frame(stream, settings);
}

template<typename SyncWriteStream>
void send_http2_settings(SyncWriteStream& stream) {
  static_assert(boost::beast::is_sync_write_stream<SyncWriteStream>::value,
                "SyncWriteStream requirements not met");

  send_http2_settings(stream, std::vector<settings_parameter>());
}

template<typename SyncWriteStream>
void send_http2_settings_ack(SyncWriteStream& stream) {
  static_assert(boost::beast::is_sync_write_stream<SyncWriteStream>::value,
                "SyncWriteStream requirements not met");

  http2_frame settings;

  auto& header = settings.header();
  header.set_length(0);
  header.type = http2_frame_header::TYPE_SETTINGS;
  header.flags = 0x00;
  header.set_stream_id(0);
  header.flags = 0x01;  // ACK

  write_http2_frame(stream, settings);
}

template<typename SyncWriteStream>
void send_http2_headers(SyncWriteStream& stream,
                        uint32_t stream_id,
                        const std::string& host,
                        const std::string& path,
                        const http_header_list& req_headers,
                        std::size_t max_payload_len = 16384) {
  static_assert(boost::beast::is_sync_write_stream<SyncWriteStream>::value,
                "SyncWriteStream requirements not met");

  std::vector<hpack::header_type> header_array{
    {":method", "GET"},
    {":scheme", "https"},
    {":path", path},
    {":authority", host},
  };
  // XXX ヘッダーが多くても現状フラグメントは行っていない
  for (const auto& h : req_headers) {
    header_array.emplace_back(hpack::header_type{
        string_util::tolower(h->key),
        h->value});
  }

  // HPACK(rfc7541)形式でヘッダーを作成
  hpack::hpack_encoder encoder;
  auto header_block = encoder.encode<http2_frame::payload_type>(header_array);

  if (header_block.size() <= max_payload_len) {
    http2_frame headers;
    auto& header = headers.header();
    header.type = http2_frame_header::TYPE_HEADERS;
    header.flags = 0x05;         // END_HEADERS | END_STREAM
    header.set_stream_id(stream_id);

    headers.header().set_length(header_block.size());
    using std::swap;
    swap(headers.payload(), header_block);

    write_http2_frame(stream, headers);
  } else {
    // HEADERS AND CONTINUATION
    // nginx相手だとENHANCE YOUR CALMが返される
    {
      http2_frame headers;
      auto& header = headers.header();
      header.type = http2_frame_header::TYPE_HEADERS;
      header.flags = 0x01;         // END_STREAM
      header.set_stream_id(stream_id);

      headers.header().set_length(max_payload_len);
      std::copy(header_block.begin(),
                header_block.begin() + max_payload_len,
                std::back_inserter(headers.payload()));

      write_http2_frame(stream, headers);
    }

    std::size_t offset = max_payload_len;
    while (offset < header_block.size()) {
      auto left_bytes = header_block.size() - offset;
      auto len = std::min(left_bytes, max_payload_len);

      http2_frame continuation;
      auto& header = continuation.header();
      header.type = http2_frame_header::TYPE_CONTINUATION;
      header.flags = 0x00;
      if (len <= max_payload_len) {
        header.flags |= 0x04;       // END_HEADERS
      }
      header.set_stream_id(stream_id);

      continuation.header().set_length(len);
      auto start = header_block.begin() + offset;
      std::copy(start,
                start + len,
                std::back_inserter(continuation.payload()));

      write_http2_frame(stream, continuation);

      offset += len;
    }
  }
}

template<typename SyncWriteStream>
void send_http2_window_update(SyncWriteStream& stream, uint32_t stream_id, uint32_t increment) {
  static_assert(boost::beast::is_sync_write_stream<SyncWriteStream>::value,
                "SyncWriteStream requirements not met");

  if (increment & 0x80000000) {
    throw std::invalid_argument("increment too large");
  }

  http2_frame window_update;

  auto& header = window_update.header();
  header.set_length(4);
  header.type = http2_frame_header::TYPE_WINDOW_UPDATE;
  header.flags = 0x00;
  header.set_stream_id(stream_id);

  auto& payload = window_update.payload();
  payload.resize(4);
  uint32_t value;
  value = htonl(increment);
  std::memcpy(&payload[0], &value, 4);

  write_http2_frame(stream, window_update);
}

template<typename SyncWriteStream>
void send_http2_goaway(SyncWriteStream& stream, uint32_t stream_id) {
  static_assert(boost::beast::is_sync_write_stream<SyncWriteStream>::value,
                "SyncWriteStream requirements not met");

  http2_frame goaway;

  auto& header = goaway.header();
  header.set_length(8);
  header.type = http2_frame_header::TYPE_GOAWAY;
  header.flags = 0x00;
  header.set_stream_id(0);

  auto& payload = goaway.payload();
  payload.resize(8);
  uint32_t value;
  value = htonl(stream_id);   // Last Stream Id
  std::memcpy(&payload[0], &value, 4);
  value = htonl(0);           // Error Code
  std::memcpy(&payload[4], &value, 4);

  write_http2_frame(stream, goaway);
}


#endif
