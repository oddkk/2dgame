#include "sprite_loader.h"
#include "config_parser.h"
#include "game_config.h"
#include "utils.h"
#include <string.h>
#include <limits.h>

enum scale_mode {
	SCALE_NONE,
	SCALE_REPEAT,
	SCALE_STRETCH,
	SCALE_CENTER,
};

enum palette_entry_type {
	PALETTE_UNSET = 0,
	PALETTE_COLOR,
	PALETTE_SAMPLER,
};

static bool load_sprite_data_from_file(struct sprite_loader_context *ctx,
									   struct cached_sprite *sprite,
									   unsigned int width, unsigned int height,
									   struct string filename);

static struct cached_sprite *alloc_sprite(struct sprite_loader_context *ctx,
										  struct string name) {
	struct cached_sprite *sprite;
	uint64_t min_last_use = ULONG_MAX;
	uint64_t min_last_use_id = 0;
	bool found = false;

	for (size_t i = 0; i < SPRITE_LOADER_CACHE; i++) {
		if (ctx->cache[i].num_users == 0 && ctx->cache[i].last_use < min_last_use) {
			min_last_use = ctx->cache[i].last_use;
			min_last_use_id = i;
			found = true;
		}
	}

	if (!found) {
		print_error("sprite", "The sprite cache table is full!");
		min_last_use_id = 0;
	}

	sprite = &ctx->cache[min_last_use_id];

	if (sprite->state != CACHED_SPRITE_UNUSED) {
		id_lookup_table_delete(&ctx->loaded_sprites, sprite->name);
	}

	sprite->last_use = ++ctx->time;
	sprite->num_users = 1;
	sprite->name = name;
	memset(sprite->sprite.data, 0, sizeof(sprite->sprite.data));


	id_lookup_table_insert(&ctx->loaded_sprites, sprite->name, min_last_use_id);

	return sprite;
}

bool load_sprite_data(struct sprite_loader_context *ctx,
					  struct cached_sprite **out,
					  unsigned int width, unsigned int height,
					  struct string sprite_name) {
	char buffer[256];
	struct string filename;
	int cache_id;
	int ret;

	cache_id = id_lookup_table_lookup(&ctx->loaded_sprites, sprite_name);
	if (cache_id >= 0) {
		ctx->cache[cache_id].num_users += 1;
		ctx->cache[cache_id].last_use = ++ctx->time;
		*out = &ctx->cache[cache_id];

		print_debug("Loading sprite (cache) '%.*s'.", LIT(sprite_name));
		return true;
	}

	filename.data = (uint8_t *)buffer;

	ret = snprintf(buffer, sizeof(buffer),
				   "assets/%.*s.sprite", LIT(sprite_name));

	if (ret < 0) {
		perror("snprintf");
		return false;
	}

	filename.length = ret;

	*out = alloc_sprite(ctx, sprite_name);

		print_debug("Loading sprite (file)  '%.*s'.", LIT(sprite_name));

	return load_sprite_data_from_file(ctx, *out, width, height, filename);
}

void free_sprite(struct cached_sprite *sprite) {
	if (sprite->num_users == 0) {
		print_error("sprite", "Sprite '%.*s' was attempted freed more times than it was allocated.",
					sprite->name);
		return;
	}

	sprite->num_users -= 1;
}

