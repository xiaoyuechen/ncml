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
  using Population = std::vector<Solution>;

  const Problem& problem_;
  Population population_;
};

}  // namespace gntsat
