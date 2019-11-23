//
// Created by nik on 11/23/19.
//

#include "maze.h"

#ifndef print_maze
#define print_maze(lvl) if(lvl != NULL)\
for (int lvly = 0; lvly < LVL_H; ++lvly){\
for (int lvlx = 0; lvlx < LVL_W; ++lvlx) putc(lvl[lvly * LVL_W + lvlx], stdout);\
putc('\n', stdout);}
#endif

int is_safe(char const* maze, int x, int y) {
	assert(maze != NULL);
	if (x >= 1 && x < LVL_W - 1 && y >= 1 && y < LVL_H - 1 && maze[y * LVL_W + x] == B_FLOOR)
		return 1;
	return 0;
}


int _solve(char* maze, int x, int y, char* sol, int exit_x, int exit_y) {
	assert(maze != NULL);
	assert(sol != NULL);
	if (x == exit_x && y == exit_y) {
		sol[y * LVL_W + x] = SOL_PATH;
		return 1;
	}

	if (is_safe(sol, x, y)) {
		sol[y * LVL_W + x] = SOL_PATH;
		if (_solve(maze, x + 1, y, sol, exit_x, exit_y)) {
			return 1;
		}
		if (_solve(maze, x, y + 1, sol, exit_x, exit_y)) {
			return 1;
		}
		if (_solve(maze, x - 1, y, sol, exit_x, exit_y)) {
			return 1;
		}
		if (_solve(maze, x, y - 1, sol, exit_x, exit_y)) {
			return 1;
		}
		sol[y * LVL_W + x] = B_FLOOR;
		return 0;
	}
	return 0;
}

void solve(char* maze, char** solution, int exit_x, int exit_y) {
	assert(maze != NULL);
	assert(solution != NULL);
	if (*solution != NULL)
		free(*solution);
	*solution = malloc(LVL_W * LVL_H);
	memcpy(*solution, maze, LVL_W * LVL_H);
	_solve(maze, 1, 1, *solution, exit_x, exit_y);
}

void overlay_solution(char* maze, char const* sol) {
	if (maze != NULL && sol != NULL) {
		for (int y = 0; y < LVL_H; ++y) {
			for (int x = 0; x < LVL_W; ++x) {
				if (sol[y * LVL_W + x] == SOL_PATH) {
					maze[y * LVL_W + x] = maze[y * LVL_W + x] == SOL_PATH ? B_FLOOR : SOL_PATH;
				}
			}
		}
	}
}


void carve_maze(char* maze, int width, int height, int x, int y) {
	int x1, y1;
	int x2, y2;
	int dx, dy;
	int dir, count;

	dir = rand() % 4;
	count = 0;
	while (count < 4) {
		dx = 0;
		dy = 0;
		switch (dir) {
			case 0:
				dx = 1;
				break;
			case 1:
				dy = 1;
				break;
			case 2:
				dx = -1;
				break;
			default:
				dy = -1;
				break;
		}
		x1 = x + dx;
		y1 = y + dy;
		x2 = x1 + dx;
		y2 = y1 + dy;
		if (x2 > 0 && x2 < width - 1 &&
			y2 > 0 && y2 < height - 1 &&
			maze[y1 * width + x1] == B_WALL &&
			maze[y2 * width + x2] == B_WALL) {
			maze[y1 * width + x1] = B_FLOOR;
			maze[y2 * width + x2] = B_FLOOR;
			x = x2;
			y = y2;
			dir = rand() % 4;
			count = 0;
		} else {
			dir = (dir + 1) % 4;
			count += 1;
		}
	}

}


char* generate_level(int* exit_x, int* exit_y) {
	int x, y;
	char* lvl = (char*) malloc(LVL_H * LVL_W);
	memset(lvl, B_WALL, LVL_H * LVL_W);
	srand(time(NULL));
	for (y = 1; y < LVL_H; y += 2) {
		for (x = 1; x < LVL_W; x += 2) {
			carve_maze(lvl, LVL_W, LVL_H, x, y);
		}
	}

	lvl[1 * LVL_W + 1] = B_FLOOR;
	*exit_x = (LVL_W - 3);
	*exit_y = (LVL_H - 2);
	lvl[*exit_y * LVL_W + *exit_x] = B_FLOOR;
	return lvl;
}

char* generate_doodads(char const* maze) {
	assert(maze != NULL);
	char* dd = calloc(LVL_H * LVL_W, sizeof(char));
	int i, dx, dy;
	#define BRICK_COUNT 80
	#define GRATE_COUNT 30
	#define OOZE_COUNT 30
	#define SKULL_COUNT 10
	#define PIPE_COUNT 30

	srand(time(NULL));
	for (i = 0; i < BRICK_COUNT; ++i) {
		while (maze[(dy = rand() % LVL_H) * LVL_W + (dx = rand() % LVL_W)] != B_WALL);
		dd[dy * LVL_W + dx] = D_BRICK;
	}
	for (i = 0; i < GRATE_COUNT; ++i) {
		while (maze[(dy = rand() % LVL_H) * LVL_W + (dx = rand() % LVL_W)] != B_WALL);
		dd[dy * LVL_W + dx] = D_GRATE;
	}
	for (i = 0; i < OOZE_COUNT; ++i) {
		while (maze[(dy = rand() % LVL_H) * LVL_W + (dx = rand() % LVL_W)] != B_FLOOR);
		dd[dy * LVL_W + dx] = D_OOZE;
	}
	for (i = 0; i < SKULL_COUNT; ++i) {
		while (maze[(dy = rand() % LVL_H) * LVL_W + (dx = rand() % LVL_W)] != B_FLOOR);
		dd[dy * LVL_W + dx] = D_SKULL;
	}
	for (i = 0; i < PIPE_COUNT; ++i) {
		while (maze[(dy = rand() % LVL_H) * LVL_W + (dx = rand() % LVL_W)] != B_WALL);
		dd[dy * LVL_W + dx] = rand() % 2 ? D_PIPE1 : D_PIPE2;
	}
	return dd;
}

void maze_test_macros() {
	printf("LVL_H    = %3d\n", LVL_H);
	printf("LVL_W    = %3d\n", LVL_W);
	printf("B_WALL   = %3d\n", B_WALL);
	printf("B_FLOOR  = %3d\n", B_FLOOR);
	printf("SOL_PATH = %3d\n", SOL_PATH);
}
