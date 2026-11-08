// Cynex: msvc crt karsiliklari
#pragma once

#include <math.h>
#include <float.h>
#include <alloca.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>

#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define _strcmpi strcasecmp
#define _wcsicmp wcscasecmp
#define _wcsnicmp wcsncasecmp
#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif
#ifndef strcmpi
#define strcmpi strcasecmp
#endif
#define _snprintf snprintf
#define _vsnprintf vsnprintf
#define _snwprintf swprintf
#define _vsnwprintf vswprintf
#define _strdup strdup
#define _wcsdup wcsdup
#define _alloca alloca
#define _fileno fileno
#define _unlink unlink
#define _chdir chdir
#define _rmdir rmdir
#define _getcwd getcwd
#define _chmod chmod
#define _isatty isatty
#define _fdopen fdopen
#define _strtoui64 strtoull
#define _strtoi64 strtoll
#define _atoi64 atoll
#define _atof atof
#define _isnan isnan
#define _finite isfinite
#define _copysign copysign
#define _hypot hypot
#define _getpid getpid
#define _stat stat
#define _fstat fstat
#define _S_IFDIR S_IFDIR
#define _S_IFREG S_IFREG
#define _S_IREAD S_IRUSR
#define _S_IWRITE S_IWUSR
#define _O_RDONLY O_RDONLY
#define _O_BINARY 0
#define O_BINARY 0
#define _MAX_INT_DIG 32
#define _TRUNCATE ((size_t)-1)
#define _CRT_SECURE_NO_WARNINGS
#define __debugbreak() __builtin_trap()
#ifndef __min
#define __min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef __max
#define __max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#define _byteswap_ushort __builtin_bswap16
#define _byteswap_ulong __builtin_bswap32
#define _byteswap_uint64 __builtin_bswap64
#define _MCW_PC 0x00030000
#define _PC_24 0x00020000
#define _PC_53 0x00010000
#define _PC_64 0x00000000
#define _MCW_RC 0x00000300
#define _RC_NEAR 0x00000000

typedef long long __time64_t;
typedef int errno_t;

#ifdef __cplusplus
extern "C" {
#endif

int _access(const char* path, int mode);
int _mkdir(const char* path);
char* _strupr(char* s);
char* _strlwr(char* s);
char* _itoa(int value, char* str, int radix);
char* _ltoa(long value, char* str, int radix);
char* _ultoa(unsigned long value, char* str, int radix);
char* _i64toa(long long value, char* str, int radix);
char* _ui64toa(unsigned long long value, char* str, int radix);
wchar_t* _itow(int value, wchar_t* str, int radix);
char* _ecvt(double value, int count, int* dec, int* sign);
char* _fcvt(double value, int count, int* dec, int* sign);
long _filelength(int fd);
unsigned int _controlfp(unsigned int newValue, unsigned int mask);
int _wtoi(const wchar_t* s);
// Cynex: clang -fms-extensions ile _rotl/_rotr/_rotl64/_rotr64 yerlesik geliyor
size_t _msize(void* p);
void* _aligned_malloc(size_t size, size_t alignment);
void _aligned_free(void* p);
int _getch(void);
int _kbhit(void);
char* _wsetlocale_dummy(int category, const wchar_t* locale);
#define _wsetlocale(c, l) _wsetlocale_dummy(c, l)

errno_t memcpy_s(void* dest, size_t destSize, const void* src, size_t count);
errno_t memmove_s(void* dest, size_t destSize, const void* src, size_t count);
errno_t strcpy_s(char* dest, size_t destSize, const char* src);
errno_t strncpy_s(char* dest, size_t destSize, const char* src, size_t count);
errno_t strcat_s(char* dest, size_t destSize, const char* src);
errno_t strncat_s(char* dest, size_t destSize, const char* src, size_t count);
errno_t fopen_s(FILE** pFile, const char* filename, const char* mode);
errno_t localtime_s(struct tm* tmDest, const time_t* sourceTime);
errno_t gmtime_s(struct tm* tmDest, const time_t* sourceTime);
errno_t _itoa_s(int value, char* buffer, size_t size, int radix);
char* strtok_s(char* str, const char* delim, char** context);
int sprintf_s(char* buffer, size_t sizeOfBuffer, const char* format, ...);
int vsprintf_s(char* buffer, size_t sizeOfBuffer, const char* format, va_list args);
int _snprintf_s(char* buffer, size_t sizeOfBuffer, size_t count, const char* format, ...);
int _vsnprintf_s(char* buffer, size_t sizeOfBuffer, size_t count, const char* format, va_list args);
int swprintf_s(wchar_t* buffer, size_t sizeOfBuffer, const wchar_t* format, ...);
int sscanf_s(const char* buffer, const char* format, ...);

#ifdef __cplusplus
}

// Cynex: <windows.h> min/max makrolarinin yerine - farkli tiplerde de calismali
#include <type_traits>
#include <utility>
template <class A, class B>
inline typename std::common_type<A, B>::type min(A a, B b) { return b < a ? b : a; }
template <class A, class B>
inline typename std::common_type<A, B>::type max(A a, B b) { return a < b ? b : a; }

template <size_t N>
inline errno_t strcpy_s(char (&dest)[N], const char* src) { return strcpy_s(dest, N, src); }
template <size_t N>
inline errno_t strncpy_s(char (&dest)[N], const char* src, size_t count) { return strncpy_s(dest, N, src, count); }
template <size_t N>
inline errno_t strcat_s(char (&dest)[N], const char* src) { return strcat_s(dest, N, src); }
template <size_t N, class... T>
inline int sprintf_s(char (&dest)[N], const char* format, T... args) { return snprintf(dest, N, format, args...); }
template <size_t N, class... T>
inline int swprintf_s(wchar_t (&dest)[N], const wchar_t* format, T... args) { return swprintf(dest, N, format, args...); }
template <size_t N, class... T>
inline int _snprintf_s(char (&dest)[N], size_t count, const char* format, T... args) { return snprintf(dest, N < count ? N : count + 1, format, args...); }
#endif
