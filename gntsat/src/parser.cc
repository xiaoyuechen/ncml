//
// Created by Qinhan Hou on 2021/4/20.
//

#include "gntsat/parser.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <utility>

#include "gntsat/problem.h"

namespace gntsat {

static const std::regex param_reg("p\\s+cnf\\s+(\\d+)\\s+(\\d+)");
static const std::regex clause_reg("(\\-?\\d+) (\\-?\\d+) (\\-?\\d+) .*");

Problem readfile(const char* filename) {
  std::ifstream infile;
  infile.open(filename);
  std::string cnf_data;

  Problem problem;

  while (getline(infile, cnf_data)) {
    std::smatch sm;
    if (cnf_data[0] == 'c') {
      continue;
    } else if (cnf_data[0] == 'p') {
      if (std::regex_search(cnf_data, sm, param_reg)) {
        problem.var_count = std::stoi(sm[1]);
        int clause_count = std::stoi(sm[2]);
        problem.cnf.reserve(clause_count);
      }
    } else if (cnf_data.size() > 0) {
      std::regex_search(cnf_data, sm, clause_reg);
      Problem::Clause clause;
      if (sm.size() == 4) {
        for (int i = 1; i != sm.size(); ++i) {
          clause[i - 1] = std::stoi(sm[i]);
        }
        problem.cnf.push_back(clause);
      }
    }
  }
  return std::move(problem);
}

}  // namespace gntsat
