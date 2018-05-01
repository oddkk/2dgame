#ifndef GAME_UTILS_H
#define GAME_UTILS_H

#include <assert.h>
#include "int.h"

void print_debug(const char *fmt, ...);
void print_error(const char *tag, const char *fmt, ...);
void panic(const char *fmt, ...);

#define myassert(expr) \
	if(!(expr)){ \
		panic("Assertion '" #expr "' failed! (" __FILE__ ":%i)",__LINE__); \
	}

#define MIN(x, y) ((x) <= (y) ? (x) : (y))
#define MAX(x, y) ((x) >= (y) ? (x) : (y))

struct box {
	int64_t left;
	int64_t right;
	int64_t top;
	int64_t bottom;
};

struct box box_from_width_height(int64_t left, int64_t top, int64_t width, int64_t height);
bool box_intersect(struct box *out, struct box, struct box);

#endif
