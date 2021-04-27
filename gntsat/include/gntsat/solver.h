#pragma once

#include <random>
#include <vector>

#include "gntsat/problem.h"

namespace gntsat {

using Solution = std::vector<bool>;

using Population = std::vector<Solution>;

class Solver {
 public:
  Solver(const Problem& problem);

  Solution run();

 private:
  const Problem& problem_;
  Population population_;
};

}  // namespace gntsat
