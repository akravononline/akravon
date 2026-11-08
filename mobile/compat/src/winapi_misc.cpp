// Cynex: kernel32 diger - zaman, hata, sistem, bellek, kod sayfasi, ini, kayit defteri
#include "cynex_handle.h"
#include "codepage_tables.h"
#include <imm.h>
#include <android/log.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <malloc.h>
#include <mutex>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

namespace
{
	thread_local DWORD t_lastError = 0;

	uint64_t MonotonicMicros()
	{
		timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
	}

	void FillSystemTime(const struct tm& t, int ms, LPSYSTEMTIME st)
	{
		st->wYear = (WORD)(t.tm_year + 1900);
		st->wMonth = (WORD)(t.tm_mon + 1);
		st->wDayOfWeek = (WORD)t.tm_wday;
		st->wDay = (WORD)t.tm_mday;
		st->wHour = (WORD)t.tm_hour;
		st->wMinute = (WORD)t.tm_min;
		st->wSecond = (WORD)t.tm_sec;
		st->wMilliseconds = (WORD)ms;
	}

	// Cynex: oyunun varsayilan kod sayfasi - locale.cfg'den gelir, turkce 1254
	UINT s_defaultCodePage = 1254;

	const uint16_t* FindTable(UINT cp)
	{
		if (cp == CP_ACP || cp == CP_OEMCP || cp == CP_THREAD_ACP)
			cp = s_defaultCodePage;
		for (const auto& e : g_cynexCodePages)
			if (e.cp == cp)
				return e.table;
		return nullptr;
	}

	UINT ResolveCodePage(UINT cp)
	{
		return (cp == CP_ACP || cp == CP_OEMCP || cp == CP_THREAD_ACP) ? s_defaultCodePage : cp;
	}

	int DecodeUtf8(const unsigned char* s, int n, wchar_t* out, int cap)
	{
		int count = 0;
		for (int i = 0; i < n;)
		{
			uint32_t c = s[i];
			int len = c < 0x80 ? 1 : (c >> 5) == 6 ? 2 : (c >> 4) == 14 ? 3 : (c >> 3) == 30 ? 4 : 1;
			if (i + len > n)
				len = 1;
			if (len == 2)
				c = ((c & 0x1F) << 6) | (s[i + 1] & 0x3F);
			else if (len == 3)
				c = ((c & 0x0F) << 12) | ((s[i + 1] & 0x3F) << 6) | (s[i + 2] & 0x3F);
			else if (len == 4)
				c = ((c & 0x07) << 18) | ((s[i + 1] & 0x3F) << 12) | ((s[i + 2] & 0x3F) << 6) | (s[i + 3] & 0x3F);
			else if (c >= 0x80)
				c = 0xFFFD;
			if (out)
			{
				if (count >= cap)
					return 0;
				out[count] = (wchar_t)c;
			}
			++count;
			i += len;
		}
		return count;
	}

	int EncodeUtf8(const wchar_t* s, int n, char* out, int cap)
	{
		int count = 0;
		for (int i = 0; i < n; ++i)
		{
			uint32_t c = (uint32_t)s[i];
			char buf[4];
			int len;
			if (c < 0x80) { buf[0] = (char)c; len = 1; }
			else if (c < 0x800) { buf[0] = (char)(0xC0 | (c >> 6)); buf[1] = (char)(0x80 | (c & 0x3F)); len = 2; }
			else if (c < 0x10000) { buf[0] = (char)(0xE0 | (c >> 12)); buf[1] = (char)(0x80 | ((c >> 6) & 0x3F)); buf[2] = (char)(0x80 | (c & 0x3F)); len = 3; }
			else { buf[0] = (char)(0xF0 | (c >> 18)); buf[1] = (char)(0x80 | ((c >> 12) & 0x3F)); buf[2] = (char)(0x80 | ((c >> 6) & 0x3F)); buf[3] = (char)(0x80 | (c & 0x3F)); len = 4; }
			if (out)
			{
				if (count + len > cap)
					return 0;
				memcpy(out + count, buf, len);
			}
			count += len;
		}
		return count;
	}

