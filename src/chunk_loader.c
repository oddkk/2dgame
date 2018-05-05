#include "chunk_loader.h"
#include "config_parser.h"
#include "str.h"
#include "texture_map.h"
#include "utils.h"
#include <stdlib.h>

static bool load_chunk_from_file(struct chunk *out,
								 struct texture_map *tex_map,
								 struct string filename);

bool load_chunk(struct chunk *out, struct texture_map *tex_map,
				int64_t x, int64_t y, int64_t z) {
	struct string filename;
	char buffer[256];
	int ret;

	filename.data = (uint8_t *)buffer;

	ret = snprintf(buffer, sizeof(buffer),
				   "assets/world/%li.%li.%li.chunk", x, y, z);

	if (ret < 0) {
		perror("snprintf");
		return false;
	}

	filename.length = ret;

	out->x = x;
	out->y = y;
	out->z = z;

	return load_chunk_from_file(out, tex_map, filename);
}

static bool load_chunk_from_file(struct chunk *out,
								 struct texture_map *tex_map,
								 struct string filename) {
	struct config_file cfg = {};
	struct {
		tex_id tex;
		bool set;
	} tile_palette[255] = {};
	bool version_specified = false;
	bool size_specified = false;
	uint64_t layer = 0;

	cfg.file = filename;
	cfg.fd = fopen((char*)filename.data, "rb");

	tile_palette[' '].set = true;

	if (!cfg.fd) {
		print_error("chunk", "Could not open chunk file %.*s.", LIT(filename));
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
			uint64_t w, h;
			bool ok = true;

			if (size_specified) {
				config_print_error(&cfg, "Size already specified.");
				continue;
			}
			ok &= config_eat_token_uint(&cfg, &w);
			ok &= config_eat_token_uint(&cfg, &h);

			if (w != CHUNK_WIDTH || h != CHUNK_HEIGHT) {
				config_print_error(&cfg,
								   "The chunk size should be %ix%i px, but we got "
								   "%zux%zu px.", CHUNK_WIDTH, CHUNK_HEIGHT, w, h);
				continue;
			}

			size_specified = true;
		} else if (string_equals(token, STR("tile"))) {
			struct string ident;
			struct string name;

			if (!config_eat_token(&cfg, &ident)) {
				config_print_error(&cfg, "Expected character for tile entry.");
				continue;
			}

			if (ident.length > 1) {
				config_print_error(&cfg, "Expected single ascii character for tile entry.");
				continue;
			}

			if (tile_palette[ident.data[0]].set) {
				config_print_error(&cfg, "Tile key %c already used.", ident.data[0]);
				continue;
			}

			tile_palette[ident.data[0]].set = true;

			if (!config_eat_token(&cfg, &name)) {
				continue;
			}

			tile_palette[ident.data[0]].tex = load_sprite(tex_map, name);
		} else if (string_equals(token, STR("layer"))) {
			if (!config_eat_token_uint(&cfg, &layer)) {
				config_print_error(&cfg, "Expected layer ID.");
				continue;
			}
		} else if (string_equals(token, STR("data"))) {
			for (size_t row = 0; row < CHUNK_HEIGHT; row += 1) {
				size_t col_len;
				if (!config_next_line(&cfg)) {
					config_print_error(&cfg,
									   "Expected %i rows of data, got %zu.",
									   CHUNK_HEIGHT, row);
					break;
				}

				col_len = MIN(CHUNK_WIDTH, cfg.line_data.length - 1);

				if (col_len < CHUNK_WIDTH) {
					config_print_error(&cfg,
									   "Expected %i columns of data, got %zu.",
									   CHUNK_WIDTH, col_len);
				}

				for (size_t col = 0; col < col_len; col += 1) {
					int palette_entry = cfg.line_data.data[col];

					if (!tile_palette[palette_entry].set) {
						config_print_error(&cfg, "Used unset tile entry %c.",
										   palette_entry);
					}

					// TODO: Implement support for multiple layers.
					if (layer == 0) {
						out->tilemap[row][col].tex_id = tile_palette[palette_entry].tex;
					}
				}
			}
		} else {
			config_print_error(&cfg, "Got unexpected token %.*s.", LIT(token));
		}
	}

	return true;
}
