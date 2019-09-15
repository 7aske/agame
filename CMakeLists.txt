cmake_minimum_required(VERSION 3.14)
project(sdlgame C)
set(CMAKE_C_STANDARD 99)

find_package(SDL2 REQUIRED SDL2)
add_executable(sdlgame main.c)

target_link_libraries(sdlgame m)
target_link_libraries(sdlgame SDL2)
target_link_libraries(sdlgame SDL2_image)
target_link_libraries(sdlgame SDL2_ttf)