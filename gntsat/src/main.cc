#include "gntsat/parser.h"

int main(int argc, char** argv) {
  auto problem = gntsat::readfile(argv[1]);

  return 0;
}
