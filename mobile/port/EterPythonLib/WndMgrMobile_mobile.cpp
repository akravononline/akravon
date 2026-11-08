// Cynex: wndMgr mobil eklentileri - dokunma bayraklari, yuva ikon olcegi
#include "../../../client/source/EterPythonLib/StdAfx.h"
#include "../../../client/source/EterPythonLib/PythonWindow.h"
#include "../../../client/source/EterPythonLib/PythonSlotWindow.h"
#include "../CynexMobileUI.h"

#include <unordered_map>

bool PyTuple_GetWindow(PyObject* poArgs, int pos, UI::CWindow ** ppRetWindow);

namespace
{
	struct SSlotScale
	{
		float sx;
		float sy;
		CGraphicImage* pImage;	// Cynex: baska resimle SetSlot yapilirsa olcek gecersiz
	};

	struct SWindowScale
	{
		float unitX = 1.0f;
		float unitY = 1.0f;
		float baseX = 1.0f;
		float baseY = 1.0f;
		bool round = false;
		int pad = 0;	// AKRAVON: ikon hucre icinde her kenardan pad px bosluklu ve ortali
		std::unordered_map<DWORD, SSlotScale> slots;
	};

	std::unordered_map<UI::CSlotWindow*, SWindowScale> g_scales;

	UI::CSlotWindow* GetSlotWindow(PyObject* poArgs)
	{
		UI::CWindow* pWin;
		if (!PyTuple_GetWindow(poArgs, 0, &pWin))
			return NULL;
		if (!pWin->IsType(UI::CSlotWindow::Type()))
			return NULL;
		return (UI::CSlotWindow*)pWin;
	}

	bool GetColor(PyObject* poArgs, int pos, D3DXCOLOR* pColor)
	{
		*pColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		PyObject* poColor;
		if (!PyTuple_GetObject(poArgs, pos, &poColor) || !PyTuple_Check(poColor) || PyTuple_Size(poColor) != 4)
			return false;
		PyTuple_GetFloat(poColor, 0, &pColor->r);
		PyTuple_GetFloat(poColor, 1, &pColor->g);
		PyTuple_GetFloat(poColor, 2, &pColor->b);
		PyTuple_GetFloat(poColor, 3, &pColor->a);
		return true;
	}

	PyObject* SetWindowFlag(PyObject* poArgs, DWORD dwFlag)
	{
		UI::CWindow* pWin;
		if (!PyTuple_GetWindow(poArgs, 0, &pWin))
			return Py_BuildException();
		int iEnable = 1;
		PyTuple_GetInteger(poArgs, 1, &iEnable);
		if (iEnable)
			pWin->AddFlag(dwFlag);
		else
			pWin->RemoveFlag(dwFlag);
		return Py_BuildNone();
	}

	PyObject* wndMgrSetTouchNative(PyObject* poSelf, PyObject* poArgs)
	{
		return SetWindowFlag(poArgs, CynexUI::FLAG_TOUCH_NATIVE);
	}

	PyObject* wndMgrSetTekDokunus(PyObject* poSelf, PyObject* poArgs)
	{
		return SetWindowFlag(poArgs, CynexUI::FLAG_SINGLE_TAP);
	}

