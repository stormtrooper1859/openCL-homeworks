cmake_minimum_required(VERSION 3.8)

find_package(OpenMP REQUIRED)
find_package(OpenCL REQUIRED)

if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

set(HEADERS classic_prefix_sum.h openCL_prefix_sum.h)
set(SOURCES main.c classic_prefix_sum.c openCL_prefix_sum.c)

add_executable(prefix_sum ${SOURCES} ${HEADERS})

target_link_libraries(prefix_sum OpenCL::OpenCL libutils)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/prefix_sum.cl ${CMAKE_CURRENT_BINARY_DIR}/prefix_sum.cl COPYONLY)
