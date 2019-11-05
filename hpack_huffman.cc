#include "hpack_huffman.h"
#include <iostream>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace hpack {

struct huffman_code {
  uint32_t  len;
  uint32_t  bits;
};

namespace {

// https://tools.ietf.org/html/rfc7541#appendix-B

huffman_code huffman_codes[] = {
{13, 0b1111111111000},                  // 0
{23, 0b11111111111111111011000},        // 1
{28, 0b1111111111111111111111100010},   // 2
{28, 0b1111111111111111111111100011},   // 3
{28, 0b1111111111111111111111100100},   // 4
{28, 0b1111111111111111111111100101},   // 5
{28, 0b1111111111111111111111100110},   // 6
{28, 0b1111111111111111111111100111},   // 7
{28, 0b1111111111111111111111101000},   // 8
{24, 0b111111111111111111101010},       // 9
{30, 0b111111111111111111111111111100}, // 10
{28, 0b1111111111111111111111101001},   // 11
{28, 0b1111111111111111111111101010},   // 12
{30, 0b111111111111111111111111111101}, // 13
{28, 0b1111111111111111111111101011},   // 14
{28, 0b1111111111111111111111101100},   // 15
{28, 0b1111111111111111111111101101},   // 16
{28, 0b1111111111111111111111101110},   // 17
{28, 0b1111111111111111111111101111},   // 18
{28, 0b1111111111111111111111110000},   // 19
{28, 0b1111111111111111111111110001},   // 20
{28, 0b1111111111111111111111110010},   // 21
{30, 0b111111111111111111111111111110}, // 22
{28, 0b1111111111111111111111110011},   // 23
{28, 0b1111111111111111111111110100},   // 24
{28, 0b1111111111111111111111110101},   // 25
{28, 0b1111111111111111111111110110},   // 26
{28, 0b1111111111111111111111110111},   // 27
{28, 0b1111111111111111111111111000},   // 28
{28, 0b1111111111111111111111111001},   // 29
{28, 0b1111111111111111111111111010},   // 30
{28, 0b1111111111111111111111111011},   // 31
{6, 0b010100},                          // 32
{10, 0b1111111000},                     // 33
{10, 0b1111111001},                     // 34
{12, 0b111111111010},                   // 35
{13, 0b1111111111001},                  // 36
{6, 0b010101},                          // 37
{8, 0b11111000},                        // 38
{11, 0b11111111010},                    // 39
{10, 0b1111111010},                     // 40
{10, 0b1111111011},                     // 41
{8, 0b11111001},                        // 42
{11, 0b11111111011},                    // 43
{8, 0b11111010},                        // 44
{6, 0b010110},                          // 45
{6, 0b010111},                          // 46
{6, 0b011000},                          // 47
{5, 0b00000},                           // 48
{5, 0b00001},                           // 49
{5, 0b00010},                           // 50
{6, 0b011001},                          // 51
{6, 0b011010},                          // 52
{6, 0b011011},                          // 53
{6, 0b011100},                          // 54
{6, 0b011101},                          // 55
{6, 0b011110},                          // 56
{6, 0b011111},                          // 57
{7, 0b1011100},                         // 58
{8, 0b11111011},                        // 59
{15, 0b111111111111100},                // 60
{6, 0b100000},                          // 61
{12, 0b111111111011},                   // 62
{10, 0b1111111100},                     // 63
{13, 0b1111111111010},                  // 64
{6, 0b100001},                          // 65
{7, 0b1011101},                         // 66
{7, 0b1011110},                         // 67
{7, 0b1011111},                         // 68
{7, 0b1100000},                         // 69
{7, 0b1100001},                         // 70
{7, 0b1100010},                         // 71
{7, 0b1100011},                         // 72
{7, 0b1100100},                         // 73
{7, 0b1100101},                         // 74
{7, 0b1100110},                         // 75
{7, 0b1100111},                         // 76
{7, 0b1101000},                         // 77
{7, 0b1101001},                         // 78
{7, 0b1101010},                         // 79
{7, 0b1101011},                         // 80
{7, 0b1101100},                         // 81
{7, 0b1101101},                         // 82
{7, 0b1101110},                         // 83
{7, 0b1101111},                         // 84
{7, 0b1110000},                         // 85
{7, 0b1110001},                         // 86
{7, 0b1110010},                         // 87
{8, 0b11111100},                        // 88
{7, 0b1110011},                         // 89
{8, 0b11111101},                        // 90
{13, 0b1111111111011},                  // 91
{19, 0b1111111111111110000},            // 92
{13, 0b1111111111100},                  // 93
{14, 0b11111111111100},                 // 94
{6, 0b100010},                          // 95
{15, 0b111111111111101},                // 96
{5, 0b00011},                           // 97
{6, 0b100011},                          // 98
{5, 0b00100},                           // 99
{6, 0b100100},                          // 100
{5, 0b00101},                           // 101
{6, 0b100101},                          // 102
{6, 0b100110},                          // 103
{6, 0b100111},                          // 104
{5, 0b00110},                           // 105
{7, 0b1110100},                         // 106
{7, 0b1110101},                         // 107
{6, 0b101000},                          // 108
{6, 0b101001},                          // 109
{6, 0b101010},                          // 110
{5, 0b00111},                           // 111
{6, 0b101011},                          // 112
{7, 0b1110110},                         // 113
{6, 0b101100},                          // 114
{5, 0b01000},                           // 115
{5, 0b01001},                           // 116
{6, 0b101101},                          // 117
{7, 0b1110111},                         // 118
{7, 0b1111000},                         // 119
{7, 0b1111001},                         // 120
{7, 0b1111010},                         // 121
{7, 0b1111011},                         // 122
{15, 0b111111111111110},                // 123
{11, 0b11111111100},                    // 124
{14, 0b11111111111101},                 // 125
{13, 0b1111111111101},                  // 126
{28, 0b1111111111111111111111111100},   // 127
{20, 0b11111111111111100110},           // 128
{22, 0b1111111111111111010010},         // 129
{20, 0b11111111111111100111},           // 130
{20, 0b11111111111111101000},           // 131
{22, 0b1111111111111111010011},         // 132
{22, 0b1111111111111111010100},         // 133
{22, 0b1111111111111111010101},         // 134
{23, 0b11111111111111111011001},        // 135
{22, 0b1111111111111111010110},         // 136
{23, 0b11111111111111111011010},        // 137
{23, 0b11111111111111111011011},        // 138
{23, 0b11111111111111111011100},        // 139
{23, 0b11111111111111111011101},        // 140
{23, 0b11111111111111111011110},        // 141
{24, 0b111111111111111111101011},       // 142
{23, 0b11111111111111111011111},        // 143
{24, 0b111111111111111111101100},       // 144
{24, 0b111111111111111111101101},       // 145
{22, 0b1111111111111111010111},         // 146
{23, 0b11111111111111111100000},        // 147
{24, 0b111111111111111111101110},       // 148
{23, 0b11111111111111111100001},        // 149
{23, 0b11111111111111111100010},        // 150
{23, 0b11111111111111111100011},        // 151
{23, 0b11111111111111111100100},        // 152
{21, 0b111111111111111011100},          // 153
{22, 0b1111111111111111011000},         // 154
{23, 0b11111111111111111100101},        // 155
{22, 0b1111111111111111011001},         // 156
{23, 0b11111111111111111100110},        // 157
{23, 0b11111111111111111100111},        // 158
{24, 0b111111111111111111101111},       // 159
{22, 0b1111111111111111011010},         // 160
{21, 0b111111111111111011101},          // 161
{20, 0b11111111111111101001},           // 162
{22, 0b1111111111111111011011},         // 163
{22, 0b1111111111111111011100},         // 164
{23, 0b11111111111111111101000},        // 165
{23, 0b11111111111111111101001},        // 166
{21, 0b111111111111111011110},          // 167
{23, 0b11111111111111111101010},        // 168
{22, 0b1111111111111111011101},         // 169
{22, 0b1111111111111111011110},         // 170
{24, 0b111111111111111111110000},       // 171
{21, 0b111111111111111011111},          // 172
{22, 0b1111111111111111011111},         // 173
{23, 0b11111111111111111101011},        // 174
{23, 0b11111111111111111101100},        // 175
{21, 0b111111111111111100000},          // 176
{21, 0b111111111111111100001},          // 177
{22, 0b1111111111111111100000},         // 178
{21, 0b111111111111111100010},          // 179
{23, 0b11111111111111111101101},        // 180
{22, 0b1111111111111111100001},         // 181
{23, 0b11111111111111111101110},        // 182
{23, 0b11111111111111111101111},        // 183
{20, 0b11111111111111101010},           // 184
{22, 0b1111111111111111100010},         // 185
{22, 0b1111111111111111100011},         // 186
{22, 0b1111111111111111100100},         // 187
{23, 0b11111111111111111110000},        // 188
{22, 0b1111111111111111100101},         // 189
{22, 0b1111111111111111100110},         // 190
{23, 0b11111111111111111110001},        // 191
{26, 0b11111111111111111111100000},     // 192
{26, 0b11111111111111111111100001},     // 193
{20, 0b11111111111111101011},           // 194
{19, 0b1111111111111110001},            // 195
{22, 0b1111111111111111100111},         // 196
{23, 0b11111111111111111110010},        // 197
{22, 0b1111111111111111101000},         // 198
{25, 0b1111111111111111111101100},      // 199
{26, 0b11111111111111111111100010},     // 200
{26, 0b11111111111111111111100011},     // 201
{26, 0b11111111111111111111100100},     // 202
{27, 0b111111111111111111111011110},    // 203
{27, 0b111111111111111111111011111},    // 204
{26, 0b11111111111111111111100101},     // 205
{24, 0b111111111111111111110001},       // 206
{25, 0b1111111111111111111101101},      // 207
{19, 0b1111111111111110010},            // 208
{21, 0b111111111111111100011},          // 209
{26, 0b11111111111111111111100110},     // 210
{27, 0b111111111111111111111100000},    // 211
{27, 0b111111111111111111111100001},    // 212
{26, 0b11111111111111111111100111},     // 213
{27, 0b111111111111111111111100010},    // 214
{24, 0b111111111111111111110010},       // 215
{21, 0b111111111111111100100},          // 216
{21, 0b111111111111111100101},          // 217
{26, 0b11111111111111111111101000},     // 218
{26, 0b11111111111111111111101001},     // 219
{28, 0b1111111111111111111111111101},   // 220
{27, 0b111111111111111111111100011},    // 221
{27, 0b111111111111111111111100100},    // 222
{27, 0b111111111111111111111100101},    // 223
{20, 0b11111111111111101100},           // 224
{24, 0b111111111111111111110011},       // 225
{20, 0b11111111111111101101},           // 226
{21, 0b111111111111111100110},          // 227
{22, 0b1111111111111111101001},         // 228
{21, 0b111111111111111100111},          // 229
{21, 0b111111111111111101000},          // 230
{23, 0b11111111111111111110011},        // 231
{22, 0b1111111111111111101010},         // 232
{22, 0b1111111111111111101011},         // 233
{25, 0b1111111111111111111101110},      // 234
{25, 0b1111111111111111111101111},      // 235
{24, 0b111111111111111111110100},       // 236
{24, 0b111111111111111111110101},       // 237
{26, 0b11111111111111111111101010},     // 238
{23, 0b11111111111111111110100},        // 239
{26, 0b11111111111111111111101011},     // 240
{27, 0b111111111111111111111100110},    // 241
{26, 0b11111111111111111111101100},     // 242
{26, 0b11111111111111111111101101},     // 243
{27, 0b111111111111111111111100111},    // 244
{27, 0b111111111111111111111101000},    // 245
{27, 0b111111111111111111111101001},    // 246
{27, 0b111111111111111111111101010},    // 247
{27, 0b111111111111111111111101011},    // 248
{28, 0b1111111111111111111111111110},   // 249
{27, 0b111111111111111111111101100},    // 250
{27, 0b111111111111111111111101101},    // 251
{27, 0b111111111111111111111101110},    // 252
{27, 0b111111111111111111111101111},    // 253
{27, 0b111111111111111111111110000},    // 254
{26, 0b11111111111111111111101110},     // 255
{30, 0b111111111111111111111111111111}, // 256
{0, 0},
};

}

