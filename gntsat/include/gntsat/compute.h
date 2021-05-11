#pragma once

#include <xmmintrin.h>

#include <algorithm>
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

inline void SetBit(uint64_t* bitstring_begin, size_t bitstring_offset_bits,
                   size_t bit_pos) noexcept {
  bit_pos += bitstring_offset_bits;
  size_t word_pos = bit_pos / 64;
  size_t bit_offset = bit_pos - word_pos * 64;
  uint64_t& word = *(bitstring_begin + word_pos);
  word |= (1ull << bit_offset);
}

inline void ResetBit(uint64_t* bitstring_begin, size_t bitstring_offset_bits,
                     size_t bit_pos, bool val) noexcept {
  bit_pos += bitstring_offset_bits;
  size_t word_pos = bit_pos / 64;
  size_t bit_offset = bit_pos - word_pos * 64;
  uint64_t& word = *(bitstring_begin + word_pos);
  word &= ~(1ull << bit_offset);
}

inline void FlipBit(uint64_t* bitstring_begin, size_t bitstring_offset_bits,
                    size_t bit_pos) noexcept {
  bit_pos += bitstring_offset_bits;
  size_t word_pos = bit_pos / 64;
  size_t bit_offset = bit_pos - word_pos * 64;
  uint64_t& word = *(bitstring_begin + word_pos);
  word ^= (1ull << bit_offset);
}

inline void WriteBit(uint64_t* bitstring_begin, size_t bitstring_offset_bits,
                     size_t bit_pos, bool val) noexcept {
  bit_pos += bitstring_offset_bits;
  size_t word_pos = bit_pos / 64;
  size_t bit_offset = bit_pos - word_pos * 64;
  uint64_t& word = *(bitstring_begin + word_pos);
  word ^= (-((int64_t)val) ^ word) & (1ull << bit_offset);
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

inline size_t CountSat(const uint64_t* bitstring_begin,
                       size_t bitstring_offset_bits, const int* cnf_begin,
                       const int* cnf_end) noexcept {
  size_t count = 0;
  for (size_t i = 0; i < (cnf_end - cnf_begin) / 3; ++i) {
    const int* clause = cnf_begin + 3 * i;
    bool sat = IsClauseSat(bitstring_begin, bitstring_offset_bits, clause);
    count += sat;
  }
  return count;
}

struct Population {
  uint64_t* individuals;
  size_t num_var;
  size_t size;
  size_t newest;
};

inline size_t CountWord(Population population) {
  return (population.size * population.num_var + 63) / 64;
}

inline Population CreatePopulation(size_t size, size_t num_var) {
  Population population;
  population.size = size;
  population.num_var = num_var;
  population.newest = 0;

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

static constexpr size_t kMaxTournamentSize = 64;
inline void TournamentSelect(size_t* out_selected, const Population& population,
                             size_t select_n, size_t tournament_size,
                             const int* cnf_begin, const int* cnf_end) {
  size_t candidadtes[kMaxTournamentSize];
  for (std::size_t i = 0; i < tournament_size; ++i)
    candidadtes[i] = rand() % population.size;

  std::sort(
      candidadtes, candidadtes + tournament_size, [&](size_t lhs, size_t rhs) {
        return CountSat(population.individuals, population.num_var * lhs,
                        cnf_begin, cnf_end) > CountSat(population.individuals,
                                                       population.num_var * rhs,
                                                       cnf_begin, cnf_end);
      });
  std::copy(candidadtes, candidadtes + select_n, out_selected);
}

inline int Improvement(const uint64_t* bitstring, size_t bitstring_offset_bits,
                       size_t flip_bit, const int* cnf_begin,
                       const int* cnf_end) {
  int before_sat_count =
      CountSat(bitstring, bitstring_offset_bits, cnf_begin, cnf_end);
  FlipBit((uint64_t*)bitstring, bitstring_offset_bits, flip_bit);
  int after_sat_count =
      CountSat(bitstring, bitstring_offset_bits, cnf_begin, cnf_end);
  FlipBit((uint64_t*)bitstring, bitstring_offset_bits, flip_bit);
  return after_sat_count - before_sat_count;
}

inline void CrossoverCC(size_t* out_child, size_t child_offset,
                        uint64_t* parentx, size_t parentx_offset,
                        uint64_t* parenty, size_t parenty_offset,
                        size_t num_var, const int* cnf_begin,
                        const int* cnf_end) noexcept {
  for (size_t i = 0; i < num_var; ++i) {
    bool use_parantx_bit = rand() % 2;
    bool val = use_parantx_bit * ReadBit(parentx, parentx_offset, i) +
               (1 - use_parantx_bit) * ReadBit(parenty, parenty_offset, i);
    WriteBit(out_child, child_offset, i, val);
  }

  size_t num_clause = (cnf_end - cnf_begin) / 3;
  size_t num_result_word = (num_clause + 63) / 64;
  static uint64_t* parentx_result =
      (uint64_t*)_mm_malloc(num_result_word * 8, 8);
  static uint64_t* parenty_result =
      (uint64_t*)_mm_malloc(num_result_word * 8, 8);
  IsCnfSat(parentx_result, 0, parentx, parentx_offset, cnf_begin, cnf_end);
  IsCnfSat(parenty_result, 0, parenty, parenty_offset, cnf_begin, cnf_end);
  for (size_t i = 0; i < num_clause; ++i) {
    int deltas[3];
    if (!ReadBit(parentx_result, 0, i) && !ReadBit(parenty_result, 0, i)) {
      for (size_t j = 0; j < 3; ++j) {
        int literal = *(cnf_begin + 3 * i + j);
        deltas[i] = Improvement(parentx, parentx_offset, abs(literal),
                                cnf_begin, cnf_end) +
                    Improvement(parenty, parenty_offset, abs(literal),
                                cnf_begin, cnf_end);
      }
      int* max_delta = std::max_element(deltas, deltas + 3);
      int k = (max_delta - deltas);
      WriteBit(out_child, child_offset, k,
               !ReadBit(parentx, parentx_offset, k));
    }
  }
}

}  // namespace gntsat
