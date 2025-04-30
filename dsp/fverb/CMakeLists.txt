# ./dsp/reverb/CMakeLists.txt
cmake_minimum_required(VERSION 3.7)

project(reverb)

set(SRC
    FVerb.cpp
)

# Create a library for the Reverb module
add_library(reverb ${SRC})

target_include_directories(reverb PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)