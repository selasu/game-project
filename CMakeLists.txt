cmake_minimum_required(VERSION 3.5)
project(game-project VERSION 1.0.0 LANGUAGES CXX)

add_library(sel_opengl SHARED
    source/macro_util.cpp
    source/render/win32_ogl.cpp
    source/render/render_ogl.cpp
)

target_compile_definitions(sel_opengl
    PUBLIC 
        ASSERTIONS_ON
    PRIVATE
        cxx_std_17 
)

add_library(sel_game SHARED
    source/game.cpp
    source/wav.cpp
    source/png.cpp
    source/stream.cpp
    source/macro_util.cpp
    source/render/render.cpp
)

target_compile_definitions(sel_game
    PUBLIC 
        ASSERTIONS_ON
    PRIVATE
        cxx_std_17 
)

add_executable(main
    source/macro_util.cpp
    source/platform/win32.cpp
)

target_compile_definitions(main 
    PUBLIC
        ASSERTIONS_ON
    PRIVATE 
        cxx_std_17
)