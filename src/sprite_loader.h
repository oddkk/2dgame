#ifndef SPRITE_LOADER_H
#define SPRITE_LOADER_H

#include "int.h"
#include "str.h"

struct sprite {
	uint8_t data[TILE_SIZE][TILE_SIZE][3];
};

bool load_sprite_data_from_file(struct sprite *out,
								unsigned int width, unsigned int height,
								struct string filename);

#endif
