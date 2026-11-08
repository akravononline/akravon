// Cynex: user32 karsiliklari - tek tam ekran pencere, mesajlar platform kuyrugundan
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <android/log.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <map>
#include "../../platform/CynexPlatform.h"

namespace
{
	struct SWindow
	{
		std::string className;
		std::string title;
		WNDPROC proc = nullptr;
		DWORD style = 0;
		DWORD exStyle = 0;
		LONG_PTR userData = 0;
		int x = 0, y = 0, w = 0, h = 0;
		bool visible = false;
	};

	std::map<std::string, WNDCLASSA> g_classes;
	std::vector<SWindow*> g_windows;
	SWindow* g_mainWindow = nullptr;
	HWND g_capture = nullptr;
	int g_cursorCount = 0;
	bool g_quit = false;
	int g_quitCode = 0;

	SWindow* W(HWND h)
	{
		for (SWindow* w : g_windows)
			if ((HWND)w == h)
				return w;
		return nullptr;
	}

	SWindow* Target(HWND h)
	{
		SWindow* w = W(h);
		return w ? w : g_mainWindow;
	}

	LRESULT Send(SWindow* w, UINT msg, WPARAM wp, LPARAM lp)
	{
		if (!w || !w->proc)
			return 0;
		return w->proc((HWND)w, msg, wp, lp);
	}

	void Resize(SWindow* w, int width, int height)
	{
		if (width == w->w && height == w->h)
			return;
		w->w = width;
		w->h = height;
		// Cynex: oyun penceresi boyutu = cizim boyutu, yuzey ekrana olceklenir
		if (w == g_mainWindow && width > 0 && height > 0)
			CynexPlatform_SetRenderSize(width, height);
		Send(w, WM_SIZE, SIZE_RESTORED, MAKELPARAM(width, height));
	}

	DWORD Now()
	{
		return GetTickCount();
	}
}

extern "C" {

ATOM RegisterClassA(const WNDCLASS* wc)
{
	if (!wc || !wc->lpszClassName)
		return 0;
	g_classes[wc->lpszClassName] = *wc;
	return (ATOM)g_classes.size();
}

ATOM RegisterClassExA(const WNDCLASSEX* wcx)
{
	if (!wcx)
		return 0;
	WNDCLASSA wc = {};
	wc.style = wcx->style;
	wc.lpfnWndProc = wcx->lpfnWndProc;
	wc.hInstance = wcx->hInstance;
	wc.hIcon = wcx->hIcon;
	wc.hCursor = wcx->hCursor;
	wc.hbrBackground = wcx->hbrBackground;
	wc.lpszMenuName = wcx->lpszMenuName;
	wc.lpszClassName = wcx->lpszClassName;
	return RegisterClassA(&wc);
}

BOOL UnregisterClassA(LPCSTR name, HINSTANCE)
{
	return name && g_classes.erase(name) ? TRUE : FALSE;
}

HWND CreateWindowExA(DWORD exStyle, LPCSTR className, LPCSTR windowName, DWORD style, int x, int y, int width, int height,
	HWND, HMENU, HINSTANCE, LPVOID param)
{
	auto it = className ? g_classes.find(className) : g_classes.end();
	if (it == g_classes.end())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return nullptr;
	}

	SWindow* w = new SWindow;
	w->className = className;
	w->title = windowName ? windowName : "";
	w->proc = it->second.lpfnWndProc;
	w->style = style;
	w->exStyle = exStyle;
	w->x = x == CW_USEDEFAULT ? 0 : x;
	w->y = y == CW_USEDEFAULT ? 0 : y;
	w->w = width == CW_USEDEFAULT ? 0 : width;
	w->h = height == CW_USEDEFAULT ? 0 : height;
	g_windows.push_back(w);
	if (!g_mainWindow)
		g_mainWindow = w;

	CREATESTRUCT cs = {};
	cs.lpCreateParams = param;
	cs.cx = w->w;
	cs.cy = w->h;
	cs.style = (LONG)style;
	cs.lpszName = windowName;
	cs.lpszClass = className;
	cs.dwExStyle = exStyle;
	Send(w, WM_CREATE, 0, (LPARAM)&cs);
	return (HWND)w;
}

