#pragma once

#include <random>
#include <vector>

#include "gntsat/problem.h"

namespace gntsat {

using Solution = std::vector<bool>;

using Population = std::vector<Solution>;

class Solver {
 public:
  struct Setting {
    std::size_t population_size;
    float clone_rate, cross_over_rate, mutation_rate;
  };

  Solver(const Problem& problem, Setting setting);

  Solution run();

 private:
  const Problem& problem_;
  Setting setting_;
  Population population_;
};

}  // namespace gntsat
