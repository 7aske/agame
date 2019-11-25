#define SDL_MAIN_HANDLED 1

#include <stdio.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <signal.h>
#include <assert.h>
#include <state.h>

#include "macro_definitions.h"
#include "draw_misc.h"

#include "entity/player.h"
#include "entity/pew.h"
#include "event/event.h"
#include "entity/enemy.h"

#include "sprites.h"
#include "maze.h"
#include "util.h"

#define printd(x) printf("%d\n")
#define printt(x, y) printf("(%d, %d)\n", x, y)

void sig_handler(int);

void quit();

void sdlerr_handler()__attribute__((noreturn));

void event_handler(SDL_Event*);

void Update(double delta_time);

void Render();

void Event(double delta_time);

float calc_light(entity_t* source, int x, int y, float current_light);

void init_game();

state_t state;

volatile static int running = 1;

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static uint32_t render_flags = SDL_RENDERER_ACCELERATED;
static TTF_Font* font;
static SDL_Texture* tex;

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
	SDL_SetWindowTitle(window, WINDOW_TITLE);
	SDL_SetWindowResizable(window, SDL_FALSE);

	renderer = SDL_CreateRenderer(window, -1, render_flags);
	if (!renderer)
		sdlerr_handler();

	surface = IMG_Load("res/sprites.png");
	tex = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	if (!tex)
		sdlerr_handler();

	font = TTF_OpenFont("res/DejaVuSans-Bold.ttf", 24);
	if (!font)
		fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());

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

		Event(elapsed_time);
		Update(elapsed_time);
		Render(tex);

		SDL_RenderPresent(renderer);
		SDL_UpdateWindowSurface(window);

		SDL_Delay(1000 / TARGET_FPS);
	}

	alist_destroy(state.light_emitters);
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
					player_move(&state.player, ev->key.keysym.scancode, &state.level);
					break;
				case SDL_SCANCODE_SPACE:
					player_shoot(&state.player, state.entities);
					break;
				case SDL_SCANCODE_R:
					event_dispatch(&state, ev_game_restart);
					break;
				case SDL_SCANCODE_O:
					overlay_solution(state.level.maze, state.level.exit_x, state.level.exit_y);
					break;
				case SDL_SCANCODE_L:
					state.l_sw = state.l_sw == sizeof(state.l_type) / sizeof(light_e) - 1 ? 0 : state.l_sw + 1;
					break;
				case SDL_SCANCODE_Q:
					quit();
					break;
			}
			break;
	}
}


void Event(double delta_time) {
	event_t* ev;
	while (!queue_isempty(state.event_queue)) {
		ev = queue_dequeue(state.event_queue);
		ev->callback(&state);
	}
}

