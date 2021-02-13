#ifndef HPACK_ENCODER_H
#define HPACK_ENCODER_H

#include <string>
#include <vector>
#include "http2_frame.h"
#include "hpack_types.h"

namespace hpack {

class hpack_encoder {
public:
  using header_list = std::vector<header_type>;

  http2_frame::payload_type encode(const header_list& headers);
};

} // namespace hpack

#endif