	// ini - basit, bolum/anahtar harf duyarsiz
	struct SIniFile
	{
		std::map<std::string, std::map<std::string, std::string>> sections;
	};

	std::string Lower(std::string s)
	{
		for (auto& c : s)
			c = (char)tolower((unsigned char)c);
		return s;
	}

	std::string Trim(const std::string& s)
	{
		size_t a = s.find_first_not_of(" \t\r\n");
		if (a == std::string::npos)
			return "";
		size_t b = s.find_last_not_of(" \t\r\n");
		return s.substr(a, b - a + 1);
	}

	bool LoadIni(const char* file, SIniFile& ini)
	{
		std::ifstream in(cynex::ResolvePath(file), std::ios::binary);
		if (!in)
			return false;
		std::string line, section;
		while (std::getline(in, line))
		{
			line = Trim(line);
			if (line.empty() || line[0] == ';' || line[0] == '#')
				continue;
			if (line[0] == '[')
			{
				size_t e = line.find(']');
				section = Lower(Trim(line.substr(1, e == std::string::npos ? std::string::npos : e - 1)));
				continue;
			}
			size_t eq = line.find('=');
			if (eq == std::string::npos)
				continue;
			ini.sections[section][Lower(Trim(line.substr(0, eq)))] = Trim(line.substr(eq + 1));
		}
		return true;
	}

	std::mutex s_iniMutex;
}

namespace cynex
{
	void SetDefaultCodePageCompat(UINT cp)
	{
		if (cp)
			s_defaultCodePage = cp;
	}
}