BOOL DestroyWindow(HWND hWnd)
{
	SWindow* w = W(hWnd);
	if (!w)
		return FALSE;
	Send(w, WM_DESTROY, 0, 0);
	for (size_t i = 0; i < g_windows.size(); ++i)
		if (g_windows[i] == w)
		{
			g_windows.erase(g_windows.begin() + i);
			break;
		}
	if (g_mainWindow == w)
		g_mainWindow = g_windows.empty() ? nullptr : g_windows.front();
	if (g_capture == hWnd)
		g_capture = nullptr;
	delete w;
	return TRUE;
}

LRESULT DefWindowProcA(HWND hWnd, UINT msg, WPARAM, LPARAM)
{
	switch (msg)
	{
		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;
	}
	return 0;
}

LRESULT CallWindowProcA(WNDPROC proc, HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	return proc ? proc(hWnd, msg, wp, lp) : 0;
}

BOOL ShowWindow(HWND hWnd, int cmd)
{
	SWindow* w = W(hWnd);
	if (!w)
		return FALSE;
	const bool was = w->visible;
	w->visible = cmd != SW_HIDE && cmd != SW_MINIMIZE && cmd != SW_SHOWMINIMIZED && cmd != SW_SHOWMINNOACTIVE;
	if (w->visible && !was)
		Send(w, WM_SIZE, SIZE_RESTORED, MAKELPARAM(w->w, w->h));
	return was;
}

BOOL UpdateWindow(HWND) { return TRUE; }

BOOL SetWindowPos(HWND hWnd, HWND, int x, int y, int cx, int cy, UINT flags)
{
	SWindow* w = W(hWnd);
	if (!w)
		return FALSE;
	if (!(flags & SWP_NOMOVE))
	{
		w->x = x;
		w->y = y;
	}
	if (flags & SWP_SHOWWINDOW)
		w->visible = true;
	if (flags & SWP_HIDEWINDOW)
		w->visible = false;
	if (!(flags & SWP_NOSIZE))
		Resize(w, cx, cy);
	return TRUE;
}

BOOL MoveWindow(HWND hWnd, int x, int y, int width, int height, BOOL)
{
	return SetWindowPos(hWnd, nullptr, x, y, width, height, SWP_NOZORDER);
}

BOOL GetClientRect(HWND hWnd, LPRECT rc)
{
	SWindow* w = Target(hWnd);
	if (!rc)
		return FALSE;
	rc->left = rc->top = 0;
	rc->right = w ? w->w : CynexPlatform_GetWidth();
	rc->bottom = w ? w->h : CynexPlatform_GetHeight();
	return TRUE;
}

BOOL GetWindowRect(HWND hWnd, LPRECT rc)
{
	SWindow* w = Target(hWnd);
	if (!rc)
		return FALSE;
	if (!w)
		return GetClientRect(hWnd, rc);
	rc->left = w->x;
	rc->top = w->y;
	rc->right = w->x + w->w;
	rc->bottom = w->y + w->h;
	return TRUE;
}

// Cynex: kenarlik/baslik yok, pencere = istemci alani
BOOL AdjustWindowRect(LPRECT, DWORD, BOOL) { return TRUE; }
BOOL AdjustWindowRectEx(LPRECT, DWORD, BOOL, DWORD) { return TRUE; }

BOOL ScreenToClient(HWND hWnd, LPPOINT pt)
{
	SWindow* w = Target(hWnd);
	if (w && pt)
	{
		pt->x -= w->x;
		pt->y -= w->y;
	}
	return TRUE;
}

