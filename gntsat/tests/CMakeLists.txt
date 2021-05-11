add_executable(
	gntsat-tests
	solver_tests.cc
	compute_tests.cc
	)

target_include_directories(gntsat-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(gntsat-tests PRIVATE gntsat-lib gtest gtest_main gmock gmock_main)

include(GoogleTest)
gtest_discover_tests(gntsat-tests)
