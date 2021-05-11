#include "gntsat/compute.h"
#include "gntsat/parser.h"

int main(int argc, const char* argv[]) {
  using namespace gntsat;
  srand(time(0));
  auto problem = gntsat::readfile(argv[1]);

  auto population = CreatePopulation(1024, problem.var_count);

  constexpr size_t best_size = 16;
  size_t bests[best_size];
  const int* cnf_begin = &problem.cnf.front()[0];
  const int* cnf_end = &problem.cnf.back()[2] + 1;
  uint64_t* child_buffer =
      (uint64_t*)_mm_malloc((population.num_var + 63) / 64 * 8, 8);

  while (true) {
    TournamentSelect(bests, population, best_size, 64, cnf_begin, cnf_end);
    size_t parentx = rand() % best_size;
    size_t parenty = rand() % best_size;

    CrossoverCC(child_buffer, 0, population.individuals,
                parentx * population.num_var, population.individuals,
                parenty * population.num_var, population.num_var, cnf_begin,
                cnf_end);

    size_t oldest = (population.newest + 1) % population.size;
    if (CountSat(population.individuals,
                 bests[best_size - 1] * population.num_var, cnf_begin,
                 cnf_end) < CountSat(child_buffer, 0, cnf_begin, cnf_end)) {
      for (size_t i = 0; i < population.num_var; ++i) {
        bool bit = ReadBit(child_buffer, 0, i);
        WriteBit(population.individuals, oldest * population.num_var, i, bit);
      }
      population.newest = oldest;
    }
  }

  return 0;
}
