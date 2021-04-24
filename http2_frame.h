#ifndef HTTP2_FRAME_H
#define HTTP2_FRAME_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <stdexcept>
#include <arpa/inet.h>
#include "dump.h"

struct http2_frame_header {
  uint8_t plength[3];
  uint8_t type;
  uint8_t flags;
  uint8_t psid[4];

  enum type {TYPE_DATA = 0,
             TYPE_HEADERS = 1,
             TYPE_PRIORITY = 2,
             TYPE_RST_STREAM = 3,
             TYPE_SETTINGS = 4,
             TYPE_PUSH_PROMISE = 5,
             TYPE_PING = 6,
             TYPE_GOAWAY = 7,
             TYPE_WINDOW_UPDATE = 8,
             TYPE_CONTINUATION = 9,
  };

  uint32_t length() const {
    uint32_t value = 0;
    std::memcpy((reinterpret_cast<uint8_t*>(&value)) + 1, plength, 3);
    return ntohl(value);
  }
  void set_length(uint32_t length) {
    if (length & 0xff000000) {
      throw std::invalid_argument("length too large");
    }
    uint32_t value = htonl(length);
    std::memcpy(plength, (reinterpret_cast<uint8_t*>(&value)) + 1, 3);
  }

  uint32_t stream_id() const {
    uint32_t value = 0;
    std::memcpy(reinterpret_cast<uint8_t*>(&value), psid, 4);
    return ntohl(value);
  }
  void set_stream_id(uint32_t sid) {
    if (sid & 0x80000000) {
      throw std::invalid_argument("sid too large");
    }
    uint32_t value = htonl(sid);
    std::memcpy(psid, &value, 4);
  }
} __attribute__ ((packed));

class http2_frame {
public:
  using payload_value_type = uint8_t;
  using payload_type = std::vector<payload_value_type>;

private:
  http2_frame_header header_;
  payload_type payload_;

public:
  http2_frame_header& header() {
    return header_;
  }

  const http2_frame_header& header() const {
    return header_;
  }

  payload_type& payload() {
    return payload_;
  }
  const payload_type& payload() const {
    return payload_;
  }

  payload_value_type* payload_buffer() {
    return &payload_[0];
  }
  const payload_value_type* payload_buffer() const {
    return &payload_[0];
  }

  uint32_t length() const {
    return header_.length();
  }
  int type() const {
    return header_.type;
  }
  int flags() const {
    return header_.flags;
  }
  uint32_t stream_id() const {
    return header_.stream_id();
  }
};

enum http2_settings_parameter {
  SETTINGS_HEADER_TABLE_SIZE      = 0x1,
  SETTINGS_ENABLE_PUSH            = 0x2,
  SETTINGS_MAX_CONCURRENT_STREAMS = 0x3,
  SETTINGS_INITIAL_WINDOW_SIZE    = 0x4,
  SETTINGS_MAX_FRAME_SIZE         = 0x5,
  SETTINGS_MAX_HEADER_LIST_SIZE   = 0x6,
};

const int HTTP2_DATA_END_STREAM_FLAG = 0x01;
const int HTTP2_DATA_PADDED_FLAG     = 0x08;

const int HTTP2_HEADERS_END_STREAM_FLAG  = 0x01;
const int HTTP2_HEADERS_END_HEADERS_FLAG = 0x04;
const int HTTP2_HEADERS_PADDED_FLAG      = 0x08;
const int HTTP2_HEADERS_PRIORITY_FLAG    = 0x20;

const int HTTP2_SETTINGS_ACK_FLAG = 0x01;

const int HTTP2_CONTINUATION_END_HEADERS_FLAG = 0x04;

enum http2_error_code {NO_ERROR             = 0x0,
                       PROTOCOL_ERROR       = 0x1,
                       INTERNAL_ERROR       = 0x2,
                       FLOW_CONTROL_ERROR   = 0x3,
                       SETTINGS_TIMEOUT     = 0x4,
                       STREAM_CLOSED        = 0x5,
                       FRAME_SIZE_ERROR     = 0x6,
                       REFUSED_STREAM       = 0x7,
                       CANCEL               = 0x8,
                       COMPRESSION_ERROR    = 0x9,
                       CONNECT_ERROR        = 0xa,
                       ENHANCE_YOUR_CALM    = 0xb,
                       INADEQUATE_SECURITY  = 0xc,
                       HTTP_1_1_REQUIRED    = 0xd,
};

void dump(const http2_frame& frame);

#endif
