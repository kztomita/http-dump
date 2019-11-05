#ifndef URL_H
#define URL_H

#include <string>
#include <optional>

struct url_components {
  std::string scheme;
  std::string host;
  std::optional<unsigned int> port;
  std::string path_and_query;
};

url_components parse_url(const std::string& url);

#endif
