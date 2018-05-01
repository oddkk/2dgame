#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void print_debug(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

void print_error(const char *tag, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "[%s] ", tag);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

void panic(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "[panic] ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(-1);
}

struct box box_from_width_height(int64_t left, int64_t top, int64_t width, int64_t height) {
	struct box res;

	res.left = left;
	res.top = top;
	res.right = left + width;
	res.bottom = top + height;

	return res;
}

bool box_intersect(struct box *out, struct box b1, struct box b2) {
	struct box res;
	res.left   = MAX(b1.left,   b2.left);
	res.right  = MIN(b1.right,  b2.right);
	res.top    = MAX(b1.top,    b2.top);
	res.bottom = MIN(b1.bottom, b2.bottom);

	if (res.right < res.left || res.bottom < res.top) {
		return false;
	}

	*out = res;
	return true;
}