	// Cynex: SetSlotScale(hwnd, slot, vnum, w, h, icon, (r,g,b,a), sx, sy)
	PyObject* wndMgrSetSlotScale(PyObject* poSelf, PyObject* poArgs)
	{
		UI::CSlotWindow* pSlotWin = GetSlotWindow(poArgs);
		if (!pSlotWin)
			return Py_BuildException();

		int iSlotIndex, iItemIndex, iWidth, iHeight;
		long long llImage;
		float fx, fy;
		if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex) || !PyTuple_GetInteger(poArgs, 2, &iItemIndex) ||
			!PyTuple_GetInteger(poArgs, 3, &iWidth) || !PyTuple_GetInteger(poArgs, 4, &iHeight) ||
			!PyTuple_GetLongLong(poArgs, 5, &llImage))
			return Py_BuildException();
		D3DXCOLOR color;
		GetColor(poArgs, 6, &color);
		if (!PyTuple_GetFloat(poArgs, 7, &fx) || !PyTuple_GetFloat(poArgs, 8, &fy))
			return Py_BuildException();

		CGraphicImage* pImage = (CGraphicImage*)llImage;
		pSlotWin->SetSlot(iSlotIndex, iItemIndex, iWidth, iHeight, pImage, color);
		g_scales[pSlotWin].slots[(DWORD)iSlotIndex] = { fx, fy, pImage };
		return Py_BuildNone();
	}

	// Cynex: SetSlotUnitSize(hwnd, w, h[, iconlar da]) - ikonlar 32px yerine w x h cizilir
	PyObject* wndMgrSetSlotUnitSize(PyObject* poSelf, PyObject* poArgs)
	{
		UI::CSlotWindow* pSlotWin = GetSlotWindow(poArgs);
		if (!pSlotWin)
			return Py_BuildException();
		int iWidth, iHeight;
		if (!PyTuple_GetInteger(poArgs, 1, &iWidth) || !PyTuple_GetInteger(poArgs, 2, &iHeight))
			return Py_BuildException();
		SWindowScale& rScale = g_scales[pSlotWin];
		rScale.unitX = iWidth > 0 ? float(iWidth) / float(UI::ITEM_WIDTH) : 1.0f;
		rScale.unitY = iHeight > 0 ? float(iHeight) / float(UI::ITEM_HEIGHT) : 1.0f;
		int iPad = 0;
		if (PyTuple_Size(poArgs) > 3)
			PyTuple_GetInteger(poArgs, 3, &iPad);
		rScale.pad = iPad > 0 ? iPad : 0;
		return Py_BuildNone();
	}

	PyObject* wndMgrGetSlotUnitSize(PyObject* poSelf, PyObject* poArgs)
	{
		UI::CSlotWindow* pSlotWin = GetSlotWindow(poArgs);
		if (!pSlotWin)
			return Py_BuildException();
		auto it = g_scales.find(pSlotWin);
		if (it == g_scales.end())
			return Py_BuildValue("(ii)", (int)UI::ITEM_WIDTH, (int)UI::ITEM_HEIGHT);
		return Py_BuildValue("(ii)", int(it->second.unitX * UI::ITEM_WIDTH + 0.5f), int(it->second.unitY * UI::ITEM_HEIGHT + 0.5f));
	}

	// Cynex: SetSlotBaseImageScale(hwnd, dosya, r, g, b, a, sx, sy)
	PyObject* wndMgrSetSlotBaseImageScale(PyObject* poSelf, PyObject* poArgs)
	{
		UI::CSlotWindow* pSlotWin = GetSlotWindow(poArgs);
		if (!pSlotWin)
			return Py_BuildException();
		char* szFileName;
		float fr, fg, fb, fa, fx, fy;
		if (!PyTuple_GetString(poArgs, 1, &szFileName) || !PyTuple_GetFloat(poArgs, 2, &fr) || !PyTuple_GetFloat(poArgs, 3, &fg) ||
			!PyTuple_GetFloat(poArgs, 4, &fb) || !PyTuple_GetFloat(poArgs, 5, &fa) || !PyTuple_GetFloat(poArgs, 6, &fx) ||
			!PyTuple_GetFloat(poArgs, 7, &fy))
			return Py_BuildException();
		pSlotWin->SetSlotBaseImage(szFileName, fr, fg, fb, fa);
		SWindowScale& rScale = g_scales[pSlotWin];
		rScale.baseX = fx;
		rScale.baseY = fy;
		return Py_BuildNone();
	}

	// Cynex: yuvarlak ikon tasarimi yok - yalniz bayrak saklanir
	PyObject* wndMgrSetSlotRoundIcons(PyObject* poSelf, PyObject* poArgs)
	{
		UI::CSlotWindow* pSlotWin = GetSlotWindow(poArgs);
		if (!pSlotWin)
			return Py_BuildException();
		int iEnable = 1;
		PyTuple_GetInteger(poArgs, 1, &iEnable);
		g_scales[pSlotWin].round = iEnable != 0;
		return Py_BuildNone();
	}

	// Cynex: arayuz olceklenmiyor (1280x720 cizim ekrana gerilir)
	PyObject* wndMgrGetUIScale(PyObject* poSelf, PyObject* poArgs)
	{
		return Py_BuildValue("f", 1.0f);
	}

	// Cynex: doku suzgeci gl tarafinda sabit - bos
	PyObject* wndMgrSetFilterMode(PyObject* poSelf, PyObject* poArgs)
	{
		return Py_BuildNone();
	}
}

