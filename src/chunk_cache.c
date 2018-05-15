#include "chunk_cache.h"
#include "chunk_loader.h"
#include "utils.h"
#include "int.h"
#include <limits.h>
#include <stdio.h>

static ssize_t get_chunk_by_coords(struct chunk_cache *cache,
								   int64_t x, int64_t y, int64_t z,
								   struct string layer) {
	for (size_t i = 0; i < CHUNK_CACHE_CAPACITY; i++) {
		if (cache->data[i].chunk.x == x &&
			cache->data[i].chunk.y == y &&
			cache->data[i].chunk.z == z &&
			string_equals(cache->data[i].chunk.layer, layer)) {
			return i;
		}
	}
	return -1;
}

struct chunk *get_chunk_layer(struct chunk_cache *cache,
							  int64_t x, int64_t y, int64_t z,
							  struct string layer) {
	ssize_t id;

	assert(cache->texmap);

	id = get_chunk_by_coords(cache, x, y, z, layer);
	if (id >= 0) {
		cache->data[id].last_use = ++cache->clock;
		if (cache->data[id].present) {
			return &cache->data[id].chunk;
		} else {
			return NULL;
		}
	}

	printf("Load chunk %zi.%zi.%zi.%.*s\n", x, y, z, LIT(layer));

	ssize_t oldest_id = -1;
	size_t oldest_time = ULONG_MAX;

	for (size_t i = 0; i < CHUNK_CACHE_CAPACITY; i++) {
		if (cache->data[i].last_use < oldest_time) {
			oldest_id = i;
			oldest_time = cache->data[i].last_use;
		}
	}

	if (oldest_id < 0) {
		print_error("chunk cache", "No available chunk cache slots.");
		oldest_id = 0;
	}
	struct cached_chunk *chunk;
	chunk = &cache->data[oldest_id];

	chunk->last_use = ++cache->clock;

	if (!load_chunk_layer(&chunk->chunk, cache->texmap, x, y, z, layer)) {
		chunk->present = false;
		return NULL;
	}

	chunk->present = true;

	return &chunk->chunk;
}

struct chunk *get_chunk(struct chunk_cache *cache,
						int64_t x, int64_t y, int64_t z) {
	return get_chunk_layer(cache, x, y, z, DEFAULT_CHUNK_NAME);
}
