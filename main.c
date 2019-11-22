#include <stdio.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>

#include "sprites.h"

// screen size
#define WIDTH 1024
#define HEIGHT 768

// apparent block size on the screen
#define DSIZE 32

// blocks count in the viewport
#define SCR_W (WIDTH / DSIZE)
#define SCR_H (HEIGHT / DSIZE)

#define WRLD_W 3
#define WRLD_H 2

#define LVL_W (WRLD_W * SCR_W)
#define LVL_H (WRLD_H * SCR_H)

#define printd(x) printf("%d\n")
#define printt(x, y) printf("(%d, %d)\n", x, y)
#define printlvl(lvl) if(lvl != NULL)\
for (int y = 0; y < LVL_H; ++y){\
for (int x = 0; x < LVL_W; ++x) putc(lvl[y * LVL_W + x] == '#' ? (char)178 : ' ', stdout);\
putc('\n', stdout);}\

typedef enum {
	DIR_UP = 0,
	DIR_RIGHT = 1,
	DIR_DOWN = 2,
	DIR_LEFT = 3,
} move_dir;

typedef struct player {
	int32_t x;
	int32_t y;
} player_t;

void sig_handler(int);

void sdlerr_handler();

void quit();

void event_handler(SDL_Event*, player_t*);

void player_move(player_t*, SDL_Scancode);

void game_loop(player_t*);

void render_loop(player_t*, SDL_Texture*);

char* generate_lvl();

void restart_lvl(player_t*);

float dist_to(int, int, int, int);

void print_fps(int const*);

void carve_maze(char*, int, int, int, int);

int bresenham(int, int, int, int, char*);

char* level = NULL;
int exit_x = -1;
int exit_y = -1;

volatile __sig_atomic_t running = 1;
volatile __sig_atomic_t l_sw = 1;

//The window we'll be rendering to
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t render_flags = SDL_RENDERER_ACCELERATED;
TTF_Font* font;

int main(int argc, char* args[]) {

	signal(SIGINT, sig_handler);

	//The surface contained by the window
	SDL_Surface* surface = NULL;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		sdlerr_handler();

	TTF_Init();
	//Create window
	window = SDL_CreateWindow("A Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH,
							  HEIGHT, SDL_WINDOW_SHOWN);
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

	font = TTF_OpenFont("/usr/share/fonts/TTF/DejaVuSans.ttf", 24);

	if (!font)
		fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());

	level = generate_lvl();
	player_t player = {1, 1, DIR_DOWN, 30};

	// generate_enemies(&enemies, 10);

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
			print_fps((int const*) &currentFPS);
		}

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
					restart_lvl(player);
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
	int xoff = (int) (player->x / SCR_W) * SCR_W;
	int yoff = (int) (player->y / SCR_H) * SCR_H;
	SDL_Rect dest;
	SDL_Rect src;
	for (int y = 0; y < SCR_H; ++y) {
		for (int x = 0; x < SCR_W; ++x) {
			dest.h = DSIZE;
			dest.w = DSIZE;
			dest.x = DSIZE * x;
			dest.y = DSIZE * y;
			switch (level[((y + yoff) * LVL_W) + x + xoff]) {
				case '#':
					query_sprite(SPR_WALL, &src);
					break;
				case ' ':
					query_sprite(SPR_FLOOR, &src);
					break;
				case 'B':
					query_sprite(SPR_LFLOOR, &src);
					break;

			}
			float light;
			float a = 1 - (10 - (random() % 25)) / 100.0f;
			float dist = dist_to(player->x, player->y, x + xoff, y + yoff);
			if (bresenham(player->x, player->y, x + xoff, y + yoff, level)) {
				// if (l_sw) {
				// calculate lighting depending on distance to the player (only light source)
				float light_amp = 2;
				light = 255.0f / (dist / light_amp) * a;
				light = light > 255 ? 255 : light;
			} else {
				light = 127.0f / dist * a;
				light = light > 127 ? 127 : light;
				// light = 0;
			};

			if (!l_sw) {
				light = 255;
			}


			SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);

			SDL_RenderCopy(renderer, tex, &src, &dest);

			if (x + xoff == player->x && y + yoff == player->y) {
				query_sprite(SPR_PLAYER, &src);
				SDL_SetTextureColorMod(tex, 255, 255, 255);
				SDL_RenderCopy(renderer, tex, &src, &dest);
			}
		}
	}
}

void carve_maze(char* maze, int width, int height, int x, int y) {

	int x1, y1;
	int x2, y2;
	int dx, dy;
	int dir, count;

	dir = random() % 4;
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
		if (x2 > 0 && x2 < width &&
			y2 > 0 && y2 < height &&
			maze[y1 * width + x1] == 35 &&
			maze[y2 * width + x2] == 35) {
			maze[y1 * width + x1] = 32;
			maze[y2 * width + x2] = 32;
			x = x2;
			y = y2;
			dir = random() % 4;
			count = 0;
		} else {
			dir = (dir + 1) % 4;
			count += 1;
		}
	}

}


char* generate_lvl() {
	int lvlh = LVL_H;
	int lvlw = LVL_W;
	int x, y;
	char* lvl = (char*) malloc(lvlh * lvlw);
	memset(lvl, 35, lvlh * lvlw);
	for (y = 1; y < lvlh; y += 2) {
		for (x = 1; x < lvlw; x += 2) {
			srandom(time(NULL));
			carve_maze(lvl, lvlw, lvlh, x, y);
		}
	}

	/* Set up the entry and exit. */
	lvl[0 * lvlw + 1] = 0;
	exit_x = (lvlw - 2);
	exit_y = (lvlh - 1);
	lvl[exit_y * lvlw + exit_x] = 0;
	return lvl;
}

float dist_to(int sx, int sy, int dx, int dy) {
	return sqrtf(powf(sx - dx, 2) + powf(sy - dy, 2));
}


void print_fps(int const* fps) {

	char buf[32];
	snprintf(buf, 32, "fps %d", *fps);
	SDL_Color white = {255, 255, 255};

	SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, buf, white);
	SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
	SDL_Rect message_rect;
	message_rect.x = 16;
	message_rect.y = HEIGHT - 32;
	message_rect.w = 150;
	message_rect.h = 32;

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
		if (lvl[y0 * LVL_W + x0] == 35) return 0;
	}
}

void restart_lvl(player_t* player) {
	free(level);
	level = generate_lvl();
	long x = 0, y = 0;
	while (level[y * LVL_W + x] == '#') {
		x = random() % LVL_W;
		y = random() % LVL_H;
	}
	player->x = x;
	player->y = y;
}
