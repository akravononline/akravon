// Cynex: EterLib/Input.cpp mobil karsiligi - directinput yerine platform katmani tus durumunu yaziyor
#include "../../../client/source/EterLib/StdAfx.h"
#include "../../../client/source/EterLib/Input.h"
#include "../../platform/CynexInput.h"

LPDIRECTINPUT8			CInputDevice::ms_lpDI = NULL;
LPDIRECTINPUTDEVICE8	CInputKeyboard::ms_lpKeyboard = NULL;
bool					CInputKeyboard::ms_bPressedKey[256];
char					CInputKeyboard::ms_diks[256];

CInputDevice::CInputDevice()
{
}

CInputDevice::~CInputDevice()
{
}

HRESULT CInputDevice::CreateDevice(HWND /*hWnd*/)
{
	return S_OK;
}

CInputKeyboard::CInputKeyboard()
{
	ResetKeyboard();
}

CInputKeyboard::~CInputKeyboard()
{
}

void CInputKeyboard::ResetKeyboard()
{
	memset(ms_diks, 0, sizeof(ms_diks));
	memset(ms_bPressedKey, 0, sizeof(ms_bPressedKey));
}

bool CInputKeyboard::InitializeKeyboard(HWND /*hWnd*/)
{
	return true;
}

void CInputKeyboard::UpdateKeyboard()
{
	CynexInput_GetKeyboardState(reinterpret_cast<unsigned char*>(ms_diks));

	for (int i = 0; i < 256; ++i)
	{
		if (ms_diks[i] & 0x80)
		{
			if (!IsPressed(i))
				KeyDown(i);
		}
		else if (IsPressed(i))
			KeyUp(i);
	}
}

void CInputKeyboard::KeyDown(int iIndex)
{
	ms_bPressedKey[iIndex] = true;
	OnKeyDown(iIndex);
}

void CInputKeyboard::KeyUp(int iIndex)
{
	ms_bPressedKey[iIndex] = false;
	OnKeyUp(iIndex);
}

bool CInputKeyboard::IsPressed(int iIndex)
{
	return ms_bPressedKey[iIndex];
}
