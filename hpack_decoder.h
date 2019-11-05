#ifndef HPACK_DECODER_H
#define HPACK_DECODER_H

#include <cstddef>
#include <vector>
#include <memory>
#include <utility>
#include <string>
#include <optional>
#include "buffer_view.h"
#include "hpack_types.h"

namespace hpack {

class hpack_dynamic_table;

struct decoding_context {
  std::unique_ptr<hpack_dynamic_table> dynamic_table;

  decoding_context();
  ~decoding_context();
};

class hpack_decoder {
private:
  buffer_view<unsigned char> buffer_view_;
  std::size_t decode_pos_;

public:
  using header_list_type = std::vector<header_type>;

public:
  hpack_decoder(const unsigned char* buffer, std::size_t length);

  header_list_type decode(decoding_context& context);

private:

  struct decoded_result {
    std::optional<header_type> header;
    bool end;

    decoded_result()
      : end(false) {}

    // T: std::optional<header_type>, header_type, std::nullopt
    template<typename T>
    decoded_result(T&& h, bool e)
      : header(std::forward<T>(h)),
        end(e) {}
  };

  int fetch_integer(std::size_t n);
  std::string fetch_string();
  header_type fetch_header(const decoding_context& context, std::size_t index);
  decoded_result decode_representation(decoding_context& context);
};

}

#endif
