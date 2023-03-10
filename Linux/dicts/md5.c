
/*

Цель:
	Этот файл содержит функционал для вычисления хэша по алгоритму MD5

*/

#include "md5.h"

static
void
Transform(
	uint32_t* buf,
	uint32_t* in
	);

static uint8_t padding[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void
MD5Init(
	MD5_CTX* context
	)

/*

Цель:
	Функция инициализирует контекст MD5

Параметры:
	MD5_CTX* context - указатель на контекст MD5 для инициализации

Возвращаемое значение:
	Нет

Примечание:
	Функция инициализирует контекст MD5. Инициализации контекста начинает вычисление нового дайджеста MD5.

*/

{
	context->bits[0] = context->bits[1] = 0;


	/*
	 * Загрузка инициализирующего вектора
	 */

	context->buf[0] = (uint32_t)0x67452301;
	context->buf[1] = (uint32_t)0xefcdab89;
	context->buf[2] = (uint32_t)0x98badcfe;
	context->buf[3] = (uint32_t)0x10325476;
}

void
MD5Update(
	MD5_CTX* context,
	uint8_t* buffer,
	uint32_t length
	)

/*

Цель:
	Функция обновляет контекст MD5 в соответствии с новыми данными

Параметры:
	MD5_CTX* context - указатель на контекст MD5. Перед вызовом MD5Update контекст должен быть инициализирован функцией MD5Init (см. описание
	функции MD5Init).
	uint8_t* buffer - буфер с данными для рассчёта MD5,
	uint32_t length - размер данных в буфере buffer

Возвращаемое значение:
	Нет

Примечание:
	Функция обновляет контекст MD5 в соответствии с новыми данными

*/

{
	uint32_t index, i, j, in[16];


	/*
	 * Вычисляем количество байт по модулю 64
	 */

	index = (context->bits[0] >> 3) & 0x3F;


	/*
	 * Обновляем количество обработанных разрядов
	 */

	if((context->bits[0] + ((uint32_t)length << 3)) < context->bits[0])
		context->bits[1]++;

	context->bits[0] += ((uint32_t)length << 3);
	context->bits[1] += ((uint32_t)length >> 29);

	while(length--)
	{
		context->input[index++] = *buffer++;

		if(index == 0x40)
		{ /* Трансформируем по необходимости */
			j = 0;

			for(i = 0; i != 16; i++)
			{
				in[i] = (((uint32_t)context->input[j + 3]) << 24)
						| (((uint32_t)context->input[j + 2]) << 16)
						| (((uint32_t)context->input[j + 1]) << 8)
						| ((uint32_t)context->input[j]);

				j += 4;
			}

			Transform(context->buf, in);
			index = 0;
		}
	}
}

void
MD5Final(
	MD5_CTX* context
	)

/*

Цель:
	Функция завершает расчёт дайджеста MD5

Параметры:
	MD5_CTX* context - контекст MD5

Возвращаемое значение:
	Нет

Примечание:
	Функция завершает расчёт дайджеста MD5. Полученное значение дайджеста сохранено в параметре digest контекста.

*/

{
	uint32_t in[16], index, padlen, i, j;


	/*
	 * Количество обработанных разрядов
	 */

	in[14] = context->bits[0];
	in[15] = context->bits[1];


	/*
	 * Вычисляем количество байт по модулю 64
	 */

	index = (context->bits[0] >> 3) & 0x3F;


	/*
	 * Длина выравнивания
	 */

	padlen = (index < 56) ? (56 - index) : (120 - index);
	MD5Update (context, padding, padlen);


	/*
	 * Длина в битах
	 */

	j = 0;

	for(i = 0; i != 14; i++)
	{
		in[i] = (((uint32_t)context->input[j + 3]) << 24) |
				(((uint32_t)context->input[j + 2]) << 16) |
				(((uint32_t)context->input[j + 1]) << 8) |
				((uint32_t)context->input[j]);

		j += 4;
	}

	Transform(context->buf, in);


	/*
	 * Сохраняем дайджест
	 */

	j = 0;

	for(i = 0; i != 4; i++)
	{
		context->digest[j] = (uint8_t)(context->buf[i] & 0xFF);
		context->digest[j + 1] = (uint8_t)((context->buf[i] >> 8) & 0xFF);
		context->digest[j + 2] = (uint8_t)((context->buf[i] >> 16) & 0xFF);
		context->digest[j + 3] = (uint8_t)((context->buf[i] >> 24) & 0xFF);

		j += 4;
	}
}

static
void
Transform(
	uint32_t* buf,
	uint32_t* in
	)

/*

Цель:
	Вспомогательная функция. Осуществляет перестановку данных по алгоритму MD5.

Параметры:
	uint32_t* buf - буфер ABCD входных данных алгоритма MD5,
	uint32_t* in - буфер входных данных алгоритма MD5

Возвращаемое значение:
	Нет

Примечание:
	Вспомогательная функция. Осуществляет перестановку данных по алгоритму MD5.

*/

{
	uint32_t a = buf[0], b = buf[1], c = buf[2], d = buf[3];


	/*
	 * Раунд 1
	 */

	FF(a, b, c, d, in[0], S11, 3614090360u);  /* 1 */
	FF(d, a, b, c, in[1], S12, 3905402710u);  /* 2 */
	FF(c, d, a, b, in[2], S13, 606105819u);   /* 3 */
	FF(b, c, d, a, in[3], S14, 3250441966u);  /* 4 */
	FF(a, b, c, d, in[4], S11, 4118548399u);  /* 5 */
	FF(d, a, b, c, in[5], S12, 1200080426u);  /* 6 */
	FF(c, d, a, b, in[6], S13, 2821735955u);  /* 7 */
	FF(b, c, d, a, in[7], S14, 4249261313u);  /* 8 */
	FF(a, b, c, d, in[8], S11, 1770035416u);  /* 9 */
	FF(d, a, b, c, in[9], S12, 2336552879u);  /* 10 */
	FF(c, d, a, b, in[10], S13, 4294925233u); /* 11 */
	FF(b, c, d, a, in[11], S14, 2304563134u); /* 12 */
	FF(a, b, c, d, in[12], S11, 1804603682u); /* 13 */
	FF(d, a, b, c, in[13], S12, 4254626195u); /* 14 */
	FF(c, d, a, b, in[14], S13, 2792965006u); /* 15 */
	FF(b, c, d, a, in[15], S14, 1236535329u); /* 16 */


	/*
	 * Раунд 2
	 */

	GG(a, b, c, d, in[1], S21, 4129170786u);  /* 17 */
	GG(d, a, b, c, in[6], S22, 3225465664u);  /* 18 */
	GG(c, d, a, b, in[11], S23, 643717713u);  /* 19 */
	GG(b, c, d, a, in[0], S24, 3921069994u);  /* 20 */
	GG(a, b, c, d, in[5], S21, 3593408605u);  /* 21 */
	GG(d, a, b, c, in[10], S22, 38016083u);   /* 22 */
	GG(c, d, a, b, in[15], S23, 3634488961u); /* 23 */
	GG(b, c, d, a, in[4], S24, 3889429448u);  /* 24 */
	GG(a, b, c, d, in[9], S21, 568446438u);   /* 25 */
	GG(d, a, b, c, in[14], S22, 3275163606u); /* 26 */
	GG(c, d, a, b, in[3], S23, 4107603335u);  /* 27 */
	GG(b, c, d, a, in[8], S24, 1163531501u);  /* 28 */
	GG(a, b, c, d, in[13], S21, 2850285829u); /* 29 */
	GG(d, a, b, c, in[2], S22, 4243563512u);  /* 30 */
	GG(c, d, a, b, in[7], S23, 1735328473u);  /* 31 */
	GG(b, c, d, a, in[12], S24, 2368359562u); /* 32 */


	/*
	 * Раунд 3
	 */

	HH(a, b, c, d, in[5], S31, 4294588738u);  /* 33 */
	HH(d, a, b, c, in[8], S32, 2272392833u);  /* 34 */
	HH(c, d, a, b, in[11], S33, 1839030562u); /* 35 */
	HH(b, c, d, a, in[14], S34, 4259657740u); /* 36 */
	HH(a, b, c, d, in[1], S31, 2763975236u);  /* 37 */
	HH(d, a, b, c, in[4], S32, 1272893353u);  /* 38 */
	HH(c, d, a, b, in[7], S33, 4139469664u);  /* 39 */
	HH(b, c, d, a, in[10], S34, 3200236656u); /* 40 */
	HH(a, b, c, d, in[13], S31, 681279174u);  /* 41 */
	HH(d, a, b, c, in[0], S32, 3936430074u);  /* 42 */
	HH(c, d, a, b, in[3], S33, 3572445317u);  /* 43 */
	HH(b, c, d, a, in[6], S34, 76029189u);    /* 44 */
	HH(a, b, c, d, in[9], S31, 3654602809u);  /* 45 */
	HH(d, a, b, c, in[12], S32, 3873151461u); /* 46 */
	HH(c, d, a, b, in[15], S33, 530742520u);  /* 47 */
	HH(b, c, d, a, in[2], S34, 3299628645u);  /* 48 */


	/*
	 * Раунд 4
	 */

	II(a, b, c, d, in[0], S41, 4096336452u);  /* 49 */
	II(d, a, b, c, in[7], S42, 1126891415u);  /* 50 */
	II(c, d, a, b, in[14], S43, 2878612391u); /* 51 */
	II(b, c, d, a, in[5], S44, 4237533241u);  /* 52 */
	II(a, b, c, d, in[12], S41, 1700485571u); /* 53 */
	II(d, a, b, c, in[3], S42, 2399980690u);  /* 54 */
	II(c, d, a, b, in[10], S43, 4293915773u); /* 55 */
	II(b, c, d, a, in[1], S44, 2240044497u);  /* 56 */
	II(a, b, c, d, in[8], S41, 1873313359u);  /* 57 */
	II(d, a, b, c, in[15], S42, 4264355552u); /* 58 */
	II(c, d, a, b, in[6], S43, 2734768916u);  /* 59 */
	II(b, c, d, a, in[13], S44, 1309151649u); /* 60 */
	II(a, b, c, d, in[4], S41, 4149444226u);  /* 61 */
	II(d, a, b, c, in[11], S42, 3174756917u); /* 62 */
	II(c, d, a, b, in[2], S43, 718787259u);   /* 63 */
	II(b, c, d, a, in[9], S44, 3951481745u);  /* 64 */


	/*
	 * Окончание
	 */

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}
