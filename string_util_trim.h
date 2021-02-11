#ifndef STRING_UTIL_TRIM_H
#define STRING_UTIL_TRIM_H

#include <string>

namespace string_util {

template<typename CharT, typename Traits, typename Alloc>
std::basic_string<CharT, Traits, Alloc>
ltrim(const std::basic_string<CharT, Traits, Alloc>& s, const CharT* chars) {
  using _String = std::basic_string<CharT, Traits, Alloc>;

  auto pos = s.find_first_not_of(chars);
  if (pos == _String::npos) {
    return s;
  }

  return s.substr(pos);
}

template<typename CharT, typename Traits, typename Alloc>
std::basic_string<CharT, Traits, Alloc>
rtrim(const std::basic_string<CharT, Traits, Alloc>& s, const CharT* chars) {
  using _String = std::basic_string<CharT, Traits, Alloc>;

  auto pos = s.find_last_not_of(chars);
  if (pos == _String::npos) {
    return s;
  }

  return s.substr(0, pos + 1);
}

template<typename CharT, typename Traits, typename Alloc>
std::basic_string<CharT, Traits, Alloc>
trim(const std::basic_string<CharT, Traits, Alloc>& s, const CharT* chars) {
  return rtrim(ltrim(s, chars), chars);
}

// for string_view

template<typename CharT, typename Traits>
std::basic_string_view<CharT, Traits>
ltrim(const std::basic_string_view<CharT, Traits>& s, const CharT* chars) {
  using _SView = std::basic_string_view<CharT, Traits>;

  auto pos = s.find_first_not_of(chars);
  if (pos == _SView::npos) {
    return s;
  }

  return s.substr(pos);
}

template<typename CharT, typename Traits>
std::basic_string_view<CharT, Traits>
rtrim(const std::basic_string_view<CharT, Traits>& s, const CharT* chars) {
  using _SView = std::basic_string_view<CharT, Traits>;

  auto pos = s.find_last_not_of(chars);
  if (pos == _SView::npos) {
    return s;
  }

  return s.substr(0, pos + 1);
}

template<typename CharT, typename Traits>
std::basic_string_view<CharT, Traits>
trim(const std::basic_string_view<CharT, Traits>& s, const CharT* chars) {
  return rtrim(ltrim(s, chars), chars);
}

}

#endif
