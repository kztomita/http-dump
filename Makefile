ALL = http-dump

CXX = g++
CXXFLAGS = -Wall -O2 -MMD -std=c++17 `pkg-config --cflags openssl`
LDFLAGS = -lboost_system -lpthread `pkg-config --libs openssl`
SOURCES = $(wildcard ./*.cc)
DEPENDS = $(SOURCES:.cc=.d)

all: $(ALL)

http-dump: main.o url.o http_header.o http_response.o httpv1.o httpv2.o http2_frame.o hpack_decoder.o hpack_table.o hpack_huffman.o sync_stream_wrapper.o dump.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	-rm -f *.o
	-rm -f *.d
	-rm -f $(ALL)

-include $(DEPENDS)
