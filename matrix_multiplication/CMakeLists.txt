cmake_minimum_required(VERSION 3.16)

find_package(OpenMP REQUIRED)
find_package(OpenCL REQUIRED)

add_executable(matrix_multiplication main.c)

target_link_libraries(matrix_multiplication OpenCL::OpenCL OpenMP::OpenMP_C)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/program.cl ${CMAKE_CURRENT_BINARY_DIR}/program.cl COPYONLY)