BOOL ClientToScreen(HWND hWnd, LPPOINT pt)
{
	SWindow* w = Target(hWnd);
	if (w && pt)
	{
		pt->x += w->x;
		pt->y += w->y;
	}
	return TRUE;
}

BOOL SetWindowTextA(HWND hWnd, LPCSTR text)
{
	SWindow* w = W(hWnd);
	if (!w)
		return FALSE;
	w->title = text ? text : "";
	return TRUE;
}

int GetWindowTextA(HWND hWnd, LPSTR buf, int max)
{
	SWindow* w = W(hWnd);
	if (!w || !buf || max <= 0)
		return 0;
	const int n = (int)std::min<size_t>(w->title.size(), (size_t)max - 1);
	memcpy(buf, w->title.data(), n);
	buf[n] = 0;
	return n;
}

int GetWindowTextLengthA(HWND hWnd)
{
	SWindow* w = W(hWnd);
	return w ? (int)w->title.size() : 0;
}

LONG_PTR GetWindowLongPtrA(HWND hWnd, int index)
{
	SWindow* w = W(hWnd);
	if (!w)
		return 0;
	switch (index)
	{
		case GWL_USERDATA: return w->userData;
		case GWL_STYLE: return (LONG_PTR)w->style;
		case GWL_EXSTYLE: return (LONG_PTR)w->exStyle;
		case GWL_WNDPROC: return (LONG_PTR)w->proc;
	}
	return 0;
}

LONG_PTR SetWindowLongPtrA(HWND hWnd, int index, LONG_PTR value)
{
	SWindow* w = W(hWnd);
	if (!w)
		return 0;
	const LONG_PTR old = GetWindowLongPtrA(hWnd, index);
	switch (index)
	{
		case GWL_USERDATA: w->userData = value; break;
		case GWL_STYLE: w->style = (DWORD)value; break;
		case GWL_EXSTYLE: w->exStyle = (DWORD)value; break;
		case GWL_WNDPROC: w->proc = (WNDPROC)value; break;
	}
	return old;
}

LONG GetWindowLongA(HWND hWnd, int index)
{
	return (LONG)GetWindowLongPtrA(hWnd, index);
}

LONG SetWindowLongA(HWND hWnd, int index, LONG value)
{
	// Cynex: 32 bit deger, isaretci icin SetWindowLongPtr kullaniliyor
	return (LONG)SetWindowLongPtrA(hWnd, index, index == GWL_WNDPROC || index == GWL_USERDATA ? 0 : (LONG_PTR)value);
}

BOOL IsWindow(HWND hWnd) { return W(hWnd) ? TRUE : FALSE; }
BOOL IsWindowVisible(HWND hWnd) { SWindow* w = W(hWnd); return w && w->visible; }
BOOL IsIconic(HWND) { return CynexPlatform_GetWindow() ? FALSE : TRUE; }
BOOL IsZoomed(HWND) { return FALSE; }
HWND GetForegroundWindow(void) { return CynexPlatform_HasFocus() ? (HWND)g_mainWindow : nullptr; }
BOOL SetForegroundWindow(HWND) { return TRUE; }
HWND GetActiveWindow(void) { return (HWND)g_mainWindow; }
HWND SetActiveWindow(HWND) { return (HWND)g_mainWindow; }
HWND GetFocus(void) { return (HWND)g_mainWindow; }
HWND SetFocus(HWND) { return (HWND)g_mainWindow; }
HWND GetDesktopWindow(void) { return nullptr; }
HWND GetParent(HWND) { return nullptr; }
HWND WindowFromPoint(POINT) { return (HWND)g_mainWindow; }

HWND FindWindowA(LPCSTR className, LPCSTR windowName)
{
	for (SWindow* w : g_windows)
		if ((!className || w->className == className) && (!windowName || w->title == windowName))
			return (HWND)w;
	return nullptr;
}

