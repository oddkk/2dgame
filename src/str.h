#ifndef GAME_STRING_H
#define GAME_STRING_H

#include "int.h"

struct string {
	size_t length;
	uint8_t *data;
};

#define STR(x) ((struct string){sizeof(x)-1, (uint8_t *)x})
#define LIT(x) (int)x.length, (char *)x.data

bool string_equals(struct string, struct string);
int read_character(struct string str, uint8_t **it);
struct string duplicate_string(struct string str);

bool string_to_uint64(struct string, uint64_t *out);
bool string_to_float(struct string, float *out);

#endif
