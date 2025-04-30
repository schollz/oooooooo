cmake_minimum_required(VERSION 3.7)

add_subdirectory(dsp)
project(softcut)
add_subdirectory(softcut-lib)
add_subdirectory(clients/softcut_jack_osc)