extern "C" {

DWORD GetTickCount(void)
{
	return (DWORD)(MonotonicMicros() / 1000ULL);
}

ULONGLONG GetTickCount64(void)
{
	return MonotonicMicros() / 1000ULL;
}

DWORD timeGetTime(void)
{
	return (DWORD)(MonotonicMicros() / 1000ULL);
}

UINT timeBeginPeriod(UINT)
{
	return 0;
}

UINT timeEndPeriod(UINT)
{
	return 0;
}

BOOL QueryPerformanceCounter(LARGE_INTEGER* c)
{
	c->QuadPart = (LONGLONG)MonotonicMicros();
	return TRUE;
}

BOOL QueryPerformanceFrequency(LARGE_INTEGER* f)
{
	f->QuadPart = 1000000;
	return TRUE;
}

void Sleep(DWORD ms)
{
	if (ms == 0)
	{
		sched_yield();
		return;
	}
	timespec ts = { (time_t)(ms / 1000), (long)(ms % 1000) * 1000000L };
	while (nanosleep(&ts, &ts) != 0 && errno == EINTR)
	{
	}
}

void GetLocalTime(LPSYSTEMTIME st)
{
	timeval tv;
	gettimeofday(&tv, nullptr);
	struct tm t;
	time_t sec = tv.tv_sec;
	localtime_r(&sec, &t);
	FillSystemTime(t, (int)(tv.tv_usec / 1000), st);
}

void GetSystemTime(LPSYSTEMTIME st)
{
	timeval tv;
	gettimeofday(&tv, nullptr);
	struct tm t;
	time_t sec = tv.tv_sec;
	gmtime_r(&sec, &t);
	FillSystemTime(t, (int)(tv.tv_usec / 1000), st);
}

void GetSystemTimeAsFileTime(LPFILETIME ft)
{
	timeval tv;
	gettimeofday(&tv, nullptr);
	uint64_t v = ((uint64_t)tv.tv_sec + 11644473600ULL) * 10000000ULL + (uint64_t)tv.tv_usec * 10ULL;
	ft->dwLowDateTime = (DWORD)v;
	ft->dwHighDateTime = (DWORD)(v >> 32);
}

BOOL FileTimeToSystemTime(const FILETIME* ft, LPSYSTEMTIME st)
{
	uint64_t v = ((uint64_t)ft->dwHighDateTime << 32) | ft->dwLowDateTime;
	time_t sec = (time_t)(v / 10000000ULL - 11644473600ULL);
	struct tm t;
	gmtime_r(&sec, &t);
	FillSystemTime(t, (int)((v / 10000ULL) % 1000ULL), st);
	return TRUE;
}

BOOL SystemTimeToFileTime(const SYSTEMTIME* st, LPFILETIME ft)
{
	struct tm t = {};
	t.tm_year = st->wYear - 1900;
	t.tm_mon = st->wMonth - 1;
	t.tm_mday = st->wDay;
	t.tm_hour = st->wHour;
	t.tm_min = st->wMinute;
	t.tm_sec = st->wSecond;
	time_t sec = timegm(&t);
	uint64_t v = ((uint64_t)sec + 11644473600ULL) * 10000000ULL + (uint64_t)st->wMilliseconds * 10000ULL;
	ft->dwLowDateTime = (DWORD)v;
	ft->dwHighDateTime = (DWORD)(v >> 32);
	return TRUE;
}

BOOL FileTimeToLocalFileTime(const FILETIME* ft, LPFILETIME out)
{
	time_t now = time(nullptr);
	struct tm lt;
	localtime_r(&now, &lt);
	int64_t v = (int64_t)(((uint64_t)ft->dwHighDateTime << 32) | ft->dwLowDateTime) + (int64_t)lt.tm_gmtoff * 10000000LL;
	out->dwLowDateTime = (DWORD)v;
	out->dwHighDateTime = (DWORD)((uint64_t)v >> 32);
	return TRUE;
}

LONG CompareFileTime(const FILETIME* a, const FILETIME* b)
{
	uint64_t x = ((uint64_t)a->dwHighDateTime << 32) | a->dwLowDateTime;
	uint64_t y = ((uint64_t)b->dwHighDateTime << 32) | b->dwLowDateTime;
	return x < y ? -1 : (x > y ? 1 : 0);
}

DWORD GetLastError(void)
{
	return t_lastError;
}

void SetLastError(DWORD dwErrCode)
{
	t_lastError = dwErrCode;
}

void OutputDebugStringA(LPCSTR s)
{
	__android_log_write(ANDROID_LOG_DEBUG, "CynexMobil", s ? s : "");
}

BOOL IsDebuggerPresent(void)
{
	return FALSE;
}

void DebugBreak(void)
{
	__builtin_trap();
}

BOOL AllocConsole(void)
{
	return TRUE;
}

BOOL FreeConsole(void)
{
	return TRUE;
}

BOOL SetConsoleTitleA(LPCSTR)
{
	return TRUE;
}

HANDLE GetStdHandle(DWORD)
{
	return NULL;
}

void GetSystemInfo(LPSYSTEM_INFO si)
{
	memset(si, 0, sizeof(*si));
	si->dwPageSize = (DWORD)sysconf(_SC_PAGESIZE);
	si->dwAllocationGranularity = 65536;
	si->dwNumberOfProcessors = (DWORD)sysconf(_SC_NPROCESSORS_ONLN);
	si->lpMinimumApplicationAddress = (LPVOID)0x10000;
	si->lpMaximumApplicationAddress = (LPVOID)(uintptr_t)0x7fffffffffffULL;
	si->dwActiveProcessorMask = (DWORD_PTR)((1ULL << std::min<DWORD>(si->dwNumberOfProcessors, 63)) - 1);
#if defined(__aarch64__)
	si->wProcessorArchitecture = 12;
#else
	si->wProcessorArchitecture = 9;
#endif
}

BOOL GetVersionExA(LPOSVERSIONINFO vi)
{
	// Cynex: windows 10 gibi gorun
	const DWORD size = vi->dwOSVersionInfoSize;
	memset((char*)vi + sizeof(DWORD), 0, size > sizeof(DWORD) ? size - sizeof(DWORD) : 0);
	vi->dwMajorVersion = 10;
	vi->dwMinorVersion = 0;
	vi->dwBuildNumber = 19045;
	vi->dwPlatformId = VER_PLATFORM_WIN32_NT;
	return TRUE;
}

DWORD GetVersion(void)
{
	return 10 | (0 << 8) | (19045u << 16);
}

BOOL GetComputerNameA(LPSTR lpBuffer, LPDWORD nSize)
{
	const char* name = "CYNEXMOBIL";
	if (!lpBuffer || !nSize || *nSize <= strlen(name))
		return FALSE;
	strcpy(lpBuffer, name);
	*nSize = (DWORD)strlen(name);
	return TRUE;
}

BOOL GetUserNameA(LPSTR lpBuffer, LPDWORD pcbBuffer)
{
	const char* name = "cynex";
	if (!lpBuffer || !pcbBuffer || *pcbBuffer <= strlen(name))
		return FALSE;
	strcpy(lpBuffer, name);
	*pcbBuffer = (DWORD)strlen(name) + 1;
	return TRUE;
}

LPSTR GetCommandLineA(void)
{
	static char s_cmd[] = "metin2client.bin";
	return s_cmd;
}

DWORD GetEnvironmentVariableA(LPCSTR lpName, LPSTR lpBuffer, DWORD nSize)
{
	const char* v = getenv(lpName);
	if (!v)
		return 0;
	const DWORD len = (DWORD)strlen(v);
	if (!lpBuffer || nSize <= len)
		return len + 1;
	strcpy(lpBuffer, v);
	return len;
}

UINT GetACP(void)
{
	return s_defaultCodePage;
}

UINT GetOEMCP(void)
{
	return s_defaultCodePage;
}

LCID GetUserDefaultLCID(void)
{
	return MAKELCID(MAKELANGID(LANG_TURKISH, SUBLANG_DEFAULT), SORT_DEFAULT);
}

LCID GetSystemDefaultLCID(void)
{
	return GetUserDefaultLCID();
}

LANGID GetUserDefaultLangID(void)
{
	return MAKELANGID(LANG_TURKISH, SUBLANG_DEFAULT);
}

LANGID GetSystemDefaultLangID(void)
{
	return GetUserDefaultLangID();
}

HMODULE LoadLibraryA(LPCSTR)
{
	SetLastError(ERROR_FILE_NOT_FOUND);
	return NULL;
}

FARPROC GetProcAddress(HMODULE, LPCSTR)
{
	return NULL;
}

BOOL FreeLibrary(HMODULE)
{
	return TRUE;
}

// bellek
HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes)
{
	// Cynex: basa boyut yaz - GlobalSize icin
	size_t* p = (size_t*)((uFlags & GMEM_ZEROINIT) ? calloc(1, dwBytes + sizeof(size_t) * 2) : malloc(dwBytes + sizeof(size_t) * 2));
	if (!p)
		return NULL;
	p[0] = dwBytes;
	return (HGLOBAL)(p + 2);
}

