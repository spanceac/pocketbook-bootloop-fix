
#ifndef __DICTS_H__
#define __DICTS_H__

#include <stdint.h>
#include "tchar.h"

#ifndef _WIN32
typedef unsigned int BOOL;

#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */
#endif /* _WIN32 */

#define DICTS_SIGNATURE				0x43494453 /* SDIC */

#pragma pack(push, 1)

typedef struct {
	uint32_t sign;
	uint32_t padding1[6];
	uint32_t licenseId;
	uint32_t padding2[8];
	char dict_name[64];
} DICT_HEADER;

typedef struct {
	char serial_number[32];
	uint32_t licensid[2];
	uint32_t const_value;
	uint32_t padding[5];
} LICENSE_BLOCK;

#pragma pack(pop)


BOOL
BatchProcess(
	const char* serial_number,
	const tchar* ppath
	);

BOOL
ProcessDict(
	const char* serial_number,
	const tchar* dict_name,
	BOOL bIgnoreNullLicenseId
	);

void
md5sum(
	const void* pdata,
	uint32_t cbdata,
	uint8_t* digest
	);

#ifdef _WIN32
char*
SerialNumber(
	const wchar_t* serial_number
	);
#endif /* _WIN32 */

#endif /* __DICTS_H__ */
