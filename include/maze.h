//
// Created by nik on 11/23/19.
//

#ifndef SDLGAME_MAZE_H
#define SDLGAME_MAZE_H

#pragma once

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "macro_definitions.h"

static int is_safe(char const* maze, int x, int y);

static int _solve(char* maze, int x, int y, char* sol, int exit_x, int exit_y);

extern void solve(char* maze, char** solution, int exit_x, int exit_y);

extern void overlay_solution(char* maze, char const* sol);

extern void carve_maze(char* maze, int width, int height, int x, int y);

extern char* generate_level(int* exit_x, int* exit_y);

extern char* generate_doodads(char const* maze);

extern void maze_test_macros();

#endif //SDLGAME_MAZE_H
