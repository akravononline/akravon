// Cynex: kullanici klasorleri - mobilde uygulama klasoru
#pragma once
#include <windows.h>
#define CSIDL_PERSONAL 0x0005
#define CSIDL_MYPICTURES 0x0027
#define CSIDL_APPDATA 0x001a
#define CSIDL_LOCAL_APPDATA 0x001c
#define CSIDL_DESKTOP 0x0000
#define CSIDL_DESKTOPDIRECTORY 0x0010
#define SHGFP_TYPE_CURRENT 0
#ifdef __cplusplus
extern "C" {
#endif
BOOL SHGetSpecialFolderPathA(HWND hwnd, LPSTR pszPath, int csidl, BOOL fCreate);
#define SHGetSpecialFolderPath SHGetSpecialFolderPathA
HRESULT SHGetFolderPathA(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);
#define SHGetFolderPath SHGetFolderPathA
#ifdef __cplusplus
}
#endif
