#include "gntsat/parser.h"
#include "gntsat/solver.h"

int main(int argc, const char* argv[]) {
  auto problem = gntsat::readfile(argv[1]);
  auto setting = gntsat::Solver::Setting{};
  setting.population_size = 800;
  setting.clone_rate = 0.1f;
  setting.cross_over_rate = 0.3f;
  setting.mutation_rate = 0.5f;
  auto solver = gntsat::Solver(problem, setting);
  solver.run();

  return 0;
}
