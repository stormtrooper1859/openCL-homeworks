cmake_minimum_required(VERSION 3.8)

project(libutils)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
    message("debug mode")
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

find_package(OpenCL REQUIRED)

set(HEADERS utils.h openCL_utils.h)

set(SOURCES utils.c openCL_utils.c)

set(CMAKE_C_STANDARD 99)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
