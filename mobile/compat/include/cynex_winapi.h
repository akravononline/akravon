// Cynex: kernel32 karsiliklari (compat/src/winapi.cpp)
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// zaman
DWORD GetTickCount(void);
#define GetCurrentTime() GetTickCount()
ULONGLONG GetTickCount64(void);
DWORD timeGetTime(void);
UINT timeBeginPeriod(UINT uPeriod);
UINT timeEndPeriod(UINT uPeriod);
BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);
BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);
void Sleep(DWORD dwMilliseconds);
void GetLocalTime(LPSYSTEMTIME lpSystemTime);
void GetSystemTime(LPSYSTEMTIME lpSystemTime);
void GetSystemTimeAsFileTime(LPFILETIME lpFileTime);
BOOL FileTimeToSystemTime(const FILETIME* lpFileTime, LPSYSTEMTIME lpSystemTime);
BOOL SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime, LPFILETIME lpFileTime);
BOOL FileTimeToLocalFileTime(const FILETIME* lpFileTime, LPFILETIME lpLocalFileTime);
LONG CompareFileTime(const FILETIME* a, const FILETIME* b);

// hata
DWORD GetLastError(void);
void SetLastError(DWORD dwErrCode);
void OutputDebugStringA(LPCSTR lpOutputString);
#define OutputDebugString OutputDebugStringA

// dosya
#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define GENERIC_EXECUTE 0x20000000
#define GENERIC_ALL 0x10000000
#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define FILE_SHARE_DELETE 0x00000004
#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5
#define FILE_ATTRIBUTE_READONLY 0x00000001
#define FILE_ATTRIBUTE_HIDDEN 0x00000002
#define FILE_ATTRIBUTE_SYSTEM 0x00000004
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_ATTRIBUTE_ARCHIVE 0x00000020
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_ATTRIBUTE_TEMPORARY 0x00000100
#define FILE_FLAG_WRITE_THROUGH 0x80000000
#define FILE_FLAG_OVERLAPPED 0x40000000
#define FILE_FLAG_NO_BUFFERING 0x20000000
#define FILE_FLAG_RANDOM_ACCESS 0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000
#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2
#define PAGE_READONLY 0x02
#define PAGE_READWRITE 0x04
#define PAGE_WRITECOPY 0x08
#define PAGE_EXECUTE_READ 0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_NOACCESS 0x01
#define SEC_COMMIT 0x8000000
#define FILE_MAP_COPY 0x0001
#define FILE_MAP_WRITE 0x0002
#define FILE_MAP_READ 0x0004
#define FILE_MAP_ALL_ACCESS 0x000F001F
#define MEM_COMMIT 0x00001000
#define MEM_RESERVE 0x00002000
#define MEM_RELEASE 0x00008000
#define MEM_DECOMMIT 0x00004000
#define MOVEFILE_REPLACE_EXISTING 0x00000001

HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
#define CreateFile CreateFileA
BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
BOOL FlushFileBuffers(HANDLE hFile);
BOOL CloseHandle(HANDLE hObject);
DWORD GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize);
DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
BOOL SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod);
BOOL SetEndOfFile(HANDLE hFile);
BOOL GetFileTime(HANDLE hFile, LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime);
HANDLE CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName);
#define CreateFileMapping CreateFileMappingA
LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap);
BOOL UnmapViewOfFile(LPCVOID lpBaseAddress);
BOOL DeleteFileA(LPCSTR lpFileName);
#define DeleteFile DeleteFileA
BOOL MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName);
#define MoveFile MoveFileA
BOOL MoveFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, DWORD dwFlags);
#define MoveFileEx MoveFileExA
BOOL CopyFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, BOOL bFailIfExists);
#define CopyFile CopyFileA
BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
#define CreateDirectory CreateDirectoryA
BOOL RemoveDirectoryA(LPCSTR lpPathName);
#define RemoveDirectory RemoveDirectoryA
DWORD GetFileAttributesA(LPCSTR lpFileName);
#define GetFileAttributes GetFileAttributesA
BOOL SetFileAttributesA(LPCSTR lpFileName, DWORD dwFileAttributes);
#define SetFileAttributes SetFileAttributesA
HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData);
#define FindFirstFile FindFirstFileA
BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData);
#define FindNextFile FindNextFileA
BOOL FindClose(HANDLE hFindFile);
DWORD GetCurrentDirectoryA(DWORD nBufferLength, LPSTR lpBuffer);
#define GetCurrentDirectory GetCurrentDirectoryA
BOOL SetCurrentDirectoryA(LPCSTR lpPathName);
#define SetCurrentDirectory SetCurrentDirectoryA
DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
#define GetModuleFileName GetModuleFileNameA
HMODULE GetModuleHandleA(LPCSTR lpModuleName);
#define GetModuleHandle GetModuleHandleA
DWORD GetTempPathA(DWORD nBufferLength, LPSTR lpBuffer);
#define GetTempPath GetTempPathA
UINT GetTempFileNameA(LPCSTR lpPathName, LPCSTR lpPrefixString, UINT uUnique, LPSTR lpTempFileName);
#define GetTempFileName GetTempFileNameA
DWORD GetFullPathNameA(LPCSTR lpFileName, DWORD nBufferLength, LPSTR lpBuffer, LPSTR* lpFilePart);
#define GetFullPathName GetFullPathNameA

// ini
DWORD GetPrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName);
#define GetPrivateProfileString GetPrivateProfileStringA
UINT GetPrivateProfileIntA(LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName);
#define GetPrivateProfileInt GetPrivateProfileIntA
BOOL WritePrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString, LPCSTR lpFileName);
#define WritePrivateProfileString WritePrivateProfileStringA

// kayit defteri - mobilde yok, hep bulunamadi doner
#define HKEY_CLASSES_ROOT ((HKEY)(ULONG_PTR)0x80000000)
#define HKEY_CURRENT_USER ((HKEY)(ULONG_PTR)0x80000001)
#define HKEY_LOCAL_MACHINE ((HKEY)(ULONG_PTR)0x80000002)
#define HKEY_USERS ((HKEY)(ULONG_PTR)0x80000003)
#define KEY_QUERY_VALUE 0x0001
#define KEY_SET_VALUE 0x0002
#define KEY_READ 0x20019
#define KEY_WRITE 0x20006
#define KEY_ALL_ACCESS 0xF003F
#define KEY_WOW64_64KEY 0x0100
#define KEY_WOW64_32KEY 0x0200
#define REG_NONE 0
#define REG_SZ 1
#define REG_EXPAND_SZ 2
#define REG_BINARY 3
#define REG_DWORD 4
#define REG_OPTION_NON_VOLATILE 0
LONG RegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
#define RegOpenKeyEx RegOpenKeyExA
LONG RegCreateKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
#define RegCreateKeyEx RegCreateKeyExA
LONG RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
#define RegQueryValueEx RegQueryValueExA
LONG RegSetValueExA(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lpData, DWORD cbData);
#define RegSetValueEx RegSetValueExA
LONG RegCloseKey(HKEY hKey);

// dll - android'de yok
HMODULE LoadLibraryA(LPCSTR lpLibFileName);
#define LoadLibrary LoadLibraryA
FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
BOOL FreeLibrary(HMODULE hLibModule);

