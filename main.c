#define SDL_MAIN_HANDLED 1

#include <stdio.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <signal.h>
#include <assert.h>

#include "macro_definitions.h"
#include "draw_misc.h"

#include "entity.h"

#include "sprites.h"
#include "maze.h"
#include "util.h"

// #include "structs/arraylist.h"
#define TARGET_FPS 60

#define printd(x) printf("%d\n")
#define printt(x, y) printf("(%d, %d)\n", x, y)


void sig_handler(int);

void quit();

void sdlerr_handler();

void event_handler(SDL_Event*, entity_t*);

void player_move(entity_t*, SDL_Scancode);

void game_loop(double delta_time, entity_t*);

void render_loop(entity_t*, SDL_Texture*);

void restart_level(entity_t*);

float calc_light(entity_t* source, int x, int y, float current_light);

void get_lights(char const* dd, alist_t* list);

void spawn_enemies(char const* lvl, alist_t* list);

static char* level = NULL;
static char* doodads = NULL;

static int exit_x = -1;
static int exit_y = -1;

enum light_e {
	L_NONE,
	L_LOS,
	L_NOLOS,
	L_DISABLED
};

volatile static int running = 1;
volatile static char l_sw = 0;
volatile static char l_type[] = {L_LOS, L_NOLOS, L_DISABLED};
volatile static int level_count = 0;


alist_t* lights = NULL;
alist_t* enemies = NULL;
alist_t* entities = NULL;

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

	surface = IMG_Load("res/sprites.png");
	SDL_Texture* tex __attribute__((cleanup(_sdldt))) = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	if (!tex)
		sdlerr_handler();

	font = TTF_OpenFont("res/DejaVuSans-Bold.ttf", 24);
	if (!font)
		fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());

	lights = alist_new(sizeof(entity_t));
	enemies = alist_new(sizeof(entity_t));
	entities = alist_new(sizeof(entity_t));

	level = generate_level(&exit_x, &exit_y);
	doodads = generate_doodads(level);
	get_lights(doodads, lights);
	spawn_enemies(level, enemies);

	entity_t player = {1, 1, 100, {}, E_PLAYER};
	player.player.dmg = 50;
	player.player.dir = DIR_DOWN;

	const double FRAME_TIME = 1.0 / 60.0;  // delta time for 60 FPS


	// setup code
	Uint32 startclock = 0;
	Uint32 deltaclock = 0;
	Uint32 lastclock = 0;
	SDL_Event event;

	while (running) {
		SDL_RenderClear(renderer);

		while (SDL_PollEvent(&event)) {
			event_handler(&event, &player);
		}

		startclock = SDL_GetTicks();
		deltaclock = lastclock - startclock;
		lastclock = startclock;
		printf("%d %d \n", deltaclock, startclock);

		game_loop(0, &player);
		render_loop(&player, tex);
		draw_help(renderer, font);
		SDL_RenderPresent(renderer);
		SDL_UpdateWindowSurface(window);

		SDL_Delay(1000 / (TARGET_FPS - deltaclock));
	}

	alist_destroy(lights);
	alist_destroy(enemies);
	alist_destroy(entities);
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
				case SDL_SCANCODE_SPACE:
					player_shoot(player, entities);
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

void player_move(entity_t* e, SDL_Scancode code) {
	assert(e->type == E_PLAYER);
	switch (code) {
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_UP:
			if (level[((e->y - 1) * LVL_W) + e->x] != B_WALL) {
				e->y -= 1;
				e->player.dir = DIR_UP;
			}
			break;
		case SDL_SCANCODE_A:
		case SDL_SCANCODE_LEFT:
			if (level[(e->y * LVL_W) + e->x - 1] != B_WALL) {
				e->x -= 1;
				e->player.dir = DIR_LEFT;
			}
			break;
		case SDL_SCANCODE_S:
		case SDL_SCANCODE_DOWN:
			if (level[((e->y + 1) * LVL_W) + e->x] != B_WALL) {
				e->y += 1;
				e->player.dir = DIR_DOWN;
			}
			break;
		case SDL_SCANCODE_D:
		case SDL_SCANCODE_RIGHT:
			if (level[(e->y * LVL_W) + e->x + 1] != B_WALL) {
				e->x += 1;
				e->player.dir = DIR_RIGHT;
			}
			break;
		default:
			break;
	}


}

