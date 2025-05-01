cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)

project(oooooooo)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED) # Add this line to find SDL2_ttf

set(SRC src/main.cpp
        src/AudioFile.cpp
        src/SoftcutClient.cpp
        src/Commands.cpp
        src/OscInterface.cpp
        src/Perlin.cpp
        src/LFO.cpp
        src/Parameter.cpp
        src/Parameters.cpp
        src/Display.cpp
        src/DisplayRing.cpp
        src/DrawFunctions.cpp
        src/BufDiskWorker.cpp
        src/Window.cpp)

add_executable(oooooooo ${SRC})

target_include_directories(oooooooo PUBLIC
        ../../softcut-lib/include
        ../../
        third-party
)

if (${APPLE})
    message("is apple")
    # need to support macports or homebrew
    target_link_directories(oooooooo PUBLIC /usr/local/lib)
    target_link_directories(oooooooo PUBLIC /opt/local/lib)
    target_include_directories(oooooooo PUBLIC /usr/local/include)
    target_include_directories(oooooooo PUBLIC /opt/local/include)
endif()

target_compile_options(oooooooo PRIVATE -Wall -Wextra -pedantic -O3)

# Link against the display library and SDL2_ttf
target_link_libraries(oooooooo fverb utilities softcut jack lo pthread sndfile ${SDL2_LIBRARIES} SDL2_ttf::SDL2_ttf)