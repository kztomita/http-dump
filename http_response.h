#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include "http_header.h"

class http_response {
public:
  using payload_type = std::vector<uint8_t>;

  uint  code_;
  std::vector<std::unique_ptr<http_header>> headers_;
  payload_type payload_;

  std::string stringify_payload() const;
};

void dump(const http_response& r);

#endif
