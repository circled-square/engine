cmake_minimum_required(VERSION 3.25)

project(GAL VERSION 0.1.0 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(deps/)
add_subdirectory(src/)
