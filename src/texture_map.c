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

	glGenTextures(1, &texmap->map_id);
	glBindTexture(GL_TEXTURE_2D, texmap->map_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
				 TILE_SIZE * TEXTURE_MAP_CAPACITY_SQUARE_ROOT,
				 TILE_SIZE * TEXTURE_MAP_CAPACITY_SQUARE_ROOT,
				 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	texmap->textures[0].num_users = -1;
}

static tex_id alloc_texture_slot(struct texture_map *texmap,
								 struct string name) {
	size_t n_iterations = 0;

	while (n_iterations < 2*TEXTURE_MAP_CAPACITY) {
		struct texture_map_texture *tex;
		tex_id id;
		id = texmap->clock_head++;
		texmap->clock_head %= TEXTURE_MAP_CAPACITY;

		tex = &texmap->textures[id];

		if (tex->num_users == 0 && !tex->referenced) {
			tex->num_users = 1;
			tex->referenced = true;
			tex->name = name;
			return id;
		}

		tex->referenced = false;
		n_iterations += 1;
	}

	print_error("texture map", "No texture slots available.");

	return TEX_NONE;
}

struct uvec2 {
	unsigned int x, y;
};

static struct uvec2 position_for_texture(tex_id id) {
	struct uvec2 result;

	result.x = (id % TEXTURE_MAP_CAPACITY_SQUARE_ROOT) * TILE_SIZE;
	result.y = (id / TEXTURE_MAP_CAPACITY_SQUARE_ROOT) * TILE_SIZE;

	return result;
}

tex_id load_texture_from_memory(struct texture_map *texmap, struct string name, uint8_t *bitmap, int width, int height) {
	tex_id texture_id;
	struct uvec2 texture_position;

	assert(width == TILE_SIZE);
	assert(height == TILE_SIZE);

	int lookup_result;
	lookup_result = id_lookup_table_lookup(&texmap->loaded_textures, name);
	if (lookup_result >= 0) {
		return lookup_result;
	}

	texture_id = alloc_texture_slot(texmap, name);
	texture_position = position_for_texture(texture_id);

	glBindTexture(GL_TEXTURE_2D, texmap->map_id);

	glTexSubImage2D(GL_TEXTURE_2D, 0,
					texture_position.x, texture_position.y,
					TILE_SIZE, TILE_SIZE,
					GL_RGB, GL_UNSIGNED_BYTE, bitmap);

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

	result = load_texture_from_memory(texmap, assetname, (uint8_t*)sprite->sprite.data, TILE_SIZE, TILE_SIZE);

	free_sprite(sprite);

	return result;
}
