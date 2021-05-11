#include "gntsat/solver.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <tuple>

namespace gntsat {

using FitnessFn = std::function<float(const Solution&)>;

bool RandBool() noexcept { return rand() % 2; }

bool IsClauseSat(const Solution& solution, const Problem::Clause& clause) {
  int sat_literal_count = 0;
  for (int i = 0; i != clause.size(); ++i) {
    int idx = abs(clause[i]);
    bool var = solution[idx];
    sat_literal_count += ((clause[i] > 0) == var);
  }
  return sat_literal_count;
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

float EvalFitness(const Yi& yi, const std::vector<float>& frequencies) {
  float fitness = 0;
  for (std::size_t i = 0; i != yi.size(); ++i)
    fitness -= !yi[i] * (1/frequencies[i]);
  return fitness;
}

static constexpr std::size_t kMaxTournamentSize = 64;
std::size_t TournamentSelect(const Population& population,
                             const FitnessFn& fitness_fn,
                             std::size_t tournament_size, float p = 1.0f) {
  auto candidadtes = std::array<std::size_t, kMaxTournamentSize>{};
  for (std::size_t i = 0; i != tournament_size; ++i) {
    candidadtes[i] = rand() % population.size();
  }
  std::sort(candidadtes.begin(), candidadtes.begin() + tournament_size,
            [&](std::size_t lhs, std::size_t rhs) {
              return fitness_fn(population[lhs]) > fitness_fn(population[rhs]);
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

std::array<std::size_t, 2> SelectCrossoverPair(const Population& population,
                                               const FitnessFn& fitness_fn,
                                               std::size_t tournament_size,
                                               float p) {
  std::array<std::size_t, 2> index_arr;
  for (int j = 0; j < 2; ++j) {
    index_arr[j] = TournamentSelect(population, fitness_fn, tournament_size, p);
  }

  return index_arr;
}

void OnePointCrossover(const Population& old_gen, Population* new_gen,
                       std::array<std::size_t, 2> arr) {
  int point = rand() % old_gen.back().size();
  new_gen->push_back(old_gen[arr[0]]);
  new_gen->push_back(old_gen[arr[1]]);

  std::swap_ranges(new_gen->rbegin()->begin(),
                   new_gen->rbegin()->begin() + point,
                   (new_gen->rbegin() + 1)->begin());
}

void Mutate(const Solution& old_gen, Solution* new_gen,
            std::size_t mutation_flip_count) {
  for (std::size_t i = 0; i != mutation_flip_count; ++i) {
    std::size_t idx = rand() % old_gen.size();
    *new_gen = old_gen;
    (*new_gen)[idx] = !(*new_gen)[idx];
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
  index_arr_.resize(setting_.population_size);
  for (int i = 0; i < setting_.population_size; ++i) {
    index_arr_[i] = i;
  }
}

void UpdateYi(const Solution& solution, const Problem& problem, Yi* yi) {
  yi->resize(problem.cnf.size());
  for (std::size_t i = 0; i != problem.cnf.size(); ++i) {
    (*yi)[i] = IsClauseSat(solution, problem.cnf[i]);
  }
}

void UpdateY(const Population& population, const Problem& problem,
             std::vector<Yi>* y) {
  y->resize(population.size());
  for (std::size_t i = 0; i != population.size(); ++i)
    UpdateYi(population[i], problem, &(*y)[i]);
}

void CountClauseSatFrequency(const std::vector<Yi>& y,
                             std::vector<float>* frequencies) {
  frequencies->resize(y.front().size());
  std::fill(frequencies->begin(), frequencies->end(), 0.0f);
  for (const auto& yi : y) {
    for (std::size_t i = 0; i != yi.size(); ++i) (*frequencies)[i] += yi[i];
  }
  for (auto& frequency : *frequencies) frequency /= y.size();
}

Solution Solver::run() {
  auto fitness_fn = [=](const Solution& solution) {
    std::size_t idx = &solution - GetCurrentGen().data();
    return EvalFitness(y_[idx], clause_sat_freqency_);
  };

  while (true) {
    UpdateY(GetCurrentGen(), problem_, &y_);
    CountClauseSatFrequency(y_, &clause_sat_freqency_);
    std::sort(index_arr_.begin(), index_arr_.end(),
              [=](size_t lhs, size_t rhs) {
                return EvalFitness(y_[lhs], clause_sat_freqency_) > EvalFitness(y_[rhs], clause_sat_freqency_);
              });
    printf("Best so far: %d\n",
           CountSatClause(GetCurrentGen()[index_arr_.front()], problem_));
    PrintBitString(GetCurrentGen()[index_arr_.front()]);
    std::cout << "Best Evaluation Fitness: " << EvalFitness(y_[index_arr_.front()], clause_sat_freqency_) << std::endl;
    if (CountSatClause(GetCurrentGen()[index_arr_.front()], problem_) ==
        problem_.cnf.size()) {
      return GetCurrentGen()[index_arr_.front()];
    }

    // Clone
    for (std::size_t i = 0; i < GetCurrentGen().size() * setting_.clone_rate;
         ++i) {
      GetNextGen().push_back(GetCurrentGen()[index_arr_[i]]);
    }

    // Crossover
    for (int i = 0; i < setting_.cross_over_rate * GetCurrentGen().size();
         ++i) {
      auto select_arr =
          SelectCrossoverPair(GetCurrentGen(), fitness_fn,
                              setting_.tournament_size, setting_.tournament_p);
      OnePointCrossover(GetCurrentGen(), &GetNextGen(), select_arr);
    }

    // Mutation
    for (int i = 0; i < setting_.mutation_rate * GetCurrentGen().size(); ++i) {
      int idx = rand() % GetCurrentGen().size();
      GetNextGen().emplace_back();
      Mutate(GetCurrentGen()[idx], &GetNextGen().back(),
             setting_.mutation_flip_count);
    }

    current_gen_ = !current_gen_;
    if (GetCurrentGen().size() > setting_.population_size) {
      for (int i = 0; i < GetCurrentGen().size() - setting_.population_size; ++i) {
        size_t tmp = *(index_arr_.rbegin() + i);
        GetCurrentGen().erase(GetCurrentGen().begin() + tmp);
      }
    }
    GetNextGen().clear();
  }
}

}  // namespace gntsat
