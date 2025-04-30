cmake_minimum_required(VERSION 3.17)
project(softcut)
set(CMAKE_CXX_STANDARD 14)

set(SRC
  src/Voice.cpp
  src/ReadWriteHead.cpp
  src/SubHead.cpp
  src/FadeCurves.cpp
  src/Svf.cpp)

include_directories(include src)

# tapefx
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../dsp)

if(APPLE)
  include_directories(/usr/local/include)
endif()

add_library(softcut STATIC ${SRC})

target_link_libraries(softcut tapefx)

target_compile_options(softcut PRIVATE -O3)