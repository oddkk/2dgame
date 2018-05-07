#include "render.h"
#include "opengl.h"
#include "utils.h"
#include "game_config.h"
#include "texture_map.h"
#include "world.h"

#define FB_WIDTH (TILE_SIZE * SCREEN_TILES_WIDTH)
#define FB_HEIGHT (TILE_SIZE * SCREEN_TILES_HEIGHT)

static const float quad_indecies[] = {
	-1, -1, 0, 0,
	-1,  1, 0, 1,
	 1,  1, 1, 1,

	-1, -1, 0, 0,
	 1, -1, 1, 0,
	 1,  1, 1, 1,
};

static void _initialize_framebuffer(struct render_context *ctx) {
	glGenTextures(1, &ctx->fbo_texture);
	glBindTexture(GL_TEXTURE_2D, ctx->fbo_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FB_WIDTH, FB_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &ctx->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, ctx->fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctx->fbo_texture, 0);

	const GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		panic("opengl", "Failed to initialize a framebuffer!");
		return;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

static void _initialize_framebuffer_shader(struct render_context *ctx) {
	GLuint vsh = create_shader_from_file("assets/shaders/framebuffer.vsh", GL_VERTEX_SHADER);
	GLuint fsh = create_shader_from_file("assets/shaders/framebuffer.fsh", GL_FRAGMENT_SHADER);

	ctx->post_shader.id = glCreateProgram();

	glAttachShader(ctx->post_shader.id, vsh);
	glAttachShader(ctx->post_shader.id, fsh);

	if (!link_shader_program(ctx->post_shader.id)) {
		panic("opengl", "Failed to compile shader");
		return;
	}

	ctx->post_shader.dither = glGetUniformLocation(ctx->post_shader.id, "in_dither");
	ctx->post_shader.framebuffer = glGetUniformLocation(ctx->post_shader.id, "in_framebuffer");
	ctx->post_shader.framebuffer_size = glGetUniformLocation(ctx->post_shader.id, "in_framebuffer_size");
	ctx->post_shader.framebuffer_screen_size = glGetUniformLocation(ctx->post_shader.id, "in_framebuffer_screen_size");
	ctx->post_shader.screen_size = glGetUniformLocation(ctx->post_shader.id, "in_screen_size");

	glUseProgram(ctx->post_shader.id);
	glUniform1i(ctx->post_shader.framebuffer, 0);
	glUniform2f(ctx->post_shader.framebuffer_size, (float)FB_WIDTH, (float)FB_HEIGHT);
	glUniform1i(ctx->post_shader.dither, 2);
	glUseProgram(0);

	ctx->screen_size_changed = true;
}

static void _initialize_sprite_shader(struct render_context *ctx) {
	GLuint vsh = create_shader_from_file("assets/shaders/sprite.vsh", GL_VERTEX_SHADER);
	GLuint fsh = create_shader_from_file("assets/shaders/sprite.fsh", GL_FRAGMENT_SHADER);

	ctx->sprite_shader.id = glCreateProgram();

	glAttachShader(ctx->sprite_shader.id, vsh);
	glAttachShader(ctx->sprite_shader.id, fsh);

	if (!link_shader_program(ctx->sprite_shader.id)) {
		panic("opengl", "Failed to compile shader");
		return;
	}

	ctx->sprite_shader.screen_size = glGetUniformLocation(ctx->sprite_shader.id, "in_screen_size");
	ctx->sprite_shader.tile_size = glGetUniformLocation(ctx->sprite_shader.id, "in_tile_size");
	ctx->sprite_shader.position = glGetUniformLocation(ctx->sprite_shader.id, "in_position");
	ctx->sprite_shader.texture = glGetUniformLocation(ctx->sprite_shader.id, "in_texture");

	glUseProgram(ctx->sprite_shader.id);
	glUniform1i(ctx->sprite_shader.texture, 0);
	glUniform2f(ctx->sprite_shader.screen_size, (float)FB_WIDTH, (float)FB_HEIGHT);
	glUniform2f(ctx->sprite_shader.tile_size, (float)TILE_SIZE, (float)TILE_SIZE);
	glUseProgram(0);
}

static void _initialize_quad(struct render_context *ctx) {
	glGenVertexArrays(1, &ctx->quad_vao);
	glBindVertexArray(ctx->quad_vao);

	glGenBuffers(1, &ctx->quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->quad_vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_indecies), quad_indecies, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void render_context_init(struct render_context *ctx) {
	_initialize_framebuffer(ctx);
	_initialize_quad(ctx);
	_initialize_framebuffer_shader(ctx);
	_initialize_sprite_shader(ctx);
}

void render_size_change(struct render_context *ctx, uint32_t width, uint32_t height) {
	ctx->screen_width  = width;
	ctx->screen_height = height;
	ctx->screen_size_changed = true;

	glViewport(0, 0, width, height);
}

void render_sprite(struct render_context *ctx, unsigned int sprite, int32_t x, int32_t y) {
	glBindVertexArray(ctx->quad_vao);

	glUseProgram(ctx->sprite_shader.id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sprite);

	glUniform2f(ctx->sprite_shader.position, (float)x, (float)y);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

static void render_chunk_tilemap(struct render_context *ctx,
								 struct chunk *chunk,
								 int64_t cam_left, int64_t cam_top) {
	struct box cam_tiles;
	struct box chunk_tiles;
	struct box visible_tiles;

	cam_tiles = box_from_width_height(
		cam_left / TILE_SIZE - 1,
		cam_top  / TILE_SIZE - 1,
		SCREEN_TILES_WIDTH  + 2,
		SCREEN_TILES_HEIGHT + 2
	);

	chunk_tiles = box_from_width_height(
		chunk->x * CHUNK_WIDTH,
		chunk->y * CHUNK_HEIGHT,
		CHUNK_WIDTH,
		CHUNK_HEIGHT
	);

	if (!box_intersect(&visible_tiles, cam_tiles, chunk_tiles)) {
		// Nothing to render
		return;
	}

	for (int64_t y = visible_tiles.top; y < visible_tiles.bottom; y++) {
		for (int64_t x = visible_tiles.left; x < visible_tiles.right; x++) {
			struct tile *tile;
			int64_t screen_x, screen_y;
			int64_t chunk_tile_x, chunk_tile_y;

			chunk_tile_x = (x - chunk_tiles.left);
			chunk_tile_y = (y - chunk_tiles.top);
			screen_x = x * TILE_SIZE - cam_left;
			screen_y = y * TILE_SIZE - cam_top;

			tile = &chunk->tilemap[chunk_tile_y][chunk_tile_x];
			render_sprite(ctx, tile->tex_id, screen_x, screen_y);
		}
	}
}

void render_tilemap(struct render_context *ctx, struct world *world) {
	int64_t cam_left = world->camera_x - (SCREEN_TILES_WIDTH  * TILE_SIZE) / 2;
	int64_t cam_top  = world->camera_y - (SCREEN_TILES_HEIGHT * TILE_SIZE) / 2;

	for (int y = 0; y < CHUNK_LOAD_DIAMETER; y++) {
		for (int x = 0; x < CHUNK_LOAD_DIAMETER; x++) {
			if (world->chunks[y][x]) {
				render_chunk_tilemap(ctx, world->chunks[y][x], cam_left, cam_top);
			}
		}
	}
}

void render_entities(struct render_context *ctx, struct world *world) {
	int64_t screen_width  = (SCREEN_TILES_WIDTH  * TILE_SIZE);
	int64_t screen_height = (SCREEN_TILES_HEIGHT * TILE_SIZE);

	int64_t cam_left   = world->camera_x - screen_width  / 2;
	int64_t cam_top    = world->camera_y - screen_height / 2;

	for (size_t i = 0; i < world->num_active_entities; i++) {
		struct entity *entity = &world->active_entities[i];
		int64_t screen_x = entity->x - cam_left;
		int64_t screen_y = entity->y - cam_top;
		if ((screen_x < screen_width &&
			 screen_x + entity->num_tiles_x * TILE_SIZE >= 0) ||
			(screen_y < screen_height ||
			 screen_y + entity->num_tiles_y * TILE_SIZE >= 0)) {
			render_sprite(ctx, entity->tex_id, screen_x, screen_y);
		}
	}
}

void render(struct render_context *ctx, struct world *world) {
	glBindFramebuffer(GL_FRAMEBUFFER, ctx->fbo);
	glViewport(0, 0, FB_WIDTH, FB_HEIGHT);
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Render game
	render_tilemap(ctx, world);
	render_entities(ctx, world);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, ctx->screen_width, ctx->screen_height);

	// Render framebuffer
	glUseProgram(ctx->post_shader.id);

	if (ctx->screen_size_changed) {
		// framebuffer-screensize
		float fb_ss_width, fb_ss_height;
		float fb_aspect, ss_aspect;

		fb_aspect = (float)FB_WIDTH / (float)FB_HEIGHT;
		ss_aspect = (float)ctx->screen_width / (float)ctx->screen_height;

		if (fb_aspect >= ss_aspect) {
			fb_ss_width = ctx->screen_width;
			fb_ss_height = ctx->screen_width / fb_aspect;
		} else {
			fb_ss_width = ctx->screen_height * fb_aspect;
			fb_ss_height = ctx->screen_height;
		}

		fb_ss_width  /= ctx->screen_width;
		fb_ss_height /= ctx->screen_height;

		glUniform2f(ctx->post_shader.screen_size, (float)ctx->screen_width, (float)ctx->screen_height);
		glUniform2f(ctx->post_shader.framebuffer_screen_size, fb_ss_width, fb_ss_height);
		ctx->screen_size_changed = false;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(ctx->quad_vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ctx->fbo_texture);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ctx->dither_texture);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