void game_loop(double delta_time, entity_t* player) {
	entity_t* e;
	entity_t* e1;
	uint j, i;
	//processing entities
	for (j = 0; j < alist_size(entities); ++j) {
		e = alist_get(entities, j);
		switch (e->type) {
			case E_PEW:
				if (!pew_move(e, level, LVL_W, B_WALL)) {
					alist_rm_idx(entities, j--);
				} else {
					for (i = 0; i < alist_size(enemies); ++i) {
						e1 = alist_get(enemies, i);
						if (e->x == e1->x && e->y == e1->y) {
							alist_rm_idx(entities, j--);
							e1->hp -= e->pew.dmg;
						}
					}
				}
				break;
		}
	}

	// processing enemies
	for (j = 0; j < alist_size(enemies); ++j) {
		e = alist_get(enemies, j);
		if (e->hp <= 0) {
			alist_rm_idx(enemies, j--);
		} else {
			// enemy_randmove(e, level, LVL_W, B_WALL);
			enemy_lockmove(e, player, level, LVL_W, B_WALL);
			if (player->x == e->x && player->y == e->y) {
				restart_level(player);
			}
		}
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
			for (uint i = 0; i < alist_size(lights); ++i) {
				entity_t* source = alist_get(lights, i);
				light = calc_light(source, x + xoff, y + yoff, light);
			}
			light = calc_light(player, x + xoff, y + yoff, light);

			SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);

			switch (lvlxy(0, 0)) {
				case B_WALL:
					load_sprite(SPR_WALL, (spr_rect*) &src);
					break;
				case B_EXIT:
					load_sprite(SPR_DLADDER, (spr_rect*) &src);
					break;
				case B_FLOOR:
					if (lvlxy(0, -1) == B_WALL) {
						load_sprite(SPR_TFLOOR, (spr_rect*) &src);
					} else {
						load_sprite(SPR_FLOOR, (spr_rect*) &src);
					}
					if (lvlxy(0, 1) == B_WALL) {
						SDL_RenderCopy(renderer, tex, &src, &dest);
						load_sprite(SPR_TWALL, (spr_rect*) &src);
					}
					break;
				case B_PATH:
					if (lvlxy(0, -1) == B_WALL) {
						load_sprite(SPR_TFLOOR, (spr_rect*) &src);
					} else {
						load_sprite(SPR_FLOOR, (spr_rect*) &src);
					}
					if (lvlxy(0, 1) == B_WALL) {
						SDL_RenderCopy(renderer, tex, &src, &dest);
						load_sprite(SPR_TWALL, (spr_rect*) &src);
					}
					SDL_RenderCopy(renderer, tex, &src, &dest);
					load_sprite(SPR_COIN, (spr_rect*) &src);
					break;

			}
			SDL_RenderCopy(renderer, tex, &src, &dest);

			switch (doodads[((y + yoff) * LVL_W) + (x + xoff)]) {
				case D_BRICK:
					load_sprite(SPR_MBRICK, (spr_rect*) &src);
					break;
				case D_GRATE:
					if (lvlxy(0, 1) != B_WALL) {
						dest.y += BSIZE;
						load_sprite(SPR_OOZEF, (spr_rect*) &src);
						SDL_RenderCopy(renderer, tex, &src, &dest);
						dest.y -= BSIZE;
						load_sprite(SPR_GRATE, (spr_rect*) &src);
					}
					break;
				case D_OOZE:
					if (lvlxy(0, -1) == B_WALL) {
						load_sprite(SPR_OOZE, (spr_rect*) &src);
						dest.y -= BSIZE;
						SDL_RenderCopy(renderer, tex, &src, &dest);
						load_sprite(SPR_OOZEF, (spr_rect*) &src);
						dest.y += BSIZE;
						if (lvlxy(0, 0) == B_PATH) {
							SDL_RenderCopy(renderer, tex, &src, &dest);
							load_sprite(SPR_COIN, (spr_rect*) &src);
						}
						if ((lvlxy(0, 0) == B_FLOOR || lvlxy(0, 0) == B_PATH) &&
							lvlxy(0, 1) == B_WALL) {
							SDL_RenderCopy(renderer, tex, &src, &dest);
							load_sprite(SPR_TWALL, (spr_rect*) &src);
						}
					}
					break;
				case D_SKULL:
					SDL_RenderCopy(renderer, tex, &src, &dest);
					load_sprite(SPR_SKULL, (spr_rect*) &src);
					if (lvlxy(0, 1) == B_WALL) {
						SDL_RenderCopy(renderer, tex, &src, &dest);
						load_sprite(SPR_TWALL, (spr_rect*) &src);
					}
					break;
				case D_PIPE1:
					load_sprite(SPR_PIPE, (spr_rect*) &src);
					break;
				case D_PIPE2:
					load_sprite(SPR_PIPE2, (spr_rect*) &src);
					break;
				case D_TORCH:
					load_sprite(SPR_TORCH, (spr_rect*) &src);
					break;
			}
			SDL_RenderCopy(renderer, tex, &src, &dest);

			// render enemies
			for (uint j = 0; j < alist_size(enemies); ++j) {
				entity_t* enemy = alist_get(enemies, j);
				if (x + xoff == enemy->x && y + yoff == enemy->y) {
					load_sprite(SPR_ENEMY1, (spr_rect*) &src);
					SDL_RenderCopy(renderer, tex, &src, &dest);
					if (lvlxy(0, 1) == B_WALL &&
						(lvlxy(0, 0) == B_FLOOR ||
						 lvlxy(0, 0) == B_PATH)) {
						load_sprite(SPR_TWALL, (spr_rect*) &src);
						SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);
						SDL_RenderCopy(renderer, tex, &src, &dest);
					}
				}
			}

			// render entities
			for (uint j = 0; j < alist_size(entities); ++j) {
				entity_t* e = alist_get(entities, j);
				if (x + xoff == e->x && y + yoff == e->y) {
					load_sprite(SPR_COIN, (spr_rect*) &src);
					SDL_RenderCopy(renderer, tex, &src, &dest);
					// if (lvlxy(0, 1) == B_WALL &&
					// 	(lvlxy(0, 0) == B_FLOOR ||
					// 	 lvlxy(0, 0) == B_PATH)) {
					// 	load_sprite(SPR_TWALL, (spr_rect*) &src);
					// 	SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);
					// 	SDL_RenderCopy(renderer, tex, &src, &dest);
					// }
				}
			}

			// render players
			if (x + xoff == player->x && y + yoff == player->y) {
				load_sprite(SPR_PLAYER, (spr_rect*) &src);
				SDL_SetTextureColorMod(tex, 255, 255, 255);
				SDL_RenderCopy(renderer, tex, &src, &dest);

				if (lvlxy(0, 1) == B_WALL &&
					(lvlxy(0, 0) == B_FLOOR ||
					 lvlxy(0, 0) == B_PATH)) {
					load_sprite(SPR_TWALL, (spr_rect*) &src);
					SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);
					SDL_RenderCopy(renderer, tex, &src, &dest);
				}
			}
		}
	}
	#undef lvlxy
}


