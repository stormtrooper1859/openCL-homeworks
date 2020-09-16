cmake_minimum_required(VERSION 3.8)

project(libutils)

set(HEADERS utils.h)

set(SOURCES utils.c)

set(CMAKE_C_STANDARD 99)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
