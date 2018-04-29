#ifndef TILE_LOADER_H
#define TILE_LOADER_H

#include "int.h"
#include "str.h"

bool load_tile_data_from_file(uint8_t *out,
							  unsigned int width, unsigned int height,
							  struct string filename);

#endif
