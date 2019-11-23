#define SDL_MAIN_HANDLED 1

#include <stdio.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <signal.h>
#include <assert.h>

#include "macro_definitions.h"

#include "sprites.h"
#include "maze.h"
#include "util.h"

#define printd(x) printf("%d\n")
#define printt(x, y) printf("(%d, %d)\n", x, y)

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

void restart_level(player_t*);

void draw_fps(int const*);

void draw_help();

char* level = NULL;
char* doodads = NULL;
char* solution = NULL;
int exit_x = -1;
int exit_y = -1;

volatile int running = 1;
volatile char l_sw = 0;
volatile char l_type[] = {1, 2, 3};

//The window we'll be rendering to
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t render_flags = SDL_RENDERER_ACCELERATED;
TTF_Font* font;

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

	surface = IMG_Load("res/0x72_16x16DungeonTileset.v1.png");
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	if (!tex)
		sdlerr_handler();

	font = TTF_OpenFont("res/DejaVuSans-Bold.ttf", 24);

	if (!font)
		fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());

	level = generate_level(&exit_x, &exit_y);
	doodads = generate_doodads(level);


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
	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
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
					solve(level, &solution, exit_x, exit_y);
					overlay_solution(level, solution);
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

void player_move(player_t* player, SDL_Scancode code) {
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

void game_loop(player_t* player) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		event_handler(&event, player);
	}

	if (player->x == exit_x && player->y == exit_y) {
		restart_level(player);
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

			light = 255;
			light_amp = 3;
			a = 1 - (10 - (rand() % 25)) / 100.0f;
			dist = dist_to(player->x, player->y, x + xoff, y + yoff);
			if (l_type[l_sw] == 1) {
				if (bresenham(player->x, player->y, x + xoff, y + yoff, level, LVL_W, B_WALL)) {
					// calculate lighting depending on distance to the player (only light source)
					light = 255.0f / (dist / light_amp) * a;
					light = light > 255 ? 255 : light;
				} else {
					light = 196.0f / dist * a;
					light = light > 196 ? 196 : light;
				}

			} else if (l_type[l_sw] == 2) {
				light = 255.0f / (dist / light_amp) * a;
				light = light > 255 ? 255 : light;
			} else if (l_type[l_sw] == 3) {
				light = 255;
			}

			SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);

			switch (level[((y + yoff) * LVL_W) + x + xoff]) {
				case B_WALL:
					load_sprite(SPR_WALL, &src);
					break;
				case B_FLOOR:
					if (level[((y + yoff - 1) * LVL_W) + x + xoff] == B_WALL) {
						load_sprite(SPR_TFLOOR, &src);
					} else {
						load_sprite(SPR_FLOOR, &src);
					}
					if (level[((y + yoff + 1) * LVL_W) + x + xoff] == B_WALL) {
						SDL_RenderCopy(renderer, tex, &src, &dest);
						load_sprite(SPR_TWALL, &src);
					}
					break;
				case SOL_PATH:
					if (level[((y + yoff - 1) * LVL_W) + x + xoff] == B_WALL) {
						load_sprite(SPR_TFLOOR, &src);
					} else {
						load_sprite(SPR_FLOOR, &src);
					}
					if (level[((y + yoff + 1) * LVL_W) + x + xoff] == B_WALL) {
						SDL_RenderCopy(renderer, tex, &src, &dest);
						load_sprite(SPR_TWALL, &src);
					}
					SDL_RenderCopy(renderer, tex, &src, &dest);
					load_sprite(SPR_COIN, &src);
					break;

			}
			switch (doodads[((y + yoff) * LVL_W) + x + xoff]) {
				case D_BRICK:
					load_sprite(SPR_MBRICK, &src);
					break;
				case D_GRATE:
					if (level[((y + yoff + 1) * LVL_W) + x + xoff] != B_WALL) {
						load_sprite(SPR_GRATE, &src);
					}
					break;
				case D_OOZE:
					if (level[((y + yoff - 1) * LVL_W) + x + xoff] == B_WALL) {
						load_sprite(SPR_OOZE, &src);
						dest.y -= BSIZE;
						SDL_RenderCopy(renderer, tex, &src, &dest);
						load_sprite(SPR_OOZEF, &src);
						dest.y += BSIZE;
						if (level[((y + yoff) * LVL_W) + x + xoff] == B_FLOOR &&
							level[((y + yoff + 1) * LVL_W) + x + xoff] == B_WALL) {
							SDL_RenderCopy(renderer, tex, &src, &dest);
							load_sprite(SPR_TWALL, &src);
						}
					}
					break;
				case D_SKULL:
					SDL_RenderCopy(renderer, tex, &src, &dest);
					load_sprite(SPR_SKULL, &src);
					if (level[((y + yoff + 1) * LVL_W) + x + xoff] == B_WALL) {
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
			}
			SDL_RenderCopy(renderer, tex, &src, &dest);

			if (x + xoff == player->x && y + yoff == player->y) {
				load_sprite(SPR_PLAYER, &src);
				SDL_SetTextureColorMod(tex, 255, 255, 255);
				SDL_RenderCopy(renderer, tex, &src, &dest);

				if (level[((y + yoff + 1) * LVL_W) + x + xoff] == B_WALL &&
					(level[((y + yoff) * LVL_W) + x + xoff] == B_FLOOR ||
					 level[((y + yoff) * LVL_W) + x + xoff] == SOL_PATH)) {
					load_sprite(SPR_TWALL, &src);
					SDL_SetTextureColorMod(tex, light, light * 0.8, light * 0.5);
					SDL_RenderCopy(renderer, tex, &src, &dest);
				}
			}
		}
	}
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

	SDL_FreeSurface(surfaceMessage1);
	SDL_FreeSurface(surfaceMessage2);
	SDL_FreeSurface(surfaceMessage3);
	SDL_FreeSurface(surfaceMessage4);

	SDL_DestroyTexture(message1);
	SDL_DestroyTexture(message2);
	SDL_DestroyTexture(message3);
	SDL_DestroyTexture(message4);
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
	SDL_FreeSurface(surfaceMessage);
	SDL_DestroyTexture(message);
}

void restart_level(player_t* player) {
	free(level);
	level = generate_level(&exit_x, &exit_y);
	free(doodads);
	doodads = generate_doodads(level);
	// while (level[(player->y = rand() % LVL_H) * LVL_W + (player->x = rand() % LVL_W)] == B_WALL);
	player->x = 1;
	player->y = 1;
}
