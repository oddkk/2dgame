#ifndef GAME_CONFIG_PARSER_H
#define GAME_CONFIG_PARSER_H

#include "str.h"
#include "int.h"
#include <stdio.h>

struct config_file {
	struct string file;
	FILE *fd;
	struct string line_data;
	struct string line;
	size_t line_num;
};

bool config_next_line(struct config_file *config);
bool config_eat_token(struct config_file *config, struct string *token);
bool config_eat_token_uint(struct config_file *config, uint64_t *out);
bool config_eat_token_float(struct config_file *config, float *out);
uint64_t config_count_remaining_tokens(struct config_file *config);
void config_print_error(struct config_file *config, const char *msg, ...);

#endif
