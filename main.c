#define SDL_MAIN_HANDLED 1

#include <stdio.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <signal.h>
#include <time.h>
#include <assert.h>

#include "sprites.h"

// screen size
#define WIDTH (1024)
#define HEIGHT (768)

// apparent block size on the screen
#define BSIZE (32)

// blocks count in the viewport
#define SCR_W (WIDTH / BSIZE)
#define SCR_H (HEIGHT / BSIZE)

#define WRLD_W (3)
#define WRLD_H (2)

#define LVL_W (WRLD_W * SCR_W)
#define LVL_H (WRLD_H * SCR_H)

#define SOL_PATH 38
#define B_WALL 35
#define B_FLOOR 32

#define D_SKULL 1
#define D_PIPE1 2
#define D_PIPE2 3
#define D_OOZE 4
#define D_GRATE 5
#define D_BRICK 6

#define printd(x) printf("%d\n")
#define printt(x, y) printf("(%d, %d)\n", x, y)
#define printlvl(lvl) if(lvl != NULL)\
for (int lvly = 0; lvly < LVL_H; ++lvly){\
for (int lvlx = 0; lvlx < LVL_W; ++lvlx) putc(lvl[lvly * LVL_W + lvlx], stdout);\
putc('\n', stdout);}\

typedef struct player {
	int x;
	int y;
} player_t;

void sig_handler(int);

void sdlerr_handler();

void quit();

void event_handler(SDL_Event*, player_t*);

void player_move(player_t*, SDL_Scancode);

void game_loop(player_t*);

void render_loop(player_t*, SDL_Texture*);

char* generate_level();

char* generate_doodads();

void restart_level(player_t*);

float dist_to(int, int, int, int);

void draw_fps(int const*);

void draw_help();

void carve_maze(char*, int, int, int, int);

int bresenham(int, int, int, int, char*);

int is_safe(char const*, int, int);

int _solve(char*, int, int, char*);

void solve(char*);

void overlay_solution(char*, char const*);

char* level = NULL;
char* doodads = NULL;
char* solution = NULL;
int exit_x = -1;
int exit_y = -1;

volatile int running = 1;
volatile int l_sw = 1;

//The window we'll be rendering to
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t render_flags = SDL_RENDERER_ACCELERATED;
TTF_Font* font;

int main(int argc, char** argv) {

	signal(SIGINT, sig_handler);

	//The surface contained by the window
	SDL_Surface* surface = NULL;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		sdlerr_handler();

	TTF_Init();
	//Create window
	window = SDL_CreateWindow("A Game",
							  SDL_WINDOWPOS_CENTERED,
							  SDL_WINDOWPOS_CENTERED,
							  WIDTH,
							  HEIGHT,
							  SDL_WINDOW_SHOWN);
	if (!window)
		sdlerr_handler();

	renderer = SDL_CreateRenderer(window, -1, render_flags);

	if (!renderer)
		sdlerr_handler();

	surface = IMG_Load("res/0x72_16x16DungeonTileset.v1.png");
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	if (!tex)
		sdlerr_handler();

	font = TTF_OpenFont("res/DejaVuSans-Bold.ttf", 24);

	if (!font)
		fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());

	level = generate_level();
	doodads = generate_doodads();


	player_t player = {1, 1};

	// setup code
	Uint32 startclock = 0;
	Uint32 deltaclock = 0;
	Uint32 currentFPS = 0;
	startclock = SDL_GetTicks();

	while (running) {
		SDL_RenderClear(renderer);

		game_loop(&player);
		render_loop(&player, tex);

		deltaclock = SDL_GetTicks() - startclock;
		startclock = SDL_GetTicks();

		if (deltaclock != 0) {
			currentFPS = 1000 / deltaclock;
			draw_fps((int const*) &currentFPS);
		}
		draw_help();
		SDL_RenderPresent(renderer);
		SDL_UpdateWindowSurface(window);

		SDL_Delay(1000 / 60);
	}
	TTF_Quit();
	return 0;
}

void sig_handler(int sig) {
	if (sig == SIGINT) {
		running = 0;
	}
}

void sdlerr_handler() {
	fprintf(stderr, "SDL_Error: %s\n", SDL_GetError());
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	exit(1);
}

void quit() {
	running = 0;
}

