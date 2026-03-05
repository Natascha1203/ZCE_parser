cmake_minimum_required(VERSION 3.5)
project(cppParser CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -O2")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(cppParser
    src/main.cpp
    src/parser.cpp
    src/t31_parser.cpp
    src/t36_parser.cpp
)