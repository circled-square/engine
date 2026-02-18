cmake_minimum_required(VERSION 3.25)
enable_testing()

project(engine VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # use position independent code for all targets since this is a shared library
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
add_compile_definitions(ENGINE_BEING_COMPILED)

add_subdirectory(deps/)
add_subdirectory(src/)
add_subdirectory(tests/ EXCLUDE_FROM_ALL)

