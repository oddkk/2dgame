#include "str.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

static inline bool is_numeric(int i)
{
	return (i >= '0' && i <= '9');
}

bool string_equals(struct string lhs, struct string rhs) {
	if (lhs.length != rhs.length) {
		return false;
	}

	for (size_t i = 0; i < lhs.length; i += 1) {
		if (lhs.data[i] != rhs.data[i]) {
			return false;
		}
	}

	return true;
}

int read_character(struct string str, uint8_t **it) {
	int result;

	if (!*it) {
		*it = str.data;
	}

	if (*it >= str.data + str.length) {
		return 0;
	}

	result = **it;
	*it += 1;

	return result;
}


bool string_to_uint64(struct string str, uint64_t *out) {
	uint64_t result = 0;
	bool is_valid = false;
	uint8_t *it = 0;
	int c = 0;

	c = read_character(str, &it);
	for (; c; c = read_character(str, &it)) {
		if (is_numeric(c)) {
			is_valid = true;
			result = result * 10 + (c - '0');
		} else {
			break;
		}
	}

	if (!is_valid) {
		return false;
	}

	*out = result;

	return true;
}

bool string_to_float(struct string str, float *out) {
	float result = 0;
	bool is_valid = false;
	uint8_t *it;
	int c = 0;
	float negate = 1.0f;

	it = str.data;

	c = read_character(str, &it);

	if (c == '-') {
		negate = -1.0f;
	} else {
		it = str.data;
	}

	while ((c = read_character(str, &it)) && is_numeric(c)) {
		is_valid = true;
		result = result * 10.0f + (float)(c - '0');
	}

	if (c == '.') {
		float n = 1.0f;
		while ((c = read_character(str, &it)) && is_numeric(c)) {
			n /= 10.0f;
			result += n * (float)(c - '0');
		}
	}

	result *= negate;

	if (!is_valid) {
		return false;
	}

	*out = result;

	return true;
}

struct string duplicate_string(struct string str) {
	struct string res;

	res.length = str.length;
	res.data = calloc(str.length, sizeof(uint8_t));
	if (!res.data) {
		res.length = 0;
		print_error("string", "Failed to duplicate string because calloc failed.");
		return res;
	}

	memcpy(res.data, str.data, res.length);

	return res;
}
