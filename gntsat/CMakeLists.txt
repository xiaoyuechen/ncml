cmake_minimum_required(VERSION 3.10)

project(gntsat)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(gntsat 
	include/gntsat/problem.h
	src/main.cc src/Parser.cpp src/Parser.h)

target_include_directories(gntsat PRIVATE include)
