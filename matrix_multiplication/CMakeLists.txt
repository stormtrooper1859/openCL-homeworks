cmake_minimum_required(VERSION 3.8)

find_package(OpenMP REQUIRED)
find_package(OpenCL REQUIRED)

if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

set(HEADERS classic_multiplication.h openCL_multiplication.h)
set(SOURCES main.c classic_multiplication.c openCL_multiplication.c)

add_executable(matrix_multiplication ${SOURCES} ${HEADERS})

target_link_libraries(matrix_multiplication OpenCL::OpenCL libutils)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/matrix_multiplication.cl ${CMAKE_CURRENT_BINARY_DIR}/matrix_multiplication.cl COPYONLY)
