#include "gntsat/io.h"

#include <cstdio>

namespace gntsat {

void PrintBitstring(const uint64_t* bitstring_begin,
                    size_t bitstring_offset_bits, size_t num_var) {
  for (int i = 0; i != num_var; ++i) {
    uint64_t word = *(bitstring_begin + (bitstring_offset_bits + i) / 64);
    size_t bit_pos = (bitstring_offset_bits + i) % 64;
    printf("%d", ((word & (1ull << bit_pos)) == 0));
  }
}

}  // namespace gntsat
