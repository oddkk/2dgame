#include "texture_map.h"
#include "render.h"
#include "utils.h"
#include "game_config.h"
#include "opengl.h"
#include "sprite_loader.h"
#include <stdio.h>
#include <stdlib.h>

void texture_map_init(struct texture_map *texmap) {
	texmap->sprite_loader_ctx = calloc(1, sizeof(struct sprite_loader_context));
}

tex_id load_texture_from_memory(struct texture_map *texmap, uint8_t *bitmap, int width, int height) {
	tex_id texture_id;
	assert(width == TILE_SIZE);
	assert(height == TILE_SIZE);

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TILE_SIZE, TILE_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texture_id;
}

tex_id load_sprite(struct texture_map *texmap, struct string assetname) {
	struct cached_sprite *sprite;
	tex_id result;

	if (!load_sprite_data(
			texmap->sprite_loader_ctx, &sprite,
			TILE_SIZE, TILE_SIZE, assetname)) {
		return 0;
	}

	result = load_texture_from_memory(texmap, (uint8_t*)sprite->sprite.data, TILE_SIZE, TILE_SIZE);

	free_sprite(sprite);

	return result;
}
