#ifndef HPACK_ENCODER_H
#define HPACK_ENCODER_H

#include <string>
#include <vector>
#include "hpack_types.h"

namespace hpack {

template<typename Container>
void encode_integer(uint8_t src, unsigned int i, std::size_t n, Container& buffer) {
  // 2 ^ n - 1
  unsigned int mask = (1 << n) - 1;

  if (i < mask) {
    buffer.push_back((src & ~mask) + (i & mask));
  } else {
    buffer.push_back(src | mask);
    i -= mask;
    while (i >= 128) {
      buffer.push_back(i % 128 + 128);
      i /= 128;
    }
    buffer.push_back(i);
  }
}

class hpack_encoder {
public:
  using header_list = std::vector<header_type>;

  // Container: std::vector<uint8_t>, and so on.
  template<typename Container>
  Container encode(const header_list& headers) {
    Container buffer;

    for (const auto& h : headers) {
      buffer.push_back(0x00);    // Literal Header Field without Indexing

      // Name Length
      encode_integer(0x00, h.first.size(), 7, buffer);

      // Name String
      std::copy(h.first.begin(), h.first.end(), std::back_inserter(buffer));

      // Value Length
      encode_integer(0x00, h.second.size(), 7, buffer);

      // Value String
      std::copy(h.second.begin(), h.second.end(), std::back_inserter(buffer));
    }

    return buffer;
  }

};

} // namespace hpack

#endif
