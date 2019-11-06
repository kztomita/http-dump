#include "hpack_encoder.h"
#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace hpack {

http2_frame::payload_type hpack_encoder::encode(const header_list& headers) {
  http2_frame::payload_type payload;

  for (const auto& h : headers) {
    if (h.first.size() >= 127) {
      throw std::runtime_error("header name is too long. Supports less than 127 characters.");
    }
    if (h.second.size() >= 127) {
      throw std::runtime_error("header value is too long. Supports less than 127 characters.");
    }
    std::size_t offset = payload.size();
    payload.resize(payload.size() + 3 + h.first.size() + h.second.size());
    unsigned char* p = &payload[offset];
    *p = 0x00;  // without Indexing
    p++;
    *p = h.first.size();
    p++;
    std::memcpy(p, &h.first[0], h.first.size());
    p += h.first.size();
    *p = h.second.size();
    p++;
    std::memcpy(p, &h.second[0], h.second.size());
  }

  return payload;
}

} // namespace hpack
