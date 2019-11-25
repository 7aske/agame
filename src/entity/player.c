//
// Created by nik on 11/25/19.
//

#include "entity/player.h"

void player_shoot(entity_t* e, alist_t* entities) {
	assert(e != (void*) 0);
	assert(e->type == E_PLAYER);

	entity_t newpew;
	newpew.x = e->x;
	newpew.y = e->y;
	newpew.hp = 1;
	newpew.type = E_PEW;
	newpew.pew.dir = e->player.dir;
	newpew.pew.dmg = e->player.dmg;
	alist_add(entities, &newpew);
}

entity_t player_new(int x, int y) {
	entity_t newplayer;
	newplayer.x = x;
	newplayer.y = y;
	newplayer.hp = DEFAULT_HP;
	newplayer.player.dmg = DEFAULT_DMG;
	newplayer.player.dir = DIR_DOWN;
	newplayer.type = E_PLAYER;
	printf("%p\n", &newplayer);
	return newplayer;
}
