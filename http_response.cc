#include "http_response.h"
#include <iostream>

std::string http_response::stringify_payload() const {
  return std::string(reinterpret_cast<const char*>(&payload_[0]), payload_.size());
}

void dump(const http_response& r) {
  std::cout << "HTTP Response" << std::endl;
  std::cout << "Status " << r.code_ << std::endl;
  std::cout << "Headers" << std::endl;
  for (const auto& h : r.headers_) {
    std::cout << h << std::endl;
  }
}

