#include <iostream>
#include <iomanip>
#include <sstream>

namespace {

void out_line(const char* start, const char* end, bool snipped) {
  std::ostringstream octets;
  std::string chars;

  octets << std::hex;
  for (auto p = start ; p < end ; p++) {
    octets << std::setw(2) << std::setfill('0') << (static_cast<int>(*p) & 0xff) << " ";

    if (*p >= 0x20 && *p <= 0x7e) {
      chars += *p;
    } else {
      chars += ".";
    }
  }
  if (snipped) {
    octets << "snipped...";
  }

  std::ostringstream line;
  line << std::left << std::setw(16*3 + 2) << octets.str() << chars;

  std::cout << line.str() <<  std::endl;
}

}

void dump(const void* p, std::size_t size, std::size_t max = 0) {
  const char* p2 = reinterpret_cast<const char*>(p);

  std::size_t last_offset = size;
  bool snip = false;
  if (max && size > max) {
    last_offset = max;
    snip = true;
  }

  for (std::size_t offset = 0 ; offset < last_offset ; offset += 16) {
    if (offset + 16 <= last_offset) {
      out_line(p2 + offset, p2 + offset + 16, false);
    } else {
      out_line(p2 + offset, p2 + last_offset, snip);
    }
  }
}
