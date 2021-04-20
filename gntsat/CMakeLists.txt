cmake_minimum_required(VERSION 3.10)

project(gntsat)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(gntsat src/main.cc)

target_include_directories(gntsat PRIVATE include)
