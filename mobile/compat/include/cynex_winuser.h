// Cynex: user32 karsiliklari - pencere/mesaj/klavye/pano (compat/src/winuser.cpp)
#pragma once

// mesajlar
#define WM_NULL 0x0000
#define WM_CREATE 0x0001
#define WM_DESTROY 0x0002
#define WM_MOVE 0x0003
#define WM_SIZE 0x0005
#define WM_ACTIVATE 0x0006
#define WM_SETFOCUS 0x0007
#define WM_KILLFOCUS 0x0008
#define WM_PAINT 0x000F
#define WM_CLOSE 0x0010
#define WM_QUIT 0x0012
#define WM_ACTIVATEAPP 0x001C
#define WM_SETCURSOR 0x0020
#define WM_SETICON 0x0080
#define WM_KEYFIRST 0x0100
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_CHAR 0x0102
#define WM_SYSKEYDOWN 0x0104
#define WM_SYSKEYUP 0x0105
#define WM_SYSCHAR 0x0106
#define WM_KEYLAST 0x0109
#define WM_IME_STARTCOMPOSITION 0x010D
#define WM_IME_ENDCOMPOSITION 0x010E
#define WM_IME_COMPOSITION 0x010F
#define WM_INITDIALOG 0x0110
#define WM_COMMAND 0x0111
#define WM_SYSCOMMAND 0x0112
#define WM_TIMER 0x0113
#define WM_MOUSEFIRST 0x0200
#define WM_MOUSEMOVE 0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_LBUTTONDBLCLK 0x0203
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_RBUTTONDBLCLK 0x0206
#define WM_MBUTTONDOWN 0x0207
#define WM_MBUTTONUP 0x0208
#define WM_MBUTTONDBLCLK 0x0209
#define WM_MOUSEWHEEL 0x020A
#define WM_MOUSELAST 0x020E
#define WM_EXITSIZEMOVE 0x0232
#define WM_IME_SETCONTEXT 0x0281
#define WM_IME_NOTIFY 0x0282
#define WM_IME_CONTROL 0x0283
#define WM_INPUTLANGCHANGE 0x0051
#define WM_CONTEXTMENU 0x007B
#define WM_USER 0x0400
#define WM_APP 0x8000

#define SIZE_RESTORED 0
#define SIZE_MINIMIZED 1
#define SIZE_MAXIMIZED 2
#define WA_INACTIVE 0
#define WA_ACTIVE 1
#define WA_CLICKACTIVE 2
#define SC_KEYMENU 0xF100
#define SC_SCREENSAVE 0xF140
#define SC_MONITORPOWER 0xF170
#define WHEEL_DELTA 120
#define GET_WHEEL_DELTA_WPARAM(w) ((short)HIWORD(w))
#define MK_LBUTTON 0x0001
#define MK_RBUTTON 0x0002
#define MK_SHIFT 0x0004
#define MK_CONTROL 0x0008
#define MK_MBUTTON 0x0010
#define HTCLIENT 1

// sanal tus kodlari
#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_CANCEL 0x03
#define VK_MBUTTON 0x04
#define VK_BACK 0x08
#define VK_TAB 0x09
#define VK_CLEAR 0x0C
#define VK_RETURN 0x0D
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_PAUSE 0x13
#define VK_CAPITAL 0x14
#define VK_KANA 0x15
#define VK_HANGUL 0x15
#define VK_JUNJA 0x17
#define VK_FINAL 0x18
#define VK_HANJA 0x19
#define VK_KANJI 0x19
#define VK_ESCAPE 0x1B
#define VK_CONVERT 0x1C
#define VK_NONCONVERT 0x1D
#define VK_SPACE 0x20
#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_END 0x23
#define VK_HOME 0x24
#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28
#define VK_SELECT 0x29
#define VK_PRINT 0x2A
#define VK_EXECUTE 0x2B
#define VK_SNAPSHOT 0x2C
#define VK_INSERT 0x2D
#define VK_DELETE 0x2E
#define VK_HELP 0x2F
#define VK_LWIN 0x5B
#define VK_RWIN 0x5C
#define VK_APPS 0x5D
#define VK_NUMPAD0 0x60
#define VK_NUMPAD1 0x61
#define VK_NUMPAD2 0x62
#define VK_NUMPAD3 0x63
#define VK_NUMPAD4 0x64
#define VK_NUMPAD5 0x65
#define VK_NUMPAD6 0x66
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_NUMPAD9 0x69
#define VK_MULTIPLY 0x6A
#define VK_ADD 0x6B
#define VK_SEPARATOR 0x6C
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL 0x6E
#define VK_DIVIDE 0x6F
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_NUMLOCK 0x90
#define VK_SCROLL 0x91
#define VK_LSHIFT 0xA0
#define VK_RSHIFT 0xA1
#define VK_LCONTROL 0xA2
#define VK_RCONTROL 0xA3
#define VK_LMENU 0xA4
#define VK_RMENU 0xA5
#define VK_OEM_1 0xBA
#define VK_OEM_PLUS 0xBB
#define VK_OEM_COMMA 0xBC
#define VK_OEM_MINUS 0xBD
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_2 0xBF
#define VK_OEM_3 0xC0
#define VK_OEM_4 0xDB
#define VK_OEM_5 0xDC
#define VK_OEM_6 0xDD
#define VK_OEM_7 0xDE
#define VK_PROCESSKEY 0xE5