static bool load_sprite_data_from_file(struct sprite_loader_context *ctx,
									   struct cached_sprite *sprite,
									   unsigned int width, unsigned int height,
									   struct string filename) {
	struct config_file cfg = {};
	struct {
		enum palette_entry_type type;
		union {
			struct {
				uint8_t r, g, b;
			} color;
			struct cached_sprite *sprite;
		};
	} palette[255] = {};
	uint64_t sprite_width, sprite_height;
	bool version_specified = false;
	bool size_specified = false;
	enum scale_mode scale = SCALE_NONE;
	struct sprite *sprite_out = &sprite->sprite;

	assert(width == TILE_SIZE);
	assert(height == TILE_SIZE);

	cfg.file = filename;
	cfg.fd = fopen((char*)filename.data, "rb");

	palette[' '].type = PALETTE_COLOR;

	if (!cfg.fd) {
		print_error("sprite", "Could not open sprite file %.*s.", LIT(filename));
		perror("fopen");
		return false;
	}

	while (config_next_line(&cfg)) {
		struct string token;

		config_eat_token(&cfg, &token);

		if (string_equals(token, STR("version"))) {
			struct string version;

			if (version_specified) {
				config_print_error(&cfg, "Version already specified.");
				continue;
			}

			config_eat_token(&cfg, &version);

			if (!string_equals(version, STR("0.1"))) {
				config_print_error(&cfg, "Expected version 0.1, got %.*s.", LIT(version));
				return false;
			}

			version_specified = true;
		} else if (string_equals(token, STR("size"))) {
			bool ok = true;

			if (size_specified) {
				config_print_error(&cfg, "Size already specified.");
				continue;
			}
			ok &= config_eat_token_uint(&cfg, &sprite_width);
			ok &= config_eat_token_uint(&cfg, &sprite_height);

			size_specified = true;
		} else if (string_equals(token, STR("color"))) {
			struct string ident;
			uint64_t r, g, b;

			if (!config_eat_token(&cfg, &ident)) {
				config_print_error(&cfg, "Expected character for palette entry.");
				continue;
			}

			if (ident.length > 1) {
				config_print_error(&cfg, "Expected single ascii character for palette entry.");
				continue;
			}

			if (config_count_remaining_tokens(&cfg) == 3) {
				bool ok = true;

				ok &= config_eat_token_uint(&cfg, &r);
				ok &= config_eat_token_uint(&cfg, &g);
				ok &= config_eat_token_uint(&cfg, &b);

				ok &= (r <= 255 && g <= 255 && b <= 255);

				if (!ok) {
					config_print_error(&cfg, "Expected rgb values for color in the range 0-255.");
					continue;
				}

				// #ff00ff is used for transparancy, so remap this
				// color to avoid unintentional transparancy.
				if (r == 0xff && g == 0 && b == 0xff) {
					r = 0xfe;
					b = 0x00;
					b = 0xfe;
				}
			} else {
				struct string name;

				if (!config_eat_token(&cfg, &name)) {
					config_print_error(&cfg, "Expected r g b color value, or 'transparant'.", LIT(name));
					continue;
				}

				if (string_equals(name, STR("transparant"))) {
					r = 0xff;
					g = 0x00;
					b = 0xff;
				} else {
					config_print_error(&cfg, "Got unexpected color '%.*s'", LIT(name));
					continue;
				}
			}

			if (palette[ident.data[0]].type != PALETTE_UNSET) {
				config_print_error(&cfg, "Palette key %c already specified.", ident.data[0]);
				continue;
			}

			palette[ident.data[0]].color.r = r;
			palette[ident.data[0]].color.g = g;
			palette[ident.data[0]].color.b = b;
			palette[ident.data[0]].type = PALETTE_COLOR;

		} else if (string_equals(token, STR("sample"))) {
			// Usage: sample sprite IDENT ASSET

			struct string asset_type;
			struct string ident;
			struct string asset;

			if (!config_eat_token(&cfg, &asset_type)) {
				config_print_error(&cfg, "Expected type of asset to sample, 'sprite'.");
				continue;
			}

			if (!string_equals(asset_type, STR("sprite"))) {
				config_print_error(&cfg, "Expected type of asset to sample, 'sprite'.");
				continue;
			}

			if (!config_eat_token(&cfg, &ident)) {
				config_print_error(&cfg, "Expected character for palette entry.");
				continue;
			}

			if (ident.length > 1) {
				config_print_error(&cfg, "Expected single ascii character for palette entry.");
				continue;
			}

			if (!config_eat_token(&cfg, &asset)) {
				config_print_error(&cfg, "Expected name of asset to sample.");
				continue;
			}

			if (palette[ident.data[0]].type != PALETTE_UNSET) {
				config_print_error(&cfg, "Palette key %c already specified.", ident.data[0]);
				continue;
			}

			load_sprite_data(ctx, &palette[ident.data[0]].sprite,
							 width, height, asset);
			palette[ident.data[0]].type = PALETTE_SAMPLER;

		} else if (string_equals(token, STR("scale"))) {
			struct string mode;

			if (!config_eat_token(&cfg, &mode)) {
				config_print_error(&cfg, "Expected a scale type after 'scale'.");
				continue;
			}

			if (string_equals(mode, STR("repeat"))) {
				scale = SCALE_REPEAT;
			} else if (string_equals(mode, STR("stretch"))) {
				scale = SCALE_STRETCH;
			} else if (string_equals(mode, STR("center"))) {
				scale = SCALE_CENTER;
			} else {
				config_print_error(&cfg, "Got unexpected scale type '%.*s'.", LIT(mode));
			}

		} else if (string_equals(token, STR("frame"))) {
			// TODO: Implement frames
		} else if (string_equals(token, STR("data"))) {
			if (!size_specified) {
				config_print_error(&cfg, "Size must be specified before sprite data is allowed.");
				return false;
			}

			if (scale == SCALE_NONE &&
				(width != sprite_width || height != sprite_height)) {
				config_print_error(&cfg, "When no repeat mode is set, the sprite size should match the expected size of %ux%u.", width, height);
			}

			for (size_t row = 0; row < sprite_height; row += 1) {
				size_t col_len;
				if (!config_next_line(&cfg)) {
					config_print_error(&cfg,
									   "Expected %zu rows of data, got %zu.",
									   sprite_height, row);
					break;
				}

				col_len = MIN(sprite_width, cfg.line_data.length - 1);

				if (col_len < sprite_width) {
					config_print_error(&cfg,
									   "Expected %zu columns of data, got %zu.",
									   sprite_width, col_len);
				}

				for (size_t col = 0; col < col_len; col += 1) {
					int palette_entry = cfg.line_data.data[col];

					if (palette[palette_entry].type == PALETTE_UNSET) {
						config_print_error(&cfg, "Used unset palette entry %c.",
										   palette_entry);
					}

					sprite_out->data[row][col][0] = palette[palette_entry].color.r;
					sprite_out->data[row][col][1] = palette[palette_entry].color.g;
					sprite_out->data[row][col][2] = palette[palette_entry].color.b;
				}
			}

			if (scale == SCALE_REPEAT) {
				for (size_t row = 0; row < sprite_height; row += 1) {
					for (size_t col = sprite_width; col < TILE_SIZE; col += 1) {
						sprite_out->data[row][col][0] = sprite_out->data[row][col % sprite_width][0];
						sprite_out->data[row][col][1] = sprite_out->data[row][col % sprite_width][1];
						sprite_out->data[row][col][2] = sprite_out->data[row][col % sprite_width][2];
					}
				}

				for (size_t row = sprite_height; row < TILE_SIZE; row += 1) {
					memcpy((void*)&sprite_out->data[row],
						(void*)&sprite_out->data[row % sprite_height],
						sizeof(sprite_out->data[0]));
				}
			}
		}
	}

	for (size_t i = 0; i < 256; i += 1) {
		if (palette[i].type == PALETTE_SAMPLER) {
			free_sprite(palette[i].sprite);
		}
	}

	return true;
}
