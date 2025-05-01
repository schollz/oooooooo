cmake_minimum_required(VERSION 3.17)
project(display)

set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(SDL2 REQUIRED)

# Create display library
add_library(display
    Display.cpp
    Display.h
)

# Include directories
target_include_directories(display PUBLIC
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries
target_link_libraries(display PUBLIC
    ${SDL2_LIBRARIES}
    lo
)

# Make this library available to other projects
set_target_properties(display PROPERTIES
    PUBLIC_HEADER "Display.h"
)