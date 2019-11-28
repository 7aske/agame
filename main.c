#define SDL_MAIN_HANDLED 1

#include <stdio.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <signal.h>
#include <assert.h>

#include "macro_definitions.h"
#include "draw_misc.h"

#include "state.h"
#include "entity/player.h"
#include "entity/pew.h"
#include "event/event.h"
#include "entity/enemy.h"

#include "sprites.h"
#include "maze.h"
#include "util.h"

#define printd(x) printf("%d\n")
#define printt(x, y) printf("(%d, %d)\n", x, y)

#define COLOR_WHITE (SDL_Color){255, 255, 255, 168}

void sig_handler(int);

void quit();

void sdlerr_handler()__attribute__((noreturn));

void event_handler(SDL_Event*);

void Update(double delta_time);

void Render();

void Event(double delta_time);

float calc_light(entity_t* source, int x, int y, float current_light, state_t* s);

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
					state_change_light(&state, 1);
					break;
				case SDL_SCANCODE_Q:
					state_change_ren(&state, 1);
					break;
				case SDL_SCANCODE_E:
					state_change_ren(&state, -1);
					break;
				case SDL_SCANCODE_K:
					quit();
					break;
				case SDL_SCANCODE_1:
					state.ren_mode = REN_BLOCKS;
					break;
				case SDL_SCANCODE_2:
					state.ren_mode = REN_ENTITIES;
					break;
				case SDL_SCANCODE_3:
					state.ren_mode = REN_SOLUTION;
					break;
				case SDL_SCANCODE_0:
					state.ren_mode = REN_ALL;
					break;
				default:
					break;
			}
			break;
		default:
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
	entity_t* e1;
	entity_t* e2;
	int j, i, jm, im;
	//processing entities
	jm = alist_size(state.entities);
	for (j = 0; j < jm; ++j) {
		e1 = alist_get(state.entities, j);
		switch (e1->type) {
			case E_PEW:
				if (!pew_move(e1, state.level.maze, LVL_W, B_WALL)) {
					alist_rm_idx(state.entities, j);
					j--, jm--;
				} else {
					im = alist_size(state.entities);
					for (i = 0; i < im; ++i) {
						e2 = alist_get(state.entities, i);
						switch (e2->type) {
							case E_ENEMY:
								if (e1->x == e2->x && e1->y == e2->y) {
									e2->hp -= e1->pew.dmg;
									alist_rm_idx(state.entities, j);
									j--, jm--, i = im, im--;
								}
								break;
							default:
								break;
						}

					}
				}
				break;
			case E_ENEMY:
				if (e1->hp <= 0) {
					if (e1->enemy.path != NULL) {
						stack_destroy(e1->enemy.path);
					}
					event_dispatch(&state, ev_score_incr);
					alist_rm_idx(state.entities, j);
					j--, jm--;
				} else {
					enemy_search(e1, &(state.player), state.level.maze, state.level.w, state.level.h,
								 state.level.b_wall, 0);
					enemy_fpath(e1, state.level.maze, state.level.w, B_WALL);
					if (state.player.x == e1->x && state.player.y == e1->y) {
						event_dispatch(&state, ev_game_restart);
					}
				}
				break;
			default:
				break;
		}
	}
	// processing player
	if (state.player.next_move > 0) {
		state.player.next_move--;
	}
	if (state.player.player.next_shot > 0) {
		state.player.player.next_shot--;
	}
	if (state.player.x == state.level.exit_x && state.player.y == state.level.exit_y) {
		event_dispatch(&state, ev_level_next);
	}
}

