#ifndef SYNC_STREAM_WRAPPER_H
#define SYNC_STREAM_WRAPPER_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>

class sync_stream_wrapper {
public:
  virtual void set_host(const std::string& host) = 0;
  virtual void connect(const boost::asio::ip::address& ip, uint16_t port) = 0;

  // For SyncReadStream requirements
  virtual std::size_t read_some(const boost::asio::mutable_buffer& buffer) = 0;
  virtual std::size_t read_some(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) = 0;

  // For SyncWriteStream requirements
  virtual std::size_t write_some(const boost::asio::const_buffer& buffer) = 0;
  virtual std::size_t write_some(const boost::asio::const_buffer& buffer, boost::system::error_code& ec) = 0;
};


class tcp_socket : public sync_stream_wrapper {
public:
  using socket_type = boost::asio::ip::tcp::socket;

private:
  boost::asio::io_context io_context_;
  socket_type socket_;

public:
  tcp_socket();

  void set_host(const std::string& host) override;
  void connect(const boost::asio::ip::address& ip, uint16_t port) override;

  std::size_t read_some(const boost::asio::mutable_buffer& buffer) override;
  std::size_t read_some(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) override;

  std::size_t write_some(const boost::asio::const_buffer& buffer) override;
  std::size_t write_some(const boost::asio::const_buffer& buffer, boost::system::error_code& ec) override;
};


class ssl_stream : public sync_stream_wrapper {
public:
  using socket_type = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

private:
  boost::asio::io_context io_context_;
  boost::asio::ssl::context ssl_context_;
  bool verify_cert_;
  std::string host_;
  std::unique_ptr<socket_type> stream_;

public:
  ssl_stream();
  ssl_stream(bool verify_cert, bool http2);

  void set_host(const std::string& host) override;
  void connect(const boost::asio::ip::address& ip, uint16_t port) override;

  std::size_t read_some(const boost::asio::mutable_buffer& buffer) override;
  std::size_t read_some(const boost::asio::mutable_buffer& buffer, boost::system::error_code& ec) override;

  std::size_t write_some(const boost::asio::const_buffer& buffer) override;
  std::size_t write_some(const boost::asio::const_buffer& buffer, boost::system::error_code& ec) override;

  socket_type& inner_stream() const;
};

#endif
