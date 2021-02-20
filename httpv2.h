#ifndef HTTPV2_H
#define HTTPV2_H

#include <cstdint>
#include <string>
#include <boost/asio.hpp>
#include "http_header_list.h"
#include "http_protocol.h"
#include "http_response.h"

class httpv2 : public http_protocol {
public:
  http_response get(const boost::asio::ip::address& ip, uint16_t port, http_scheme scheme, const std::string& host, const std::string& path, const http_header_list& headers) override;
};

#endif
