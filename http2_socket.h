#ifndef HTTP2_SOCKET_H
#define HTTP2_SOCKET_H

#include <cstring>
#include "http2_frame.h"

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

template<typename SyncWriteStream>
void write_http2_frame(SyncWriteStream& socket, const http2_frame& frame) {
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


template<typename SyncWriteStream>
void send_http2_window_update(SyncWriteStream& stream, uint32_t stream_id, uint32_t increment) {
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


#endif
