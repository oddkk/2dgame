#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include "int.h"
#include "game_config.h"

#define SPRITE_MAP_SIZE 256

struct sprite_map_entry {
	uint32_t tex_id;
	uint32_t num_uses;
};

struct sprite_map {
	struct sprite_map_entry sprites[SPRITE_MAP_SIZE];
};

#endif
