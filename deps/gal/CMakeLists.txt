cmake_minimum_required(VERSION 3.25)

project(GAL VERSION 0.1.0 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # use position independent code for all targets since this is a shared library
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

add_compile_definitions(GAL_BEING_COMPILED)

option(GAL_AS_SHARED_LIB "mark gal's symbols so they are exported if included in a shared library" OFF)

add_subdirectory(deps/)
add_subdirectory(src/)
