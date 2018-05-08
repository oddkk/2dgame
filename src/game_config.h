#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#define TILE_SIZE 16
#define WORLD_WIDTH 100
#define WORLD_HEIGHT 100

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 9

#define SCREEN_TILES_WIDTH 16
#define SCREEN_TILES_HEIGHT 9

#define MAX_ACTIVE_ENTITIES 256

#define MAX_TEXTURES 256

#define CHUNK_LOAD_DIAMETER 3

#define SPRITE_LOADER_CACHE 64

// The texture map capacity is expressed as the number of sprites
// along a side of a square.
#define TEXTURE_MAP_CAPACITY_SQUARE_ROOT 64
#define TEXTURE_MAP_CAPACITY (TEXTURE_MAP_CAPACITY_SQUARE_ROOT*TEXTURE_MAP_CAPACITY_SQUARE_ROOT)

#endif
