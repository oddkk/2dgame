#ifndef GAME_RENDER_H
#define GAME_RENDER_H

#include "int.h"

struct texture_map;

struct render_context {
	unsigned int fbo;
	unsigned int fbo_texture;
	unsigned int quad_vao;
	unsigned int quad_vbo;
	unsigned int dither_texture;

	struct _post_shader {
		unsigned int id;
		unsigned int screen_size;
		unsigned int dither;
		unsigned int framebuffer;
		unsigned int framebuffer_size;
		unsigned int framebuffer_screen_size;
	} post_shader;

	struct _sprite_shader {
		unsigned int id;
		unsigned int texture;
		unsigned int position;
		unsigned int screen_size;
		unsigned int tile_size;
	} sprite_shader;

	uint32_t screen_width;
	uint32_t screen_height;
	bool screen_size_changed;

	struct texture_map *texture_map;
};

void render_context_init(struct render_context *ctx);
void render_size_change(struct render_context *ctx, uint32_t width, uint32_t height);

struct world;

void render(struct render_context *ctx, struct world *world);

#endif
