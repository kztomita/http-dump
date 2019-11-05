#ifndef STRING_UTIL_TOLOWER_H
#define STRING_UTIL_TOLOWER_H

#include <algorithm>
#include <iterator>
#include <locale>
#include <string>

namespace string_util {

inline std::string tolower(const std::string& str) {
  std::string s;

  std::transform(str.begin(), str.end(), std::back_inserter(s),
                 [](unsigned char c){ return std::tolower(c); });

  return s;
}

inline std::string toupper(const std::string& str) {
  std::string s;

  std::transform(str.begin(), str.end(), std::back_inserter(s),
                 [](unsigned char c){ return std::toupper(c); });

  return s;
}

}


#endif
