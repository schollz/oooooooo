cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 17)

project(utilities)

# Ensure the source file is included
add_library(utilities STATIC Utilities.cpp)

# Make sure the headers are available
target_include_directories(utilities PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(utilities PRIVATE -O3)