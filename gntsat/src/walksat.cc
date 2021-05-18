#include <chrono>

#include "gntsat/compute.h"
#include "gntsat/io.h"
#include "gntsat/parser.h"

void RandomInit(uint64_t* out_solution, size_t num_var) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint64_t> distribution(
      std::numeric_limits<uint64_t>::min(),
      std::numeric_limits<uint64_t>::max());

  size_t word_count = (num_var + 63) / 64;
  for (int i = 0; i < word_count; ++i) {
    out_solution[i] = distribution(gen);
  }
}

int main(int argc, const char* argv[]) {
  const auto start = std::chrono::steady_clock::now();
  using namespace gntsat;
  srand(time(0));
  auto problem = gntsat::readfile(argv[1]);

  size_t word_count = (problem.var_count + 1 + 63) / 64;
  size_t byte_count = word_count * 8;
  uint64_t* solution = (uint64_t*)_mm_malloc(byte_count, 8);
  const int* cnf_begin = &problem.cnf.front()[0];
  const int* cnf_end = &problem.cnf.back()[2] + 1;

  while (true) {
    RandomInit(solution, problem.var_count + 1);
    WalkMutation(solution, 0, 1000, 0.57, cnf_begin, cnf_end);
    size_t sat = CountSat(solution, 0, cnf_begin, cnf_end);
    if (sat == problem.cnf.size()) goto SAT;
    PrintBitstring(solution, 0, problem.var_count + 1);
    printf("\t%d\n", (int)sat);
    printf("RESART---------------------------\n");
  }

SAT:
  printf("!!!!!!!!!!!!!!!!!!!!\n");
  const auto end = std::chrono::steady_clock::now();
  int ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
               .count();
  printf("TIME: %d\n", ms);
  printf("FS: %llu\n", g_num_flips);
}