LPVOID GlobalLock(HGLOBAL hMem)
{
	return hMem;
}

BOOL GlobalUnlock(HGLOBAL)
{
	return TRUE;
}

HGLOBAL GlobalFree(HGLOBAL hMem)
{
	if (hMem)
		free((size_t*)hMem - 2);
	return NULL;
}

SIZE_T GlobalSize(HGLOBAL hMem)
{
	return hMem ? ((size_t*)hMem - 2)[0] : 0;
}

HLOCAL LocalAlloc(UINT uFlags, SIZE_T uBytes)
{
	return GlobalAlloc(uFlags, uBytes);
}

HLOCAL LocalFree(HLOCAL hMem)
{
	return GlobalFree(hMem);
}

LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
	void* p = mmap(lpAddress, dwSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	return p == MAP_FAILED ? NULL : p;
}

BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType)
{
	// Cynex: MEM_RELEASE'de boyut 0 gelir - oyun VirtualAlloc kullanmiyor, sizinti kabul
	if (dwSize)
		munmap(lpAddress, dwSize);
	return TRUE;
}

BOOL VirtualProtect(LPVOID, SIZE_T, DWORD, PDWORD lpflOldProtect)
{
	if (lpflOldProtect)
		*lpflOldProtect = PAGE_READWRITE;
	return TRUE;
}

