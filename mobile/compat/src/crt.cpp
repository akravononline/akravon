// Cynex: msvc crt fonksiyonlari
#include <windows.h>
#include <iphlpapi.h>
#include <stdarg.h>
#include <malloc.h>
#include <time.h>
#include "../../platform/CynexPlatform.h"

extern "C" {

// Cynex: msvc derleyicisinin kayan nokta kullanildiginda aradigi sembol
int _fltused = 0x9875;

static char* IntToStr(unsigned long long v, bool neg, char* str, int radix)
{
	char buf[72];
	int i = 0;
	if (radix < 2 || radix > 36)
		radix = 10;
	do
	{
		const int d = (int)(v % (unsigned)radix);
		buf[i++] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
		v /= (unsigned)radix;
	} while (v);
	char* p = str;
	if (neg)
		*p++ = '-';
	while (i)
		*p++ = buf[--i];
	*p = 0;
	return str;
}

char* _itoa(int value, char* str, int radix)
{
	const bool neg = radix == 10 && value < 0;
	return IntToStr(neg ? (unsigned long long)(-(long long)value) : (unsigned long long)(unsigned int)value, neg, str, radix);
}

char* _ltoa(long value, char* str, int radix)
{
	const bool neg = radix == 10 && value < 0;
	return IntToStr(neg ? (unsigned long long)(-(long long)value) : (unsigned long long)value, neg, str, radix);
}

char* _ultoa(unsigned long value, char* str, int radix)
{
	return IntToStr(value, false, str, radix);
}

char* _i64toa(long long value, char* str, int radix)
{
	const bool neg = radix == 10 && value < 0;
	return IntToStr(neg ? (unsigned long long)(-value) : (unsigned long long)value, neg, str, radix);
}

char* _ui64toa(unsigned long long value, char* str, int radix)
{
	return IntToStr(value, false, str, radix);
}

wchar_t* _itow(int value, wchar_t* str, int radix)
{
	char buf[40];
	_itoa(value, buf, radix);
	int i = 0;
	for (; buf[i]; ++i)
		str[i] = (wchar_t)buf[i];
	str[i] = 0;
	return str;
}

errno_t _itoa_s(int value, char* buffer, size_t size, int radix)
{
	char tmp[40];
	_itoa(value, tmp, radix);
	if (strlen(tmp) >= size)
		return ERANGE;
	strcpy(buffer, tmp);
	return 0;
}

char* _strupr(char* s)
{
	for (char* p = s; *p; ++p)
		*p = (char)toupper((unsigned char)*p);
	return s;
}

char* _strlwr(char* s)
{
	for (char* p = s; *p; ++p)
		*p = (char)tolower((unsigned char)*p);
	return s;
}

char* _ecvt(double value, int count, int* dec, int* sign)
{
	static thread_local char buf[64];
	if (count < 1)
		count = 1;
	if (count > 17)
		count = 17;
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "%.*e", count - 1, value);
	*sign = tmp[0] == '-';
	const char* p = tmp + (*sign ? 1 : 0);
	int n = 0;
	for (; *p && *p != 'e'; ++p)
		if (*p != '.')
			buf[n++] = *p;
	buf[n] = 0;
	*dec = (*p == 'e') ? atoi(p + 1) + 1 : 1;
	if (value == 0.0)
		*dec = 0;
	return buf;
}

char* _fcvt(double value, int count, int* dec, int* sign)
{
	static thread_local char buf[400];
	char tmp[400];
	snprintf(tmp, sizeof(tmp), "%.*f", count < 0 ? 0 : count, value);
	*sign = tmp[0] == '-';
	const char* p = tmp + (*sign ? 1 : 0);
	const char* dot = strchr(p, '.');
	*dec = dot ? (int)(dot - p) : (int)strlen(p);
	int n = 0;
	for (; *p; ++p)
		if (*p != '.')
			buf[n++] = *p;
	buf[n] = 0;
	return buf;
}

unsigned int _controlfp(unsigned int newValue, unsigned int mask)
{
	// Cynex: x87 hassasiyet ayari arm/x64 sse'de anlamsiz
	return newValue & mask;
}

int _wtoi(const wchar_t* s)
{
	return (int)wcstol(s, nullptr, 10);
}

size_t _msize(void* p)
{
	return malloc_usable_size(p);
}

void* _aligned_malloc(size_t size, size_t alignment)
{
	void* p = nullptr;
	if (alignment < sizeof(void*))
		alignment = sizeof(void*);
	return posix_memalign(&p, alignment, size) == 0 ? p : nullptr;
}

void _aligned_free(void* p)
{
	free(p);
}

int _getch(void)
{
	return 0;
}

int _kbhit(void)
{
	return 0;
}

char* _wsetlocale_dummy(int, const wchar_t*)
{
	return (char*)"C";
}

errno_t memcpy_s(void* dest, size_t destSize, const void* src, size_t count)
{
	if (count == 0)
		return 0;
	if (!dest || !src || destSize < count)
	{
		if (dest && destSize)
			memset(dest, 0, destSize);
		return EINVAL;
	}
	memcpy(dest, src, count);
	return 0;
}

errno_t memmove_s(void* dest, size_t destSize, const void* src, size_t count)
{
	if (count == 0)
		return 0;
	if (!dest || !src || destSize < count)
		return EINVAL;
	memmove(dest, src, count);
	return 0;
}

