cmake_minimum_required(VERSION 3.25)
enable_testing()

project(engine VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)

add_subdirectory(deps/ EXCLUDE_FROM_ALL)
add_subdirectory(tests/)
add_subdirectory(src/)
