#ifndef HTTP_HEADER_LIST_H
#define HTTP_HEADER_LIST_H

#include <list>
#include <memory>
#include "http_header.h"

using http_header_list = std::list<std::unique_ptr<http_header>>;

#endif
