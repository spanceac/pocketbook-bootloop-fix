
/*

Цель:
	Программа лицензирования словарей Abbyy Lingvo для PocketBook
	Распространяется под лицензией GNU GPLv3+
	Лицензия GNU GPL версии 3 или новее: https://gnu.org/licenses/gpl.html

*/

#ifndef _WIN32
#include <dirent.h>
#include <fnmatch.h>
#else
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#endif /* _WIN32 */
#include <locale.h>
#include "md5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tchar.h"
#include "dicts.h"


const tchar szUsage[]							= TEXT("\nИспользование:\n")
		TEXT("\tdicts --serial_number серийный_номер_устройства имя_файла_словаря\n")
		TEXT("\tdicts --batch --serial_number серийный_номер_устройства папка_со_словарями\n\n")
		TEXT("\t--serial_number\t\tсерийный номер устройства;\n")
		TEXT("\t--batch\t\t\tпакетный режим работы;\n")
		TEXT("\t--help\t-h\t\tвывести это сообщение;\n")
		TEXT("\tимя_файла_словаря\tимя файла словаря для лицензирования;\n")
		TEXT("\tпапка_со_словарями\tимя папки со словарями для лицензирования.\n\n");

const tchar szDuplicateParameter[]				= TEXT("Дублирующий параметр: %s. Проигнорирован.\n");
const tchar szUnknownParameter[]				= TEXT("Неизвестный параметр: %s!\n");
const tchar szInvalidParameter[]				= TEXT("Ошибка в формате параметра: %s!\n");
const tchar szSerialNumberIsAbsent[]			= TEXT("Отсутствует серийный номер устройства!\n");
const tchar szPathIsAbsent[]					= TEXT("Не задан путь к папке со словарями!\n");
const tchar szDictIsAbsent[]					= TEXT("Отсутствует имя файла словаря!\n");

const tchar szSerialNumber[]					= TEXT("serial_number");
const tchar szBatch[]							= TEXT("batch");
const tchar szHelp[]							= TEXT("help");
const tchar szHelpShort[]						= TEXT("h");

const tchar szCantAllocMemory[]					= TEXT("Ошибка выделения памяти!\n");
const tchar szCantOpenDir[]						= TEXT("Невозможно открыть каталог %s!\n");
const tchar szCantOpenFile[]					= TEXT("Ошибка открытия файла %s!\n");
const tchar szCantCreateFile[]					= TEXT("Невозможно создать файл %s!\n");
const tchar szCorruptedDict[]					= TEXT("Словарь %s повреждён!\n");
const tchar szCantWriteFile[]					= TEXT("Невозможно записать файл %s!\n");
const tchar szDictNameMask[]					= TEXT("*.dic");
const tchar szReadOnly[]						= TEXT("rb");
const tchar szWriteOnly[]						= TEXT("w");
const tchar szLicExt[]							= TEXT(".lic");

const char szDigestByte[]						= "%02x";
#ifdef _WIN32
const char szLicFileFormat[]					= "[%s]\n%s \"%s\"\n"; /* Это какой-то косяк */
#else
const char szLicFileFormat[]					= "[%s]\r\n%s \"%s\"\r\n";
#endif /* _WIN32 */
const char szLocale[]							= "";


