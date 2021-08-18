cmake_minimum_required(VERSION 3.8...3.17)

project(qt-rr-tcp LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)

# dependencies
add_subdirectory(deps)

# base library
add_subdirectory(lib)

# examples
add_subdirectory(example-server)
