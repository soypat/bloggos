# This file defines the build for the tests in this project.
# using the proposed test framework.
cmake_minimum_required(VERSION 3.6) # 3.6 required for regex filtering.

project(framework_example C)

# C99 is best C. Add coverage support.
add_definitions(-std=c99 --coverage)

# Output the executables and objects to the current directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

include_directories(PUBLIC .)

# test_sources will contain all build source code.
file(GLOB_RECURSE test_sources "*.c")
# Filter out any unwanted source files that are not part of your project
list(FILTER test_sources EXCLUDE REGEX "CMakeFiles")
# We exclude the examples directory which contains standalone
# executables that are not part of the test suite.
list(FILTER test_sources EXCLUDE REGEX "examples")

# Build the test executable.
add_executable(tests.bin ${test_sources})

# Link the gcov library for coverage instrumentation support.
# Required to get test coverage.
target_link_libraries(tests.bin gcov)