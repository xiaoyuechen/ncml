include(FetchContent)

FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.10.0)

set(INSTALL_GTEST
	OFF
	CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

set_target_properties(gtest gtest_main gmock gmock_main
	PROPERTIES FOLDER "googletest")

target_compile_options(gtest PRIVATE -w)

