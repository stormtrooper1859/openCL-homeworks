cmake_minimum_required(VERSION 3.8)

if(APPLE)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(OpenMP_C "${CMAKE_C_COMPILER}")
        set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -Wno-unused-command-line-argument")
        set(OpenMP_C_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY ${OpenMP_C_LIB_NAMES})
    endif()
endif()

find_package(OpenMP REQUIRED)
find_package(OpenCL REQUIRED)

if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
endif()

add_executable(matrix_multiplication main.c)

target_link_libraries(matrix_multiplication OpenCL::OpenCL)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/program.cl ${CMAKE_CURRENT_BINARY_DIR}/program.cl COPYONLY)
