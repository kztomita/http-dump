#ifndef HTTP_H
#define HTTP_H

#include <cstdint>
#include <string>
#include <boost/asio.hpp>
#include "http_response.h"

class http {
public:
  http_response get(const boost::asio::ip::address& ip, uint16_t port, const std::string& host, const std::string& path);
};

#endif
