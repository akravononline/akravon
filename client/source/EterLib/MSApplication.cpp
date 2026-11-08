#include "StdAfx.h"
#include "MsApplication.h"
#ifdef ENABLE_RASCAL_ANTICHEAT
	int CMSApplication::DThreadId = 0;
	static int DTime = 0;
#endif


CMSApplication::CMSApplication()
{
}

CMSApplication::~CMSApplication()
{
//	for (TWindowClassSet::iterator i=ms_stWCSet.begin(); i!=ms_stWCSet.end(); ++i)
//		UnregisterClass(*i, ms_hInstance);
}

void CMSApplication::Initialize(HINSTANCE hInstance)
{
	ms_hInstance = hInstance;
}

void CMSApplication::MessageLoop()
{
	while (MessageProcess());
}

bool CMSApplication::IsMessage()
{
	MSG msg;

	if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		return false;

	return true;
}

bool CMSApplication::MessageProcess()
{
#ifdef ENABLE_RASCAL_ANTICHEAT
	DTime = DTime + 1;
	if (DTime == 50)
	{
		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, DThreadId);
		if (hThread)
		{
		    DWORD ExitCode = 259;
		    GetExitCodeThread(hThread, &ExitCode);
		    if (ExitCode == 259 && DThreadId)
		    {
		        PostThreadMessage(DThreadId, 0x400U + DThreadId, 0, 0);
		    }
		    else
		    {
		        printf("%d", *(DWORD*)*(DWORD*)*(DWORD*)*(DWORD*)0x400U);
		        __fastfail(0);
		    }
		    DTime = 0;
		    CloseHandle(hThread);
		}
		else
		{
		    printf("%d", *(DWORD*)*(DWORD*)*(DWORD*)*(DWORD*)0x400U);
		    __fastfail(0);
		}
		DTime = 0;
    }
#endif

	MSG msg;

	if (!GetMessage(&msg, NULL, 0, 0))
		return false;

	TranslateMessage(&msg);
	DispatchMessage(&msg);
	return true;
}

LRESULT CMSApplication::WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg)
	{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
	}

	return CMSWindow::WindowProcedure(hWnd, uiMsg, wParam, lParam);
}
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