// pencere
#define WS_OVERLAPPED 0x00000000L
#define WS_POPUP 0x80000000L
#define WS_CHILD 0x40000000L
#define WS_MINIMIZE 0x20000000L
#define WS_VISIBLE 0x10000000L
#define WS_DISABLED 0x08000000L
#define WS_CLIPSIBLINGS 0x04000000L
#define WS_CLIPCHILDREN 0x02000000L
#define WS_MAXIMIZE 0x01000000L
#define WS_CAPTION 0x00C00000L
#define WS_BORDER 0x00800000L
#define WS_DLGFRAME 0x00400000L
#define WS_SYSMENU 0x00080000L
#define WS_THICKFRAME 0x00040000L
#define WS_MINIMIZEBOX 0x00020000L
#define WS_MAXIMIZEBOX 0x00010000L
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_EX_TOPMOST 0x00000008L
#define WS_EX_APPWINDOW 0x00040000L
#define CS_VREDRAW 0x0001
#define CS_HREDRAW 0x0002
#define CS_DBLCLKS 0x0008
#define CS_OWNDC 0x0020
#define CS_CLASSDC 0x0040
#define CW_USEDEFAULT ((int)0x80000000)
#define SW_HIDE 0
#define SW_SHOWNORMAL 1
#define SW_NORMAL 1
#define SW_SHOWMINIMIZED 2
#define SW_SHOWMAXIMIZED 3
#define SW_MAXIMIZE 3
#define SW_SHOWNOACTIVATE 4
#define SW_SHOW 5
#define SW_MINIMIZE 6
#define SW_SHOWMINNOACTIVE 7
#define SW_SHOWNA 8
#define SW_RESTORE 9
#define SW_SHOWDEFAULT 10
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0004
#define SWP_NOREDRAW 0x0008
#define SWP_NOACTIVATE 0x0010
#define SWP_FRAMECHANGED 0x0020
#define SWP_SHOWWINDOW 0x0040
#define SWP_HIDEWINDOW 0x0080
#define HWND_TOP ((HWND)0)
#define HWND_BOTTOM ((HWND)1)
#define HWND_TOPMOST ((HWND)-1)
#define HWND_NOTOPMOST ((HWND)-2)
#define HWND_MESSAGE ((HWND)-3)
#define GWL_WNDPROC (-4)
#define GWL_HINSTANCE (-6)
#define GWL_ID (-12)
#define GWL_STYLE (-16)
#define GWL_EXSTYLE (-20)
#define GWL_USERDATA (-21)
#define GWLP_USERDATA (-21)
#define GWLP_WNDPROC (-4)
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#define SM_CXFULLSCREEN 16
#define SM_CYFULLSCREEN 17
#define SM_CXFRAME 32
#define SM_CYFRAME 33
#define SM_CYCAPTION 4
#define SM_CXBORDER 5
#define SM_CYBORDER 6
#define SM_CXDLGFRAME 7
#define SM_CYDLGFRAME 8
#define SM_CYMENU 15
#define PM_NOREMOVE 0x0000
#define PM_REMOVE 0x0001
#define PM_NOYIELD 0x0002

typedef struct tagWNDCLASSEXA
{
	UINT cbSize;
	UINT style;
	WNDPROC lpfnWndProc;
	int cbClsExtra;
	int cbWndExtra;
	HINSTANCE hInstance;
	HICON hIcon;
	HCURSOR hCursor;
	HBRUSH hbrBackground;
	LPCSTR lpszMenuName;
	LPCSTR lpszClassName;
	HICON hIconSm;
} WNDCLASSEXA, WNDCLASSEX, *LPWNDCLASSEX;

