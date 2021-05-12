#include <bitset>

#include "gntsat/compute.h"
#include "gntsat/io.h"
#include "gtest/gtest.h"

namespace gntsat {

TEST(ReadBitTest, CanReadBit) {
  uint64_t word = 0b0011010;
  auto bitset = std::bitset<64>(word);
  for (size_t i = 0; i < 64; ++i) {
    EXPECT_EQ(bitset[i], ReadBit(&word, i));
  }
}

TEST(WriteBitTest, CanWriteBit) {
  uint64_t word = 0b0011010;
  WriteBit(&word, 2, false);
  EXPECT_EQ(word, 0b0011010);
  WriteBit(&word, 2, true);
  EXPECT_EQ(word, 0b0011110);
}

TEST(FlipBitTest, CanFlipBit) {
  uint64_t word = 0b0011010;
  FlipBit(&word, 2);
  EXPECT_EQ(word, 0b0011110);
}

TEST(IsClauseSatTest, CanTestSatClause) {
  int clause[3] = {-2, 4, 16};
  uint64_t word = 0;
  EXPECT_TRUE(IsClauseSat(&word, 0, clause));
}

TEST(IsClauseSatTest, CanTestUnsatClause) {
  int clause[3] = {2, 4, -64};
  uint64_t words[2] = {0b01011, 0b1};
  EXPECT_FALSE(IsClauseSat(words, 0, clause));
}

TEST(CountSatTest, CanCountSat) {
  uint64_t result[3] = {0b11, ~0ull, 0b11};
  EXPECT_EQ(CountSat(result, 1, 63 + 64 + 2), 67);
}

TEST(InitPopulationTest, CanRandomInit) {
  auto population = CreatePopulation(8, 65);
  for (int i = 0; i < 8 * 65; i += 65) {
    PrintBitstring(population.individuals, i, 65);
    printf("\n");
  }
}

}  // namespace gntsat
