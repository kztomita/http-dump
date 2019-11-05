#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <string>
#include "global.h"
#include "http_response.h"      // for dump()
#include "hpack_table.h"        // for dump()

inline void debug_message(const std::string& s) {
  if (g_verbose) {
    std::cout << s << std::endl;
  }
}

template<typename T>
void debug_dump(const T& object) {
  if (g_verbose) {
    dump(object);
  }
}

#endif
