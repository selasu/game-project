cmake_minimum_required(VERSION 3.5)
project(game-project VERSION 1.0.0 LANGUAGES CXX)

add_library(game SHARED
    game.cpp
)

target_compile_definitions(game
    PUBLIC 
        ASSERTIONS_ON
    PRIVATE
        cxx_std_17 
)

add_executable(main
    platform/win32.cpp
    platform/render/ogl.cpp
)

target_compile_definitions(main PRIVATE cxx_std_17)