void Render() {
	#undef lvlxy
	#define lvlxy(ox, oy) state.level.maze[((y + yoff + (oy)) * LVL_W) + (x + xoff + (ox))]
	#define dlvlxy(ox, oy) state.level.doodads[((y + yoff + (oy)) * LVL_W) + (x + xoff + (ox))]
	assert(state.level.doodads != NULL);
	assert(state.level.maze != NULL);
	float light;
	char text_buf[128];
	int i, x, y;
	int xoff = (int) (state.player.x / SCR_W) * SCR_W;
	int yoff = (int) (state.player.y / SCR_H) * SCR_H;
	SDL_Rect dest;
	SDL_Rect src;
	dest.h = BSIZE;
	dest.w = BSIZE;
	for (y = 0; y < SCR_H; ++y) {
		for (x = 0; x < SCR_W; ++x) {
			dest.x = BSIZE * x;
			dest.y = BSIZE * y;

			// rendering lights
			light = 0.0f;
			for (i = 0; i < alist_size(state.light_emitters); ++i) {
				entity_t* source = alist_get(state.light_emitters, i);
				if (dist_to(source->x, source->y, x + xoff, y + yoff) < 10) {
					light = calc_light(source, x + xoff, y + yoff, light, &state);
				}
			}
			light = calc_light(&state.player, x + xoff, y + yoff, light, &state);
			SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);

			if (state.ren_mode == REN_BLOCKS || state.ren_mode == REN_ALL) {

				// rendering background
				switch (lvlxy(0, 0)) {
					case B_WALL:
						load_sprite(SPR_WALL, (spr_rect*) &src);
						break;
					case B_EXIT:
						load_sprite(SPR_DLADDER, (spr_rect*) &src);
						break;
					case B_PATH:
					case B_FLOOR:
						if (lvlxy(0, -1) == B_WALL) {
							load_sprite(SPR_TFLOOR, (spr_rect*) &src);
						} else {
							load_sprite(SPR_FLOOR, (spr_rect*) &src);
						}
						break;

				}
				SDL_RenderCopy(renderer, tex, &src, &dest);

				// rendering doodads
				switch (dlvlxy(0, 0)) {
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
						}
						break;
					case D_SKULL:
						SDL_RenderCopy(renderer, tex, &src, &dest);
						load_sprite(SPR_SKULL, (spr_rect*) &src);
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

			}
			if (state.ren_mode == REN_ENTITIES || state.ren_mode == REN_ALL) {
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
							if (x + xoff == e->x && y + yoff == e->y) {
								load_sprite(SPR_ENEMY1, (spr_rect*) &src);
								SDL_SetTextureColorMod(tex, light, light * 0.8 * (e->hp / E_DEF_HP),
													   light * 0.5 * (e->hp / E_DEF_HP));
								SDL_RenderCopy(renderer, tex, &src, &dest);
								SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);
							}
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
			}
			if (state.ren_mode == REN_SOLUTION || state.ren_mode == REN_ALL) {
				if (lvlxy(0, 0) == B_PATH) {
					load_sprite(SPR_COIN, (spr_rect*) &src);
					SDL_RenderCopy(renderer, tex, &src, &dest);
				}
			}

			// rendering player
			if (x + xoff == state.player.x && y + yoff == state.player.y) {
				load_sprite(SPR_PLAYER, (spr_rect*) &src);
				SDL_RenderCopy(renderer, tex, &src, &dest);
			}

			if (state.ren_mode == REN_BLOCKS || state.ren_mode == REN_ALL) {
				if (lvlxy(0, 1) == B_WALL && (lvlxy(0, 0) == B_FLOOR || lvlxy(0, 0) == B_PATH)) {
					load_sprite(SPR_TWALL, (spr_rect*) &src);
					SDL_RenderCopy(renderer, tex, &src, &dest);
				}
			}
		}
	}
	snprintf(text_buf, 127, "Level: %d | Score: %d", state.level_count + 1, state.score);
	draw_text(renderer, font, text_buf, 10, 10, &(COLOR_WHITE));
	draw_help(renderer, font);

	#undef lvlxy
}


void init_game() {
	entity_t player;
	state.entities = alist_new(sizeof(entity_t));
	state.light_emitters = alist_new(sizeof(entity_t));
	state.event_queue = queue_new(sizeof(event_t));

	state.level.doodads = NULL;
	state.level.maze = NULL;

	state.light = L_AREA;

	player = player_new(1, 1);
	memcpy(&state.player, &player, sizeof(entity_t));

	state.ren_mode = REN_ENTITIES;

	event_dispatch(&state, ev_game_restart);
}

float calc_light(entity_t* source, int x, int y, float current_light, state_t* s) {
	assert(source != NULL);
	float intensity = 3;
	if (source->type == E_LIGHT) {
		intensity = source->light.intensity;
	}
	float rel_light = 0, dist, a;
	a = 1.0f - (50.0f - (rand() % 25)) / 100.0f;
	dist = dist_to(source->x, source->y, x, y);

	switch (s->light) {
		case L_NONE:
			rel_light = 0.0f;
			break;
		case L_LOS:
			if (bresenham(source->x, source->y, x, y, s->level.maze, s->level.w, s->level.b_wall)) {
				rel_light = 255.0f / (dist / intensity) * a;
				rel_light = rel_light > 255 ? 255 : rel_light;
			} else {
				rel_light = 170.0f / dist * a;
				rel_light = rel_light > 170 ? 170 : rel_light;
			}
			break;
		case L_AREA:
			rel_light = 255.0f / (dist / intensity) * a;
			if (rel_light > 255) {
				rel_light = 255;
			} else if (rel_light < 40) {
				rel_light = 0;
			}

			break;
		case L_ALL:
			rel_light = 255.0f;
			break;
	}
	return fmaxf(rel_light, current_light);
}
