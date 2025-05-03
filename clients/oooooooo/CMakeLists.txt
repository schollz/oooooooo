cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 17)
project(oooooooo)

# Find SDL2 - handle Windows properly
if(WIN32)
  # On Windows, find SDL2 components manually to avoid SDL2main
  find_path(SDL2_INCLUDE_DIR 
    NAMES SDL.h
    PATH_SUFFIXES SDL2
    PATHS 
      /mingw64/include
      $ENV{MINGW_PREFIX}/include
  )
  
  find_library(SDL2_LIBRARY
    NAMES SDL2
    PATHS 
      /mingw64/lib
      $ENV{MINGW_PREFIX}/lib
  )
  
  find_library(SDL2_TTF_LIBRARY
    NAMES SDL2_ttf
    PATHS 
      /mingw64/lib
      $ENV{MINGW_PREFIX}/lib
  )
  
  if(SDL2_INCLUDE_DIR AND SDL2_LIBRARY AND SDL2_TTF_LIBRARY)
    set(SDL2_FOUND TRUE)
    set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
    set(SDL2_LIBRARIES ${SDL2_LIBRARY})
    set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY})
  else()
    message(FATAL_ERROR "SDL2 not found")
  endif()
else()
  find_package(SDL2 REQUIRED)
  find_package(SDL2_ttf REQUIRED)
endif()

# Find JACK library - handle Windows naming difference
if(WIN32)
  # On Windows, JACK library is typically named jack64
  find_library(JACK_LIBRARY 
    NAMES jack64 jack
    PATHS 
      /mingw64/lib
      $ENV{MINGW_PREFIX}/lib
    NO_DEFAULT_PATH
  )
  find_path(JACK_INCLUDE_DIR 
    NAMES jack/jack.h
    PATHS 
      /mingw64/include
      $ENV{MINGW_PREFIX}/include
    NO_DEFAULT_PATH
  )
  
  if(JACK_LIBRARY AND JACK_INCLUDE_DIR)
    set(JACK_FOUND TRUE)
    set(JACK_LIBRARIES ${JACK_LIBRARY})
    set(JACK_INCLUDE_DIRS ${JACK_INCLUDE_DIR})
    message(STATUS "Found JACK: ${JACK_LIBRARY}")
  else()
    message(FATAL_ERROR "JACK library not found")
  endif()
else()
  # On Linux/macOS, use pkg-config
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(JACK REQUIRED jack)
endif()

# Find other dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(LO REQUIRED liblo)
pkg_check_modules(SNDFILE REQUIRED sndfile)

set(SRC 
  src/main.cpp
  src/AudioFile.cpp
  src/SoftcutClient.cpp
  src/Commands.cpp
  src/OscInterface.cpp
  src/HelpSystem.cpp
  src/KeyboardHandler.cpp
  src/Perlin.cpp
  src/LFO.cpp
  src/Parameter.cpp
  src/Parameters.cpp
  src/Display.cpp
  src/DisplayRing.cpp
  src/DrawFunctions.cpp
  src/BufDiskWorker.cpp
  src/Window.cpp
)

add_executable(oooooooo ${SRC})

# CRITICAL: Tell SDL2 we handle our own main function
if(WIN32)
  target_compile_definitions(oooooooo PRIVATE SDL_MAIN_HANDLED)
endif()

target_include_directories(oooooooo PUBLIC
  ../../softcut-lib/include
  ../../
  third-party
)

if(WIN32)
  # Windows-specific includes/libraries
  target_include_directories(oooooooo PRIVATE ${JACK_INCLUDE_DIRS})
  target_include_directories(oooooooo PRIVATE ${LO_INCLUDE_DIRS})
  target_include_directories(oooooooo PRIVATE ${SNDFILE_INCLUDE_DIRS})
  target_include_directories(oooooooo PRIVATE ${SDL2_INCLUDE_DIRS})
elseif(APPLE)
  message("is apple")
  # need to support macports or homebrew
  target_link_directories(oooooooo PUBLIC /usr/local/lib)
  target_link_directories(oooooooo PUBLIC /opt/local/lib)
  target_include_directories(oooooooo PUBLIC /usr/local/include)
  target_include_directories(oooooooo PUBLIC /opt/local/include)
endif()

target_compile_options(oooooooo PRIVATE -Wall -Wextra -pedantic -O3)

# Link against the libraries
if(WIN32)
  target_link_libraries(oooooooo 
    fverb 
    utilities 
    softcut 
    ${JACK_LIBRARIES}
    ${LO_LIBRARIES}
    ${SNDFILE_LIBRARIES}
    pthread
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
  )
else()
  target_link_libraries(oooooooo 
    fverb 
    utilities 
    softcut 
    jack 
    lo 
    pthread 
    sndfile 
    ${SDL2_LIBRARIES} 
    SDL2_ttf::SDL2_ttf
  )
endif()