int GetClassNameA(HWND hWnd, LPSTR buf, int max)
{
	SWindow* w = W(hWnd);
	if (!w || !buf || max <= 0)
		return 0;
	strncpy(buf, w->className.c_str(), max - 1);
	buf[max - 1] = 0;
	return (int)strlen(buf);
}

BOOL EnumWindows(WNDENUMPROC proc, LPARAM lp)
{
	for (SWindow* w : g_windows)
		if (!proc((HWND)w, lp))
			break;
	return TRUE;
}

DWORD GetWindowThreadProcessId(HWND, LPDWORD pid)
{
	if (pid)
		*pid = GetCurrentProcessId();
	return GetCurrentThreadId();
}

BOOL FlashWindow(HWND, BOOL) { return TRUE; }

BOOL FlashWindowEx(PFLASHWINFO)
{
	// Cynex: gorev cubugu yanip sonmesi yerine kisa titresim
	CynexPlatform_Vibrate(60);
	return TRUE;
}

HWND GetTopWindow(HWND) { return (HWND)g_mainWindow; }

HWND GetWindow(HWND hWnd, UINT cmd)
{
	if (cmd == GW_CHILD)
		return nullptr;
	for (size_t i = 0; i < g_windows.size(); ++i)
		if ((HWND)g_windows[i] == hWnd)
			return cmd == GW_HWNDNEXT && i + 1 < g_windows.size() ? (HWND)g_windows[i + 1] : nullptr;
	return nullptr;
}

BOOL InvalidateRect(HWND, const RECT*, BOOL) { return TRUE; }

HWND SetCapture(HWND hWnd)
{
	HWND old = g_capture;
	g_capture = hWnd;
	return old;
}

BOOL ReleaseCapture(void)
{
	g_capture = nullptr;
	return TRUE;
}

HWND GetCapture(void) { return g_capture; }

// ---------------------------------------------------------------- mesajlar
BOOL PeekMessageA(LPMSG msg, HWND, UINT, UINT, UINT removeFlags)
{
	if (g_quit)
	{
		if (msg)
		{
			memset(msg, 0, sizeof(*msg));
			msg->message = WM_QUIT;
			msg->wParam = (WPARAM)g_quitCode;
		}
		return TRUE;
	}

	CynexPlatform_PumpEvents(0);

	uint32_t m;
	uintptr_t wp;
	intptr_t lp;
	if (!CynexPlatform_PeekMessage(&m, &wp, &lp, (removeFlags & PM_REMOVE) ? 1 : 0))
	{
		// Cynex: arka plandayken islemciyi yakmasin
		if (!CynexPlatform_GetWindow())
			CynexPlatform_PumpEvents(30);
		return FALSE;
	}

	if ((removeFlags & PM_REMOVE) && m == WM_QUIT)
	{
		g_quit = true;
		g_quitCode = (int)wp;
	}

	if (msg)
	{
		msg->hwnd = (HWND)g_mainWindow;
		msg->message = m;
		msg->wParam = wp;
		msg->lParam = lp;
		msg->time = Now();
		int x, y;
		CynexPlatform_GetMousePos(&x, &y);
		msg->pt.x = x;
		msg->pt.y = y;
	}
	return TRUE;
}

BOOL GetMessageA(LPMSG msg, HWND hWnd, UINT minMsg, UINT maxMsg)
{
	for (;;)
	{
		if (PeekMessageA(msg, hWnd, minMsg, maxMsg, PM_REMOVE))
			return msg->message == WM_QUIT ? FALSE : TRUE;
		CynexPlatform_PumpEvents(16);
	}
}

BOOL TranslateMessage(const MSG*)
{
	// Cynex: WM_CHAR platform katmaninda uretiliyor
	return FALSE;
}

LRESULT DispatchMessageA(const MSG* msg)
{
	if (!msg)
		return 0;
	return Send(Target(msg->hwnd), msg->message, msg->wParam, msg->lParam);
}

