#ifndef STRING_UTIL_JOIN_H
#define STRING_UTIL_JOIN_H

#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <initializer_list>

namespace string_util {

template<class Container>
std::string join(const char* delimiter, const Container& c) {
  std::ostringstream os;
  if (!c.empty()) {
    auto last = std::prev(c.cend());
    std::copy(c.cbegin(), last, std::ostream_iterator<std::string>(os, ","));
    os << *last;
  }
  return os.str();
}

// 初期化子リストも受け付けられるように
inline std::string
join(const char* delimiter, std::initializer_list<std::string> strings) {
  return join(delimiter, std::vector<std::string>(strings.begin(), strings.end()
));
}

}
#endif
