cmake_minimum_required(VERSION 3.5)
project(main)

add_executable(main
    resources/icon/resource.rc
    main.cpp
)

target_link_libraries(main PRIVATE engine)