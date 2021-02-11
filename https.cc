#include "https.h"
#include <boost/asio/ssl.hpp>
#include "debug.h"
#include "socket.h"

namespace asio = boost::asio;

// Ref.
// https://www.boost.org/doc/libs/1_66_0/doc/html/boost_asio/overview/ssl.html

http_response https::get(const boost::asio::ip::address& ip, uint16_t port, const std::string& host, const std::string& path, const http_header_list& headers) {
  asio::io_service io_service;

  asio::ssl::context ctx(asio::ssl::context::tlsv12_client);
  asio::ssl::stream<asio::ip::tcp::socket> stream(io_service, ctx);

  // TODO 証明書

  stream.lowest_layer().connect(asio::ip::tcp::endpoint(ip, port));

  stream.handshake(boost::asio::ssl::stream_base::client);

  std::ostringstream request_stream;
  request_stream << "GET " << path << " HTTP/1.1\r\n"
                 << "Host: " << host << "\r\n";
                 //<< "Connection: Close\r\n"
  for (const auto& h : headers) {
    request_stream << h->key << ": " << h->value << "\r\n";
  }
  request_stream << "\r\n";

  debug_message("Sending Request");
  debug_message(request_stream.str());
  asio::write(stream, asio::buffer(request_stream.str()));

  return read_response(stream);
}
