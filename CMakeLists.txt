cmake_minimum_required(VERSION 3.8)
project(openCL_homeworks C)

set(CMAKE_C_STANDARD 99)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
    message("debug mode")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

add_subdirectory (matrix_multiplication)
