#ifndef HTTP_PROTOCOL_H
#define HTTP_PROTOCOL_H

#include <cstdint>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "global.h"
#include "http_header_list.h"
#include "http_response.h"
#include "sync_stream_wrapper.h"

enum class http_scheme {
  http,
  https,
};

class http_protocol {
public:
  virtual http_response get(const boost::asio::ip::address& ip, uint16_t port, http_scheme scheme, const std::string& host, const std::string& path, const http_header_list& headers) = 0;
};

inline std::unique_ptr<sync_stream_wrapper> create_socket(http_scheme scheme, bool use_http2 = false) {
  if (scheme == http_scheme::https) {
    return std::make_unique<ssl_stream>(g_verify_cert, use_http2);
  } else {
    return std::make_unique<tcp_socket>();
  }
}

#endif
