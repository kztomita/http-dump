#ifndef STRING_UTIL_SPLIT_H
#define STRING_UTIL_SPLIT_H

#include <string>
#include <vector>
#include <stdexcept>

namespace string_util {

template<typename CharT, typename Traits, typename Alloc>
std::vector<std::basic_string<CharT, Traits, Alloc> >
split(const std::basic_string<CharT, Traits, Alloc>& s,
      const CharT* delimiter) {
  using _String = std::basic_string<CharT, Traits, Alloc>;

  auto delimiter_size = _String(delimiter).size();
  if (delimiter_size == 0) {
    throw std::invalid_argument("empty delimiter");
  }

  std::vector<_String> results;

  typename _String::size_type pos = 0;

  while (true) {
    typename _String::size_type next = s.find(delimiter, pos);
    if (next == _String::npos) {
      results.push_back(s.substr(pos));
      break;
    }
    results.push_back(s.substr(pos, next - pos));

    pos = next + delimiter_size;
  }

  return results;
}

inline std::vector<std::string>
split(const char* s, const char* delimiter) {
  return split(std::string(s), delimiter);
}

}

#endif
