cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)

project(softcut_jack_osc)

# Add the display subdirectory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../display ${CMAKE_CURRENT_BINARY_DIR}/display)

set(SRC src/main.cpp
        src/SoftcutClient.cpp
        src/Commands.cpp
        src/OscInterface.cpp
        src/BufDiskWorker.cpp
        src/Window.cpp)

add_executable(softcut_jack_osc ${SRC})

target_include_directories(softcut_jack_osc PUBLIC
        ../../softcut-lib/include
        ../../
        third-party
)

if (${APPLE})
    message("is apple")
    # need to support macports or homebrew
    target_link_directories(softcut_jack_osc PUBLIC /usr/local/lib)
    target_link_directories(softcut_jack_osc PUBLIC /opt/local/lib)
    target_include_directories(softcut_jack_osc PUBLIC /usr/local/include)
    target_include_directories(softcut_jack_osc PUBLIC /opt/local/include)
endif()

target_compile_options(softcut_jack_osc PRIVATE -Wall -Wextra -pedantic -O3)

# Link against the display library
target_link_libraries(softcut_jack_osc fverb utilities softcut jack lo pthread sndfile display)