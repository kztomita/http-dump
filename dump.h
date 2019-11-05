#ifndef DUMP_H
#define DUMP_H

#include <cstddef>

void dump(const void* p, std::size_t size, std::size_t max = 0xffffffff);

#endif
