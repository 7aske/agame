#include <stdio.h>
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
#define S_WIDTH (WIDTH / DSIZE)
#define S_HEIGHT (HEIGHT / DSIZE)

#define LVL_W 3 * S_WIDTH
#define LVL_H 2 * S_HEIGHT

#define printt(x, y) printf("(%d, %d)\n", x, y)
#define printlvl(lvl) if(lvl != NULL)\
for (int y = 0; y < LVL_H; ++y){\
for (int x = 0; x < LVL_W; ++x) putc(lvl[y * LVL_W + x], stdout);\
putc('\n', stdout);}\

typedef struct player {
	uint32_t x;
	uint32_t y;
} player_t;

void sig_handler(int);

void sdlerr_handler();

void quit();

void event_handler(SDL_Event*, player_t*);

void player_move(player_t*, SDL_Scancode);

void game_loop(player_t*);

void render_loop(player_t*);

char* generate_lvl();

void generate_rooms(char*, int, int);

char* level0 = NULL;

unsigned char cur_lvl = 0;

volatile __sig_atomic_t running = 1;

//The window we'll be rendering to
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;


int main(int argc, char* args[]) {
	level0 = generate_lvl();

	signal(SIGINT, sig_handler);

	//The surface contained by the window
	SDL_Surface* surface = NULL;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		sdlerr_handler();

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


	player_t player = {1, 1};


	while (running) {

		SDL_RenderClear(renderer);

		render_loop(&player);
		game_loop(&player);


		int xoff = (int) (player.x / S_WIDTH) * S_WIDTH;
		int yoff = (int) (player.y / S_HEIGHT) * S_HEIGHT;
		// printt(player.x, player.y);
		// printt(xoff, yoff);
		for (int y = 0; y < S_HEIGHT; ++y) {
			for (int x = 0; x < S_WIDTH; ++x) {
				SDL_Rect dest;
				SDL_Rect src;
				dest.h = DSIZE;
				dest.w = DSIZE;
				dest.x = DSIZE * x;
				dest.y = DSIZE * y;
				switch (level0[((y + yoff) * LVL_W) + x + xoff]) {
					case '#':
						query_sprite(SPR_WALL, &src);
						break;
					case ' ':
						query_sprite(SPR_FLOOR, &src);
						break;

				}
				SDL_RenderCopy(renderer, tex, &src, &dest);

				if (x + xoff == player.x && y + yoff == player.y) {
					query_sprite(SPR_PLAYER, &src);
					SDL_RenderCopy(renderer, tex, &src, &dest);
				}
			}
		}

		SDL_RenderPresent(renderer);
		SDL_UpdateWindowSurface(window);

		SDL_Delay(1000 / 60);
	}

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
					printf("%d\n", ev->key.keysym.scancode);
					player_move(player, ev->key.keysym.scancode);
			}
			break;
	}
}

void player_move(player_t* player, SDL_Scancode code) {
	switch (code) {
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_UP:
			// if (level0[((player->y - 1) * LVL_W) + player->x] != '#') {
			player->y -= 1;
			// }
			break;
		case SDL_SCANCODE_A:
		case SDL_SCANCODE_LEFT:
			// if (level0[(player->y * LVL_W) + player->x - 1] != '#') {
			player->x -= 1;
			// }
			break;
		case SDL_SCANCODE_S:
		case SDL_SCANCODE_DOWN:
			// if (level0[((player->y + 1) * LVL_W) + player->x] != '#') {
			player->y += 1;
			// }
			break;
		case SDL_SCANCODE_D:
		case SDL_SCANCODE_RIGHT:
			// if (level0[(player->y * LVL_W) + player->x + 1] != '#') {
			player->x += 1;
			// }
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

void render_loop(player_t* player) {

}


char* generate_lvl() {
	int lvl_h = LVL_H;
	int lvl_w = LVL_W;
	char* lvl = (char*) malloc(lvl_h * lvl_w);
	memset(lvl, 35, lvl_h * lvl_w);
	for (int y = 0; y < lvl_h; ++y) {
		for (int x = 0; x < lvl_w; ++x) {
			if (x == 0 || y == 0 || x == lvl_w - 1 || y == lvl_h - 1) {
				lvl[y * lvl_w + x] = 35;
			} else if (y % (S_HEIGHT / 3) == 0 && y % S_HEIGHT != 0) {
				lvl[y * lvl_w + x] = 32;
			} else if (x % (S_WIDTH / 4) == 0 && x % S_WIDTH != 0) {
				lvl[y * lvl_w + x] = 32;
			}
		}
	}
	generate_rooms(lvl, lvl_w, lvl_h);
	return lvl;
}

void generate_rooms(char* lvl, int lvlw, int lvlh) {
	int rwmax = 14;
	int rwmin = 4;
	int rhmax = 14;
	int rhmin = 4;

	int num_rooms = 12;

	// for (int i = 0; i < num_rooms; ++i) {
	// 	int rx = S_WIDTH * (i % 2) + 1;
	// 	int ry = S_HEIGHT * (i % 6) + 1;
	// 	int rw = (rand() % (rwmax - rwmin)) + rwmin;
	// 	int rh = (rand() % (rhmax - rhmin)) + rhmin;
	// 	printt(rx, ry);
	// 	printt(rw, rh);
	// 	for (int y = ry; y < ry + rh; ++y) {
	// 		for (int x = rx; x < rx + rw; ++x) {
	// 			if (x == rx || y == ry || x == rx + rw - 1 || y == ry + rh - 1) {
	// 				lvl[y * lvlw + x] = 35;
	// 			} else {
	// 				lvl[y * lvlw + x] = 32;
	// 			}
	// 		}
	// 	}
	// }


	printlvl(lvl)
}