typedef struct tagWNDCLASSA
{
	UINT style;
	WNDPROC lpfnWndProc;
	int cbClsExtra;
	int cbWndExtra;
	HINSTANCE hInstance;
	HICON hIcon;
	HCURSOR hCursor;
	HBRUSH hbrBackground;
	LPCSTR lpszMenuName;
	LPCSTR lpszClassName;
} WNDCLASSA, WNDCLASS, *LPWNDCLASS;

typedef struct tagCREATESTRUCTA
{
	LPVOID lpCreateParams;
	HINSTANCE hInstance;
	HMENU hMenu;
	HWND hwndParent;
	int cy;
	int cx;
	int y;
	int x;
	LONG style;
	LPCSTR lpszName;
	LPCSTR lpszClass;
	DWORD dwExStyle;
} CREATESTRUCT, *LPCREATESTRUCT;

typedef struct tagSTICKYKEYS { UINT cbSize; DWORD dwFlags; } STICKYKEYS, *LPSTICKYKEYS;
#define SPI_GETSTICKYKEYS 0x003A
#define SPI_SETSTICKYKEYS 0x003B
#define SKF_STICKYKEYSON 0x00000001
#define SKF_AVAILABLE 0x00000002
#define SKF_HOTKEYACTIVE 0x00000004
#define SKF_CONFIRMHOTKEY 0x00000008

typedef struct _devicemodeA
{
	BYTE dmDeviceName[32];
	WORD dmSpecVersion;
	WORD dmDriverVersion;
	WORD dmSize;
	WORD dmDriverExtra;
	DWORD dmFields;
	short dmOrientation;
	short dmPaperSize;
	short dmPaperLength;
	short dmPaperWidth;
	short dmScale;
	short dmCopies;
	short dmDefaultSource;
	short dmPrintQuality;
	short dmColor;
	short dmDuplex;
	short dmYResolution;
	short dmTTOption;
	short dmCollate;
	BYTE dmFormName[32];
	WORD dmLogPixels;
	DWORD dmBitsPerPel;
	DWORD dmPelsWidth;
	DWORD dmPelsHeight;
	DWORD dmDisplayFlags;
	DWORD dmDisplayFrequency;
} DEVMODEA, DEVMODE, *LPDEVMODE;
#define DM_BITSPERPEL 0x00040000L
#define DM_PELSWIDTH 0x00080000L
#define DM_PELSHEIGHT 0x00100000L
#define DM_DISPLAYFREQUENCY 0x00400000L
#define CDS_FULLSCREEN 0x00000004
#define CDS_TEST 0x00000002
#define DISP_CHANGE_SUCCESSFUL 0
#define DISP_CHANGE_RESTART 1
#define DISP_CHANGE_FAILED -1
#define ENUM_CURRENT_SETTINGS ((DWORD)-1)

#define MB_OK 0x00000000L
#define MB_OKCANCEL 0x00000001L
#define MB_YESNO 0x00000004L
#define MB_ICONHAND 0x00000010L
#define MB_ICONSTOP MB_ICONHAND
#define MB_ICONERROR MB_ICONHAND
#define MB_ICONQUESTION 0x00000020L
#define MB_ICONEXCLAMATION 0x00000030L
#define MB_ICONWARNING MB_ICONEXCLAMATION
#define MB_ICONINFORMATION 0x00000040L
#define MB_TOPMOST 0x00040000L
#define MB_SYSTEMMODAL 0x00001000L
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7

#define IDC_ARROW ((LPCSTR)32512)
#define IDC_WAIT ((LPCSTR)32514)
#define IDI_APPLICATION ((LPCSTR)32512)
#define IMAGE_BITMAP 0
#define IMAGE_ICON 1
#define IMAGE_CURSOR 2
#define LR_DEFAULTCOLOR 0x00000000
#define LR_VGACOLOR 0x00000080
#define LR_LOADFROMFILE 0x00000010
#define LR_DEFAULTSIZE 0x00000040
#define ICON_SMALL 0
#define ICON_BIG 1
#define MAKEINTRESOURCE(i) ((LPSTR)((ULONG_PTR)((WORD)(i))))
#define MAKEINTRESOURCEA MAKEINTRESOURCE
#define CF_TEXT 1
#define CF_UNICODETEXT 13
#define COLOR_WINDOW 5
#define COLOR_BTNFACE 15