errno_t strcpy_s(char* dest, size_t destSize, const char* src)
{
	if (!dest || !destSize)
		return EINVAL;
	if (!src)
	{
		dest[0] = 0;
		return EINVAL;
	}
	const size_t n = strlen(src);
	if (n >= destSize)
	{
		dest[0] = 0;
		return ERANGE;
	}
	memcpy(dest, src, n + 1);
	return 0;
}

errno_t strncpy_s(char* dest, size_t destSize, const char* src, size_t count)
{
	if (!dest || !destSize)
		return EINVAL;
	if (!src)
	{
		dest[0] = 0;
		return EINVAL;
	}
	size_t n = strnlen(src, count == _TRUNCATE ? destSize - 1 : count);
	if (n >= destSize)
	{
		if (count == _TRUNCATE)
			n = destSize - 1;
		else
		{
			dest[0] = 0;
			return ERANGE;
		}
	}
	memcpy(dest, src, n);
	dest[n] = 0;
	return 0;
}

errno_t strcat_s(char* dest, size_t destSize, const char* src)
{
	if (!dest || !src)
		return EINVAL;
	const size_t a = strnlen(dest, destSize);
	const size_t b = strlen(src);
	if (a + b >= destSize)
		return ERANGE;
	memcpy(dest + a, src, b + 1);
	return 0;
}

errno_t strncat_s(char* dest, size_t destSize, const char* src, size_t count)
{
	if (!dest || !src)
		return EINVAL;
	const size_t a = strnlen(dest, destSize);
	size_t b = strnlen(src, count);
	if (a + b >= destSize)
	{
		if (count != _TRUNCATE)
			return ERANGE;
		b = destSize - a - 1;
	}
	memcpy(dest + a, src, b);
	dest[a + b] = 0;
	return 0;
}

errno_t fopen_s(FILE** pFile, const char* filename, const char* mode)
{
	*pFile = fopen(filename, mode);
	return *pFile ? 0 : errno;
}

errno_t localtime_s(struct tm* tmDest, const time_t* sourceTime)
{
	return localtime_r(sourceTime, tmDest) ? 0 : EINVAL;
}

errno_t gmtime_s(struct tm* tmDest, const time_t* sourceTime)
{
	return gmtime_r(sourceTime, tmDest) ? 0 : EINVAL;
}

char* strtok_s(char* str, const char* delim, char** context)
{
	return strtok_r(str, delim, context);
}

int vsprintf_s(char* buffer, size_t sizeOfBuffer, const char* format, va_list args)
{
	return vsnprintf(buffer, sizeOfBuffer, format, args);
}

int sprintf_s(char* buffer, size_t sizeOfBuffer, const char* format, ...)
{
	va_list va;
	va_start(va, format);
	const int r = vsnprintf(buffer, sizeOfBuffer, format, va);
	va_end(va);
	return r;
}

int _vsnprintf_s(char* buffer, size_t sizeOfBuffer, size_t count, const char* format, va_list args)
{
	const size_t n = (count == _TRUNCATE || count + 1 > sizeOfBuffer) ? sizeOfBuffer : count + 1;
	return vsnprintf(buffer, n, format, args);
}

int _snprintf_s(char* buffer, size_t sizeOfBuffer, size_t count, const char* format, ...)
{
	va_list va;
	va_start(va, format);
	const int r = _vsnprintf_s(buffer, sizeOfBuffer, count, format, va);
	va_end(va);
	return r;
}

int swprintf_s(wchar_t* buffer, size_t sizeOfBuffer, const wchar_t* format, ...)
{
	va_list va;
	va_start(va, format);
	const int r = vswprintf(buffer, sizeOfBuffer, format, va);
	va_end(va);
	return r;
}

int sscanf_s(const char* buffer, const char* format, ...)
{
	// Cynex: oyunda yalnizca sayi okunuyor (%s boyut parametresi yok)
	va_list va;
	va_start(va, format);
	const int r = vsscanf(buffer, format, va);
	va_end(va);
	return r;
}

// Cynex: hwid icin cihaza ozel kalici sahte mac (android id ozeti)
DWORD GetIpNetTable(PMIB_IPNETTABLE IpNetTable, PULONG SizePointer, BOOL)
{
	if (!SizePointer)
		return ERROR_INVALID_PARAMETER;
	if (!IpNetTable || *SizePointer < sizeof(MIB_IPNETTABLE))
	{
		*SizePointer = sizeof(MIB_IPNETTABLE);
		return ERROR_INSUFFICIENT_BUFFER;
	}
	memset(IpNetTable, 0, sizeof(MIB_IPNETTABLE));
	IpNetTable->dwNumEntries = 1;
	MIB_IPNETROW& row = IpNetTable->table[0];
	row.dwType = 3;
	row.dwPhysAddrLen = 6;

	const char* id = CynexPlatform_GetDeviceId();
	uint64_t h = 1469598103934665603ULL;	// fnv-1a
	for (const char* p = id ? id : ""; *p; ++p)
	{
		h ^= (unsigned char)*p;
		h *= 1099511628211ULL;
	}
	for (int i = 0; i < 6; ++i)
		row.bPhysAddr[i] = (BYTE)(h >> (i * 8));
	row.bPhysAddr[0] = (BYTE)((row.bPhysAddr[0] & 0xFE) | 0x02);	// Cynex: yerel yonetimli, tekil adres
	return NO_ERROR;
}

}	// extern "C"
