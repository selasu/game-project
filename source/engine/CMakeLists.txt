cmake_minimum_required(VERSION 3.5)
project(engine VERSION 1.0.0 LANGUAGES CXX)

add_library(engine
    src/platform/os/win32.h
    src/platform/os/win32.cpp
    src/platform/render/ogl.cpp
)

target_include_directories(engine
    PUBLIC 
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(engine PRIVATE cxx_std_17)

target_compile_definitions(engine
    PUBLIC ASSERTIONS_ON
)