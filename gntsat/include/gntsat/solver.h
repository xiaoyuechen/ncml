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
  Population& GetCurrentGen() noexcept { return population_[current_gen_]; }
  Population& GetNextGen() noexcept { return population_[!current_gen_]; }

  const Problem& problem_;
  Setting setting_;
  Population population_[2];
  bool current_gen_ = 0;
};

}  // namespace gntsat
