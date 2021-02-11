#ifndef HPACK_TYPES_H
#define HPACK_TYPES_H

#include <string>
#include <utility>

namespace hpack {
  using header_type = std::pair<std::string, std::string>;
}


#endif
