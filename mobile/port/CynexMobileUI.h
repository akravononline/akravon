// Cynex: mobil arayuz ortak tanimlari (EterPythonLib + UserInterface)
#pragma once

namespace CynexUI
{
	// Cynex: CWindow bayraklari, 1..11 kullanimda - ust bitler bos
	enum : DWORD
	{
		FLAG_TOUCH_NATIVE	= (1u << 28),	// dokunmayi pencere kendisi okur (joystick, saldiri)
		FLAG_SINGLE_TAP		= (1u << 29),	// iki dokunus kapisindan muaf
	};
}

namespace UI { class CSlotWindow; }
class CGraphicImageInstance;

void CynexSlot_BeforeIcon(UI::CSlotWindow* pWnd, DWORD dwSlotNumber, CGraphicImageInstance* pInstance, float fPosX, float fPosY);
void CynexSlot_BeforeBase(UI::CSlotWindow* pWnd, CGraphicImageInstance* pInstance);
float CynexSlot_CoolHalf(UI::CSlotWindow* pWnd, DWORD dwSlotNumber);
void CynexSlot_Forget(UI::CSlotWindow* pWnd);
void CynexWndMgr_InitMobile();
