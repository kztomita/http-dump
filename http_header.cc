#include "http_header.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include "string_util_trim.h"

http_header::http_header(const std::string& key, const std::string& value)
  : key(key),
    value(value) {
}

http_header_ptr create_http_header_from_header_line_or_null(const std::string& line) {
  // 改行コード削除
  std::string stripped;
  std::copy_if(line.begin(), line.end(), std::back_inserter(stripped),
               [](char c){return c != '\r' && c != '\n' ;});

  auto pos = stripped.find(":");
  if (pos == std::string::npos) {
    return http_header_ptr();      // null
  }

  auto stripped_view = std::string_view(stripped);

  auto name = std::string_view(stripped_view.substr(0, pos));
  name = string_util::trim(name, " ");

  auto value = line.substr(pos + 1, stripped_view.size() - pos - 1);
  value = string_util::trim(value, " ");

  return std::make_unique<http_header>(std::string(name), std::string(value));
}

http_header_ptr create_http_header_from_header_line(const std::string& line) {
  auto h = create_http_header_from_header_line_or_null(line);
  if (!h) {
    std::ostringstream os;
    os << "invalid format (" << line << ")";
    throw std::invalid_argument(os.str());
  }
  return h;
}

std::ostream& operator<<(std::ostream& os, const http_header& h) {
  os << h.key << ": " << h.value;
  return os;
}
