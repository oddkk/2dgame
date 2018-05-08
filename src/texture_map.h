#ifndef GAME_TEXTURE_MAP_H
#define GAME_TEXTURE_MAP_H

#include "int.h"
#include "str.h"
#include "game_config.h"
#include "idlookuptable.h"

typedef unsigned int tex_id;
#define TEX_NONE 0

struct sprite_loader_context;

struct texture_map_texture {
	struct string name;
	uint64_t num_users;
	bool referenced;
};

struct texture_map {
	struct sprite_loader_context *sprite_loader_ctx;

	// The OpenGL texture id of this map.
	unsigned int map_id;

	// A counter that is incremented every time a texture is aquired.
	uint64_t time;
	size_t clock_head;

	struct id_lookup_table loaded_textures;
	struct texture_map_texture textures[TEXTURE_MAP_CAPACITY];
};

void bitmap_to_rgb(uint16_t *bitmap, uint8_t *out, uint8_t r, uint8_t g, uint8_t b);
void texture_map_init(struct texture_map *texmap);
tex_id load_texture_from_memory(struct texture_map *texmap, struct string name, uint8_t *bitmap, int width, int height);
tex_id load_sprite(struct texture_map *texmap, struct string assetname);

#endif
