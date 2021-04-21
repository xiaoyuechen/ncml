#pragma once

#include <vector>
#include <random>

#include "gntsat/problem.h"

namespace gntsat {

struct Solution {
  std::vector<bool> bit_string;
};

class Solver {
 public:
  Solver(const Problem& problem);

  Solution run();

 private:
  using Polulation = std::vector<Solution>;

  const Problem& problem_;
  Polulation polulation_;
};

}  // namespace gntsat
