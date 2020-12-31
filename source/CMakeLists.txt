cmake_minimum_required(VERSION 3.5)
project(game-project VERSION 1.0.0 LANGUAGES CXX)

add_executable(main
    platform/os/win32.cpp
    platform/render/ogl.cpp
    platform/render/ogl.h
    platform/event.h
    platform/os.h

    util/assert.h
)

target_compile_definitions(main PRIVATE cxx_std_17)

target_compile_definitions(main
    PUBLIC ASSERTIONS_ON
)