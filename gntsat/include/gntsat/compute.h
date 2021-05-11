#pragma once

#include <xmmintrin.h>

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <random>

namespace gntsat {

inline bool ReadBit(const uint64_t* bitstring_begin,
                    size_t bitstring_offset_bits, size_t bit_pos) noexcept {
  bit_pos += bitstring_offset_bits;
  size_t word_pos = bit_pos / 64;
  size_t bit_offset = bit_pos - word_pos * 64;
  uint64_t word = *(bitstring_begin + word_pos);
  return word & (1ull << bit_offset);
}

inline void WriteBit(uint64_t* bitstring_begin, size_t bitstring_offset_bits,
                     size_t bit_pos, bool val) noexcept {
  bit_pos += bitstring_offset_bits;
  size_t word_pos = bit_pos / 64;
  size_t bit_offset = bit_pos - word_pos * 64;
  uint64_t& word = *(bitstring_begin + word_pos);
  word |= (1ull << bit_offset);
}

inline bool IsClauseSat(const uint64_t* bitstring_begin,
                        size_t bitstring_offset_bits,
                        const int* clause) noexcept {
  int sat_literal_count = 0;
  for (const int* literal = clause; literal < clause + 3; ++literal) {
    int bit_pos = abs(*literal);
    bool var = ReadBit(bitstring_begin, bitstring_offset_bits, bit_pos);
    sat_literal_count += ((*literal > 0) == var);
  }
  return sat_literal_count;
}

inline void IsCnfSat(uint64_t* out_result, size_t result_offset_bits,
                     const uint64_t* bitstring_begin,
                     size_t bitstring_offset_bits, const int* cnf_begin,
                     const int* cnf_end) noexcept {
  for (size_t i = 0; i < (cnf_end - cnf_begin) / 3; ++i) {
    const int* clause = cnf_begin + 3 * i;
    bool sat = IsClauseSat(bitstring_begin, bitstring_offset_bits, clause);
    WriteBit(out_result, result_offset_bits, i, sat);
  }
}

inline size_t CountSat(const uint64_t* result_begin, size_t result_offset_bits,
                       size_t num_clause) noexcept {
  size_t count = 0;
  for (size_t i = 0; i < num_clause / 64; ++i) {
    uint64_t word = *(result_begin + i);
#ifdef __GNUC__
    count += __builtin_popcountll(*(result_begin + i));
#else
    auto bitset = std::bitset<64>(word);
    count += bitset.count();
#endif
  }
  for (size_t i = 0; i < result_offset_bits; ++i)
    count -= (bool)(*result_begin & (1ull << i));
  for (size_t i = 0; i < (num_clause + result_offset_bits) % 64; ++i)
    count += (bool)(*(result_begin + num_clause / 64) & (1ull << i));

  return count;
}

struct Population {
  uint64_t* individuals;
  size_t num_var;
  size_t size;
};

inline size_t CountWord(Population population) {
  return (population.size * population.num_var + 63) / 64;
}

inline Population CreatePopulation(size_t size, size_t num_var) {
  Population population;
  population.size = size;
  population.num_var = num_var;

  size_t word_count = CountWord(population);
  size_t byte_count = word_count * 8;
  population.individuals = (uint64_t*)_mm_malloc(byte_count, 8);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint64_t> distribution(
      std::numeric_limits<uint64_t>::min(),
      std::numeric_limits<uint64_t>::max());
  for (uint64_t* word = population.individuals;
       word < population.individuals + word_count; ++word) {
    *word = distribution(gen);
  }
  return population;
}

inline void DestroyPopulation(Population population) {
  _mm_free(population.individuals);
}

/*
struct Population {
  uint64_t* solutions;
  uint64_t* clause_sat_arrs;
  uint32_t* clause_sat_freqencies;
  size_t size;
};


inline void ComputeClauseSatArrs(const int* cnf_begin, const int* cnf_end,
                                 const uint64_t* solutions_begin,
                                 const uint64_t* solutions_end, uint64_t* dst,
                                 size_t var_count) {
  size_t clause_count = (cnf_end - cnf_begin) / 3;
  for (size_t sol_bit_pos = 0;
       sol_bit_pos < (solutions_end - solutions_begin) * 64;
       sol_bit_pos += var_count) {
    for (const int* clause = cnf_begin; clause < cnf_end; clause += 3) {
      int sat_literal_count = 0;
      for (const int* literal = clause; literal < clause + 3; ++literal) {
        int idx = abs(*literal);
        size_t var_bit_pos = sol_bit_pos + idx;
        uint64_t word = *(solutions_begin + (var_bit_pos / 64));
        int pos_in_word = var_bit_pos % 64;
        bool var = word & (1ull << pos_in_word);
        sat_literal_count += ((*literal > 0) == var);
      }
    }
  }
}

*/
}  // namespace gntsat
