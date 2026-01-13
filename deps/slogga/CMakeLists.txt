cmake_minimum_required(VERSION 3.25)

if(DEFINED slogga_GUARD)
    if(NOT slogga_GUARD STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
        return() # Project has been already included by someone else
    endif()
else()
    set(slogga_GUARD ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "slogga guard")
endif()


project(slogga LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(slogga STATIC src/log.cpp)
target_include_directories(slogga PUBLIC "include/")