int _tmain(int argc, tchar* argv[])
{
	int i, ret = EXIT_FAILURE;
	char* serial_number = NULL;
	BOOL bBatch = FALSE;
	const tchar* in_name = NULL;

	setlocale(LC_ALL, szLocale);

	if(argc < 2)
	{
		_tprintf(szUsage);
		goto ErrorExit;
	}

	/*
	 * Разбор параметров командной строки
	 */

	for(i = 1; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			if(argv[i][1] == '-')
			{
				/*
				 * Длинные параметры
				 */

				if(!_tstrcmp(argv[i] + 2, szSerialNumber))
				{
					if(serial_number)
					{
						_tprintf(szDuplicateParameter, argv[i++]);
						continue;
					}

					i++;

					if(i >= argc)
					{
						_tprintf(szInvalidParameter, argv[--i]);
						goto ErrorExit;
					}

#ifdef _WIN32
					serial_number = SerialNumber(argv[i]);

					if(!serial_number)
						goto ErrorExit;
#else
					serial_number = argv[i];
#endif /* _WIN32 */
					continue;
				}

				if(!_tstrcmp(argv[i] + 2, szBatch))
				{
					if(bBatch)
					{
						_tprintf(szDuplicateParameter, argv[i++]);
						continue;
					}

					bBatch = TRUE;
					continue;
				}

				if(!_tstrcmp(argv[i] + 2, szHelp))
				{
					_tprintf(szUsage);
					ret = EXIT_SUCCESS;
					goto ErrorExit;
				}
			}
			else
			{
				/*
				 * Короткие параметры
				 */

				if(!_tstrcmp(argv[i] + 1, szHelpShort))
				{
					_tprintf(szUsage);
					ret = EXIT_SUCCESS;
					goto ErrorExit;
				}
			}
		}
		else
		{
			if(in_name)
				_tprintf(szDuplicateParameter, argv[i]);
			else
				in_name = argv[i];

			continue;
		}

		_tprintf(szUnknownParameter, argv[i]);
		goto ErrorExit;
	}

	if(!serial_number)
	{
		_tprintf(szSerialNumberIsAbsent);
		goto ErrorExit;
	}

	if(!in_name)
	{
		_tprintf(bBatch ? szPathIsAbsent : szDictIsAbsent);
		goto ErrorExit;
	}

	if(bBatch)
	{
		if(!BatchProcess(serial_number, in_name))
			goto ErrorExit;
	}
	else
	{
		if(!ProcessDict(serial_number, in_name, TRUE))
			goto ErrorExit;
	}

	ret = EXIT_SUCCESS;

ErrorExit:

#ifdef _WIN32
	if(serial_number)
		free(serial_number);
#endif /* _WIN32 */

	return ret;
}

BOOL
BatchProcess(
	const char* serial_number,
	const tchar* ppath
	)

/*

Цель:
	Функция производит пакетную обработку файлов словарей в указанной директории

Параметры:
	const char* serial_number - серийный номер устройства PocketBook,
	const tchar* ppath - директория со словарями

Возвращаемое значение:
	TRUE - в случае удачного завершения,
	FALSE - в противном случае

Примечание:
	Функция производит пакетную обработку файлов словарей в указанной директории. Поиск файлов словарей происходит по маске: *.dic.

*/

