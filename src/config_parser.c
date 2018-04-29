#include "config_parser.h"
#include "utils.h"
#include <stdarg.h>

#define COMMENT_CHAR '#'

static inline bool is_alpha(int i) {
	return (i >= 'a' && i <= 'z') || (i >= 'A' && i <= 'U');
}

static inline bool is_numeric(int i) {
	return (i >= '0' && i <= '9');
}

static inline bool is_alphanumeric(int i) {
	return is_alpha(i) || is_numeric(i);
}

static inline bool is_symbol(int i) {
	return (i >= '!' && i <= '/')
		|| (i >= ':' && i <= '@')
		|| (i >= '[' && i <= '`')
		|| (i >= '{' && i <= '~');
}

static inline bool is_token_character(int i) {
	return is_alphanumeric(i)
		|| (is_symbol(i) && i != COMMENT_CHAR);
}

static void config_trim_whitespace(struct config_file *config) {
	size_t n = 0;
	for (size_t i = 0; i < config->line.length; ++i) {
		if (is_token_character(config->line.data[i])) {
			break;
		}
		n += 1;
	}
	config->line.data += n;
	config->line.length -= n;
}

bool config_next_line(struct config_file *config) {
	assert(config->file.data);

	if (!config->fd) {
		// TODO: We should ensure the file name is as long as it say it is.
		config->fd = fopen((char *)config->file.data, "rb");

		if (!config->fd) {
			perror("map fopen");
			return false;
		}
	}

	do {

		ssize_t line_length;

		line_length =
		    getline((char **)&config->line_data.data,
			    &config->line_data.length, config->fd);

		if (line_length == -1) {
			return false;
		}

		config->line_num += 1;
		config->line.data = config->line_data.data;
		config->line.length = line_length;

		bool empty_line = true;

		for (size_t i = 0; i < config->line.length; ++i) {
			if (config->line.data[i] == COMMENT_CHAR) {
				config->line.length = i;
				break;
			} else if (is_token_character(config->line.data[i])) {
				empty_line = false;
			}
		}

		if (empty_line) {
			config->line.length = 0;
		}

	} while (config->line.length == 0);

	config_trim_whitespace(config);

	return true;
}

bool config_eat_token(struct config_file *config, struct string *token) {
	config_trim_whitespace(config);

	token->data = config->line.data;
	token->length = 0;

	for (size_t i = 0; i < config->line.length; ++i) {
		if (is_token_character(config->line.data[i])) {
			token->length += 1;
		} else {
			break;
		}
	}

	if (token->length == 0) {
		return false;
	}

	config->line.data += token->length;
	config->line.length -= token->length;

	config_trim_whitespace(config);

	return true;
}

bool config_eat_token_uint(struct config_file *config, uint64_t *out) {
	struct string token;

	if (!config_eat_token(config, &token)) {
		return false;
	}

	uint64_t result;

	if (!string_to_uint64(token, &result)) {
		return false;
	}

	*out = result;

	return true;
}

bool config_eat_token_float(struct config_file *config, float *out) {
	struct string token;

	if (!config_eat_token(config, &token)) {
		return false;
	}

	float result;

	if (!string_to_float(token, &result)) {
		return false;
	}

	*out = result;

	return true;
}

uint64_t config_count_remaining_tokens(struct config_file *config) {
	uint64_t result = 0;
	bool in_word = false;
	for (uint64_t i = 0; i < config->line.length; ++i) {
		uint8_t c = config->line.data[i];
		if (is_alphanumeric(c)) {
			if (!in_word) {
				in_word = true;
				result += 1;
			}
		} else {
			in_word = false;
		}
	}

	return result;
}

void config_print_error(struct config_file *config, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "Error at %.*s:%zu: ", LIT(config->file), config->line_num);
	vfprintf(stderr, fmt, ap);
	fputs("\n", stderr);

	va_end(ap);
}
