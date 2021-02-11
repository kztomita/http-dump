#ifndef HTTPS_H
#define HTTPS_H

#include <cstdint>
#include <string>
#include <boost/asio.hpp>
#include "http_header_list.h"
#include "http_response.h"

class https {
public:
  http_response get(const boost::asio::ip::address& ip, uint16_t port, const std::string& host, const std::string& path, const http_header_list& headers);
};

#endif
