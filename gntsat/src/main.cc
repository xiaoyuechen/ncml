#include <algorithm>
#include <iostream>
#include "gntsat/compute.h"
#include "gntsat/io.h"
#include "gntsat/parser.h"

int main(int argc, const char* argv[]) {
  using namespace gntsat;
  srand(time(0));
  auto problem = gntsat::readfile(argv[1]);

  auto population = CreatePopulation(16,
                                     problem.var_count + 1);

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

//        for (size_t i = 0; i < population.size * population.num_var;
//             i += population.num_var) {
//          PrintBitstring(population.individuals, i, population.num_var);
//          if (i / population.num_var == population.newest) printf("<<======");
//          printf("\n");
//        }

    TournamentSelect(bests, population, best_size, 8, cnf_begin, cnf_end);
    size_t parentx = rand() % best_size;
    size_t parenty = rand() % best_size;
    while (parentx == parenty) {
      parenty = rand() % best_size;
    }
    PrintBitstring(population.individuals, parentx * population.num_var, population.num_var);
    std::cout << std::endl;
    PrintBitstring(population.individuals, parenty * population.num_var, population.num_var);
    std::cout << std::endl;
    CrossoverCC(child_buffer, 0, population.individuals,
                parentx * population.num_var, population.individuals,
                parenty * population.num_var, population.num_var, cnf_begin,
                cnf_end);
    std::cout << "Before Mutation" << std::endl;
    PrintBitstring(child_buffer, 0, population.num_var);
    std::cout << std::endl;
    WalkMutation(child_buffer, 0, 8, 0.4, cnf_begin, cnf_end);
    std::cout << "After Mutation" << std::endl;
    PrintBitstring(child_buffer, 0, population.num_var);
    std::cout << std::endl;
    std::cout << "👀" << std::endl;
    size_t oldest = (population.newest + 1) % population.size;
    if (CountSat(population.individuals,
                 bests[best_size - 1] * population.num_var, cnf_begin,
                 cnf_end) < CountSat(child_buffer, 0, cnf_begin, cnf_end)) {
      for (size_t i = 0; i < population.num_var; ++i) {
        bool bit = ReadBit(child_buffer, i);
        WriteBit(population.individuals, oldest * population.num_var, bit);
      }
      population.newest = oldest;
    }
  }

  return 0;
}
