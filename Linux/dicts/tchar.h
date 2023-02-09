
#ifndef __TCHAR_H__
#define __TCHAR_H__

#undef TEXT

#ifdef _WIN32

#define TEXT(quote) L##quote

typedef wchar_t tchar;

#define _tmain wmain
#define _tfopen _wfopen
#define _tprintf wprintf
#define _tfprintf wfprintf
#define _tsprintf wsprintf
#define _tstrcmp wcscmp
#define _tstrcpy wcscpy
#define _tstrcat wcscat
#define _tstrlen wcslen
#else

#define TEXT(quote) quote

typedef char tchar;

#define _tmain main
#define _tfopen fopen
#define _tprintf printf
#define _tfprintf fprintf
#define _tsprintf sprintf
#define _tstrcmp strcmp
#define _tstrcpy strcpy
#define _tstrcat strcat
#define _tstrlen strlen
#endif /* _WIN32 */

#endif /* __TCHAR_H__ */
