#define SDL_MAIN_HANDLED 1

#include <stdio.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <signal.h>
#include <assert.h>

#include "macro_definitions.h"
#include "structs/arraylist.h"
#include "draw_misc.h"

#include "entity.h"

#include "sprites.h"
#include "maze.h"
#include "util.h"

#define printd(x) printf("%d\n")
#define printt(x, y) printf("(%d, %d)\n", x, y)


void sig_handler(int);

void quit();

void sdlerr_handler();

void event_handler(SDL_Event*, entity_t*);

void player_move(entity_t*, SDL_Scancode);

void game_loop(entity_t*);

void render_loop(entity_t*, SDL_Texture*);

void restart_level(entity_t*);

float calc_light(entity_t* source, int x, int y, float current_light);

void get_lights(char const* dd, alist_t* list);

static char* level = NULL;
static char* doodads = NULL;

static int exit_x = -1;
static int exit_y = -1;

volatile static int running = 1;
volatile static char l_sw = 0;
volatile static char l_type[] = {1, 2, 3};

alist_t* lights = NULL;

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static uint32_t render_flags = SDL_RENDERER_ACCELERATED;

int main(int argc, char** argv) {
	maze_test_macros();
	signal(SIGINT, sig_handler);

	//The surface contained by the window
	SDL_Surface* surface = NULL;
	TTF_Font* font;
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
	SDL_Texture* tex __attribute__((cleanup(_sdldt))) = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	if (!tex)
		sdlerr_handler();

	font = TTF_OpenFont("res/DejaVuSans-Bold.ttf", 24);
	if (!font)
		fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());

	lights = alist_new(sizeof(entity_t));

	level = generate_level(&exit_x, &exit_y);
	doodads = generate_doodads(level);
	get_lights(doodads, lights);

	entity_t player = {1, 1, {}, E_PLAYER};

	// setup code
	Uint32 startclock = 0;
	Uint32 deltaclock = 0;
	Uint32 currentFPS = 0;
	startclock = SDL_GetTicks();
	int target_fps = 60;

	while (running) {
		SDL_RenderClear(renderer);

		game_loop(&player);
		render_loop(&player, tex);

		deltaclock = SDL_GetTicks() - startclock;
		startclock = SDL_GetTicks();

		if (deltaclock != 0) {
			currentFPS = 1000 / deltaclock;
			draw_fps(renderer, font, (int const*) &currentFPS);
		}

		draw_help(renderer, font);

		SDL_RenderPresent(renderer);
		SDL_UpdateWindowSurface(window);
		SDL_Delay(1000 / target_fps);
	}
	alist_destroy(lights);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();
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
	TTF_Quit();
	SDL_Quit();
	exit(1);
}

void quit() {
	running = 0;
}

void event_handler(SDL_Event* ev, entity_t* player) {
	assert(player->type == E_PLAYER);
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
					overlay_solution(level, exit_x, exit_y);
					break;
				case SDL_SCANCODE_L:
					l_sw = l_sw == sizeof(l_type) - 1 ? 0 : l_sw + 1;
					break;
				case SDL_SCANCODE_Q:
					quit();
					break;
			}
			break;
	}
}

void player_move(entity_t* player, SDL_Scancode code) {
	assert(player->type == E_PLAYER);
	switch (code) {
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_UP:
			if (level[((player->y - 1) * LVL_W) + player->x] != B_WALL) {
				player->y -= 1;
			}
			break;
		case SDL_SCANCODE_A:
		case SDL_SCANCODE_LEFT:
			if (level[(player->y * LVL_W) + player->x - 1] != B_WALL) {
				player->x -= 1;
			}
			break;
		case SDL_SCANCODE_S:
		case SDL_SCANCODE_DOWN:
			if (level[((player->y + 1) * LVL_W) + player->x] != B_WALL) {
				player->y += 1;
			}
			break;
		case SDL_SCANCODE_D:
		case SDL_SCANCODE_RIGHT:
			if (level[(player->y * LVL_W) + player->x + 1] != B_WALL) {
				player->x += 1;
			}
			break;
		default:
			break;
	}


}

void game_loop(entity_t* player) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		event_handler(&event, player);
	}

	if (player->x == exit_x && player->y == exit_y) {
		restart_level(player);
	}
}

