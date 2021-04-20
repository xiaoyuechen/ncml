#pragma once

#include <array>
#include <vector>

namespace gntsat {

struct Problem {
  using Clause = std::array<int, 3>;
  int var_count;
  std::vector<Clause> cnf;
};

}  // namespace gntsat
