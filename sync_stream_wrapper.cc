#include "sync_stream_wrapper.h"
#include <openssl/ssl.h>

namespace asio = boost::asio;

tcp_socket::tcp_socket()
  : socket_(io_context_) {
}

void tcp_socket::connect(const boost::asio::ip::address& ip, uint16_t port) {
  socket_.connect(asio::ip::tcp::endpoint(ip, port));
}

std::size_t tcp_socket::read_some(const boost::asio::mutable_buffer& buffer) {
  return socket_.read_some(buffer);
}
std::size_t tcp_socket::read_some(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) {
  return socket_.read_some(buffer, ec);
}

std::size_t tcp_socket::write_some(const boost::asio::const_buffer& buffer) {
  return socket_.write_some(buffer);
}
std::size_t tcp_socket::write_some(const boost::asio::const_buffer& buffer, boost::system::error_code& ec) {
  return socket_.write_some(buffer, ec);
}


ssl_stream::ssl_stream()
  : ssl_stream(false) {
}

ssl_stream::ssl_stream(bool http2)
  : ssl_context_(asio::ssl::context::tlsv12_client) {

  if (http2) {
    auto native_ctx = ssl_context_.native_handle();
    SSL_CTX_set_alpn_protos(native_ctx, (const unsigned char *)"\x02h2", 3);
  }

  stream_.reset(new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(io_context_, ssl_context_));
}

void ssl_stream::connect(const boost::asio::ip::address& ip, uint16_t port) {
  // TODO 証明書
  stream_->lowest_layer().connect(asio::ip::tcp::endpoint(ip, port));

  stream_->handshake(asio::ssl::stream_base::client);
}

std::size_t ssl_stream::read_some(const boost::asio::mutable_buffer& buffer) {
  return stream_->read_some(buffer);
}
std::size_t ssl_stream::read_some(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) {
  return stream_->read_some(buffer, ec);
}

std::size_t ssl_stream::write_some(const boost::asio::const_buffer& buffer) {
  return stream_->write_some(buffer);
}
std::size_t ssl_stream::write_some(const boost::asio::const_buffer& buffer, boost::system::error_code& ec) {
  return stream_->write_some(buffer, ec);
}

boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& ssl_stream::inner_stream() const {
  return *stream_;
}

