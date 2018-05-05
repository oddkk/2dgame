#ifndef SPRITE_LOADER_H
#define SPRITE_LOADER_H

#include "int.h"
#include "str.h"

bool load_sprite_data_from_file(uint8_t *out,
								unsigned int width, unsigned int height,
								struct string filename);

#endif
