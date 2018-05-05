#ifndef SPRITE_LOADER_H
#define SPRITE_LOADER_H

#include "int.h"
#include "str.h"
#include "game_config.h"
#include "idlookuptable.h"

struct sprite {
	uint8_t data[TILE_SIZE][TILE_SIZE][3];
};

enum cached_sprite_state {
	CACHED_SPRITE_UNUSED = 0,
	CACHED_SPRITE_LOADING,
	CACHED_SPRITE_ACTIVE,
};

struct cached_sprite {
	struct string name;
	struct sprite sprite;
	enum cached_sprite_state state;
	uint32_t num_users;
	uint64_t last_use;
};

struct sprite_loader_context {
	struct cached_sprite cache[SPRITE_LOADER_CACHE];
	struct id_lookup_table loaded_sprites;

	// A counter that is incremented every time a resource is acuired.
	uint64_t time;
};

bool load_sprite_data(struct sprite_loader_context *ctx,
					  struct cached_sprite **out,
					  unsigned int width, unsigned int height,
					  struct string sprite_name);

void free_sprite(struct cached_sprite *sprite);

#endif
