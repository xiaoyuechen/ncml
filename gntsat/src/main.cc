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

int main(int argc, const char* argv[]) {
  const auto start = std::chrono::steady_clock::now();
  auto crossover_type = CrossOverType::UNIFORM;
  // --crossover=cc
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

  auto population = CreatePopulation(1000, problem.var_count + 1);

  constexpr size_t best_size = 4;
  size_t bests[best_size];
  const int* cnf_begin = &problem.cnf.front()[0];
  const int* cnf_end = &problem.cnf.back()[2] + 1;
  uint64_t* child_buffer =
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
    printf("Best %d\n", (int)most_sat);
    PrintBitstring(population.individuals, best_individual_offset,
                   population.num_var);
    printf("\n");
    if (most_sat == problem.cnf.size()) {
      std::cout << "=== 🐂 ===" << std::endl;
      break;
    }

    TournamentSelect(bests, population, best_size, 64, cnf_begin, cnf_end);
    size_t parentx = bests[rand() % best_size];
    size_t parenty = bests[rand() % best_size];
    while (parentx == parenty) {
      parenty = bests[rand() % best_size];
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
    }

    WalkMutation(child_buffer, 0, 1000, 0.57, cnf_begin, cnf_end);
    size_t oldest = (population.newest + 1) % population.size;
    if (CountSat(population.individuals,
                 bests[best_size - 1] * population.num_var, cnf_begin,
                 cnf_end) < CountSat(child_buffer, 0, cnf_begin, cnf_end)) {
      for (size_t i = 0; i < population.num_var; ++i) {
        bool bit = ReadBit(child_buffer, i);
        WriteBit(population.individuals, oldest * population.num_var + i, bit);
      }
      population.newest = oldest;
    }
  }

  const auto end = std::chrono::steady_clock::now();
  int ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
               .count();
  printf("TIME: %d\n", ms);
  printf("FS: %llu\n", g_num_flips);
  return 0;
}
