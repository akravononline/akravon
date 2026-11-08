#ifndef __INC_ETERLIB_DEBUG_H__
#define __INC_ETERLIB_DEBUG_H__

#include <windows.h>
#include "ServiceDefs.h"

extern void SetLogLevel(UINT uLevel);

#define Log(...)
#define Logn(...)
#define Logf(...)
#define Lognf(...)
#define Trace(...)
#define Tracen(...)
#define Tracenf(...)
#define Tracef(...)
#ifdef CYNEX_MOBILE
// Cynex: mobilde hata satirlari logcat'e
extern void CynexTraceError(const char* c_szFormat, ...);
#define TraceError(...) CynexTraceError(__VA_ARGS__)
#define TraceErrorWithoutEnter(...) CynexTraceError(__VA_ARGS__)
#else
#define TraceError(...)
#define TraceErrorWithoutEnter(...)
#endif
extern void LogBox(const char* c_szMsg, const char * c_szCaption = NULL, HWND hWnd = NULL);
extern void LogBoxf(const char* c_szMsg, ...);

extern void LogFile(const char* c_szMsg);
extern void LogFilef(const char * c_szMessage, ...);
extern void OpenConsoleWindow(void);
extern void CloseConsoleWindow();
extern void SetupLog(void);

extern void OpenLogFile(bool bUseLogFile = true);
extern void CloseLogFile();

extern HWND g_PopupHwnd;

#define CHECK_RETURN(flag, string)			\
	if (flag)								\
	{										\
		LogBox(string);						\
		return;								\
	}										\

#endif
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