#ifdef __cplusplus
extern "C" {
#endif

ATOM RegisterClassExA(const WNDCLASSEX* lpwcx);
#define RegisterClassEx RegisterClassExA
ATOM RegisterClassA(const WNDCLASS* lpWndClass);
#define RegisterClass RegisterClassA
BOOL UnregisterClassA(LPCSTR lpClassName, HINSTANCE hInstance);
#define UnregisterClass UnregisterClassA
HWND CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
	HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
#define CreateWindowEx CreateWindowExA
#define CreateWindowA(c, n, s, x, y, w, h, p, m, i, l) CreateWindowExA(0, c, n, s, x, y, w, h, p, m, i, l)
#define CreateWindow CreateWindowA
BOOL DestroyWindow(HWND hWnd);
LRESULT DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#define DefWindowProc DefWindowProcA
LRESULT CallWindowProcA(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#define CallWindowProc CallWindowProcA
BOOL ShowWindow(HWND hWnd, int nCmdShow);
BOOL UpdateWindow(HWND hWnd);
BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
BOOL MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
BOOL GetClientRect(HWND hWnd, LPRECT lpRect);
BOOL GetWindowRect(HWND hWnd, LPRECT lpRect);
BOOL AdjustWindowRect(LPRECT lpRect, DWORD dwStyle, BOOL bMenu);
BOOL AdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle);
BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint);
BOOL ClientToScreen(HWND hWnd, LPPOINT lpPoint);
BOOL SetWindowTextA(HWND hWnd, LPCSTR lpString);
#define SetWindowText SetWindowTextA
int GetWindowTextA(HWND hWnd, LPSTR lpString, int nMaxCount);
#define GetWindowText GetWindowTextA
LONG GetWindowLongA(HWND hWnd, int nIndex);
#define GetWindowLong GetWindowLongA
LONG SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong);
#define SetWindowLong SetWindowLongA
LONG_PTR GetWindowLongPtrA(HWND hWnd, int nIndex);
#define GetWindowLongPtr GetWindowLongPtrA
LONG_PTR SetWindowLongPtrA(HWND hWnd, int nIndex, LONG_PTR dwNewLong);
#define SetWindowLongPtr SetWindowLongPtrA
BOOL IsWindow(HWND hWnd);
HMENU GetMenu(HWND hWnd);
HMENU CreatePopupMenu(void);
HMENU CreateMenu(void);
BOOL DestroyMenu(HMENU hMenu);
BOOL InsertMenuA(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem);
#define InsertMenu InsertMenuA
BOOL AppendMenuA(HMENU hMenu, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem);
#define AppendMenu AppendMenuA
BOOL TrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y, int nReserved, HWND hWnd, const RECT* prcRect);
#define MF_STRING 0x00000000L
#define MF_BYCOMMAND 0x00000000L
#define MF_BYPOSITION 0x00000400L
#define MF_SEPARATOR 0x00000800L
#define MF_GRAYED 0x00000001L
#define MF_CHECKED 0x00000008L
#define TPM_LEFTALIGN 0x0000L
#define TPM_RIGHTALIGN 0x0008L
#define TPM_BOTTOMALIGN 0x0020L
#define TPM_RETURNCMD 0x0100L
#define TPM_RIGHTBUTTON 0x0002L
BOOL SetRect(LPRECT lprc, int xLeft, int yTop, int xRight, int yBottom);
BOOL SetRectEmpty(LPRECT lprc);
BOOL PtInRect(const RECT* lprc, POINT pt);
BOOL OffsetRect(LPRECT lprc, int dx, int dy);
BOOL IntersectRect(LPRECT lprcDst, const RECT* lprcSrc1, const RECT* lprcSrc2);
BOOL IsWindowVisible(HWND hWnd);
BOOL IsIconic(HWND hWnd);
BOOL IsZoomed(HWND hWnd);
HWND GetForegroundWindow(void);
BOOL SetForegroundWindow(HWND hWnd);
HWND GetActiveWindow(void);
HWND SetActiveWindow(HWND hWnd);
HWND GetFocus(void);
HWND SetFocus(HWND hWnd);
HWND FindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName);
HWND WindowFromPoint(POINT Point);
HWND GetParent(HWND hWnd);
int GetClassNameA(HWND hWnd, LPSTR lpClassName, int nMaxCount);
#define GetClassName GetClassNameA
typedef BOOL (*WNDENUMPROC)(HWND, LPARAM);
BOOL EnumWindows(WNDENUMPROC lpEnumFunc, LPARAM lParam);
DWORD GetWindowThreadProcessId(HWND hWnd, LPDWORD lpdwProcessId);
#define FindWindow FindWindowA
HWND GetDesktopWindow(void);
BOOL FlashWindow(HWND hWnd, BOOL bInvert);
typedef struct { UINT cbSize; HWND hwnd; DWORD dwFlags; UINT uCount; DWORD dwTimeout; } FLASHWINFO, *PFLASHWINFO;
#define FLASHW_STOP 0
#define FLASHW_CAPTION 0x00000001
#define FLASHW_TRAY 0x00000002
#define FLASHW_ALL (FLASHW_CAPTION | FLASHW_TRAY)
#define FLASHW_TIMER 0x00000004
#define FLASHW_TIMERNOFG 0x0000000C
BOOL FlashWindowEx(PFLASHWINFO pfwi);
HWND GetTopWindow(HWND hWnd);
HWND GetWindow(HWND hWnd, UINT uCmd);
#define GW_HWNDFIRST 0
#define GW_HWNDLAST 1
#define GW_HWNDNEXT 2
#define GW_HWNDPREV 3
#define GW_OWNER 4
#define GW_CHILD 5
int GetWindowTextLengthA(HWND hWnd);
#define GetNextWindow(hWnd, wCmd) GetWindow(hWnd, wCmd)
#define GetWindowTextLength GetWindowTextLengthA
BOOL InvalidateRect(HWND hWnd, const RECT* lpRect, BOOL bErase);
HWND SetCapture(HWND hWnd);
BOOL ReleaseCapture(void);
HWND GetCapture(void);

