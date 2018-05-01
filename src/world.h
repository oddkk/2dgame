#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include "int.h"
#include "game_config.h"

struct tile {
	uint32_t tex_id;
};

struct entity {
	int tex_id;

	// Top left of the entity
	int x, y;

	// Size in the world. Used for collition
	uint8_t width, height;
	uint8_t num_tiles_x, num_tiles_y;
};

struct chunk {
	int32_t x, y, z;

	struct tile tilemap[CHUNK_HEIGHT][CHUNK_WIDTH];
};

struct world {
	uint64_t tick;

	// The center of the camera view
	int64_t camera_x, camera_y;
	uint64_t layer;

	struct tile tilemap[WORLD_HEIGHT][WORLD_HEIGHT];
	struct entity active_entities[MAX_ACTIVE_ENTITIES];
	size_t num_active_entities;

	struct chunk *chunks[CHUNK_LOAD_DIAMETER][CHUNK_LOAD_DIAMETER];
};

struct entity *allocate_entity(struct world *world);

#endif
