cmake_minimum_required(VERSION 3.14)

option(GNTSAT_BUILD_TESTS "Build all of GNTSAT's own tests." OFF)

project(gntsat)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(gntsat-lib
	include/gntsat/problem.h
	include/gntsat/parser.h
	include/gntsat/solver.h
	include/gntsat/compute.h
	include/gntsat/io.h
	src/parser.cc
	src/solver.cc
	src/io.cc
	)

target_include_directories(gntsat-lib
	PUBLIC include
	PRIVATE src)


add_executable(gntsat src/main.cc)
target_link_libraries(gntsat PRIVATE gntsat-lib)


add_executable(walksat src/walksat.cc)
target_link_libraries(walksat PRIVATE gntsat-lib)

add_subdirectory(extern)
add_subdirectory(tests)
