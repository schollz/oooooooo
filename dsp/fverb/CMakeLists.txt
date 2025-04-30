cmake_minimum_required(VERSION 3.7)
set(CMAKE_CXX_STANDARD 17)

project(fverb)


# Add your own library
add_library(fverb STATIC FVerb.cpp)

# Include Faust headers and your own headers
target_include_directories(fverb PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/faust/architecture
)

# target_link_libraries(fverb PUBLIC faust)