void GlobalMemoryStatus(LPMEMORYSTATUS ms)
{
	struct sysinfo si;
	sysinfo(&si);
	memset(ms, 0, sizeof(*ms));
	ms->dwLength = sizeof(*ms);
	ms->dwTotalPhys = (SIZE_T)si.totalram * si.mem_unit;
	ms->dwAvailPhys = (SIZE_T)si.freeram * si.mem_unit;
	ms->dwTotalVirtual = ms->dwTotalPhys;
	ms->dwAvailVirtual = ms->dwAvailPhys;
	ms->dwMemoryLoad = ms->dwTotalPhys ? (DWORD)(100 - ms->dwAvailPhys * 100 / ms->dwTotalPhys) : 0;
}

BOOL GlobalMemoryStatusEx(LPMEMORYSTATUSEX ms)
{
	struct sysinfo si;
	sysinfo(&si);
	const DWORD len = ms->dwLength;
	memset(ms, 0, sizeof(*ms));
	ms->dwLength = len;
	ms->ullTotalPhys = (DWORDLONG)si.totalram * si.mem_unit;
	ms->ullAvailPhys = (DWORDLONG)si.freeram * si.mem_unit;
	ms->ullTotalVirtual = ms->ullTotalPhys;
	ms->ullAvailVirtual = ms->ullAvailPhys;
	ms->dwMemoryLoad = ms->ullTotalPhys ? (DWORD)(100 - ms->ullAvailPhys * 100 / ms->ullTotalPhys) : 0;
	return TRUE;
}

BOOL IsBadReadPtr(const void* lp, UINT_PTR)
{
	return lp == NULL;
}

BOOL IsBadWritePtr(LPVOID lp, UINT_PTR)
{
	return lp == NULL;
}

HANDLE GetProcessHeap(void)
{
	static int s_heap;
	return &s_heap;
}

LPVOID HeapAlloc(HANDLE, DWORD dwFlags, SIZE_T dwBytes)
{
	return (dwFlags & HEAP_ZERO_MEMORY) ? calloc(1, dwBytes) : malloc(dwBytes);
}

BOOL HeapFree(HANDLE, DWORD, LPVOID lpMem)
{
	free(lpMem);
	return TRUE;
}

// kod sayfasi
int MultiByteToWideChar(UINT CodePage, DWORD, LPCSTR src, int n, LPWSTR dst, int cap)
{
	if (!src)
		return 0;
	const bool terminated = n < 0;
	if (terminated)
		n = (int)strlen(src) + 1;
	const UINT cp = ResolveCodePage(CodePage);
	if (cp == CP_UTF8)
		return DecodeUtf8((const unsigned char*)src, n, cap ? dst : nullptr, cap);

	const uint16_t* table = FindTable(cp);
	if (!cap)
		return n;
	if (cap < n)
	{
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return 0;
	}
	for (int i = 0; i < n; ++i)
	{
		const unsigned char c = (unsigned char)src[i];
		dst[i] = (c < 0x80 || !table) ? (wchar_t)c : (wchar_t)table[c - 0x80];
	}
	return n;
}

int WideCharToMultiByte(UINT CodePage, DWORD, LPCWSTR src, int n, LPSTR dst, int cap, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
	if (!src)
		return 0;
	if (n < 0)
		n = (int)wcslen(src) + 1;
	if (lpUsedDefaultChar)
		*lpUsedDefaultChar = FALSE;
	const UINT cp = ResolveCodePage(CodePage);
	if (cp == CP_UTF8)
		return EncodeUtf8(src, n, cap ? dst : nullptr, cap);

	const uint16_t* table = FindTable(cp);
	if (!cap)
		return n;
	if (cap < n)
	{
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return 0;
	}
	const char def = lpDefaultChar ? *lpDefaultChar : '?';
	for (int i = 0; i < n; ++i)
	{
		const uint32_t c = (uint32_t)src[i];
		char out = def;
		if (c < 0x80)
			out = (char)c;
		else if (table)
		{
			for (int k = 0; k < 128; ++k)
			{
				if (table[k] == c)
				{
					out = (char)(0x80 + k);
					break;
				}
			}
		}
		else if (c < 0x100)
			out = (char)c;
		if (out == def && c >= 0x80 && lpUsedDefaultChar)
			*lpUsedDefaultChar = TRUE;
		dst[i] = out;
	}
	return n;
}

