cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 17)

project(tapefx)

# Ensure the source file is included
add_library(tapefx STATIC Follower.cpp TapeFX.cpp)

# Make sure the headers are available
target_include_directories(tapefx PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(tapefx PRIVATE -O3)