BOOL PostMessageA(HWND, UINT msg, WPARAM wp, LPARAM lp)
{
	CynexPlatform_PostMessage(msg, wp, lp);
	return TRUE;
}

LRESULT SendMessageA(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	return Send(Target(hWnd), msg, wp, lp);
}

void PostQuitMessage(int code)
{
	CynexPlatform_PostMessage(WM_QUIT, (uintptr_t)code, 0);
}

BOOL WaitMessage(void)
{
	CynexPlatform_PumpEvents(16);
	return TRUE;
}

UINT_PTR SetTimer(HWND, UINT_PTR id, UINT, void*) { return id ? id : 1; }
BOOL KillTimer(HWND, UINT_PTR) { return TRUE; }

int MessageBoxA(HWND, LPCSTR text, LPCSTR caption, UINT type)
{
	CynexPlatform_ShowMessage(caption, text);
	if ((type & 0xF) == MB_YESNO)
		return IDYES;
	return IDOK;
}

BOOL MessageBeep(UINT) { return TRUE; }

// ---------------------------------------------------------------- sistem
int GetSystemMetrics(int index)
{
	switch (index)
	{
		case SM_CXSCREEN:
		case SM_CXFULLSCREEN:
			return CynexPlatform_GetScreenWidth();
		case SM_CYSCREEN:
		case SM_CYFULLSCREEN:
			return CynexPlatform_GetScreenHeight();
	}
	return 0;
}

BOOL SystemParametersInfoA(UINT action, UINT param, PVOID pv, UINT)
{
	if (!pv)
		return TRUE;
	switch (action)
	{
		case SPI_GETSTICKYKEYS:
		{
			LPSTICKYKEYS sk = (LPSTICKYKEYS)pv;
			sk->dwFlags = 0;
			return TRUE;
		}
		case SPI_SETSTICKYKEYS:
			return TRUE;
	}
	// Cynex: bilinmeyenlerde cikis alanini sifirla
	if (param > 0 && param < 4096)
		memset(pv, 0, param);
	return TRUE;
}

LONG ChangeDisplaySettingsA(LPDEVMODE, DWORD) { return DISP_CHANGE_SUCCESSFUL; }

BOOL EnumDisplaySettingsA(LPCSTR, DWORD mode, LPDEVMODE dm)
{
	if (!dm || (mode != ENUM_CURRENT_SETTINGS && mode != 0))
		return FALSE;
	dm->dmBitsPerPel = 32;
	dm->dmPelsWidth = CynexPlatform_GetScreenWidth();
	dm->dmPelsHeight = CynexPlatform_GetScreenHeight();
	dm->dmDisplayFrequency = 60;
	dm->dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
	return TRUE;
}

// ---------------------------------------------------------------- klavye / fare
SHORT GetAsyncKeyState(int vk)
{
	return CynexPlatform_IsVKeyDown(vk) ? (SHORT)0x8000 : 0;
}

SHORT GetKeyState(int vk)
{
	return CynexPlatform_IsVKeyDown(vk) ? (SHORT)0xFF80 : 0;
}

BOOL GetKeyboardState(PBYTE state)
{
	if (!state)
		return FALSE;
	for (int i = 0; i < 256; ++i)
		state[i] = CynexPlatform_IsVKeyDown(i) ? 0x80 : 0;
	return TRUE;
}

HKL GetKeyboardLayout(DWORD)
{
	return (HKL)(uintptr_t)0x041F041F;	// Cynex: tr-TR
}

UINT MapVirtualKeyA(UINT code, UINT) { return code; }

BOOL GetCursorPos(LPPOINT pt)
{
	if (!pt)
		return FALSE;
	int x, y;
	CynexPlatform_GetMousePos(&x, &y);
	CynexPlatform_MarkCursorRead();
	pt->x = x;
	pt->y = y;
	if (g_mainWindow)
	{
		pt->x += g_mainWindow->x;
		pt->y += g_mainWindow->y;
	}
	return TRUE;
}

