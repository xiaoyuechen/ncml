#pragma once

#include <xmmintrin.h>

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>

namespace gntsat {

inline size_t g_num_flips = 0;

inline bool ReadBit(const uint64_t* bitstring_begin, size_t bit_pos) noexcept {
  size_t word_pos = bit_pos / 64;
  size_t bit_offset = bit_pos - word_pos * 64;
  uint64_t word = *(bitstring_begin + word_pos);
  return word & (1ull << bit_offset);
}

inline void SetBit(uint64_t* bitstring_begin, size_t bit_pos) noexcept {
  ++g_num_flips;
  size_t word_pos = bit_pos / 64;
  size_t bit_offset = bit_pos - word_pos * 64;
  uint64_t& word = *(bitstring_begin + word_pos);
  word |= (1ull << bit_offset);
}

inline void ResetBit(uint64_t* bitstring_begin, size_t bit_pos,
                     bool val) noexcept {
  ++g_num_flips;
  size_t word_pos = bit_pos / 64;
  size_t bit_offset = bit_pos - word_pos * 64;
  uint64_t& word = *(bitstring_begin + word_pos);
  word &= ~(1ull << bit_offset);
}

inline void FlipBit(uint64_t* bitstring_begin, size_t bit_pos) noexcept {
  ++g_num_flips;
  size_t word_pos = bit_pos / 64;
  size_t bit_offset = bit_pos - word_pos * 64;
  uint64_t& word = *(bitstring_begin + word_pos);
  word ^= (1ull << bit_offset);
}

inline void WriteBit(uint64_t* bitstring_begin, size_t bit_pos,
                     bool val) noexcept {
  ++g_num_flips;
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
    bool var = ReadBit(bitstring_begin, bitstring_offset_bits + bit_pos);
    sat_literal_count += ((*literal > 0) == var);
  }
  return sat_literal_count;
}