// surec / thread
#define THREAD_PRIORITY_LOWEST -2
#define THREAD_PRIORITY_BELOW_NORMAL -1
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_ABOVE_NORMAL 1
#define THREAD_PRIORITY_HIGHEST 2
#define THREAD_PRIORITY_TIME_CRITICAL 15
#define THREAD_PRIORITY_IDLE -15
#define CREATE_SUSPENDED 0x00000004
#define STILL_ACTIVE 259
HANDLE GetCurrentProcess(void);
DWORD GetCurrentProcessId(void);
HANDLE GetCurrentThread(void);
DWORD GetCurrentThreadId(void);
HANDLE CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
BOOL SetThreadPriority(HANDLE hThread, int nPriority);
int GetThreadPriority(HANDLE hThread);
DWORD ResumeThread(HANDLE hThread);
DWORD SuspendThread(HANDLE hThread);
BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode);
BOOL GetExitCodeThread(HANDLE hThread, LPDWORD lpExitCode);
void ExitProcess(UINT uExitCode);
BOOL TerminateProcess(HANDLE hProcess, UINT uExitCode);
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
DWORD WaitForMultipleObjects(DWORD nCount, const HANDLE* lpHandles, BOOL bWaitAll, DWORD dwMilliseconds);
HANDLE CreateEventA(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName);
#define CreateEvent CreateEventA
BOOL SetEvent(HANDLE hEvent);
BOOL ResetEvent(HANDLE hEvent);
HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName);
#define CreateMutex CreateMutexA
HANDLE OpenMutexA(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName);
#define OpenMutex OpenMutexA
BOOL ReleaseMutex(HANDLE hMutex);
HANDLE CreateSemaphoreA(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCSTR lpName);
#define CreateSemaphore CreateSemaphoreA
BOOL ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount);

void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
BOOL InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount);
void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
BOOL TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

LONG InterlockedIncrement(volatile LONG* p);
LONG InterlockedDecrement(volatile LONG* p);
LONG InterlockedExchange(volatile LONG* p, LONG v);
LONG InterlockedExchangeAdd(volatile LONG* p, LONG v);
LONG InterlockedCompareExchange(volatile LONG* p, LONG exchange, LONG comparand);

// bellek
#define LMEM_FIXED 0x0000
#define LMEM_ZEROINIT 0x0040
#define LPTR (LMEM_FIXED | LMEM_ZEROINIT)
#define GMEM_FIXED 0x0000
#define GMEM_MOVEABLE 0x0002
#define GMEM_ZEROINIT 0x0040
#define GMEM_DDESHARE 0x2000
#define GHND (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR (GMEM_FIXED | GMEM_ZEROINIT)
HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes);
LPVOID GlobalLock(HGLOBAL hMem);
BOOL GlobalUnlock(HGLOBAL hMem);
HGLOBAL GlobalFree(HGLOBAL hMem);
SIZE_T GlobalSize(HGLOBAL hMem);
HLOCAL LocalAlloc(UINT uFlags, SIZE_T uBytes);
HLOCAL LocalFree(HLOCAL hMem);
LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
BOOL VirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
void GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer);
BOOL GlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer);
BOOL IsBadReadPtr(const void* lp, UINT_PTR ucb);
BOOL IsBadWritePtr(LPVOID lp, UINT_PTR ucb);
HANDLE GetProcessHeap(void);
LPVOID HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
BOOL HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
#define HEAP_ZERO_MEMORY 0x00000008

// sistem
void GetSystemInfo(LPSYSTEM_INFO lpSystemInfo);
BOOL GetVersionExA(LPOSVERSIONINFO lpVersionInformation);
#define GetVersionEx GetVersionExA
DWORD GetVersion(void);
BOOL GetComputerNameA(LPSTR lpBuffer, LPDWORD nSize);
#define GetComputerName GetComputerNameA
BOOL GetUserNameA(LPSTR lpBuffer, LPDWORD pcbBuffer);
#define GetUserName GetUserNameA
LPSTR GetCommandLineA(void);
#define GetCommandLine GetCommandLineA
DWORD GetEnvironmentVariableA(LPCSTR lpName, LPSTR lpBuffer, DWORD nSize);
#define GetEnvironmentVariable GetEnvironmentVariableA
UINT GetACP(void);
UINT GetOEMCP(void);
LCID GetUserDefaultLCID(void);
LCID GetSystemDefaultLCID(void);
LANGID GetUserDefaultLangID(void);
LANGID GetSystemDefaultLangID(void);
BOOL IsDebuggerPresent(void);
BOOL AllocConsole(void);
BOOL FreeConsole(void);
BOOL SetConsoleTitleA(LPCSTR lpConsoleTitle);
#define SetConsoleTitle SetConsoleTitleA
HANDLE GetStdHandle(DWORD nStdHandle);
#define STD_INPUT_HANDLE ((DWORD)-10)
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define STD_ERROR_HANDLE ((DWORD)-12)
void DebugBreak(void);

