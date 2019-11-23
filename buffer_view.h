#ifndef BUFFER_VIEW_H
#define BUFFER_VIEW_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>

template<typename T>
class buffer_view {
  static_assert(sizeof(T) == 1, "T is must be char or unsigned char.");
private:
  const T* buffer_;
  std::size_t length_;

public:
  using value_type = T;

  buffer_view() : buffer_view(nullptr, 0) {}

  buffer_view(const T* buffer, std::size_t length)
    : buffer_(buffer),
      length_(length) {}

  buffer_view(const buffer_view& other) = default;
  buffer_view& operator=(const buffer_view& other) = default;
  buffer_view(buffer_view&& other) = default;
  buffer_view& operator=(buffer_view&& other) = default;

  const T* address(std::size_t offset) const {
    if (offset >= length_) {
      throw std::range_error("invalid offset");
    }
    return buffer_ + offset;
  }

  std::size_t size() {
    return length_;
  }

  T at(std::size_t offset) const {
    if (offset >= length_) {
      throw std::range_error("invalid offset");
    }
    return buffer_[offset];
  }

  uint16_t at_uint16(std::size_t offset) const {
    std::size_t end = offset + 1;
    if (end < offset || end >= length_) {
      throw std::range_error("invalid offset");
    }
    uint16_t value = 0;
    memcpy(&value, buffer_ + offset, 2);
    return ntohs(value);
  }

  uint32_t at_uint32(std::size_t offset) const {
    std::size_t end = offset + 3;
    if (end < offset || end >= length_) {
      throw std::range_error("invalid offset");
    }
    uint32_t value = 0;
    memcpy(&value, buffer_ + offset, 4);
    return ntohl(value);
  }

  buffer_view subview(std::size_t offset, std::size_t length) const {
    std::size_t end = offset + length;
    if (end < offset || end > length_) {
      throw std::range_error("invalid offset or length");
    }
    return buffer_view(buffer_ + offset, length);
  }

  bool test_range(std::size_t offset, std::size_t length) {
    std::size_t end = offset + 1;
    if (end < offset || end > length_) {
      return false;
    }
    return true;
  }
};


#endif