void Update(double delta_time) {
	entity_t* e;
	entity_t* e1;
	int j, i;


	//processing entities
	for (j = 0; j < alist_size(state.entities); ++j) {
		e = alist_get(state.entities, j);
		switch (e->type) {
			case E_PEW:
				if (!pew_move(e, state.level.maze, LVL_W, B_WALL)) {
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
			if (e->enemy.path != NULL) {
				stack_destroy(e->enemy.path);
			}
			event_dispatch(&state, ev_score_incr);
			alist_rm_idx(state.enemies, j--);

		} else {
			enemy_search(e, &(state.player), state.level.maze, state.level.w, state.level.h, state.level.b_wall, 0);
			enemy_fpath(e, state.level.maze, state.level.w, B_WALL);
			if (state.player.x == e->x && state.player.y == e->y) {
				event_dispatch(&state, ev_game_restart);
			}
		}
	}
	// processing player
	if (state.player.player.next_move > 0) {
		state.player.player.next_move--;
	}
	if (state.player.player.next_shot > 0) {
		state.player.player.next_shot--;
	}
	if (state.player.x == state.level.exit_x && state.player.y == state.level.exit_y) {
		state.level_count++;
		event_dispatch(&state, ev_level_next);
	}
}

void Render() {
	#undef lvlxy
	#define lvlxy(ox, oy) state.level.maze[((y + yoff + (oy)) * LVL_W) + (x + xoff + (ox))]

	assert(state.level.doodads != NULL);
	assert(state.level.maze != NULL);

	float light;
	char text_buf[128];
	int xoff = (int) (state.player.x / SCR_W) * SCR_W;
	int yoff = (int) (state.player.y / SCR_H) * SCR_H;
	int i;
	SDL_Rect dest;
	SDL_Rect src;
	for (int y = 0; y < SCR_H; ++y) {
		for (int x = 0; x < SCR_W; ++x) {
			dest.h = BSIZE;
			dest.w = BSIZE;
			dest.x = BSIZE * x;
			dest.y = BSIZE * y;

			// rendering lights
			light = 0;
			for (i = 0; i < alist_size(state.light_emitters); ++i) {
				entity_t* source = alist_get(state.light_emitters, i);
				if (dist_to(x + xoff, y + yoff, source->x, source->y) < 10) {
					light = calc_light(source, x + xoff, y + yoff, light);
				}
			}

			light = calc_light(&state.player, x + xoff, y + yoff, light);

			SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);

			// rendering background
			switch (lvlxy(0, 0)) {
				case B_WALL:
					load_sprite(SPR_WALL, (spr_rect*) &src);
					break;
				case B_EXIT:
					load_sprite(SPR_DLADDER, (spr_rect*) &src);
					if (lvlxy(0, 1) == B_WALL) {
						SDL_RenderCopy(renderer, tex, &src, &dest);
						load_sprite(SPR_TWALL, (spr_rect*) &src);
					}
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

			// rendering doodads
			switch (state.level.doodads[((y + yoff) * LVL_W) + (x + xoff)]) {
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

			// rendering enemies
			for (i = 0; i < alist_size(state.enemies); ++i) {
				entity_t* enemy = alist_get(state.enemies, i);
				if (x + xoff == enemy->x && y + yoff == enemy->y) {
					load_sprite(SPR_ENEMY1, (spr_rect*) &src);
					SDL_SetTextureColorMod(tex, light, light * 0.8 * (enemy->hp / E_DEF_HP),
										   light * 0.5 * (enemy->hp / E_DEF_HP));
					SDL_RenderCopy(renderer, tex, &src, &dest);
					SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);
					if (lvlxy(0, 1) == B_WALL &&
						(lvlxy(0, 0) == B_FLOOR ||
						 lvlxy(0, 0) == B_PATH)) {
						load_sprite(SPR_TWALL, (spr_rect*) &src);
						SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);
						SDL_RenderCopy(renderer, tex, &src, &dest);
					}
				}
			}

			// rendering entities
			for (i = 0; i < alist_size(state.entities); ++i) {
				entity_t* e = alist_get(state.entities, i);
				assert(e != NULL);
				switch (e->type) {
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
							load_sprite(SPR_FBOY, (spr_rect*) &src);
							dest.y -= 10;
							SDL_RenderCopy(renderer, tex, &src, &dest);
							dest.y += 10;
						}
						break;
				}

			}

			// rendering player
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
	snprintf(text_buf, 127, "Level: %d | Score: %d", state.level_count + 1, state.score);
	SDL_Color color = {255, 255, 255, 168};
	draw_text(renderer, font, text_buf, 10, 10, &color);
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
	a = 1.0f - (10.0f - (rand() % 25)) / 100.0f;
	dist = dist_to(source->x, source->y, x, y);

	switch (state.l_type[state.l_sw]) {
		case L_NONE:
			rel_light = 0;
			break;
		case L_LOS:
			if (bresenham(source->x, source->y, x, y, state.level.maze, LVL_W, B_WALL)) {
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

void init_game() {
	state.entities = alist_new(sizeof(entity_t));
	state.enemies = alist_new(sizeof(entity_t));
	state.light_emitters = alist_new(sizeof(entity_t));
	state.event_queue = queue_new(sizeof(event_t));

	state.level.doodads = NULL;
	state.level.maze = NULL;

	state.l_sw = 0;
	state.l_type[0] = L_LOS;
	state.l_type[1] = L_NOLOS;
	state.l_type[2] = L_DISABLED;

	state.player = player_new(1, 1);

	event_dispatch(&state, ev_game_restart);
}
