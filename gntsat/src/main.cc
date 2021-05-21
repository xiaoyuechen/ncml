#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

#include "gntsat/compute.h"
#include "gntsat/io.h"
#include "gntsat/parser.h"

enum class CrossOverType {
  CC,
  FF,
  UNIFORM,
  ONEPOINT,
  TWOPOINT,
  _COUNT,
};

constexpr size_t kPopulationSize = 128;
constexpr size_t kBestSize = 4;
constexpr size_t kMaxFlip = 100;
constexpr float kWalkChance = 0.57;
constexpr size_t kTournamentSize = 32;

int main(int argc, const char* argv[]) {
  const auto start = std::chrono::steady_clock::now();
  auto crossover_type = CrossOverType::UNIFORM;
  std::string crossover_names[] = {"cc", "ff", "uniform", "onepoint",
                                   "twopoint"};
  for (int i = 1; i < argc - 1; ++i) {
    auto arg = std::string(argv[i]);
    std::string delimiter = "=";
    std::string arg_name = arg.substr(0, arg.find(delimiter));
    std::string arg_val = arg.substr(arg.find(delimiter) + 1);
    if (arg_name == "--crossover") {
      for (size_t j = 0; j < (size_t)CrossOverType::_COUNT; ++j) {
        if (crossover_names[j] == arg_val) {
          crossover_type = (CrossOverType)j;
          printf("Crossover type: %d\n", crossover_type);
          break;
        }
      }
    }
  }

  using namespace gntsat;
  srand(time(0));
  auto problem = gntsat::readfile(argv[argc - 1]);

  auto population = CreatePopulation(kPopulationSize, problem.var_count + 1);

  size_t bests[kBestSize];
  const int* cnf_begin = &problem.cnf.front()[0];
  const int* cnf_end = &problem.cnf.back()[2] + 1;
  uint64_t* child_buffer =
      (uint64_t*)_mm_malloc((population.num_var + 63) / 64 * 8, 8);
  uint64_t* child_buffer_b =
      (uint64_t*)_mm_malloc((population.num_var + 63) / 64 * 8, 8);

  while (true) {
    size_t best_individual_offset;
    size_t most_sat = 0;
    for (size_t i = 0; i < population.size * population.num_var;
         i += population.num_var) {
      size_t sat = CountSat(population.individuals, i, cnf_begin, cnf_end);
      if (sat > most_sat) {
        most_sat = sat;
        best_individual_offset = i;
      }
    }
    PrintBitstring(population.individuals, best_individual_offset,
                   population.num_var);
    printf("\t%d\n", (int)most_sat);
    if (most_sat == problem.cnf.size()) {
      std::cout << "=== 🐂 ===" << std::endl;
      break;
    }

    TournamentSelect(bests, population, kBestSize, kTournamentSize, cnf_begin,
                     cnf_end);
    size_t parentx = bests[rand() % kBestSize];
    size_t parenty = bests[rand() % kBestSize];
    while (parentx == parenty) {
      parenty = bests[rand() % kBestSize];
    }

    if (crossover_type == CrossOverType::CC) {
      CrossoverCC(child_buffer, population.individuals,
                  parentx * population.num_var, population.individuals,
                  parenty * population.num_var, population.num_var, cnf_begin,
                  cnf_end);
    } else if (crossover_type == CrossOverType::FF) {
      CrossoverFF(child_buffer, population.individuals,
                  parentx * population.num_var, population.individuals,
                  parenty * population.num_var, population.num_var, cnf_begin,
                  cnf_end);
    } else if (crossover_type == CrossOverType::UNIFORM) {
      CrossoverUniform(child_buffer, population.individuals,
                       parentx * population.num_var, population.individuals,
                       parenty * population.num_var, population.num_var);
    } else if (crossover_type == CrossOverType::ONEPOINT) {
      CrossoverOnePoint(child_buffer, child_buffer_b, population.individuals,
                        parentx * population.num_var, population.individuals,
                        parenty * population.num_var, population.num_var);
    } else if (crossover_type == CrossOverType::TWOPOINT) {
      CrossoverTwoPoint(child_buffer, child_buffer_b, population.individuals,
                        parentx * population.num_var, population.individuals,
                        parenty * population.num_var, population.num_var);
    }

    uint64_t* children[2] = {child_buffer, child_buffer_b};
    size_t num_child = 1;
    if (crossover_type == CrossOverType::ONEPOINT ||
        crossover_type == CrossOverType::TWOPOINT)
      num_child = 2;
    for (size_t i = 0; i < num_child; ++i) {
      uint64_t* child = children[i];
      WalkMutation(child, 0, kMaxFlip, kWalkChance, cnf_begin, cnf_end);
      size_t oldest = (population.newest + 1) % population.size;
      if (CountSat(population.individuals,
                   bests[kBestSize - 1] * population.num_var, cnf_begin,
                   cnf_end) < CountSat(child, 0, cnf_begin, cnf_end)) {
        for (size_t j = 0; j < population.num_var; ++j) {
          bool bit = ReadBit(child, j);
          WriteBit(population.individuals, oldest * population.num_var + j,
                   bit);
        }
        population.newest = oldest;
      }
    }
  }

  const auto end = std::chrono::steady_clock::now();
  int ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
               .count();
  printf("TIME: %d\n", ms);
  printf("FS: %llu\n", g_num_flips);
  return 0;
}
