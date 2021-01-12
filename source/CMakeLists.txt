cmake_minimum_required(VERSION 3.5)
project(game-project VERSION 1.0.0 LANGUAGES CXX)

add_library(sel_opengl SHARED
    render/win32_ogl.cpp
    render/render_ogl.cpp
)

target_compile_definitions(sel_opengl
    PUBLIC 
        ASSERTIONS_ON
    PRIVATE
        cxx_std_17 
)

add_library(sel_game SHARED
    game.cpp
    asset/wav.cpp
)

target_compile_definitions(sel_game
    PUBLIC 
        ASSERTIONS_ON
    PRIVATE
        cxx_std_17 
)

add_executable(main
    platform/win32.cpp
)

target_compile_definitions(main 
    PUBLIC
        ASSERTIONS_ON
    PRIVATE 
        cxx_std_17
)