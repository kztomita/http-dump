#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

#include <iostream>
#include <string>
#include <memory>

struct http_header {
  std::string key;
  std::string value;

  http_header() = default;
  http_header(const std::string& key, const std::string& value);
};

std::ostream& operator<<(std::ostream& os, const http_header& h);

std::unique_ptr<http_header> create_http_header_from_header_line_or_null(const std::string& line);
std::unique_ptr<http_header> create_http_header_from_header_line(const std::string& line);

#endif
