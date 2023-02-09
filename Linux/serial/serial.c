
/*

Цель:
	Программа для генерации привязки программного обеспечения PocketBook 626 к серийным номерам устройства и карты памяти
	Распространяется под лицензией GNU GPLv3+
	Лицензия GNU GPL версии 3 или новее: https://gnu.org/licenses/gpl.html

*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tchar.h"
#include "serial.h"


const tchar szUsage[] 						= TEXT("\nИспользование:\n")
		TEXT("\tserial --serial_number серийный_номер_устройства --sd_serial серийный_номер_карты_памяти [имя_выходного_файла]\n\n")
		TEXT("\t--serial_number\t\tсерийный номер устройства;\n")
		TEXT("\t--sd_serial\t\tсерийный номер карты памяти (0x12345678);\n")
		TEXT("\t--help\t-h\t\tвывести это сообщение;\n")
		TEXT("\tимя_выходного_файла\tимя выходного файла. По умолчанию .freezestatus.\n\n");

const tchar szDuplicateParameter[]			= TEXT("Дублирующий параметр: %s. Проигнорирован.\n");
const tchar szUnknownParameter[]			= TEXT("Неизвестный параметр: %s!\n");
const tchar szInvalidParameter[]			= TEXT("Ошибка в формате параметра: %s!\n");
const tchar szSerialNumberIsAbsent[]		= TEXT("Отсутствует серийный номер устройства!\n");
const tchar szSdSerialIsAbsent[]			= TEXT("Отсутствует серийный номер карты памяти!\n");
const tchar szCantCreateFile[]				= TEXT("Невозможно создать файл: %s!\n");
const tchar szCantWriteFile[]				= TEXT("Невозможно записать файл: %s!\n");
const tchar szSdSerialFormat[]				= TEXT("%x");
const tchar szWriteOnly[]					= TEXT("wb");
const tchar szDefaultOutName[]				= TEXT(".freezestatus");

const tchar szSerialNumber[] 				= TEXT("serial_number");
const tchar szSdSerial[] 					= TEXT("sd_serial");
const tchar szHelp[] 						= TEXT("help");
const tchar szHelpShort[] 					= TEXT("h");


int _tmain(int argc, tchar* argv[])
{
	int i, n;
	tchar* serial_number = NULL;
	BOOL bsd = FALSE;
	uint32_t sd_serial, status;
	const tchar* out_name = NULL;
	FILE* f;

	if(argc < 2)
	{
		_tprintf(szUsage);
		return EXIT_FAILURE;
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
						return EXIT_FAILURE;
					}

					serial_number = argv[i];
					continue;
				}

				if(!_tstrcmp(argv[i] + 2, szSdSerial))
				{
					if(bsd)
					{
						_tprintf(szDuplicateParameter, argv[i++]);
						continue;
					}

					i++;

					if(i >= argc)
					{
						_tprintf(szInvalidParameter, argv[--i]);
						return EXIT_FAILURE;
					}

					n = _tsscanf(argv[i], szSdSerialFormat, &sd_serial);

					if(!n || (n == -1))
					{
						_tprintf(szInvalidParameter, argv[--i]);
						return EXIT_FAILURE;
					}

					bsd = TRUE;
					continue;
				}

				if(!_tstrcmp(argv[i] + 2, szHelp))
				{
					_tprintf(szUsage);
					return EXIT_SUCCESS;
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
					return EXIT_SUCCESS;
				}
			}
		}
		else
		{
			if(out_name)
				_tprintf(szDuplicateParameter, argv[i]);
			else
				out_name = argv[i];

			continue;
		}

		_tprintf(szUnknownParameter, argv[i]);
		return EXIT_FAILURE;
	}

	if(!serial_number)
	{
		_tprintf(szSerialNumberIsAbsent);
		return EXIT_FAILURE;
	}

	if(!bsd)
	{
		_tprintf(szSdSerialIsAbsent);
		return EXIT_FAILURE;
	}

	/*
	 * Вычисление привязки
	 */

	status = calc_freezestatus(serial_number, sd_serial);

	/*
	 * Сохранение привязки в файл
	 */

	if(!out_name)
		out_name = szDefaultOutName;

	f = _tfopen(out_name, szWriteOnly);

	if(!f)
	{
		_tprintf(szCantCreateFile, out_name);
		return EXIT_FAILURE;
	}

	if(!fwrite(&status, sizeof(status), 1, f))
	{
		_tprintf(szCantWriteFile, out_name);
		fclose(f);
		return EXIT_FAILURE;
	}

	fclose(f);
	return EXIT_SUCCESS;
}

uint32_t
calc_freezestatus(
	const tchar* serial_number,
	uint32_t sd_serial
	)

/*

Цель:
	Функция вычисляет привязку программного обеспечения PocketBook 626 к серийным номерам устройства и карты памяти

Параметры:
	const char* serial_number - указатель на строку, содержащую серийный номер устройства,
	uint32_t sd_serial - значение серийного номера карты памяти

Возвращаемое значение:
	Привязка программного обеспечения PocketBook 626 к серийным номерам устройства и карты памяти

Примечание:
	Функция вычисляет привязку программного обеспечения PocketBook 626 к серийным номерам устройства и карты памяти. Информация была почерпнута из
	анализа	monitor.app R626.5.14.340 (20160524_153358).

*/

{
	size_t Length, i;
	uint32_t status;

	Length = _tstrlen(serial_number);
	status = 0;

	for(i = 0; i != Length; i++)
		status = (61 * status) + serial_number[i];

	status = (0x2D10A39B * (status ^ 0x6ECA1735)) + (0x51670EDF * (sd_serial ^ 0x3AC19B9E)) + (0x36DCC025 * status) + sd_serial;
	return status;
}