void render_loop(entity_t* player, SDL_Texture* tex) {
	#undef lvlxy
	#define lvlxy(ox, oy) level[((y + yoff + (oy)) * LVL_W) + (x + xoff + (ox))]

	assert(doodads != NULL);
	assert(level != NULL);
	float light;

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

			light = 0;
			for (int i = 0; i < 10; ++i) {
				entity_t* source = alist_get(lights, i);
				light = calc_light(source, x + xoff, y + yoff, light);
			}
			light = calc_light(player, x + xoff, y + yoff, light);

			SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);

			switch (lvlxy(0, 0)) {
				case B_WALL:
					load_sprite(SPR_WALL, &src);
					break;
				case B_FLOOR:
					if (lvlxy(0, -1) == B_WALL) {
						load_sprite(SPR_TFLOOR, &src);
					} else {
						load_sprite(SPR_FLOOR, &src);
					}
					if (lvlxy(0, 1) == B_WALL) {
						SDL_RenderCopy(renderer, tex, &src, &dest);
						load_sprite(SPR_TWALL, &src);
					}
					break;
				case B_PATH:
					if (lvlxy(0, -1) == B_WALL) {
						load_sprite(SPR_TFLOOR, &src);
					} else {
						load_sprite(SPR_FLOOR, &src);
					}
					if (lvlxy(0, 1) == B_WALL) {
						SDL_RenderCopy(renderer, tex, &src, &dest);
						load_sprite(SPR_TWALL, &src);
					}
					SDL_RenderCopy(renderer, tex, &src, &dest);
					load_sprite(SPR_COIN, &src);
					break;

			}
			SDL_RenderCopy(renderer, tex, &src, &dest);

			switch (doodads[((y + yoff) * LVL_W) + (x + xoff)]) {
				case D_BRICK:
					load_sprite(SPR_MBRICK, &src);
					break;
				case D_GRATE:
					if (lvlxy(0, 1) != B_WALL) {
						dest.y += BSIZE;
						load_sprite(SPR_OOZEF, &src);
						SDL_RenderCopy(renderer, tex, &src, &dest);
						dest.y -= BSIZE;
						load_sprite(SPR_GRATE, &src);
					}
					break;
				case D_OOZE:
					if (lvlxy(0, -1) == B_WALL) {
						load_sprite(SPR_OOZE, &src);
						dest.y -= BSIZE;
						SDL_RenderCopy(renderer, tex, &src, &dest);
						load_sprite(SPR_OOZEF, &src);
						dest.y += BSIZE;
						if (lvlxy(0, 0) == B_PATH) {
							SDL_RenderCopy(renderer, tex, &src, &dest);
							load_sprite(SPR_COIN, &src);
						}
						if ((lvlxy(0, 0) == B_FLOOR || lvlxy(0, 0) == B_PATH) &&
							lvlxy(0, 1) == B_WALL) {
							SDL_RenderCopy(renderer, tex, &src, &dest);
							load_sprite(SPR_TWALL, &src);
						}
					}
					break;
				case D_SKULL:
					SDL_RenderCopy(renderer, tex, &src, &dest);
					load_sprite(SPR_SKULL, &src);
					if (lvlxy(0, 1) == B_WALL) {
						SDL_RenderCopy(renderer, tex, &src, &dest);
						load_sprite(SPR_TWALL, &src);
					}
					break;
				case D_PIPE1:
					load_sprite(SPR_PIPE, &src);
					break;
				case D_PIPE2:
					load_sprite(SPR_PIPE2, &src);
					break;
				case D_TORCH:
					load_sprite(SPR_TORCH, &src);
					break;
			}
			SDL_RenderCopy(renderer, tex, &src, &dest);

			if (x + xoff == player->x && y + yoff == player->y) {
				load_sprite(SPR_PLAYER, &src);
				SDL_SetTextureColorMod(tex, 255, 255, 255);
				SDL_RenderCopy(renderer, tex, &src, &dest);

				if (lvlxy(0, 1) == B_WALL &&
					(lvlxy(0, 0) == B_FLOOR ||
					 lvlxy(0, 0) == B_PATH)) {
					load_sprite(SPR_TWALL, &src);
					SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);
					SDL_RenderCopy(renderer, tex, &src, &dest);
				}
			}
		}
	}
	#undef lvlxy
}


float calc_light(entity_t* source, int x, int y, float current_light) {
	float intensity = 4;
	if (source->type == E_LIGHT) {
		intensity = source->light.intensity;
	}
	float rel_light = 0, dist, a;;


	a = 1 - (10 - (rand() % 25)) / 100.0f;
	dist = dist_to(source->x, source->y, x, y);
	if (l_type[l_sw] == 1) {
		if (bresenham(source->x, source->y, x, y, level, LVL_W, B_WALL)) {
			rel_light = 255.0f / (dist / intensity) * a;
			rel_light = rel_light > 255 ? 255 : rel_light;
		} else {
			rel_light = 196.0f / dist * a;
			rel_light = rel_light > 196 ? 196 : rel_light;
		}

	} else if (l_type[l_sw] == 2) {
		rel_light = 255.0f / (dist / intensity) * a;
		rel_light = rel_light > 255 ? 255 : rel_light;
	} else if (l_type[l_sw] == 3) {
		rel_light = 255;
	}
	return fmaxf(rel_light, current_light);
}

void restart_level(entity_t* player) {
	free(level);
	level = generate_level(&exit_x, &exit_y);
	free(doodads);
	doodads = generate_doodads(level);
	get_lights(doodads, lights);
	player->x = 1;
	player->y = 1;
}

void get_lights(char const* dd, alist_t* list) {
	alist_clear(list);
	entity_t source;
	for (int y = 0; y < LVL_H; ++y) {
		for (int x = 0; x < LVL_W; ++x) {
			if (dd[y * LVL_W + x] == D_TORCH) {
				source.x = x;
				source.y = y;
				source.light.intensity = 1.0f;
				source.type = E_LIGHT;
				alist_add(list, &source);
			}
		}
	}
}