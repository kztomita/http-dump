#include "url.h"
#include <regex>
#include <stdexcept>

url_components parse_url(const std::string& url) {
  std::smatch m;
  if (!std::regex_search(url, m, std::regex(R"((^[a-z][a-z0-9+.-]*)://([^ /]+?)(?::(\d+))?(/[\S]*)?$)"))) {
    throw std::invalid_argument("invalid url.");
  }

  url_components c;
  c.scheme = m[1].str();
  c.host = m[2].str();

  if (m[3].matched) {
    c.port = static_cast<unsigned int>(std::stoi(m[3].str()));
  }

  if (m[4].matched) {
    c.path_and_query = m[4].str();
  } else {
    c.path_and_query = "/";
  }

  return c;
}
