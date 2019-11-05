#ifndef H2_H
#define H2_H

#include <cstdint>
#include <string>
#include <boost/asio.hpp>
#include "http_response.h"

class h2 {
public:
  http_response get(const boost::asio::ip::address& ip, uint16_t port, const std::string& host, const std::string& path);
};

#endif
