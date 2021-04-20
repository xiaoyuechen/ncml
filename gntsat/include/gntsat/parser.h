//
// Created by Qinhan Hou on 2021/4/20.
//

#pragma once
#include "gntsat/problem.h"

namespace gntsat {

    class Parser {
    public:
        Problem readfile(const char* filename);
    };
}


