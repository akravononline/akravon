// Cynex: cef yerine android tarayicisi (platform katmani acar)
#include "../../../client/source/UserInterface/StdAfx.h"
#include "../../../client/source/UserInterface/CefWebBrowser.h"
#include "../../platform/CynexPlatform.h"

namespace
{
	RECT s_rcBrowser = { 0, 0, 0, 0 };
}

extern "C" int CefWebBrowser_Startup(HINSTANCE hInstance)
{
	return 1;
}

extern "C" void CefWebBrowser_Cleanup()
{
}

extern "C" void CefWebBrowser_Destroy()
{
}

extern "C" int CefWebBrowser_Show(HWND parent, const char* addr, const RECT* rcWebBrowser)
{
	if (rcWebBrowser)
		s_rcBrowser = *rcWebBrowser;
	if (addr && *addr)
		CynexPlatform_OpenURL(addr);
	return 1;
}

extern "C" void CefWebBrowser_Hide()
{
}

extern "C" void CefWebBrowser_Move(const RECT* rcWebBrowser)
{
	if (rcWebBrowser)
		s_rcBrowser = *rcWebBrowser;
}

extern "C" int CefWebBrowser_IsVisible()
{
	return 0;
}

extern "C" const RECT& CefWebBrowser_GetRect()
{
	return s_rcBrowser;
}
