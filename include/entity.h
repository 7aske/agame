//
// Created by nik on 11/23/19.
//

#ifndef SDLGAME_ENTITY_H
#define SDLGAME_ENTITY_H

enum entities {
	E_NONE,
	E_PLAYER,
	E_LIGHT,
	E_ENEMY,
};

typedef struct player {
} player_t;

typedef struct enemy {
} enemy_t;

typedef struct light {
	float intensity;
} light_t;


typedef struct entity {
	int x;
	int y;
	union {
		player_t player;
		enemy_t enemy;
		light_t light;
	};
	enum entities type;
} entity_t;


#endif //SDLGAME_ENTITY_H
