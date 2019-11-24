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

typedef enum lights {
	L_NONE,
	L_LOS,
	L_NOLOS,
	L_DISABLED
} light_e;

typedef struct game_state {
	entity_t player;
	alist_t* lights;
	alist_t* enemies;
	alist_t* entities;
	char* level;
	char* doodads;
	int lvl_exit_x;
	int lvl_exit_y;
	int level_count;
	light_e l_sw;
	light_e l_type[3];
} game_state_t;

void sig_handler(int);

void quit();

void sdlerr_handler();

void event_handler(SDL_Event*);

void player_move(entity_t*, SDL_Scancode);

void Update(double delta_time);

void Render(SDL_Texture*);

void restart_level();

float calc_light(entity_t* source, int x, int y, float current_light);

void get_lights(char const* dd, alist_t* list);

void spawn_enemies(char const* lvl, alist_t* list);

void init_game();

game_state_t state;

volatile static int running = 1;

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static uint32_t render_flags = SDL_RENDERER_ACCELERATED;
static TTF_Font* font;

int main(int argc, char** argv) {
	maze_test_macros();
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

	surface = IMG_Load("res/sprites.png");
	SDL_Texture* tex __attribute__((cleanup(_sdldt))) = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	if (!tex)
		sdlerr_handler();

	font = TTF_OpenFont("res/DejaVuSans-Bold.ttf", 24);
	if (!font)
		fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());

	const double FRAME_TIME = 1.0 / 60.0;  // delta time for 60 FPS
	init_game();

	// setup code
	Uint64 prev_time = SDL_GetTicks();
	Uint64 now_time = 0;
	Uint64 elapsed_time = 0;
	SDL_Event event;

	while (running) {
		SDL_RenderClear(renderer);

		now_time = SDL_GetTicks();
		elapsed_time = now_time - prev_time;
		prev_time = now_time;
		// printf("%f %lu %lu\n", 1000.0 / elapsed_time, elapsed_time, now_time);

		while (SDL_PollEvent(&event)) {
			event_handler(&event);
		}

		Update(elapsed_time);
		Render( tex);
		SDL_RenderPresent(renderer);
		SDL_UpdateWindowSurface(window);

		SDL_Delay(1000 / TARGET_FPS);
	}

	alist_destroy(state.lights);
	alist_destroy(state.enemies);
	alist_destroy(state.entities);
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

void event_handler(SDL_Event* ev) {
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
					player_move(&state.player, ev->key.keysym.scancode);
					break;
				case SDL_SCANCODE_SPACE:
					player_shoot(&state.player, state.entities);
					break;
				case SDL_SCANCODE_R:
					restart_level();
					break;
				case SDL_SCANCODE_O:
					overlay_solution(state.level, state.lvl_exit_x, state.lvl_exit_y);
					break;
				case SDL_SCANCODE_L:
					state.l_sw = state.l_sw == sizeof(state.l_type)/sizeof(light_e) - 1 ? 0 : state.l_sw + 1;
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
			if (state.level[((e->y - 1) * LVL_W) + e->x] != B_WALL) {
				e->y -= 1;
				e->player.dir = DIR_UP;
			}
			break;
		case SDL_SCANCODE_A:
		case SDL_SCANCODE_LEFT:
			if (state.level[(e->y * LVL_W) + e->x - 1] != B_WALL) {
				e->x -= 1;
				e->player.dir = DIR_LEFT;
			}
			break;
		case SDL_SCANCODE_S:
		case SDL_SCANCODE_DOWN:
			if (state.level[((e->y + 1) * LVL_W) + e->x] != B_WALL) {
				e->y += 1;
				e->player.dir = DIR_DOWN;
			}
			break;
		case SDL_SCANCODE_D:
		case SDL_SCANCODE_RIGHT:
			if (state.level[(e->y * LVL_W) + e->x + 1] != B_WALL) {
				e->x += 1;
				e->player.dir = DIR_RIGHT;
			}
			break;
		default:
			break;
	}


}

