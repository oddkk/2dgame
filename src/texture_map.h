#ifndef GAME_TEXTURE_MAP_H
#define GAME_TEXTURE_MAP_H

#include "int.h"
#include "str.h"

typedef unsigned int tex_id;

struct texture_map {
};

void bitmap_to_rgb(uint16_t *bitmap, uint8_t *out, uint8_t r, uint8_t g, uint8_t b);
void texture_map_init(struct texture_map *texmap);
tex_id load_texture_from_memory(struct texture_map *texmap, uint8_t *bitmap, int width, int height);
tex_id load_texture_from_file(struct texture_map *texmap, struct string filename);
tex_id load_tile(struct texture_map *texmap, struct string assetname);
tex_id load_bitmap(struct texture_map *texmap, uint16_t *bitmap, uint8_t r, uint8_t g, uint8_t b);

tex_id load_dither(struct texture_map *texmap, uint64_t *bitmap);


#endif