BOOL IsDBCSLeadByte(BYTE)
{
	return FALSE;
}

BOOL IsDBCSLeadByteEx(UINT, BYTE)
{
	return FALSE;
}

LPSTR CharNextA(LPCSTR lpsz)
{
	return (LPSTR)(*lpsz ? lpsz + 1 : lpsz);
}

LPSTR CharPrevA(LPCSTR lpszStart, LPCSTR lpszCurrent)
{
	return (LPSTR)(lpszCurrent > lpszStart ? lpszCurrent - 1 : lpszStart);
}

LPSTR CharNextExA(WORD, LPCSTR lpCurrentChar, DWORD)
{
	return CharNextA(lpCurrentChar);
}

LPSTR CharPrevExA(WORD, LPCSTR lpStart, LPCSTR lpCurrentChar, DWORD)
{
	return CharPrevA(lpStart, lpCurrentChar);
}

LPSTR CharUpperA(LPSTR lpsz)
{
	if ((uintptr_t)lpsz < 0x10000)
		return (LPSTR)(uintptr_t)toupper((int)(uintptr_t)lpsz);
	for (char* p = lpsz; *p; ++p)
		*p = (char)toupper((unsigned char)*p);
	return lpsz;
}

LPSTR CharLowerA(LPSTR lpsz)
{
	if ((uintptr_t)lpsz < 0x10000)
		return (LPSTR)(uintptr_t)tolower((int)(uintptr_t)lpsz);
	for (char* p = lpsz; *p; ++p)
		*p = (char)tolower((unsigned char)*p);
	return lpsz;
}

int lstrlenA(LPCSTR s)
{
	return s ? (int)strlen(s) : 0;
}

int lstrlenW(LPCWSTR s)
{
	return s ? (int)wcslen(s) : 0;
}

LPSTR lstrcpyA(LPSTR a, LPCSTR b)
{
	return strcpy(a, b);
}

LPSTR lstrcpynA(LPSTR a, LPCSTR b, int n)
{
	if (n <= 0)
		return a;
	strncpy(a, b, n - 1);
	a[n - 1] = 0;
	return a;
}

LPSTR lstrcatA(LPSTR a, LPCSTR b)
{
	return strcat(a, b);
}

int lstrcmpA(LPCSTR a, LPCSTR b)
{
	return strcmp(a, b);
}

int lstrcmpiA(LPCSTR a, LPCSTR b)
{
	return strcasecmp(a, b);
}

int wsprintfA(LPSTR lpOut, LPCSTR lpFmt, ...)
{
	va_list va;
	va_start(va, lpFmt);
	int r = vsnprintf(lpOut, 1024, lpFmt, va);
	va_end(va);
	return r;
}

int CompareStringA(LCID, DWORD dwCmpFlags, LPCSTR a, int na, LPCSTR b, int nb)
{
	std::string x = na < 0 ? std::string(a) : std::string(a, na);
	std::string y = nb < 0 ? std::string(b) : std::string(b, nb);
	const int r = (dwCmpFlags & NORM_IGNORECASE) ? strcasecmp(x.c_str(), y.c_str()) : strcmp(x.c_str(), y.c_str());
	return r < 0 ? CSTR_LESS_THAN : (r > 0 ? CSTR_GREATER_THAN : CSTR_EQUAL);
}

UINT GetSystemDirectoryA(LPSTR lpBuffer, UINT uSize)
{
	return lpBuffer ? (UINT)snprintf(lpBuffer, uSize, "/system/lib") : 0;
}

UINT GetWindowsDirectoryA(LPSTR lpBuffer, UINT uSize)
{
	return lpBuffer ? (UINT)snprintf(lpBuffer, uSize, "/system") : 0;
}

DWORD GetFileVersionInfoSizeA(LPCSTR, LPDWORD lpdwHandle)
{
	if (lpdwHandle)
		*lpdwHandle = 0;
	return 0;
}

BOOL GetFileVersionInfoA(LPCSTR, DWORD, DWORD, LPVOID)
{
	return FALSE;
}

