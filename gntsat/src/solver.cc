#include "gntsat/solver.h"

#include <cstdlib>
#include <ctime>
#include <iostream>

namespace gntsat {

bool RandBool() noexcept { return rand() % 2; }

bool IsClauseSat(const Solution& solution, const Problem::Clause& clause) {
  for (int i = 0; i != clause.size(); ++i) {
    int idx = abs(clause[i]);
    bool var = solution.bit_string[idx];
    if ((clause[i] > 0) == var) {
      return true;
    }
  }
  return false;
}

int CountSatClause(const Solution& solution, const Problem& problem) noexcept {
  int sat_count = 0;
  for (const auto& clause : problem.cnf) {
    sat_count += IsClauseSat(solution, clause);
  }
  return sat_count;
}

void GenRandClause(Solution* out_solution, int size) {
  out_solution->bit_string.resize(size);
  for (int i = 0; i != size; ++i) {
    out_solution->bit_string[i] = RandBool();
  }
}

Solver::Solver(const Problem& problem) : problem_(problem) { srand(time(0)); }

Solution Solver::run() {}

}  // namespace gntsat
