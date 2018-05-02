#ifndef CHUNK_LOADER_H
#define CHUNK_LOADER_H

#include "int.h"
#include "str.h"
#include "world.h"

struct texture_map;

bool load_chunk(struct chunk *out, struct texture_map *, int64_t x, int64_t y, int64_t z);

#endif