void CynexSlot_BeforeIcon(UI::CSlotWindow* pWnd, DWORD dwSlotNumber, CGraphicImageInstance* pInstance, float fPosX, float fPosY)
{
	if (g_scales.empty())
		return;
	auto it = g_scales.find(pWnd);
	if (it == g_scales.end())
		return;
	SWindowScale& rScale = it->second;
	float fx = rScale.unitX, fy = rScale.unitY;
	auto slot = rScale.slots.find(dwSlotNumber);
	if (slot != rScale.slots.end())
	{
		if (slot->second.pImage == pInstance->GetGraphicImagePointer())
		{
			fx = slot->second.sx;
			fy = slot->second.sy;
		}
		else
			rScale.slots.erase(slot);
	}
	if (rScale.pad > 0 && slot == rScale.slots.end() && rScale.unitX > 0.0f)
	{
		// hucre (unit*32) icinde pad bosluk: olcek kuculur, ikon ortaya kayar (cok hucreli esyada da ayni pad)
		const float cellW = rScale.unitX * float(UI::ITEM_WIDTH);
		const float k = (cellW - 2.0f * rScale.pad) / cellW;
		fx *= k;
		fy *= k;
		pInstance->SetPosition(fPosX + rScale.pad, fPosY + rScale.pad);
	}
	pInstance->SetScale(fx, fy);
}

void CynexSlot_BeforeBase(UI::CSlotWindow* pWnd, CGraphicImageInstance* pInstance)
{
	if (g_scales.empty())
		return;
	auto it = g_scales.find(pWnd);
	if (it != g_scales.end())
		pInstance->SetScale(it->second.baseX, it->second.baseY);
}

float CynexSlot_CoolHalf(UI::CSlotWindow* pWnd, DWORD dwSlotNumber)
{
	if (!g_scales.empty())
	{
		auto it = g_scales.find(pWnd);
		if (it != g_scales.end())
		{
			auto slot = it->second.slots.find(dwSlotNumber);
			const float fx = slot != it->second.slots.end() ? slot->second.sx : it->second.unitX;
			return float(UI::ITEM_WIDTH) * fx * 0.5f;
		}
	}
	return float(UI::ITEM_WIDTH) * 0.5f;
}

// AKRAVON: hucrenin gercek (olcekli) boyutu - secim/aktif efektleri icin
bool CynexSlot_CellSize(UI::CSlotWindow* pWnd, DWORD dwSlotNumber, int iPlacedX, int iPlacedY, float* pfW, float* pfH)
{
	if (g_scales.empty())
		return false;
	auto it = g_scales.find(pWnd);
	if (it == g_scales.end())
		return false;
	auto slot = it->second.slots.find(dwSlotNumber);
	const float fx = slot != it->second.slots.end() ? slot->second.sx : it->second.unitX;
	const float fy = slot != it->second.slots.end() ? slot->second.sy : it->second.unitY;
	*pfW = float(UI::ITEM_WIDTH) * fx * float(iPlacedX < 1 ? 1 : iPlacedX);
	*pfH = float(UI::ITEM_HEIGHT) * fy * float(iPlacedY < 1 ? 1 : iPlacedY);
	return true;
}

bool CynexSlot_IsRound(UI::CSlotWindow* pWnd)
{
	auto it = g_scales.find(pWnd);
	return it != g_scales.end() && it->second.round;
}

void CynexSlot_Forget(UI::CSlotWindow* pWnd)
{
	if (!g_scales.empty())
		g_scales.erase(pWnd);
}

void CynexWndMgr_InitMobile()
{
	static PyMethodDef s_methods[] =
	{
		{ "SetTouchNative",			wndMgrSetTouchNative,			METH_VARARGS },
		{ "SetTekDokunus",			wndMgrSetTekDokunus,			METH_VARARGS },
		{ "SetSlotScale",			wndMgrSetSlotScale,				METH_VARARGS },
		{ "SetSlotUnitSize",		wndMgrSetSlotUnitSize,			METH_VARARGS },
		{ "GetSlotUnitSize",		wndMgrGetSlotUnitSize,			METH_VARARGS },
		{ "SetSlotBaseImageScale",	wndMgrSetSlotBaseImageScale,	METH_VARARGS },
		{ "SetSlotRoundIcons",		wndMgrSetSlotRoundIcons,		METH_VARARGS },
		{ "GetUIScale",				wndMgrGetUIScale,				METH_VARARGS },
		{ "SetFilterMode",			wndMgrSetFilterMode,			METH_VARARGS },
		{ NULL, NULL, 0 },
	};
	Py_InitModule("wndMgr", s_methods);
}
