cmake_minimum_required(VERSION 2.8.12)
project(OpenTimelineIO)

# project wide settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wno-deprecated-register")

add_subdirectory(src)
add_subdirectory(python)
add_subdirectory(contrib)
