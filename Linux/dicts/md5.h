
#ifndef __MD5_H__
#define __MD5_H__

#include <stdint.h>


/*
 * F, G, H и I базовые функции MD5
 */

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))


/*
 * ROTATE_LEFT циклический сдвиг x влево на n разрядов
 */

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))


/*
 * FF, GG, HH, и II трансформации для раундов 1, 2, 3, и 4
 */

#define FF(a, b, c, d, x, s, t) \
	{ \
		(a) += F((b), (c), (d)) + (x) + (uint32_t)(t); \
		(a) = ROTATE_LEFT ((a), (s)); \
		(a) += (b); \
	}

#define GG(a, b, c, d, x, s, t) \
	{ \
		(a) += G((b), (c), (d)) + (x) + (uint32_t)(t); \
		(a) = ROTATE_LEFT ((a), (s)); \
		(a) += (b); \
	}

#define HH(a, b, c, d, x, s, t) \
	{ \
		(a) += H((b), (c), (d)) + (x) + (uint32_t)(t); \
		(a) = ROTATE_LEFT ((a), (s)); \
		(a) += (b); \
	}

#define II(a, b, c, d, x, s, t) \
	{ \
		(a) += I((b), (c), (d)) + (x) + (uint32_t)(t); \
		(a) = ROTATE_LEFT ((a), (s)); \
		(a) += (b); \
	}


/*
 * Константы MD5
 */

#define S11 7
#define S12 12
#define S13 17
#define S14 22

#define S21 5
#define S22 9
#define S23 14
#define S24 20

#define S31 4
#define S32 11
#define S33 16
#define S34 23

#define S41 6
#define S42 10
#define S43 15
#define S44 21


/*
 * Контекст MD5
 */

typedef struct {
  uint32_t bits[2];   /* Количество обработанных разрядов по модулю 2 ^ 64 */
  uint32_t buf[4];    /* Вспомогательный буфер                             */
  uint8_t input[64];  /* Входной буфер                                     */
  uint8_t digest[16]; /* Актуальный дайджест после вызова MD5Final         */
} MD5_CTX;

void
MD5Init(
	MD5_CTX* context
	);

void
MD5Update(
	MD5_CTX* context,
	uint8_t* buffer,
	uint32_t length
	);

void
MD5Final(
	MD5_CTX* context
	);

#endif /* __MD5_H__ */