BOOL PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
#define PeekMessage PeekMessageA
BOOL GetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
#define GetMessage GetMessageA
BOOL TranslateMessage(const MSG* lpMsg);
LRESULT DispatchMessageA(const MSG* lpMsg);
#define DispatchMessage DispatchMessageA
BOOL PostMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#define PostMessage PostMessageA
LRESULT SendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#define SendMessage SendMessageA
void PostQuitMessage(int nExitCode);
BOOL WaitMessage(void);
UINT_PTR SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, void* lpTimerFunc);
BOOL KillTimer(HWND hWnd, UINT_PTR uIDEvent);

int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
#define MessageBox MessageBoxA
BOOL MessageBeep(UINT uType);

int GetSystemMetrics(int nIndex);
BOOL SystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
#define SystemParametersInfo SystemParametersInfoA
LONG ChangeDisplaySettingsA(LPDEVMODE lpDevMode, DWORD dwFlags);
#define ChangeDisplaySettings ChangeDisplaySettingsA
BOOL EnumDisplaySettingsA(LPCSTR lpszDeviceName, DWORD iModeNum, LPDEVMODE lpDevMode);
#define EnumDisplaySettings EnumDisplaySettingsA

SHORT GetAsyncKeyState(int vKey);
SHORT GetKeyState(int nVirtKey);
BOOL GetKeyboardState(PBYTE lpKeyState);
HKL GetKeyboardLayout(DWORD idThread);
UINT MapVirtualKeyA(UINT uCode, UINT uMapType);
#define MapVirtualKey MapVirtualKeyA
BOOL GetCursorPos(LPPOINT lpPoint);
BOOL SetCursorPos(int X, int Y);
HCURSOR SetCursor(HCURSOR hCursor);
HCURSOR GetCursor(void);
int ShowCursor(BOOL bShow);
BOOL ClipCursor(const RECT* lpRect);
HCURSOR LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName);
#define LoadCursor LoadCursorA
HCURSOR LoadCursorFromFileA(LPCSTR lpFileName);
#define LoadCursorFromFile LoadCursorFromFileA
HICON LoadIconA(HINSTANCE hInstance, LPCSTR lpIconName);
#define LoadIcon LoadIconA
HANDLE LoadImageA(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad);
#define LoadImage LoadImageA
BOOL DestroyCursor(HCURSOR hCursor);
BOOL DestroyIcon(HICON hIcon);
HICON CreateIconFromResourceEx(PBYTE presbits, DWORD dwResSize, BOOL fIcon, DWORD dwVer, int cxDesired, int cyDesired, UINT Flags);
int LoadStringA(HINSTANCE hInstance, UINT uID, LPSTR lpBuffer, int cchBufferMax);
#define LoadString LoadStringA

BOOL OpenClipboard(HWND hWndNewOwner);
BOOL CloseClipboard(void);
BOOL EmptyClipboard(void);
HANDLE GetClipboardData(UINT uFormat);
HANDLE SetClipboardData(UINT uFormat, HANDLE hMem);
BOOL IsClipboardFormatAvailable(UINT format);
UINT RegisterClipboardFormatA(LPCSTR lpszFormat);
#define RegisterClipboardFormat RegisterClipboardFormatA

HINSTANCE ShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);
#define ShellExecute ShellExecuteA

#ifdef __cplusplus
}
#endif
