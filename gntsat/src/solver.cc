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

void GenRandSolution(Solution* out_solution, int size) {
  out_solution->bit_string.resize(size);
  for (int i = 0; i != size; ++i) {
    out_solution->bit_string[i] = RandBool();
  }
}

void PrintBitString(const Solution& solution) {
  for (int i = 0; i != solution.bit_string.size(); ++i) {
    std::cout << solution.bit_string[i];
  }
  std::cout << std::endl;
}

void PrintPopulation(const Solver::Population& population,
                     const Problem& problem) {
  for (int i = 0; i != population.size(); ++i) {
    PrintBitString(population[i]);
    std::cout << CountSatClause(population[i], problem);
    std::cout << std::endl;
  }
}

Solver::Solver(const Problem& problem) : problem_(problem) {
  srand(time(0));
  int initial_population_count = 800;
  population_.resize(initial_population_count);
  for (int i = 0; i != population_.size(); ++i) {
    do {
      GenRandSolution(&population_[i], problem_.var_count);
    } while (CountSatClause(population_[i], problem_) <
             ((7.f / 8.f) * problem_.cnf.size()));
  }
  PrintPopulation(population_, problem_);
}

Solution Solver::run() {}

}  // namespace gntsat