void event_handler(SDL_Event* ev, player_t* player) {
	switch (ev->type) {
		case SDL_QUIT:
			quit();
			break;
		case SDL_KEYDOWN:
			switch (ev->key.keysym.scancode) {
				case SDL_SCANCODE_W:
				case SDL_SCANCODE_UP:
				case SDL_SCANCODE_A:
				case SDL_SCANCODE_LEFT:
				case SDL_SCANCODE_S:
				case SDL_SCANCODE_DOWN:
				case SDL_SCANCODE_D:
				case SDL_SCANCODE_RIGHT:
					player_move(player, ev->key.keysym.scancode);
					break;
				case SDL_SCANCODE_R:
					restart_level(player);
					break;
				case SDL_SCANCODE_O:
					solve(level);
					overlay_solution(level, solution);
					break;
				case SDL_SCANCODE_L:
					l_sw = !l_sw;
					break;
				case SDL_SCANCODE_Q:
					quit();
					break;
			}
			break;
	}
}

void player_move(player_t* player, SDL_Scancode code) {
	switch (code) {
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_UP:
			if (level[((player->y - 1) * LVL_W) + player->x] != '#') {
				player->y -= 1;
			}
			break;
		case SDL_SCANCODE_A:
		case SDL_SCANCODE_LEFT:
			if (level[(player->y * LVL_W) + player->x - 1] != '#') {
				player->x -= 1;
			}
			break;
		case SDL_SCANCODE_S:
		case SDL_SCANCODE_DOWN:
			if (level[((player->y + 1) * LVL_W) + player->x] != '#') {
				player->y += 1;
			}
			break;
		case SDL_SCANCODE_D:
		case SDL_SCANCODE_RIGHT:
			if (level[(player->y * LVL_W) + player->x + 1] != '#') {
				player->x += 1;
			}
			break;
		default:
			break;
	}


}

void game_loop(player_t* player) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		event_handler(&event, player);
	}

}

