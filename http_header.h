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

using http_header_ptr = std::unique_ptr<http_header>;

std::ostream& operator<<(std::ostream& os, const http_header& h);

http_header_ptr create_http_header_from_header_line_or_null(const std::string& line);
http_header_ptr create_http_header_from_header_line(const std::string& line);

#endif
