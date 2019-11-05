#include "http_header.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <iterator>

http_header::http_header(const std::string& key, const std::string& value)
  : key(key),
    value(value) {
}

http_header create_http_header_from_header_line(const std::string& line) {
  // 改行コード削除
  std::string stripped;
  std::copy_if(line.begin(), line.end(), std::back_inserter(stripped),
               [](char c){return c != '\r' && c != '\n' ;});

  auto pos = stripped.find(": ");
  if (pos == std::string::npos) {
    std::ostringstream os;
    os << "invalid format (" << stripped << ")";
    throw std::invalid_argument(os.str());
  }
  return http_header(stripped.substr(0, pos), stripped.substr(pos + 2));
}

std::ostream& operator<<(std::ostream& os, const http_header& h) {
  os << h.key << ": " << h.value;
  return os;
}
