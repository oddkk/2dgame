#ifndef GAME_CHUNK_CACHE_H
#define GAME_CHUNK_CACHE_H

#include "int.h"
#include "str.h"
#include "game_config.h"
#include "world.h"

#define DEFAULT_CHUNK_NAME STR("default")

struct cached_chunk {
	uint64_t last_use;
	bool present;
	struct chunk chunk;
};

struct texture_map;

struct chunk_cache {
	struct cached_chunk data[CHUNK_CACHE_CAPACITY];
	struct texture_map *texmap;
	uint64_t clock;
};

struct chunk *get_chunk(struct chunk_cache *cache,
						int64_t x, int64_t y, int64_t z);

struct chunk *get_chunk_layer(struct chunk_cache *cache,
							  int64_t x, int64_t y, int64_t z,
							  struct string layer);

#endif
