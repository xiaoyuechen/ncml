#include "gntsat/solver.h"

#include <cstdlib>
#include <ctime>

namespace gntsat {

bool rand_bool() noexcept { return rand() % 2; }

int count_sat_clauses(const Solution& solution,
                      const Problem& problem) noexcept {
  int sat_count = 0;
  for (const auto& clause : problem.cnf) {
    for (int i = 0; i != clause.size(); ++i) {
      int idx = abs(clause[i]);
      bool var = solution.bit_string[idx];

      if (clause[i] > 0) {
        sat_count += var;
        break;
      } else {
        sat_count += !var;
      }
    }
  }
  return sat_count;
}

void gen_random_sol(Solution* out_solution, int size) {
  out_solution->bit_string.resize(size);
  for (int i = 0; i != size; ++i) {
    out_solution->bit_string[i] = rand_bool();
  }
}

Solver::Solver(const Problem& problem) : problem_(problem) { srand(time(0)); }

Solution Solver::run() {}

}  // namespace gntsat
