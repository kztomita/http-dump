#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <getopt.h>
#include "debug.h"
#include "socket.h"
#include "global.h"
#include "http_response.h"
#include "http.h"
#include "https.h"
#include "h2.h"
#include "url.h"

void usage(const char* message = nullptr) {
  if (message) {
    std::cerr << message << std::endl << std::endl;
  }
  std::cerr << "Usage:" << std::endl;
  std::cerr << "http-client <options> <url>" << std::endl << std::endl;
  std::cerr << "-v:       verbose mode" << std::endl;
  std::cerr << "--http2:  use http/2 (https only)" << std::endl;

  exit(1);
}


int main(int argc, char *argv[]) {
  namespace asio = boost::asio;

  bool use_http2 = false;

  if (argc < 2) {
    usage();
  }

  static struct option long_options[] = {
    {"http2",     no_argument, 0,  1 },
    {0,                     0, 0,  0 },
  };
  while (true) {
    int opt = getopt_long(argc - 1, argv, "v",
                      long_options, NULL);
    if (opt == -1) {
      break;
    }
    switch (opt) {
    case 1:     // --http2
      use_http2 = true;
      break;
    case 'v':
      g_verbose = true;
      break;
    case '?':
      usage();
    }
  }

  std::string url(argv[argc - 1]);

  try {
    url_components c = parse_url(url);

    uint16_t port = 0;
    if (c.scheme == "http") {
      port = 80;
    } else if (c.scheme == "https") {
      port = 443;
    } else {
      std::cerr << "Unsupported scheme." << std::endl;
      return 1;
    }
    if (c.port) {
      port = c.port.value();
    }

    boost::system::error_code error;
    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto endpoints = resolver.resolve(asio::ip::tcp::v4(), c.host, "", error);
    if (error) {
      std::cerr << error.message() << std::endl;
      return 1;
    }
    boost::asio::ip::address remote_address(endpoints.begin()->endpoint().address());

    if (c.scheme == "http") {
      http protocol;
      http_response r = protocol.get(remote_address, port, c.host, c.path_and_query);
      debug_dump(r);
      std::cout << r.stringify_payload() << std::endl;
    } else if (c.scheme == "https") {
      if (use_http2) {
        h2 protocol;
        http_response r = protocol.get(remote_address, port, c.host, c.path_and_query);
        debug_dump(r);
        std::cout << r.stringify_payload() << std::endl;
      } else {
        https protocol;
        http_response r = protocol.get(remote_address, port, c.host, c.path_and_query);
        debug_dump(r);
        std::cout << r.stringify_payload() << std::endl;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}

