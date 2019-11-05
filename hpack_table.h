#ifndef HPACK_TABLE_H
#define HPACK_TABLE_H

#include <cstddef>
#include <optional>
#include <vector>
#include "hpack_types.h"

namespace hpack {

header_type hpack_static_table_at(std::size_t index);

class hpack_dynamic_table {
private:
  std::vector<header_type> entries_;
  std::size_t max_size_ = 4096;
  std::size_t current_table_size_ = 0;
public:
  std::size_t size() const;
  std::size_t table_size() const;
  std::size_t maximum_table_size() const;
  void set_maximum_table_size(std::size_t size);
  header_type at(std::size_t index) const;
  void add(const header_type& header);
  void dump() const;

private:
  std::size_t entry_size(const header_type& entry) const;
  std::size_t calc_table_size() const;
  void remove_tail();
  void evict();
};

void dump(const hpack_dynamic_table& table);

std::optional<header_type>
hpack_table_at(const hpack_dynamic_table& dynamic_table, std::size_t index);

}

#endif
