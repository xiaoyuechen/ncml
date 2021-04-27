#include "gntsat/solver.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <tuple>

namespace gntsat {

bool RandBool() noexcept { return rand() % 2; }

bool IsClauseSat(const Solution& solution, const Problem::Clause& clause) {
  for (int i = 0; i != clause.size(); ++i) {
    int idx = abs(clause[i]);
    bool var = solution[idx];
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
  out_solution->resize(size);
  for (int i = 0; i != size; ++i) {
    (*out_solution)[i] = RandBool();
  }
}

void PrintBitString(const Solution& solution) {
  for (int i = 0; i != solution.size(); ++i) {
    std::cout << solution[i];
  }
  std::cout << std::endl;
}

void PrintPopulation(const Population& population, const Problem& problem) {
  for (int i = 0; i != population.size(); ++i) {
    PrintBitString(population[i]);
    std::cout << CountSatClause(population[i], problem);
    std::cout << std::endl;
  }
}

Solution OnePointCrossOver(const Solution& sol_1, const Solution& sol_2, int point) {
    std::vector<bool>::const_iterator begin_point = sol_1.bit_string.begin() + point * 4;
    std::vector<bool>::const_iterator end_point = sol_1.bit_string.end();
    std::vector<int> temp_vec(begin_point, end_point);


}

float EvalFitness(const Solution& solution, const Problem& problem) {
  return CountSatClause(solution, problem);
}

int RankSelect(int size) {
  int sum = ((1 + size) * size) / 2;
  int r = rand() % sum;
  return floor((1 + sqrt(1 + 8 * r)) / 2) - 1;
}

static constexpr std::size_t kMaxTournamentSize = 64;
std::size_t TournamentSelect(const Population& population,
                             const Problem& problem,
                             std::size_t tournament_size, float p = 1.0f) {
  auto candidadtes = std::array<std::size_t, kMaxTournamentSize>{};
  for (std::size_t i = 0; i != tournament_size; ++i) {
    candidadtes[i] = rand() % population.size();
  }
  std::sort(candidadtes.begin(), candidadtes.begin() + tournament_size,
            [&](std::size_t lhs, std::size_t rhs) {
              return EvalFitness(population[lhs], problem) <
                     EvalFitness(population[rhs], problem);
            });
  for (std::size_t i = 0; i != tournament_size; ++i) {
    float r = rand() / (float)RAND_MAX;
    if (r < p) return candidadtes[i];
  }
  return candidadtes[tournament_size - 1];
}

void Clone(const Population& old_gen, Population* new_gen, float rate) {
  for (std::size_t i = 0; i != (std::size_t)old_gen.size() * rate; ++i) {
    new_gen->push_back(old_gen[i]);
  }
}

Solver::Solver(const Problem& problem, Setting setting)
    : problem_(problem), setting_(setting) {
  srand(time(0));
  GetCurrentGen().resize(setting_.population_size);
  for (int i = 0; i != GetCurrentGen().size(); ++i) {
    do {
      GenRandSolution(&GetCurrentGen()[i], problem_.var_count);
    } while (CountSatClause(GetCurrentGen()[i], problem_) <
             ((7.f / 8.f) * problem_.cnf.size()));
  }
}

Solution Solver::run() {
  while (true) {
    std::sort(GetCurrentGen().begin(), GetCurrentGen().end(),
              [=](const Solution& lhs, const Solution& rhs) {
                return EvalFitness(lhs, problem_) > EvalFitness(rhs, problem_);
              });
    if (CountSatClause(GetCurrentGen().back(), problem_)) {
      return GetCurrentGen().back();
    }

    Clone(GetCurrentGen(), &GetNextGen(), setting_.clone_rate);
  }
}

}  // namespace gntsat