{
#ifdef _WIN32
	int Length;
	LPWSTR pSearchName = NULL, pFileName = NULL;
	PWIN32_FIND_DATAW pwfd = NULL;
	HANDLE hFind =  INVALID_HANDLE_VALUE;
	BOOL bRet = FALSE;

	Length = lstrlenW(ppath);
	pSearchName = (LPWSTR)malloc(sizeof(WCHAR) * (Length + 20));

	if(!pSearchName)
	{
		_tprintf(szCantAllocMemory);
		goto ErrorExit;
	}

	lstrcpyW(pSearchName, ppath);

	if(pSearchName[Length - 1] != '\\')
	{
		pSearchName[Length] = '\\';
		pSearchName[Length + 1] = 0;
	}

	lstrcatW(pSearchName, szDictNameMask);
	pwfd = (PWIN32_FIND_DATAW)malloc(sizeof(*pwfd));

	if(!pwfd)
	{
		_tprintf(szCantAllocMemory);
		goto ErrorExit;
	}

	hFind = FindFirstFileW(pSearchName, pwfd);

	if(hFind == INVALID_HANDLE_VALUE)
	{
		_tprintf(szCantOpenDir, ppath);
		goto ErrorExit;
	}

	do
	{
		Length = lstrlenW(ppath) + lstrlenW(pwfd->cFileName);
		pFileName = (LPWSTR)malloc(sizeof(WCHAR) * (Length + 10));

		if(!pFileName)
		{
			_tprintf(szCantAllocMemory);
			goto ErrorExit;
		}

		lstrcpyW(pFileName, ppath);
		Length = lstrlenW(ppath);

		if(pFileName[Length - 1] != '\\')
		{
			pFileName[Length] = '\\';
			pFileName[Length + 1] = 0;
		}

		lstrcatW(pFileName, pwfd->cFileName);

		if(!ProcessDict(serial_number, pFileName, FALSE))
			break;

		free(pFileName);
		pFileName = NULL;
	}
	while(FindNextFileW(hFind, pwfd));

	bRet = TRUE;

ErrorExit:

	if(pFileName)
		free(pFileName);

	if(hFind != INVALID_HANDLE_VALUE)
		FindClose(hFind);

	if(pwfd)
		free(pwfd);

	if(pSearchName)
		free(pSearchName);

	return bRet;
#else
	DIR* d = NULL;
	size_t plen, len;
	struct dirent* dd;
	BOOL bRet = FALSE;
	char* file_name = NULL;

	d = opendir(ppath);

	if(!d)
	{
		printf(szCantOpenDir, ppath);
		return FALSE;
	}

	plen = strlen(ppath);


	/*
	 * Пребираем содержимое папки
	 */

	for(;;)
	{
		dd = readdir(d);

		if(!dd)
		{
			bRet = TRUE;
			break;
		}

		if((dd->d_type == DT_REG) && !fnmatch(szDictNameMask, dd->d_name, 0))
		{
			/*
			 * Обрабатываем каждый найденный файл соответствующий заданным критериям
			 */

			len = strlen(dd->d_name);
			file_name = malloc(plen + len + 10);

			if(!file_name)
			{
				printf(szCantAllocMemory);
				break;
			}

			strcpy(file_name, ppath);

			if(ppath[plen - 1] != '/')
			{
				file_name[plen] = '/';
				file_name[plen + 1] = 0;
			}

			strcat(file_name, dd->d_name);

			if(!ProcessDict(serial_number, file_name, FALSE))
				break;

			free(file_name);
			file_name = NULL;
		}
	}

	if(file_name)
		free(file_name);

	if(d)
		closedir(d);

	return bRet;
#endif /* _WIN32 */
}

BOOL
ProcessDict(
	const char* serial_number,
	const tchar* dict_name,
	BOOL bIgnoreNullLicenseId
	)

/*

Цель:
	Функция генерирует лицензию к указанному словарю

Параметры:
	const char* serial_number - серийный номер устройства PocketBook,
	const tchar* dict_name - имя файла словаря,
	BOOL bIgnoreNullLicenseId - параметр должен быть равен
	TRUE - если необходимо пропустить словарь с LicenseId равным нулю,
	FALSE - в противном случае

Возвращаемое значение:
	TRUE - в случае удачного завершения,
	FALSE - в противном случае

Примечание:
	Функция генерирует лицензию, привязанную к серийному номеру устройства, для указанного словаря

*/

