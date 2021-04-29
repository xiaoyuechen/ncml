#include "gntsat/parser.h"
#include "gntsat/solver.h"

int main(int argc, const char* argv[]) {
  auto problem = gntsat::readfile(argv[1]);
  auto setting = gntsat::Solver::Setting{};
  setting.population_size = 8;
  setting.clone_rate = 0.02f;
  setting.cross_over_rate = 0.2f;
  setting.mutation_rate = 0.6f;
  setting.mutation_flip_count = 25;
  setting.tournament_p = 0.9f;
  setting.tournament_size = 4;
  auto solver = gntsat::Solver(problem, setting);
  solver.run();

  return 0;
}
