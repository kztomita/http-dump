#include "hpack_encoder.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace hpack {

namespace {
void encode_integer(uint8_t src, unsigned int i, std::size_t n, http2_frame::payload_type& payload) {
  // 2 ^ n - 1
  unsigned int mask = (1 << n) - 1;

  if (i < mask) {
    payload.push_back((src & ~mask) + (i & mask));
  } else {
    payload.push_back(src | mask);
    i -= mask;
    while (i >= 128) {
      payload.push_back(i % 128 + 128);
      i /= 128;
    }
    payload.push_back(i);
  }
}

}  // unnamed namespace

http2_frame::payload_type hpack_encoder::encode(const header_list& headers) {
  http2_frame::payload_type payload;

  for (const auto& h : headers) {
    payload.push_back(0x00);    // Literal Header Field without Indexing

    // Name Length
    encode_integer(0x00, h.first.size(), 7, payload);

    // Name String
    std::copy(h.first.begin(), h.first.end(), std::back_inserter(payload));

    // Value Length
    encode_integer(0x00, h.second.size(), 7, payload);

    // Value String
    std::copy(h.second.begin(), h.second.end(), std::back_inserter(payload));
  }

  return payload;
}

} // namespace hpack
