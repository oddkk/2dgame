#ifndef GAME_LINMAT_H
#define GAME_LINMAT_H

typedef union {
	int m[2][2];
	struct {
		int m00, m10,
			m01, m11;
	};
} imat2;

typedef union {
	int m[2];
	struct {
		int x, y;
	};
} ivec2;

#define IMAT2_FLIP_X ((imat2){.m={{-1,0},{0,1}}})
#define IMAT2_FLIP_Y ((imat2){{1,0},{0,-1}})

static inline imat2 imat2_from(int m00, int m10, int m01, int m11) {
	imat2 result = {};

	result.m00 = m00;
	result.m01 = m01;
	result.m10 = m10;
	result.m11 = m11;

	return result;
}

static inline imat2 imat2_identity() {
	imat2 result;

	result = imat2_from(1, 0,
						0, 1);

	return result;
}

static inline imat2 imat2_flipx() {
	imat2 result = {};

	result = imat2_from(-1, 0,
						0, 1);

	return result;
}

static inline imat2 imat2_flipy() {
	imat2 result = {};

	result = imat2_from(1, 0,
						0, -1);

	return result;
}

static inline imat2 imat2_rotate(int angle) {
	imat2 result;

	switch (angle) {
	case 0:
		result = imat2_identity();
		break;

	case 90:
		result = imat2_from(0, -1,
							1,  0);
		break;

	case 180:
		result = imat2_from(-1,  0,
							 0, -1);
		break;

	case 270:
		result = imat2_from( 0, 1,
							-1, 0);
		break;
	}

	return result;
}

static inline imat2 imat2_multiply(imat2 lhs, imat2 rhs) {
	imat2 result;

	result.m00 = lhs.m00 * rhs.m00 + lhs.m10 * rhs.m01;
	result.m10 = lhs.m00 * rhs.m10 + lhs.m10 * rhs.m11;
	result.m01 = lhs.m01 * rhs.m00 + lhs.m11 * rhs.m01;
	result.m11 = lhs.m01 * rhs.m10 + lhs.m11 * rhs.m11;

	return result;
}

static inline ivec2 imat2_multiply_ivec2(imat2 lhs, ivec2 rhs) {
	ivec2 result;

	result.x = lhs.m00 * rhs.x + lhs.m10 * rhs.y;
	result.y = lhs.m01 * rhs.x + lhs.m11 * rhs.y;

	return result;
}

static inline ivec2 ivec2_from(int x, int y) {
	ivec2 result;

	result.x = x;
	result.y = y;

	return result;
}

static inline ivec2 ivec2_add(ivec2 lhs, ivec2 rhs) {
	ivec2 result;

	result.x = lhs.x + rhs.x;
	result.y = lhs.y + rhs.y;

	return result;
}

#endif