BOOL SetCursorPos(int x, int y)
{
	if (g_mainWindow)
	{
		x -= g_mainWindow->x;
		y -= g_mainWindow->y;
	}
	CynexPlatform_SetMousePos(x, y);
	return TRUE;
}

// Cynex: donanim imleci yok, imlec dokunmatikte cizilmez
static HCURSOR s_cursor = nullptr;
HCURSOR SetCursor(HCURSOR c) { HCURSOR old = s_cursor; s_cursor = c; return old; }
HCURSOR GetCursor(void) { return s_cursor; }
int ShowCursor(BOOL show) { return show ? ++g_cursorCount : --g_cursorCount; }
BOOL ClipCursor(const RECT*) { return TRUE; }
HCURSOR LoadCursorA(HINSTANCE, LPCSTR) { return (HCURSOR)(uintptr_t)0x10; }
HCURSOR LoadCursorFromFileA(LPCSTR) { return (HCURSOR)(uintptr_t)0x11; }
HICON LoadIconA(HINSTANCE, LPCSTR) { return (HICON)(uintptr_t)0x12; }
HANDLE LoadImageA(HINSTANCE, LPCSTR, UINT, int, int, UINT) { return (HANDLE)(uintptr_t)0x13; }
BOOL DestroyCursor(HCURSOR) { return TRUE; }
BOOL DestroyIcon(HICON) { return TRUE; }
HICON CreateIconFromResourceEx(PBYTE, DWORD, BOOL, DWORD, int, int, UINT) { return (HICON)(uintptr_t)0x14; }

int LoadStringA(HINSTANCE, UINT, LPSTR buf, int max)
{
	if (buf && max > 0)
		buf[0] = 0;
	return 0;
}

// ---------------------------------------------------------------- menu (tepsi menusu yok)
HMENU GetMenu(HWND) { return nullptr; }
HMENU CreatePopupMenu(void) { return (HMENU)(uintptr_t)0x20; }
HMENU CreateMenu(void) { return (HMENU)(uintptr_t)0x21; }
BOOL DestroyMenu(HMENU) { return TRUE; }
BOOL InsertMenuA(HMENU, UINT, UINT, UINT_PTR, LPCSTR) { return TRUE; }
BOOL AppendMenuA(HMENU, UINT, UINT_PTR, LPCSTR) { return TRUE; }
BOOL TrackPopupMenu(HMENU, UINT, int, int, int, HWND, const RECT*) { return FALSE; }

// ---------------------------------------------------------------- dikdortgen
BOOL SetRect(LPRECT rc, int l, int t, int r, int b)
{
	if (!rc)
		return FALSE;
	rc->left = l;
	rc->top = t;
	rc->right = r;
	rc->bottom = b;
	return TRUE;
}

BOOL SetRectEmpty(LPRECT rc) { return SetRect(rc, 0, 0, 0, 0); }

BOOL PtInRect(const RECT* rc, POINT pt)
{
	return rc && pt.x >= rc->left && pt.x < rc->right && pt.y >= rc->top && pt.y < rc->bottom;
}

BOOL OffsetRect(LPRECT rc, int dx, int dy)
{
	if (!rc)
		return FALSE;
	rc->left += dx;
	rc->right += dx;
	rc->top += dy;
	rc->bottom += dy;
	return TRUE;
}

BOOL IntersectRect(LPRECT dst, const RECT* a, const RECT* b)
{
	if (!dst || !a || !b)
		return FALSE;
	dst->left = std::max(a->left, b->left);
	dst->top = std::max(a->top, b->top);
	dst->right = std::min(a->right, b->right);
	dst->bottom = std::min(a->bottom, b->bottom);
	if (dst->left >= dst->right || dst->top >= dst->bottom)
	{
		SetRectEmpty(dst);
		return FALSE;
	}
	return TRUE;
}

// ---------------------------------------------------------------- pano
static std::string s_clipText;

