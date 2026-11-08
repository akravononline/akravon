// Cynex: windowsx makrolari
#pragma once
#include <windows.h>
#define GetWindowStyle(hwnd) ((DWORD)GetWindowLong(hwnd, GWL_STYLE))
#define GetWindowExStyle(hwnd) ((DWORD)GetWindowLong(hwnd, GWL_EXSTYLE))
#define GET_X_LPARAM_(lp) ((int)(short)LOWORD(lp))
