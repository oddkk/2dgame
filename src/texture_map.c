#include "texture_map.h"
#include "render.h"
#include "utils.h"
#include "game_config.h"
#include "opengl.h"
#include "tile_loader.h"
#include <stdio.h>

void bitmap_to_rgb(uint16_t *bitmap, uint8_t *out, uint8_t r, uint8_t g, uint8_t b) {
	// TODO: Make this general instead of relying on the tile size to
	// be 16.
	for (size_t i = 0; i < 16*16; ++i) {
		uint16_t data = bitmap[i / 16];
		if (data & (1 << (i % 16))) {
			out[i*3+0] = r;
			out[i*3+1] = g;
			out[i*3+2] = b;
		} else {
			out[i*3+0] = 0;
			out[i*3+1] = 0;
			out[i*3+2] = 0;
		}
	}
}

void texture_map_init(struct texture_map *texmap) {(void)texmap;}

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

tex_id load_bitmap(struct texture_map *texmap, uint16_t *bitmap, uint8_t r, uint8_t g, uint8_t b) {
	uint8_t sprite_color_data[3*16*16] = {};
	bitmap_to_rgb(bitmap, sprite_color_data, r, g, b);
	return load_texture_from_memory(texmap, sprite_color_data, 16, 16);
}

tex_id load_dither(struct texture_map *texmap, uint64_t *bitmap) {
	uint8_t out[3*16*16] = {};
	for (size_t i = 0; i < 16*16; ++i) {
		uint64_t data = bitmap[i / 16];
		uint8_t cell = (data >> ((16 - (i % 16)) * 4)) & 0xf;
		uint8_t value = ((uint32_t)cell * 255) / 0xf;

		out[i*3+0] = value;
		out[i*3+1] = value;
		out[i*3+2] = value;
	}

	return load_texture_from_memory(texmap, out, 16, 16);
}

tex_id load_texture_from_file(struct texture_map *texmap,
							  struct string filename) {
	uint8_t buffer[TILE_SIZE*TILE_SIZE*3] = {};

	if (!load_tile_data_from_file(buffer, TILE_SIZE, TILE_SIZE, filename)) {
		return 0;
	}

	return load_texture_from_memory(texmap, buffer, TILE_SIZE, TILE_SIZE);
}

tex_id load_tile(struct texture_map *texmap, struct string assetname) {
	char buffer[256];
	struct string filename;
	int ret;

	filename.data = (uint8_t *)buffer;

	ret = snprintf(buffer, sizeof(buffer),
				   "assets/%.*s.tile", LIT(assetname));

	if (ret < 0) {
		perror("snprintf");
		return false;
	}

	filename.length = ret;

	return load_texture_from_file(texmap, filename);
}
