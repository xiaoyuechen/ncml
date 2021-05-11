#pragma once

#include "gntsat/compute.h"

namespace gntsat {

void PrintBitstring(const uint64_t* bitstring_begin,
                    size_t bitstring_offset_bits, size_t num_var);

}  // namespace gntsat