void render_loop(player_t* player, SDL_Texture* tex) {
	assert(doodads != NULL);
	assert(level != NULL);
	float light_amp, light, a, dist;
	int xoff = (int) (player->x / SCR_W) * SCR_W;
	int yoff = (int) (player->y / SCR_H) * SCR_H;
	SDL_Rect dest;
	SDL_Rect src;
	for (int y = 0; y < SCR_H; ++y) {
		for (int x = 0; x < SCR_W; ++x) {
			dest.h = BSIZE;
			dest.w = BSIZE;
			dest.x = BSIZE * x;
			dest.y = BSIZE * y;

			light_amp = 3;
			a = 1 - (10 - (rand() % 25)) / 100.0f;
			dist = dist_to(player->x, player->y, x + xoff, y + yoff);
			if (bresenham(player->x, player->y, x + xoff, y + yoff, level)) {
				// calculate lighting depending on distance to the player (only light source)
				light = 255.0f / (dist / light_amp) * a;
				light = light > 255 ? 255 : light;
			} else {
				light = 196.0f / dist * a;
				light = light > 196 ? 196 : light;
			}

			if (!l_sw) {
				light = 255;
			}
			SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);

			switch (level[((y + yoff) * LVL_W) + x + xoff]) {
				case B_WALL:
					query_sprite(SPR_WALL, &src);
					break;
				case B_FLOOR:
					if (level[((y + yoff - 1) * LVL_W) + x + xoff] == B_WALL) {
						query_sprite(SPR_TFLOOR, &src);
					} else {
						query_sprite(SPR_FLOOR, &src);
					}
					if (level[((y + yoff + 1) * LVL_W) + x + xoff] == B_WALL) {
						SDL_RenderCopy(renderer, tex, &src, &dest);
						query_sprite(SPR_TWALL, &src);
					}
					break;
				case SOL_PATH:
					if (level[((y + yoff - 1) * LVL_W) + x + xoff] == B_WALL) {
						query_sprite(SPR_TFLOOR, &src);
					} else {
						query_sprite(SPR_FLOOR, &src);
					}
					if (level[((y + yoff + 1) * LVL_W) + x + xoff] == B_WALL) {
						SDL_RenderCopy(renderer, tex, &src, &dest);
						query_sprite(SPR_TWALL, &src);
					}
					SDL_RenderCopy(renderer, tex, &src, &dest);
					query_sprite(SPR_COIN, &src);
					break;

			}
			switch (doodads[((y + yoff) * LVL_W) + x + xoff]) {
				case D_BRICK:
					query_sprite(SPR_MBRICK, &src);
					break;
				case D_GRATE:
					if (level[((y + yoff + 1) * LVL_W) + x + xoff] != B_WALL) {
						query_sprite(SPR_GRATE, &src);
					}
					break;
				case D_OOZE:
					if (level[((y + yoff - 1) * LVL_W) + x + xoff] == B_WALL) {
						query_sprite(SPR_OOZE, &src);
						dest.y -= BSIZE;
						SDL_RenderCopy(renderer, tex, &src, &dest);
						query_sprite(SPR_OOZEF, &src);
						dest.y += BSIZE;
						if (level[((y + yoff) * LVL_W) + x + xoff] == B_WALL) {
							SDL_RenderCopy(renderer, tex, &src, &dest);
							query_sprite(SPR_TWALL, &src);
						}
					}
					break;
				case D_SKULL:
					SDL_RenderCopy(renderer, tex, &src, &dest);
					query_sprite(SPR_SKULL, &src);
					if (level[((y + yoff + 1) * LVL_W) + x + xoff] == B_WALL) {
						SDL_RenderCopy(renderer, tex, &src, &dest);
						query_sprite(SPR_TWALL, &src);
					}
					break;
				case D_PIPE1:
					query_sprite(SPR_PIPE, &src);
					break;
				case D_PIPE2:
					query_sprite(SPR_PIPE2, &src);
					break;
			}
			SDL_RenderCopy(renderer, tex, &src, &dest);

			if (x + xoff == player->x && y + yoff == player->y) {
				query_sprite(SPR_PLAYER, &src);
				SDL_SetTextureColorMod(tex, 255, 255, 255);
				SDL_RenderCopy(renderer, tex, &src, &dest);

				if (level[((y + yoff + 1) * LVL_W) + x + xoff] == B_WALL &&
					(level[((y + yoff) * LVL_W) + x + xoff] == B_FLOOR ||
					 level[((y + yoff) * LVL_W) + x + xoff] == SOL_PATH)) {
					query_sprite(SPR_TWALL, &src);
					SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);
					SDL_RenderCopy(renderer, tex, &src, &dest);
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


char* generate_level() {
	int x, y;
	char* lvl = (char*) malloc(LVL_H * LVL_W);
	memset(lvl, B_WALL, LVL_H * LVL_W);
	for (y = 1; y < LVL_H; y += 2) {
		for (x = 1; x < LVL_W; x += 2) {
			srand(time(NULL));
			carve_maze(lvl, LVL_W, LVL_H, x, y);
		}
	}

	lvl[1 * LVL_W + 1] = B_FLOOR;
	exit_x = (LVL_W - 3);
	exit_y = (LVL_H - 2);
	lvl[exit_y * LVL_W + exit_x] = B_FLOOR;
	return lvl;
}

float dist_to(int sx, int sy, int dx, int dy) {
	return sqrtf(powf(sx - dx, 2) + powf(sy - dy, 2));
}

void draw_help() {
	#define LINE_H 16
	#define CHAR_W 8
	char* line1 = "O - solve maze",
			* line2 = "L - toggle light",
			* line3 = "R - generate new maze",
			* line4 = "Q - quit game";
	SDL_Color white = {255, 255, 255};

	SDL_Surface* surfaceMessage1 = TTF_RenderText_Solid(font, line1, white);
	SDL_Surface* surfaceMessage2 = TTF_RenderText_Solid(font, line2, white);
	SDL_Surface* surfaceMessage3 = TTF_RenderText_Solid(font, line3, white);
	SDL_Surface* surfaceMessage4 = TTF_RenderText_Solid(font, line4, white);
	SDL_Texture* message1 = SDL_CreateTextureFromSurface(renderer, surfaceMessage1);
	SDL_Texture* message2 = SDL_CreateTextureFromSurface(renderer, surfaceMessage2);
	SDL_Texture* message3 = SDL_CreateTextureFromSurface(renderer, surfaceMessage3);
	SDL_Texture* message4 = SDL_CreateTextureFromSurface(renderer, surfaceMessage4);

	SDL_Rect message_rect;

	message_rect.x = 5;
	message_rect.y = HEIGHT - 1 * LINE_H;
	message_rect.w = strlen(line1) * CHAR_W;
	message_rect.h = LINE_H;
	SDL_RenderCopy(renderer, message1, NULL, &message_rect);

	message_rect.x = 5;
	message_rect.y = HEIGHT - 2 * LINE_H;
	message_rect.w = strlen(line2) * CHAR_W;
	message_rect.h = LINE_H;
	SDL_RenderCopy(renderer, message2, NULL, &message_rect);

	message_rect.x = 5;
	message_rect.y = HEIGHT - 3 * LINE_H;
	message_rect.w = strlen(line3) * CHAR_W;
	message_rect.h = LINE_H;
	SDL_RenderCopy(renderer, message3, NULL, &message_rect);

	message_rect.x = 5;
	message_rect.y = HEIGHT - 4 * LINE_H;
	message_rect.w = strlen(line4) * CHAR_W;
	message_rect.h = LINE_H;
	SDL_RenderCopy(renderer, message4, NULL, &message_rect);
}

void draw_fps(int const* fps) {
	#define LINE_H 16
	#define CHAR_W 8
	char buf[32];
	snprintf(buf, 32, "fps %d", *fps);
	SDL_Color white = {255, 255, 255};

	SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, buf, white);
	SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
	SDL_Rect message_rect;
	message_rect.x = 5;
	message_rect.y = 5;
	message_rect.w = strlen(buf) * CHAR_W;
	message_rect.h = LINE_H;

	SDL_RenderCopy(renderer, message, NULL, &message_rect);
}

int bresenham(int x0, int y0, int x1, int y1, char* lvl) {
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2; /* error value e_xy */

	for (;;) {
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y0 += sy;
		}
		if (x0 == x1 && y0 == y1) return 1;
		if (lvl[y0 * LVL_W + x0] == B_WALL) return 0;
	}
}

int is_safe(char const* maze, int x, int y) {
	assert(maze != NULL);
	if (x >= 1 && x < LVL_W - 1 && y >= 1 && y < LVL_H - 1 && maze[y * LVL_W + x] == B_FLOOR)
		return 1;
	return 0;
}


int _solve(char* maze, int x, int y, char* sol) {
	if (x == exit_x && y == exit_y) {
		sol[y * LVL_W + x] = SOL_PATH;
		return 1;
	}

	if (is_safe(sol, x, y)) {
		sol[y * LVL_W + x] = SOL_PATH;
		if (_solve(maze, x + 1, y, sol)) {
			return 1;
		}
		if (_solve(maze, x, y + 1, sol)) {
			return 1;
		}
		if (_solve(maze, x - 1, y, sol)) {
			return 1;
		}
		if (_solve(maze, x, y - 1, sol)) {
			return 1;
		}
		sol[y * LVL_W + x] = B_FLOOR;
		return 0;
	}
	return 0;
}

void solve(char* maze) {
	if (solution != NULL)
		free(solution);
	solution = malloc(LVL_W * LVL_H);
	memcpy(solution, maze, LVL_W * LVL_H);
	_solve(maze, 1, 1, solution);
}

void restart_level(player_t* player) {
	free(level);
	level = generate_level();
	free(doodads);
	doodads = generate_doodads();
	while (level[(player->y = rand() % LVL_H) * LVL_W + (player->x = rand() % LVL_W)] == B_WALL);
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

char* generate_doodads() {
	assert(level != NULL);
	char* dd = calloc(LVL_H * LVL_W, sizeof(char));
	int i, dx, dy;
	#define BRICK_COUNT 80
	#define GRATE_COUNT 30
	#define OOZE_COUNT 30
	#define SKULL_COUNT 10
	#define PIPE_COUNT 30

	srand(time(NULL));
	for (i = 0; i < BRICK_COUNT; ++i) {
		while (level[(dy = rand() % LVL_H) * LVL_W + (dx = rand() % LVL_W)] != B_WALL);
		dd[dy * LVL_W + dx] = D_BRICK;
	}
	for (i = 0; i < GRATE_COUNT; ++i) {
		while (level[(dy = rand() % LVL_H) * LVL_W + (dx = rand() % LVL_W)] != B_WALL);
		dd[dy * LVL_W + dx] = D_GRATE;
	}
	for (i = 0; i < OOZE_COUNT; ++i) {
		while (level[(dy = rand() % LVL_H) * LVL_W + (dx = rand() % LVL_W)] != B_FLOOR);
		dd[dy * LVL_W + dx] = D_OOZE;
	}
	for (i = 0; i < SKULL_COUNT; ++i) {
		while (level[(dy = rand() % LVL_H) * LVL_W + (dx = rand() % LVL_W)] != B_FLOOR);
		dd[dy * LVL_W + dx] = D_SKULL;
	}
	for (i = 0; i < PIPE_COUNT; ++i) {
		while (level[(dy = rand() % LVL_H) * LVL_W + (dx = rand() % LVL_W)] != B_WALL);
		dd[dy * LVL_W + dx] = rand() % 2 ? D_PIPE1 : D_PIPE2;
	}
	return dd;
}
