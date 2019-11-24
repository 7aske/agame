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

typedef struct maze {
	char* maze;
	int w;
	int h;
	int b_wall;
	int exit_x;
	int exit_y;
} maze_t;

static char* solution = NULL;

static int is_safe(char const* maze, int x, int y);

static int _solve(char* maze, int x, int y, char* sol, int exit_x, int exit_y);

static void solve(char* maze, int exit_x, int exit_y);

extern void overlay_solution(char* maze, int exit_x, int exit_y);

extern void carve_maze(char* maze, int width, int height, int x, int y);

extern maze_t generate_maze();

extern char* generate_doodads(char const* maze);

extern void maze_test_macros();

#endif //SDLGAME_MAZE_H
