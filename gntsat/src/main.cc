#include "gntsat/parser.h"
#include "gntsat/solver.h"

int main(int argc, char** argv) {
  auto problem = gntsat::readfile(argv[1]);
  auto solver = gntsat::Solver(problem);

  return 0;
}
