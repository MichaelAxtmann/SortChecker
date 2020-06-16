cmake_minimum_required(VERSION 3.10)

project(SortChecker)

include_directories(extern/highwayhash)

add_library(checker STATIC src/hash.cpp)
target_include_directories(checker INTERFACE ./include/)
