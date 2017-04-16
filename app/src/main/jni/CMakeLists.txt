cmake_minimum_required(VERSION 3.4.1)

project(fluid)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=format-security")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fexceptions -Wall")

add_library(
    particles
    SHARED
    gl_code.cpp
    fluid.cpp
    engine/compute.cpp
    engine/geometry.cpp
    engine/shader.cpp
    engine/texture.cpp
    utils/file.c
    utils/shader.c
    utils/log.c
)

target_link_libraries(particles log android GLESv2)