BOOL OpenClipboard(HWND) { return TRUE; }
BOOL CloseClipboard(void) { return TRUE; }
BOOL EmptyClipboard(void) { return TRUE; }
BOOL IsClipboardFormatAvailable(UINT format) { return format == CF_TEXT || format == CF_UNICODETEXT; }
UINT RegisterClipboardFormatA(LPCSTR) { return 0xC000; }

HANDLE GetClipboardData(UINT format)
{
	// Cynex: android panosu utf-8 -> oyunun kod sayfasi
	const char* utf8 = CynexPlatform_GetClipboardText();
	const int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
	std::vector<wchar_t> wide(wlen > 0 ? wlen : 1, 0);
	if (wlen > 0)
		MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wide.data(), wlen);

	if (format == CF_UNICODETEXT)
	{
		HGLOBAL h = GlobalAlloc(GMEM_ZEROINIT, wide.size() * sizeof(wchar_t));
		if (h)
			memcpy(GlobalLock(h), wide.data(), wide.size() * sizeof(wchar_t));
		return h;
	}

	const int alen = WideCharToMultiByte(CP_ACP, 0, wide.data(), -1, nullptr, 0, nullptr, nullptr);
	HGLOBAL h = GlobalAlloc(GMEM_ZEROINIT, alen > 0 ? alen : 1);
	if (h && alen > 0)
		WideCharToMultiByte(CP_ACP, 0, wide.data(), -1, (char*)GlobalLock(h), alen, nullptr, nullptr);
	return h;
}

HANDLE SetClipboardData(UINT format, HANDLE mem)
{
	if (!mem)
		return nullptr;
	const void* p = GlobalLock(mem);
	std::vector<wchar_t> wide;
	if (format == CF_UNICODETEXT)
		wide.assign((const wchar_t*)p, (const wchar_t*)p + wcslen((const wchar_t*)p) + 1);
	else
	{
		const int n = MultiByteToWideChar(CP_ACP, 0, (const char*)p, -1, nullptr, 0);
		wide.resize(n > 0 ? n : 1, 0);
		if (n > 0)
			MultiByteToWideChar(CP_ACP, 0, (const char*)p, -1, wide.data(), n);
	}
	const int ulen = WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, nullptr, 0, nullptr, nullptr);
	std::string utf8(ulen > 0 ? ulen : 1, 0);
	if (ulen > 0)
		WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, &utf8[0], ulen, nullptr, nullptr);
	CynexPlatform_SetClipboardText(utf8.c_str());
	GlobalFree(mem);	// Cynex: windows'ta sahiplik sisteme gecer
	return mem;
}

// ---------------------------------------------------------------- kabuk
HINSTANCE ShellExecuteA(HWND, LPCSTR, LPCSTR file, LPCSTR, LPCSTR, INT)
{
	if (file && (!strncmp(file, "http://", 7) || !strncmp(file, "https://", 8)))
	{
		CynexPlatform_OpenURL(file);
		return (HINSTANCE)(uintptr_t)33;
	}
	return (HINSTANCE)(uintptr_t)2;	// Cynex: dosya acma yok
}

BOOL Shell_NotifyIconA(DWORD, PNOTIFYICONDATA) { return TRUE; }

BOOL SHGetSpecialFolderPathA(HWND, LPSTR path, int, BOOL create)
{
	// Cynex: belgelerim/resimlerim -> uygulama veri klasoru (ekran goruntuleri buraya)
	if (!path)
		return FALSE;
	snprintf(path, MAX_PATH, "%s", CynexPlatform_GetDataPath());
	if (create)
		mkdir(path, 0755);
	return TRUE;
}

HRESULT SHGetFolderPathA(HWND hwnd, int csidl, HANDLE, DWORD, LPSTR path)
{
	return SHGetSpecialFolderPathA(hwnd, path, csidl, TRUE) ? S_OK : E_FAIL;
}

}	// extern "C"