inline void IsCnfSat(uint64_t* out_result, const uint64_t* bitstring_begin,
                     size_t bitstring_offset_bits, const int* cnf_begin,
                     const int* cnf_end) noexcept {
  for (size_t i = 0; i < (cnf_end - cnf_begin) / 3; ++i) {
    const int* clause = cnf_begin + 3 * i;
    bool sat = IsClauseSat(bitstring_begin, bitstring_offset_bits, clause);
    WriteBit(out_result, i, sat);
    --g_num_flips;
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
  FlipBit((uint64_t*)bitstring, bitstring_offset_bits + flip_bit);
  int after_sat_count =
      CountSat(bitstring, bitstring_offset_bits, cnf_begin, cnf_end);
  FlipBit((uint64_t*)bitstring, bitstring_offset_bits + flip_bit);
  return after_sat_count - before_sat_count;
}

inline int BreakCount(const uint64_t* bitstring, size_t bitstring_offset_bits,
                      size_t flip_bit, const int* cnf_begin,
                      const int* cnf_end) {
  size_t num_clause = (cnf_end - cnf_begin) / 3;
  size_t num_result_word = (num_clause + 63) / 64;
  static uint64_t* originRes = (uint64_t*)_mm_malloc(num_result_word * 8, 8);
  static uint64_t* flipedRes = (uint64_t*)_mm_malloc(num_result_word * 8, 8);
  IsCnfSat(originRes, bitstring, bitstring_offset_bits, cnf_begin, cnf_end);
  FlipBit((uint64_t*)bitstring, bitstring_offset_bits + flip_bit);
  IsCnfSat(flipedRes, bitstring, bitstring_offset_bits, cnf_begin, cnf_end);
  FlipBit((uint64_t*)bitstring, bitstring_offset_bits + flip_bit);

  for (int i = 0; i < num_result_word; ++i) {
    uint64_t andRes = originRes[i] & flipedRes[i];
    originRes[i] ^= andRes;
  }
  return (int)CountSat(originRes, 0, num_clause);
}

inline void WalkMutation(uint64_t* bitstring, size_t bitstring_offset_bits,
                         int MAX_FLIPS, float probability, const int* cnf_begin,
                         const int* cnf_end) {
  size_t num_clause = (cnf_end - cnf_begin) / 3;
  size_t num_result_word = (num_clause + 63) / 64;
  static uint64_t* originRes = (uint64_t*)_mm_malloc(num_result_word * 8, 8);
  static uint64_t* unsat_arr = (uint64_t*)_mm_malloc(num_clause * 8, 8);
  size_t currentSize = 0;
  for (int i = 0; i < MAX_FLIPS; ++i) {
    IsCnfSat(originRes, bitstring, bitstring_offset_bits, cnf_begin, cnf_end);
    for (int j = 0; j < num_clause; ++j) {
      if (!ReadBit(originRes, j)) {
        unsat_arr[currentSize++] = j;
      }
    }
    if (currentSize == 0) {
      return;
    }
    const int* clause = cnf_begin + unsat_arr[rand() % currentSize] * 3;
    currentSize = 0;
    bool freebie_move_flag = false;
    int break_count_arr[3] = {};
    for (int k = 0; k < 3; ++k) {
      int variable_c = abs(clause[k]);
      break_count_arr[k] = BreakCount(bitstring, bitstring_offset_bits,
                                      variable_c, cnf_begin, cnf_end);
    }
    for (int k = 0; k < 3; ++k) {
      // Variable_c is the index of variable in one certain clause
      int variable_c = abs(clause[k]);
      if (break_count_arr[k] == 0) {
        FlipBit(bitstring, bitstring_offset_bits + variable_c);
        freebie_move_flag = true;
        break;
      }
    }

    // If there is not a freebie movement, then ..
    if (!freebie_move_flag) {
      if ((rand() / (double)RAND_MAX) < probability) {
        int flip_index = rand() % 3;
        FlipBit(bitstring, bitstring_offset_bits + abs(clause[flip_index]));
      } else {
        int* min_ele = std::min_element(break_count_arr, break_count_arr + 3);
        int flip_index = min_ele - break_count_arr;
        FlipBit(bitstring, bitstring_offset_bits + abs(clause[flip_index]));
      }
    }
  }
}

inline void CrossoverUniform(uint64_t* out_child, const uint64_t* parentx,
                             size_t parentx_offset, const uint64_t* parenty,
                             size_t parenty_offset, size_t num_var) noexcept {
  for (size_t i = 0; i < num_var; ++i) {
    bool use_parantx_bit = rand() % 2;
    bool val = use_parantx_bit * ReadBit(parentx, parentx_offset + i) +
               (1 - use_parantx_bit) * ReadBit(parenty, parenty_offset + i);
    WriteBit(out_child, i, val);
  }
}

inline void CrossoverCC(uint64_t* out_child, const uint64_t* parentx,
                        size_t parentx_offset, const uint64_t* parenty,
                        size_t parenty_offset, size_t num_var,
                        const int* cnf_begin, const int* cnf_end) noexcept {
  CrossoverUniform(out_child, parentx, parentx_offset, parenty, parenty_offset,
                   num_var);

  size_t num_clause = (cnf_end - cnf_begin) / 3;
  size_t num_result_word = (num_clause + 63) / 64;
  static uint64_t* parentx_result =
      (uint64_t*)_mm_malloc(num_result_word * 8, 8);
  static uint64_t* parenty_result =
      (uint64_t*)_mm_malloc(num_result_word * 8, 8);
  IsCnfSat(parentx_result, parentx, parentx_offset, cnf_begin, cnf_end);
  IsCnfSat(parenty_result, parenty, parenty_offset, cnf_begin, cnf_end);
  for (size_t i = 0; i < num_clause; ++i) {
    int deltas[3];
    if (!ReadBit(parentx_result, i) && !ReadBit(parenty_result, i)) {
      for (size_t j = 0; j < 3; ++j) {
        int literal = *(cnf_begin + 3 * i + j);
        deltas[j] = Improvement(parentx, parentx_offset, abs(literal),
                                cnf_begin, cnf_end) +
                    Improvement(parenty, parenty_offset, abs(literal),
                                cnf_begin, cnf_end);
      }
      int* max_delta = std::max_element(deltas, deltas + 3);
      int k = (max_delta - deltas);
      const int literal = *(cnf_begin + 3 * i + k);
      size_t bit_pos = abs(literal);
      WriteBit(out_child, bit_pos, !ReadBit(parentx, parentx_offset + bit_pos));
    }
  }
}

inline void CrossoverFF(uint64_t* out_child, const uint64_t* parentx,
                        size_t parentx_offset, const uint64_t* parenty,
                        size_t parenty_offset, size_t num_var,
                        const int* cnf_begin, const int* cnf_end) noexcept {
  CrossoverUniform(out_child, parentx, parentx_offset, parenty, parenty_offset,
                   num_var);

  size_t num_clause = (cnf_end - cnf_begin) / 3;
  size_t num_result_word = (num_clause + 63) / 64;
  static uint64_t* parentx_result =
      (uint64_t*)_mm_malloc(num_result_word * 8, 8);
  static uint64_t* parenty_result =
      (uint64_t*)_mm_malloc(num_result_word * 8, 8);
  IsCnfSat(parentx_result, parentx, parentx_offset, cnf_begin, cnf_end);
  IsCnfSat(parenty_result, parenty, parenty_offset, cnf_begin, cnf_end);

  for (size_t i = 0; i < num_clause; ++i) {
    const uint64_t* chosen_parent = nullptr;
    size_t chosen_parent_offset = 0;
    if (ReadBit(parentx_result, i) && !ReadBit(parenty_result, i)) {
      chosen_parent = parentx;
      chosen_parent_offset = parentx_offset;
    } else if (!ReadBit(parentx_result, i) && ReadBit(parenty_result, i)) {
      chosen_parent = parenty;
      chosen_parent_offset = parenty_offset;
    }

    if (chosen_parent) {
      const int* clause = cnf_begin + 3 * i;
      for (size_t i = 0; i < 3; ++i) {
        size_t bit_pos = abs(clause[i]);
        WriteBit(out_child, bit_pos,
                 ReadBit(chosen_parent, chosen_parent_offset + bit_pos));
      }
    }
  }
}

inline void CrossoverOnePoint(uint64_t* out_child1, uint64_t* out_child2,
                              const uint64_t* parentx, size_t parentx_offset,
                              const uint64_t* parenty, size_t parenty_offset,
                              size_t num_var) noexcept {
  int point =
      rand() % num_var;  // randomly select a point for swapping two individuals
  for (size_t i = 0; i < point; ++i) {
    bool val1 = ReadBit(parentx, parentx_offset + i);
    WriteBit(out_child1, i, val1);
    bool val2 = ReadBit(parenty, parenty_offset + i);
    WriteBit(out_child2, i, val2);
  }
  for (size_t i = point; i < num_var; ++i) {
    bool val1 = ReadBit(parenty, parenty_offset + i);
    WriteBit(out_child1, i, val1);
    bool val2 = ReadBit(parentx, parentx_offset + i);
    WriteBit(out_child2, i, val2);
  }
}

inline void CrossoverTwoPoint(uint64_t* out_child1, uint64_t* out_child2,
                              uint64_t* parentx, size_t parentx_offset,
                              uint64_t* parenty, size_t parenty_offset,
                              size_t num_var) noexcept {
  int point1 =
      rand() % num_var;  // randomly select a point for swapping two individuals
  int point2 = rand() % num_var;
  while (point1 == point2) {
    point2 = rand() % num_var;
  }
  if (point1 > point2) {
    int temp = point1;
    point1 = point2;
    point2 = temp;
  }
  for (size_t i = 0; i < point1; ++i) {
    bool val1 = ReadBit(parentx, parentx_offset + i);
    WriteBit(out_child1, i, val1);
    bool val2 = ReadBit(parenty, parenty_offset + i);
    WriteBit(out_child2, i, val2);
  }
  for (size_t i = point1; i < point2; ++i) {
    bool val1 = ReadBit(parenty, parenty_offset + i);
    WriteBit(out_child1, i, val1);
    bool val2 = ReadBit(parentx, parentx_offset + i);
    WriteBit(out_child2, i, val2);
  }
  for (size_t i = point2; i < num_var; ++i) {
    bool val1 = ReadBit(parentx, parentx_offset + i);
    WriteBit(out_child1, i, val1);
    bool val2 = ReadBit(parenty, parenty_offset + i);
    WriteBit(out_child2, i, val2);
  }
}

}  // namespace gntsat
