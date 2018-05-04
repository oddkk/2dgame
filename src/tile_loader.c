#include "tile_loader.h"
#include "config_parser.h"
#include "game_config.h"
#include "utils.h"
#include <string.h>

enum scale_mode {
	SCALE_NONE,
	SCALE_REPEAT,
	SCALE_STRETCH,
	SCALE_CENTER,
};

bool load_tile_data_from_file(uint8_t *out,
							  unsigned int width, unsigned int height,
							  struct string filename) {
	struct config_file cfg = {};
	struct {
		uint8_t r, g, b;
		bool set;
	} palette[255] = {};
	uint64_t sprite_width, sprite_height;
	bool version_specified = false;
	bool size_specified = false;
	enum scale_mode scale = SCALE_NONE;

	assert(width == TILE_SIZE);
	assert(height == TILE_SIZE);

	cfg.file = filename;
	cfg.fd = fopen((char*)filename.data, "rb");

	palette[' '].set = true;

	if (!cfg.fd) {
		print_error("tile", "Could not open tile file %.*s.", LIT(filename));
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
			bool ok = true;

			if (!config_eat_token(&cfg, &ident)) {
				config_print_error(&cfg, "Expected character for palette entry.");
				continue;
			}

			if (ident.length > 1) {
				config_print_error(&cfg, "Expected single ascii character for palette entry.");
				continue;
			}

			ok &= config_eat_token_uint(&cfg, &r);
			ok &= config_eat_token_uint(&cfg, &g);
			ok &= config_eat_token_uint(&cfg, &b);

			ok &= (r <= 255 && g <= 255 && b <= 255);

			if (!ok) {
				config_print_error(&cfg, "Expected rgb values for color in the range 0-255.");
				continue;
			}

			if (palette[ident.data[0]].set) {
				config_print_error(&cfg, "Palette key %c already specified.", ident.data[0]);
				continue;
			}

			palette[ident.data[0]].r = r;
			palette[ident.data[0]].g = g;
			palette[ident.data[0]].b = b;
			palette[ident.data[0]].set = true;
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

					if (!palette[palette_entry].set) {
						config_print_error(&cfg, "Used unset palette entry %c.",
										   palette_entry);
					}

					out[(col + row * TILE_SIZE) * 3 + 0] = palette[palette_entry].r;
					out[(col + row * TILE_SIZE) * 3 + 1] = palette[palette_entry].g;
					out[(col + row * TILE_SIZE) * 3 + 2] = palette[palette_entry].b;
				}
			}

			if (scale == SCALE_REPEAT) {
				for (size_t row = 0; row < sprite_height; row += 1) {
					for (size_t col = sprite_width; col < TILE_SIZE; col += 1) {
						size_t in_index  = 3 * (row * TILE_SIZE + (col % sprite_width));
						size_t out_index = 3 * (row * TILE_SIZE + col);
						out[out_index + 0] = out[in_index + 0];
						out[out_index + 1] = out[in_index + 1];
						out[out_index + 2] = out[in_index + 2];
					}
				}

				for (size_t row = sprite_height; row < TILE_SIZE; row += 1) {
					memcpy((void*)&out[TILE_SIZE * 3 * row],
						(void*)&out[TILE_SIZE * 3 * (row % sprite_height)],
						sizeof(uint8_t) * TILE_SIZE * 3);
				}
			}
		}
	}

	return true;
}