struct tree_node {
  std::unique_ptr<tree_node> left;
  std::unique_ptr<tree_node> right;
  int value;

  tree_node() : value(-1) {}
};

namespace {

class huffman_tree {
public: // XXX
  std::unique_ptr<tree_node> root_;

public:
  huffman_tree()
    : root_(std::make_unique<tree_node>()) {
  }

  const tree_node* get_root() const noexcept {
    return root_.get();
  }

  void insert_code(const huffman_code& code, int value) {
    tree_node* node = root_.get();

    for (unsigned int i = 0 ; i < code.len ; i++) {

      if (!((code.bits >> (code.len - i - 1)) & 0x1)) {
        if (!node->left) {
          // ノード追加
          if (node->value != -1) {
            throw std::logic_error("incorrect code.");
          }
          node->left = std::make_unique<tree_node>();
        }
        node = node->left.get();
      } else {
        if (!node->right) {
          // ノード追加
          if (node->value != -1) {
            throw std::logic_error("incorrect code.");
          }
          node->right = std::make_unique<tree_node>();
        }
        node = node->right.get();
      }
    }
    node->value = value;
  }
};

void binstr(std::ostream& os, uint32_t bits, int length) {
  for (int i = 0 ; i < length ; i++) {
    if (bits & (0x80000000UL >> i)) {
      os << "1";
    } else { 
      os << "0";
    }
  } 
}

void dump_walk_node(const tree_node* node, int depth, uint32_t bits) {
  if (node->left) {
    dump_walk_node(node->left.get(), depth + 1, bits);
  }
  if (node->right) {
    dump_walk_node(node->right.get(), depth + 1, bits | (1 << (31 - depth)));
  }
  if (!node->left && !node->right) {
    binstr(std::cout, bits, depth);
    std::cout << " " << node->value << std::endl;
  }
}

[[maybe_unused]]
void dump(const huffman_tree& tree) {
  dump_walk_node(tree.get_root(), 0, 0);
}

// factory
std::unique_ptr<huffman_tree> create_huffman_tree() {
  auto tree(std::make_unique<huffman_tree>());

  int i = 0;
  while (huffman_codes[i].len) {
    tree->insert_code(huffman_codes[i], i);
    i++;
  }

  return tree;
}

std::unique_ptr<huffman_tree> treep = create_huffman_tree();

} // unnamed namespace


std::vector<unsigned char> decode_huffman(const unsigned char* start, std::size_t length) {

  unsigned int offset = 0;
  unsigned int offset_end = length * 8;

  std::vector<unsigned char> decoded;

  auto root = treep->get_root();
  auto node = root;

  while (offset < offset_end) {
    // fetch a bit
    unsigned int bit = (start[offset >> 3] & (0x80 >> (offset & 7))) ? 1 : 0;

    if (bit) {
      if (!node->right) {
        throw std::logic_error("decode error.");
      }
      node = node->right.get();
    } else {
      if (!node->left) {
        throw std::logic_error("decode error.");
      }
      node = node->left.get();
    }
    if (node->value != -1) {
      // found
      if (node->value == 256) {
        break;
      }
      decoded.push_back(node->value);
      node = root;
    }

    offset++;
  }

  return decoded;
}

} // namespace hpack