{
	FILE *fin = NULL, *fout = NULL;
	DICT_HEADER hdr;
	LICENSE_BLOCK lic_block;
	uint8_t digest[16];
	char digest_str[33];
	tchar* license_file = NULL;
	size_t Length, i;
	BOOL bRet = FALSE;


	/*
	 * Открываем словарь и читаем его заголовок
	 */

	fin = _tfopen(dict_name, szReadOnly);

	if(!fin)
	{
		_tprintf(szCantOpenFile, dict_name);
		goto ErrorExit;
	}

	if(fread(&hdr, sizeof(hdr), 1, fin) != 1)
	{
		_tprintf(szCorruptedDict, dict_name);
		goto ErrorExit;
	}

	if(hdr.sign != DICTS_SIGNATURE)
	{
		_tprintf(szCorruptedDict, dict_name);
		goto ErrorExit;
	}

	if(!bIgnoreNullLicenseId && !hdr.licenseId)
	{ /* Если лицензия не требуется */
		bRet = TRUE;
		goto ErrorExit;
	}


	/*
	 * Формируем лицензионный блок
	 */

	memset(&lic_block, 0, sizeof(lic_block));
	strcpy(lic_block.serial_number, serial_number);

	lic_block.licensid[0] = hdr.licenseId;
	lic_block.licensid[1] = hdr.licenseId;
	lic_block.const_value = 5;


	/*
	 * Считаем MD5 лицензионного блока
	 */

	md5sum(&lic_block, sizeof(lic_block), digest);

	for(i = 0; i != 16; i++)
		sprintf(digest_str + (2 * i), szDigestByte, digest[i]);

	digest_str[32] = 0;
	Length = _tstrlen(dict_name);

	license_file = (tchar*)malloc(sizeof(tchar) * (Length + 10));

	if(!license_file)
	{
		_tprintf(szCantAllocMemory);
		goto ErrorExit;
	}

	_tstrcpy(license_file, dict_name);

	for(i = Length; i; i--)
	{
		if(license_file[i] == TEXT('.'))
		{
			license_file[i] = 0;
			break;
		}
	}

	_tstrcat(license_file, szLicExt);


	/*
	 * Создаём файл лицензии
	 */

	fout = _tfopen(license_file, szWriteOnly);

	if(!fout)
	{
		_tprintf(szCantCreateFile, license_file);
		goto ErrorExit;
	}

	if(fprintf(fout, szLicFileFormat, serial_number, digest_str, hdr.dict_name) < 0)
	{
		_tprintf(szCantWriteFile, license_file);
		goto ErrorExit;
	}

	bRet = TRUE;

ErrorExit:

	if(fout)
		fclose(fout);

	if(license_file)
		free(license_file);

	if(fin)
		fclose(fin);

	return bRet;
}

void
md5sum(
	const void* pdata,
	uint32_t cbdata,
	uint8_t* digest
	)

/*

Цель:
	Функция вычисляет MD5-хэш блока данных

Параметры:
	const void* pdata - указатель на блок данных, для которого необходимо рассчитать значение MD5,
	uint32_t cbdata - размер блока данных, на который указывает pdata,
	uint8_t* digest - указатель на массив, в который будет сохранено вычисленное значение хэша. Размер этого массива должен быть не менее 16 байт.

Возвращаемое значение:
	Нет

Примечание:
	Функция вычисляет MD5-хэш блока данных

*/

{
	MD5_CTX ctx;

	MD5Init(&ctx);
	MD5Update(&ctx, (uint8_t*)pdata, cbdata);
	MD5Final(&ctx);

	memcpy(digest, ctx.digest, 16);
}

#ifdef _WIN32
char*
SerialNumber(
	const wchar_t* serial_number
	)

/*

Цель:
	Вспомогательная функция. Преобразовывает строку серийного номера устройства из кодировки UTF-16 в кодировку UTF-8.

Параметры:
	const wchar_t* serial_number - серийный номер устройства в кодировке UTF-16

Возвращаемое значание:
	Серийный номер устройства в кодировке UTF-8

Примечание:
	Вспомогательная функция. Преобразовывает строку серийного номера устройства из кодировки UTF-16 в кодировку UTF-8. Память, выделенная под 
	преобразованную строку, должна быть освобождена вызовом free.

*/

{
	int Length;
	char* str;

	Length = WideCharToMultiByte(CP_UTF8, 0, serial_number, -1, NULL, 0, NULL, NULL);

	if(!Length)
		return NULL;

	str = (char*)malloc(Length);

	if(!str)
	{
		_tprintf(szCantAllocMemory);
		return NULL;
	}

	memset(str, 0, Length);
	WideCharToMultiByte(CP_UTF8, 0, serial_number, -1, str, Length, NULL, NULL);
	return str;
}
#endif /* _WIN32 */
