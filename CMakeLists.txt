cmake_minimum_required(VERSION 3.8...3.17)

project(qt-rr-tcp LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)

# dependencies
add_subdirectory(deps)

# base library
add_subdirectory(lib)

# examples
option(BUILD_EXAMPLES "Build example applications" OFF)

# On Windows disable console if not needed
if (${BUILD_EXAMPLES})
    message(STATUS "Example applications are enabled")
    add_subdirectory(example-client)
    add_subdirectory(example-server)
else ()
    message(STATUS "Example applications are disabled")
endif ()
