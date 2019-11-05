#include "http.h"
#include "debug.h"
#include "socket.h"

namespace asio = boost::asio;

http_response http::get(const boost::asio::ip::address& ip, uint16_t port, const std::string& host, const std::string& path) {

  asio::io_service io_service;
  asio::ip::tcp::socket socket(io_service);

  socket.connect(asio::ip::tcp::endpoint(ip, port));

  std::ostringstream request_stream;
  request_stream << "GET " << path << " HTTP/1.1\r\n"
                 << "Host: " << host << "\r\n"
                 //<< "Connection: Close\r\n"
                 << "\r\n";
  debug_message("Sending Request");
  debug_message(request_stream.str());
  asio::write(socket, asio::buffer(request_stream.str()));

  return read_response(socket);
}


