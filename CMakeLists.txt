cmake_minimum_required(VERSION 3.25)

project(engine VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 23)

add_subdirectory(deps/)
add_subdirectory(src/)