float calc_light(entity_t* source, int x, int y, float current_light) {
	assert(source != NULL);
	float intensity = 3;
	if (source->type == E_LIGHT) {
		intensity = source->light.intensity;
	}
	float rel_light = 0, dist, a;


	a = 1 - (10 - (rand() % 25)) / 100.0f;
	dist = dist_to(source->x, source->y, x, y);
	if (l_type[l_sw] == L_LOS) {
		if (bresenham(source->x, source->y, x, y, level, LVL_W, B_WALL)) {
			rel_light = 255.0f / (dist / intensity) * a;
			rel_light = rel_light > 255 ? 255 : rel_light;
		} else {
			rel_light = 170.0f / dist * a;
			rel_light = rel_light > 170 ? 170 : rel_light;
		}

	} else if (l_type[l_sw] == L_NOLOS) {
		rel_light = 255.0f / (dist / intensity) * a;
		rel_light = rel_light > 255 ? 255 : rel_light;
	} else if (l_type[l_sw] == L_DISABLED) {
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
	spawn_enemies(level, enemies);
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

void spawn_enemies(char const* lvl, alist_t* list) {
	assert(lvl != NULL);
	alist_clear(list);
	#define ENEMY_COUNT 20
	entity_t enemy = enemy_new(0, 0);
	int i, dx, dy;
	for (i = 0; i < ENEMY_COUNT; ++i) {
		while (lvl[(dy = rand() % LVL_H) * LVL_W + (dx = rand() % LVL_W)] != B_FLOOR);
		enemy.x = dx;
		enemy.y = dy;
		alist_add(list, &enemy);
	}
}
