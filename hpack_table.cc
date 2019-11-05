#include "hpack_table.h"
#include <iostream>
#include <vector>
#include <stdexcept>

namespace hpack {

namespace {
const std::vector<hpack::header_type> hpack_static_table{
  {"", ""},
  {":authority", ""},           // 1
  {":method", "GET"},
  {":method", "POST"},
  {":path", "/"},
  {":path", "/index.html"},
  {":scheme", "http"},
  {":scheme", "https"},
  {":status", "200"},
  {":status", "204"},
  {":status", "206"},           // 10
  {":status", "304"},
  {":status", "400"},
  {":status", "404"},
  {":status", "500"},
  {":accept-charset", ""},
  {":accept-encoding", "gzip, deflate"},
  {":accept-language", ""},
  {":accept-ranges", ""},
  {":accept", ""},
  {":access-control-allow-origin", ""}, // 20
  {":age", ""},
  {":allow", ""},
  {":authorization", ""},
  {":cache-control", ""},
  {":content-disposition", ""},
  {":content-encoding", ""},
  {":content-language", ""},
  {":content-length", ""},
  {":content-location", ""},
  {":content-range", ""},       // 30
  {":content-type", ""},
  {":cookie", ""},
  {":date", ""},
  {":etag", ""},
  {":expect", ""},
  {":expires", ""},
  {":from", ""},
  {":host", ""},
  {":if-match", ""},
  {":if-modified-since", ""},   // 40
  {":if-none-match", ""},
  {":if-range", ""},
  {":if-unmodified-since", ""},
  {":last-modified", ""},
  {":link", ""},
  {":location", ""},
  {":max-forwards", ""},
  {":proxy-authenticate", ""},
  {":proxy-authorization", ""},
  {":range", ""},               // 50
  {":referer", ""},
  {":refresh", ""},
  {":retry-after", ""},
  {":server", ""},
  {":set-cookie", ""},
  {":strict-transport-security", ""},
  {":transfer-encoding", ""},
  {":user-agent", ""},
  {":vary", ""},
  {":via", ""},                 // 60
  {":www-authenticate", ""},
};
}  // unnamed namespace

header_type
hpack_static_table_at(std::size_t index) {
  if (index == 0 || index >= hpack_static_table.size()) {
    throw std::invalid_argument("out of range.");
  }
  return header_type(hpack_static_table[index]);
}


std::size_t hpack_dynamic_table::size() const {
  return entries_.size();
}

std::size_t hpack_dynamic_table::entry_size(const header_type& entry) const {
  // Ref. rfc7541 4.1.  Calculating Table Size
  return entry.first.size() + entry.second.size() + 32;
}

std::size_t hpack_dynamic_table::calc_table_size() const {
  std::size_t size = 0;
  for (const auto& e : entries_) {
    size += entry_size(e);
  }
  return size;
}

std::size_t hpack_dynamic_table::table_size() const {
  return current_table_size_;
}

std::size_t hpack_dynamic_table::maximum_table_size() const {
  return max_size_;
}

void hpack_dynamic_table::set_maximum_table_size(std::size_t size) {
  max_size_ = size;
  evict();
}

header_type hpack_dynamic_table::at(std::size_t index) const {
  if (index >= size()) {
    throw std::invalid_argument("out of range.");
  }

  // 新しいエントリほど若いindexとする必要がある。
  // 性能的に新しいエントリは末尾に追加したいので、
  // indexは末尾を0とする形に変換する。
  return header_type(entries_[size() - 1 - index]);
}

void hpack_dynamic_table::add(const header_type& header) {
  entries_.push_back(header);
  current_table_size_ += entry_size(header);
  evict();
}

void hpack_dynamic_table::remove_tail() {
  if (entries_.size() > 0) {
    // XXX vectorの先頭エントリ削除なので直したい
    current_table_size_ -= entry_size(*entries_.begin());
    entries_.erase(entries_.begin());
  }
}

void hpack_dynamic_table::evict() {
  while (size() > 0 && table_size() > maximum_table_size()) {
    remove_tail();
  }
}

void hpack_dynamic_table::dump() const {
  std::cout << "Table Size: " << table_size() << " (Maximum:" << max_size_ << ")" << std::endl;

  auto calculated_size = calc_table_size();
  if (table_size() != calculated_size) {
    std::cout << "ERROR: Calculated Table Size mismatched (" << calculated_size << ")" << std::endl;
  }

  int i = 1;
  for (auto e = entries_.rbegin(); e != entries_.rend(); ++e) {
    std::cout << "[" << i << "] " << e->first << " " << e->second << std::endl;
    i++;
  }
}

void dump(const hpack_dynamic_table& table) {
  table.dump();
}

std::optional<hpack::header_type>
hpack_table_at(const hpack_dynamic_table& dynamic_table, std::size_t index) {
  if (index == 0) {
    return std::nullopt;
  }
  if (index >= 1 && index <= hpack_static_table.size()) {
    return std::make_optional(hpack_static_table_at(index));
  }
  if (index >= hpack_static_table.size() + 1 && index <= hpack_static_table.size() + dynamic_table.size()) {
    return std::make_optional(dynamic_table.at(index - hpack_static_table.size() - 1));
  }
  return std::nullopt;
}

}  // namespace hpack
