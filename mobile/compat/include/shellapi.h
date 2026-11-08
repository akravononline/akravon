// Cynex: gorev cubugu simgesi - mobilde yok (SetMessage bildirim yerine log)
#pragma once
#include <windows.h>

typedef struct _NOTIFYICONDATAA
{
	DWORD cbSize;
	HWND hWnd;
	UINT uID;
	UINT uFlags;
	UINT uCallbackMessage;
	HICON hIcon;
	CHAR szTip[128];
	DWORD dwState;
	DWORD dwStateMask;
	CHAR szInfo[256];
	union { UINT uTimeout; UINT uVersion; };
	CHAR szInfoTitle[64];
	DWORD dwInfoFlags;
	GUID guidItem;
	HICON hBalloonIcon;
} NOTIFYICONDATAA, NOTIFYICONDATA, *PNOTIFYICONDATA;

#define NIM_ADD 0x00000000
#define NIM_MODIFY 0x00000001
#define NIM_DELETE 0x00000002
#define NIM_SETFOCUS 0x00000003
#define NIM_SETVERSION 0x00000004
#define NIF_MESSAGE 0x00000001
#define NIF_ICON 0x00000002
#define NIF_TIP 0x00000004
#define NIF_STATE 0x00000008
#define NIF_INFO 0x00000010
#define NIIF_NONE 0x00000000
#define NIIF_INFO 0x00000001
#define NIIF_WARNING 0x00000002
#define NIIF_ERROR 0x00000003
#define NIIF_USER 0x00000004

#ifdef __cplusplus
extern "C" {
#endif
BOOL Shell_NotifyIconA(DWORD dwMessage, PNOTIFYICONDATA lpData);
#define Shell_NotifyIcon Shell_NotifyIconA
#ifdef __cplusplus
}
#endif
