#ifndef HPACK_HUFFMAN_H
#define HPACK_HUFFMAN_H

#include <cstddef>
#include <vector>

namespace hpack {

std::vector<unsigned char> decode_huffman(const unsigned char* start, std::size_t length);

}  // namespace hpack

#endif
