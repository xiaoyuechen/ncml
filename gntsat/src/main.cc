#include <cstdio>
#include "Parser.h"

int main(int argc, char** argv) {

    std::printf(argv[1]);
    gntsat::Parser parser;
    parser.readfile(argv[1]);

    return 0;
}
