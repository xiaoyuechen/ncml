//
// Created by Qinhan Hou on 2021/4/20.
//

#pragma once
#include <fstream>
#include <iostream>
#include "Parser.h"
#include "gntsat/problem.h"
#include <regex>


namespace gntsat {
     void Parser::readfile(const char* filename) {
        std::ifstream infile;
        infile.open(filename);
        std::string cnf_data;

        std::regex rex[] = {
                std::regex("p cnf (\d+) (\d+)"),
                std::regex("(\d+) (\d+) (\d+) (\d+)")
        };

        Problem res_pro;

        while (getline(infile, cnf_data)) {
            std::smatch sm;
            if (cnf_data[0] == 'c') {
                continue;
            } else if (cnf_data[0] == 'p') {
                if (regex_search(cnf_data, sm, rex[0])) {
                    int var_num = std::stoi(sm[0]);
                    res_pro.var_count = var_num;
                    std::cout << "var number is: " << var_num << std::endl;
                }

            } else {
                bool if_data = regex_search(cnf_data, sm, rex[1]);
            }

        }
    }
}
