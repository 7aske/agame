cmake_minimum_required(VERSION 3.14)
project(agame C)
set(CMAKE_C_STANDARD 99)
set(TARGET agame)

include_directories(include internal)
add_subdirectory(src)
add_executable(${TARGET} main.c src/maze.c src/util.c internal/cleanup_functions.h internal/draw_misc.h include/entity.h)

find_package(SDL2 REQUIRED SDL2)

target_link_libraries(${TARGET} m)
target_link_libraries(${TARGET} SDL2)
target_link_libraries(${TARGET} SDL2_image)
target_link_libraries(${TARGET} SDL2_ttf)