void Update(double delta_time) {
	entity_t* e;
	entity_t* e1;
	uint j, i;
	//processing entities
	for (j = 0; j < alist_size(state.entities); ++j) {
		e = alist_get(state.entities, j);
		switch (e->type) {
			case E_PEW:
				if (!pew_move(e, state.level, LVL_W, B_WALL)) {
					alist_rm_idx(state.entities, j--);
				} else {
					for (i = 0; i < alist_size(state.enemies); ++i) {
						e1 = alist_get(state.enemies, i);
						if (e->x == e1->x && e->y == e1->y) {
							alist_rm_idx(state.entities, j--);
							e1->hp -= e->pew.dmg;
						}
					}
				}
				break;
		}
	}

	// processing enemies
	for (j = 0; j < alist_size(state.enemies); ++j) {
		e = alist_get(state.enemies, j);
		if (e->hp <= 0) {
			alist_rm_idx(state.enemies, j--);
		} else {
			// enemy_randmove(e, level, LVL_W, B_WALL);
			enemy_lockmove(e, &state.player, state.level, LVL_W, B_WALL);
			if (state.player.x == e->x && state.player.y == e->y) {
				restart_level(state.player);
			}
		}
	}

	if (state.player.x == state.lvl_exit_x && state.player.y == state.lvl_exit_y) {
		restart_level(&state.player);
	}
}

void Render(SDL_Texture* tex) {
	#undef lvlxy
	#define lvlxy(ox, oy) state.level[((y + yoff + (oy)) * LVL_W) + (x + xoff + (ox))]

	assert(state.doodads != NULL);
	assert(state.level != NULL);

	float light;

	int xoff = (int) (state.player.x / SCR_W) * SCR_W;
	int yoff = (int) (state.player.y / SCR_H) * SCR_H;

	SDL_Rect dest;
	SDL_Rect src;
	for (int y = 0; y < SCR_H; ++y) {
		for (int x = 0; x < SCR_W; ++x) {
			dest.h = BSIZE;
			dest.w = BSIZE;
			dest.x = BSIZE * x;
			dest.y = BSIZE * y;

			light = 0;
			for (uint i = 0; i < alist_size(state.lights); ++i) {
				entity_t* source = alist_get(state.lights, i);
				light = calc_light(source, x + xoff, y + yoff, light);
			}

			light = calc_light(&state.player, x + xoff, y + yoff, light);

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

			switch (state.doodads[((y + yoff) * LVL_W) + (x + xoff)]) {
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
			for (uint j = 0; j < alist_size(state.enemies); ++j) {
				entity_t* enemy = alist_get(state.enemies, j);
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
			for (uint j = 0; j < alist_size(state.entities); ++j) {
				entity_t* e = alist_get(state.entities, j);
				switch (e->type){
					case E_NONE:
						break;
					case E_PLAYER:
						break;
					case E_LIGHT:
						break;
					case E_ENEMY:
						break;
					case E_PEW:
						if (x + xoff == e->x && y + yoff == e->y) {
							load_sprite(SPR_COIN, (spr_rect*) &src);
							SDL_RenderCopy(renderer, tex, &src, &dest);
						}
						break;
				}

			}

			// render players
			if (x + xoff == state.player.x && y + yoff == state.player.y) {
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
	draw_help(renderer, font);
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

	switch (state.l_type[state.l_sw]){
		case L_NONE:
			rel_light = 0;
			break;
		case L_LOS:
			if (bresenham(source->x, source->y, x, y, state.level, LVL_W, B_WALL)) {
				rel_light = 255.0f / (dist / intensity) * a;
				rel_light = rel_light > 255 ? 255 : rel_light;
			} else {
				rel_light = 170.0f / dist * a;
				rel_light = rel_light > 170 ? 170 : rel_light;
			}
			break;
		case L_NOLOS:
			rel_light = 255.0f / (dist / intensity) * a;
			rel_light = rel_light > 255 ? 255 : rel_light;
			break;
		case L_DISABLED:
			rel_light = 255;
			break;
	}
	return fmaxf(rel_light, current_light);
}

void restart_level() {
	free(state.level);
	state.level = generate_level(&state.lvl_exit_x, &state.lvl_exit_y);
	free(state.doodads);
	state.doodads = generate_doodads(state.level);
	get_lights(state.doodads, state.lights);
	spawn_enemies(state.level, state.enemies);
	state.player.x = 1;
	state.player.y = 1;
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

void init_game() {
	state.entities = alist_new(sizeof(entity_t));
	state.enemies = alist_new(sizeof(entity_t));
	state.lights = alist_new(sizeof(entity_t));

	state.level_count = 0;
	state.level = generate_level(&state.lvl_exit_x, &state.lvl_exit_y);
	state.doodads = generate_doodads(state.level);
	get_lights(state.doodads, state.lights);
	spawn_enemies(state.level, state.enemies);

	state.l_sw = 0;
	state.l_type[0] = L_LOS;
	state.l_type[1] = L_NOLOS;
	state.l_type[2] = L_DISABLED;

	state.player.x = 1;
	state.player.y = 1;
	state.player.hp = 100;
	state.player.player.dmg = 50;
	state.player.player.dir = DIR_DOWN;
	state.player.type = E_PLAYER;
}
