
#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdint.h>

typedef unsigned int BOOL;

#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

uint32_t
calc_freezestatus(
	const tchar* serial_number,
	uint32_t sd_serial
	);

FILE*
serial_test(
	uint32_t a1
	);

#endif /* __SERIAL_H__ */