// kod sayfasi - 949, 936, 950, 932, 1250-1258, 874, 65001
#define CP_ACP 0
#define CP_OEMCP 1
#define CP_MACCP 2
#define CP_THREAD_ACP 3
#define CP_UTF7 65000
#define CP_UTF8 65001
#define MB_PRECOMPOSED 0x00000001
#define MB_ERR_INVALID_CHARS 0x00000008
#define WC_COMPOSITECHECK 0x00000200
#define WC_NO_BEST_FIT_CHARS 0x00000400
int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte,
	LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
BOOL IsDBCSLeadByte(BYTE TestChar);
BOOL IsDBCSLeadByteEx(UINT CodePage, BYTE TestChar);
LPSTR CharNextA(LPCSTR lpsz);
LPSTR CharNextExA(WORD CodePage, LPCSTR lpCurrentChar, DWORD dwFlags);
LPSTR CharPrevExA(WORD CodePage, LPCSTR lpStart, LPCSTR lpCurrentChar, DWORD dwFlags);
#define CharNext CharNextA
LPSTR CharPrevA(LPCSTR lpszStart, LPCSTR lpszCurrent);
#define CharPrev CharPrevA
LPSTR CharUpperA(LPSTR lpsz);
#define CharUpper CharUpperA
LPSTR CharLowerA(LPSTR lpsz);
#define CharLower CharLowerA
int lstrlenA(LPCSTR lpString);
int lstrlenW(LPCWSTR lpString);
#define NORM_IGNORECASE 0x00000001
#define CSTR_LESS_THAN 1
#define CSTR_EQUAL 2
#define CSTR_GREATER_THAN 3
int CompareStringA(LCID Locale, DWORD dwCmpFlags, LPCSTR lpString1, int cchCount1, LPCSTR lpString2, int cchCount2);
#define CompareString CompareStringA
UINT GetSystemDirectoryA(LPSTR lpBuffer, UINT uSize);
#define GetSystemDirectory GetSystemDirectoryA
UINT GetWindowsDirectoryA(LPSTR lpBuffer, UINT uSize);
#define GetWindowsDirectory GetWindowsDirectoryA
typedef struct tagVS_FIXEDFILEINFO { DWORD dwSignature; DWORD dwStrucVersion; DWORD dwFileVersionMS; DWORD dwFileVersionLS; DWORD dwProductVersionMS; DWORD dwProductVersionLS; DWORD dwFileFlagsMask; DWORD dwFileFlags; DWORD dwFileOS; DWORD dwFileType; DWORD dwFileSubtype; DWORD dwFileDateMS; DWORD dwFileDateLS; } VS_FIXEDFILEINFO;
DWORD GetFileVersionInfoSizeA(LPCSTR lptstrFilename, LPDWORD lpdwHandle);
#define GetFileVersionInfoSize GetFileVersionInfoSizeA
BOOL GetFileVersionInfoA(LPCSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
#define GetFileVersionInfo GetFileVersionInfoA
BOOL VerQueryValueA(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen);
#define VerQueryValue VerQueryValueA
#define lstrlen lstrlenA
LPSTR lstrcpyA(LPSTR lpString1, LPCSTR lpString2);
#define lstrcpy lstrcpyA
LPSTR lstrcpynA(LPSTR lpString1, LPCSTR lpString2, int iMaxLength);
#define lstrcpyn lstrcpynA
LPSTR lstrcatA(LPSTR lpString1, LPCSTR lpString2);
#define lstrcat lstrcatA
int lstrcmpA(LPCSTR lpString1, LPCSTR lpString2);
#define lstrcmp lstrcmpA
int lstrcmpiA(LPCSTR lpString1, LPCSTR lpString2);
#define lstrcmpi lstrcmpiA
int wsprintfA(LPSTR lpOut, LPCSTR lpFmt, ...);
#define wsprintf wsprintfA

#ifdef __cplusplus
}
#endif
