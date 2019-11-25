//
// Created by nik on 11/25/19.
//

#ifndef AGAME_PLAYER_H
#define AGAME_PLAYER_H

#pragma once

#include "entity.h"
#include "../../internal/structs/arraylist.h"

extern entity_t player_new(int x, int y);

extern void player_shoot(entity_t* e, alist_t* entities);


#endif //AGAME_PLAYER_H
