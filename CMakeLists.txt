cmake_minimum_required(VERSION 3.0)
project(shame)

add_compile_options(-Wall -Wextra -O3 -g -fPIC -std=c++17)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)

add_subdirectory(shame)
add_subdirectory(examples)