BOOL VerQueryValueA(LPCVOID, LPCSTR, LPVOID* lplpBuffer, PUINT puLen)
{
	if (lplpBuffer)
		*lplpBuffer = NULL;
	if (puLen)
		*puLen = 0;
	return FALSE;
}

int GetLocaleInfoA(LCID, LCTYPE LCType, LPSTR lpLCData, int cchData)
{
	char buf[16];
	if (LCType == LOCALE_IDEFAULTANSICODEPAGE || LCType == LOCALE_IDEFAULTCODEPAGE)
		snprintf(buf, sizeof(buf), "%u", s_defaultCodePage);
	else if (LCType == LOCALE_SABBREVLANGNAME)
		strcpy(buf, "TRK");
	else
		buf[0] = 0;
	const int len = (int)strlen(buf) + 1;
	if (!lpLCData || cchData == 0)
		return len;
	if (cchData < len)
		return 0;
	memcpy(lpLCData, buf, len);
	return len;
}

BOOL GetKeyboardLayoutNameA(LPSTR pwszKLID)
{
	strcpy(pwszKLID, "0000041F");	// Cynex: turkce q
	return TRUE;
}

// ini
DWORD GetPrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName)
{
	if (!lpReturnedString || nSize == 0)
		return 0;
	std::string value = lpDefault ? lpDefault : "";
	{
		std::lock_guard<std::mutex> lock(s_iniMutex);
		SIniFile ini;
		if (lpFileName && lpAppName && lpKeyName && LoadIni(lpFileName, ini))
		{
			auto s = ini.sections.find(Lower(lpAppName));
			if (s != ini.sections.end())
			{
				auto k = s->second.find(Lower(lpKeyName));
				if (k != s->second.end())
					value = k->second;
			}
		}
	}
	strncpy(lpReturnedString, value.c_str(), nSize - 1);
	lpReturnedString[nSize - 1] = 0;
	return (DWORD)strlen(lpReturnedString);
}

UINT GetPrivateProfileIntA(LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName)
{
	char buf[64];
	char def[32];
	snprintf(def, sizeof(def), "%d", nDefault);
	GetPrivateProfileStringA(lpAppName, lpKeyName, def, buf, sizeof(buf), lpFileName);
	return (UINT)atoi(buf);
}

BOOL WritePrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString, LPCSTR lpFileName)
{
	if (!lpAppName || !lpFileName)
		return FALSE;
	std::lock_guard<std::mutex> lock(s_iniMutex);
	SIniFile ini;
	LoadIni(lpFileName, ini);
	auto& sec = ini.sections[Lower(lpAppName)];
	if (!lpKeyName)
		ini.sections.erase(Lower(lpAppName));
	else if (!lpString)
		sec.erase(Lower(lpKeyName));
	else
		sec[Lower(lpKeyName)] = lpString;
	std::ofstream out(cynex::ResolvePath(lpFileName, true), std::ios::binary | std::ios::trunc);
	if (!out)
		return FALSE;
	for (const auto& s : ini.sections)
	{
		out << "[" << s.first << "]\r\n";
		for (const auto& k : s.second)
			out << k.first << "=" << k.second << "\r\n";
	}
	return TRUE;
}

// kayit defteri - yok
LONG RegOpenKeyExA(HKEY, LPCSTR, DWORD, REGSAM, PHKEY phkResult)
{
	if (phkResult)
		*phkResult = NULL;
	return ERROR_FILE_NOT_FOUND;
}

LONG RegCreateKeyExA(HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY phkResult, LPDWORD)
{
	if (phkResult)
		*phkResult = NULL;
	return ERROR_ACCESS_DENIED;
}

LONG RegQueryValueExA(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD)
{
	return ERROR_FILE_NOT_FOUND;
}

LONG RegSetValueExA(HKEY, LPCSTR, DWORD, DWORD, const BYTE*, DWORD)
{
	return ERROR_ACCESS_DENIED;
}

LONG RegCloseKey(HKEY)
{
	return ERROR_SUCCESS;
}

}	// extern "C"
