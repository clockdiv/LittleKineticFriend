cmake_minimum_required(VERSION 3.15)

project(little_kinetic_helper C)

# set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Do not build raylib's example programs.
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(raylib)

add_executable(lilkin
    src/main.c
)

target_link_libraries(lilkin PRIVATE raylib)

target_include_directories(lilkin PRIVATE
    raygui/src
)
