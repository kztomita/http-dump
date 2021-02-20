#include "httpv1.h"
#include "debug.h"
#include "socket.h"
#include "sync_stream_wrapper.h"

namespace asio = boost::asio;

http_response httpv1::get(const boost::asio::ip::address& ip, uint16_t port, http_scheme scheme, const std::string& host, const std::string& path, const http_header_list& headers) {
  std::unique_ptr<sync_stream_wrapper> socket = create_socket(scheme);

  socket->set_host(host);
  socket->connect(ip, port);

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
  asio::write(*socket, asio::buffer(request_stream.str()));

  return read_response(*socket);
}


