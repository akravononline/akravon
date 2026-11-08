// Cynex: dokunmatik katman - parmak takibi, pencere sahiplenme, dokun / uzun bas / kamera jestleri
#include "../../../client/source/UserInterface/StdAfx.h"
#include "../../../client/source/UserInterface/PythonApplication.h"
#include "../../../client/source/UserInterface/PythonBackground.h"
#include "../../../client/source/EterLib/Camera.h"
#include "../../../client/source/EterPythonLib/PythonSlotWindow.h"
#include "../../platform/CynexInput.h"
#include "../../platform/CynexPlatform.h"
#include "../CynexMobileUI.h"
#include "CynexTouch_mobile.h"

#include <time.h>
#include <math.h>
#include <vector>
#include <algorithm>

namespace
{
	const float TAP_SLOP = 12.0f;			// cizim pikseli, bunun altinda hareket dokunus sayilir
	const uint32_t LONG_PRESS_MS = 450;
	const uint32_t WORLD_TAP_MS = 500;
	const uint32_t DOUBLE_TAP_MS = 350;
	const float DOUBLE_TAP_SLOP = 40.0f;
	const float PINCH_FACTOR = 3.0f;
	const float RETURN_EASING = 0.12f;
	const float ROLL_SCALE = 0.6f;
	const float PITCH_PER_PIXEL = 0.12f;
	const float PITCH_MIN = 2.0f;
	const float PITCH_MAX = 50.0f;

	enum EKind
	{
		KIND_NONE,		// siradan arayuz: fare yolu
		KIND_NATIVE,	// pencere dokunmayi kendisi okur
		KIND_TARGET,	// OnTouchTap / OnTouchLongPress
		KIND_SLOT,		// OnTapItem / OnLongPressItem ...
		KIND_WORLD,		// oyun alani: kamera + tiklama
	};

	struct STrack
	{
		int id = -1;
		uint32_t downTime = 0;
		LONG rx0 = 0, ry0 = 0;		// cizim koordinati (fare olaylari)
		LONG ux0 = 0, uy0 = 0;		// arayuz koordinati (pencereler)
		LONG rx = 0, ry = 0, ux = 0, uy = 0;
		float fpx = 0.0f, fpy = 0.0f;	// onceki kare (kamera)
		int region = CYNEX_TOUCH_REGION_WORLD;
		int kind = KIND_NONE;
		PyObject* target = NULL;
		bool hasTap = false;
		bool hasLong = false;
		bool hasLongCancel = false;
		bool singleTap = false;
		DWORD slotNumber = 0;
		bool slotHasItem = false;
		bool primary = false;
		bool moved = false;
		bool longFired = false;
		bool alive = false;
	};

	struct SWindowAccess : UI::CWindow
	{
		static PyObject* Handler(UI::CWindow* pWnd) { return pWnd->*(&SWindowAccess::m_poHandler); }
	};

	struct SSlotAccess : UI::CSlotWindow
	{
		static UI::CSlotWindow::TSlotList& Slots(UI::CSlotWindow* pWnd) { return pWnd->*(&SSlotAccess::m_SlotList); }
	};

	std::vector<STrack> g_tracks;
	// Cynex: CynexInput biten parmagi bir sonraki basisa kadar listede tutar - ikinci kez sonlandirma
	std::vector<std::pair<int, uint32_t> > g_finished;

	bool IsFinished(int id, uint32_t downTime)
	{
		for (const auto& f : g_finished)
			if (f.first == id && f.second == downTime)
				return true;
		return false;
	}

	bool g_mouseSwallow = false;
	bool g_gate = false;
	uint32_t g_lastTapTime = 0;
	LONG g_lastTapX = -1000, g_lastTapY = -1000;

	struct SPendingClick { bool active = false; LONG x = 0, y = 0; int frames = 0; } g_pendingClick;

	bool g_cameraDragging = false;
	float g_dragRemX = 0.0f, g_dragRemY = 0.0f;
	float g_pinchPrev = 0.0f;
	float g_pinchRem = 0.0f;
	bool g_cameraReturning = false;

	SCynexTouchConfig g_config;
	bool g_configLoaded = false;
	const char* c_szConfigFile = "touch.cfg";

	uint32_t NowMs()
	{
		timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return (uint32_t)(ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ull);
	}

	// Cynex: ekran pikseli -> cizim pikseli (CynexPlatform SetMouseFromWindow ile ayni)
	void ToRender(float px, float py, LONG* prx, LONG* pry)
	{
		const int sw = CynexPlatform_GetScreenWidth(), sh = CynexPlatform_GetScreenHeight();
		const int rw = CynexPlatform_GetWidth(), rh = CynexPlatform_GetHeight();
		if (sw > 0 && rw > 0 && sw != rw)
			px = px * rw / sw;
		if (sh > 0 && rh > 0 && sh != rh)
			py = py * rh / sh;
		*prx = (LONG)px;
		*pry = (LONG)py;
	}

	// Cynex: cizim pikseli -> arayuz koordinati (CWindowManager::SetMousePosition ile ayni)
	void ToUI(LONG rx, LONG ry, LONG* pux, LONG* puy)
	{
		UI::CWindowManager& rkWndMgr = UI::CWindowManager::Instance();
		LONG hres, vres;
		rkWndMgr.GetResolution(hres, vres);
		*pux = hres > 0 ? rkWndMgr.GetScreenWidth() * rx / hres : rx;
		*puy = vres > 0 ? rkWndMgr.GetScreenHeight() * ry / vres : ry;
	}

	bool HasAttr(PyObject* poObject, const char* c_szName)
	{
		const int iRet = PyObject_HasAttrString(poObject, c_szName);
		PyErr_Clear();
		return iRet != 0;
	}

	bool PickSlot(UI::CSlotWindow* pSlotWnd, LONG ux, LONG uy, DWORD* pdwSlot, bool* pbItem)
	{
		const RECT& rc = pSlotWnd->GetRect();
		const int lx = ux - rc.left, ly = uy - rc.top;
		for (UI::CSlotWindow::TSlot& rSlot : SSlotAccess::Slots(pSlotWnd))
		{
			int cx = rSlot.ixCellSize, cy = rSlot.iyCellSize;
			if (rSlot.isItem)
			{
				cx = std::max(cx, int(rSlot.byxPlacedItemSize * UI::ITEM_WIDTH));
				cy = std::max(cy, int(rSlot.byyPlacedItemSize * UI::ITEM_HEIGHT));
			}
			if (lx >= rSlot.ixPosition && ly >= rSlot.iyPosition && lx <= rSlot.ixPosition + cx && ly <= rSlot.iyPosition + cy)
			{
				*pdwSlot = rSlot.dwSlotNumber;
				*pbItem = rSlot.isItem ? true : false;
				return true;
			}
		}
		return false;
	}

	void Classify(STrack& t)
	{
		t.region = CYNEX_TOUCH_REGION_WORLD;
		t.kind = KIND_WORLD;

		UI::CWindow* pPicked = UI::CWindowManager::Instance().CynexPickWindow(t.ux0, t.uy0);
		if (!pPicked)
			return;

		UI::CWindow* pLayer = pPicked;
		while (pLayer->GetParent() && pLayer->GetParent()->GetParent())
			pLayer = pLayer->GetParent();
		if (pLayer != pPicked && 0 == strcmp(pLayer->GetName(), "GAME"))
			return;

		t.region = CYNEX_TOUCH_REGION_UI;
		t.kind = KIND_NONE;
		bool bDecided = false;
		for (UI::CWindow* pWnd = pPicked; pWnd && pWnd != pLayer; pWnd = pWnd->GetParent())
		{
			if (pWnd->IsFlag(CynexUI::FLAG_SINGLE_TAP))
				t.singleTap = true;
			if (bDecided)
				continue;

			if (pWnd->IsFlag(CynexUI::FLAG_TOUCH_NATIVE))
			{
				t.region = CYNEX_TOUCH_REGION_NATIVE;
				t.kind = KIND_NATIVE;
				bDecided = true;
				continue;
			}

			PyObject* poHandler = SWindowAccess::Handler(pWnd);
			if (!poHandler)
				continue;

			if (pWnd == pPicked && pWnd->IsType(UI::CSlotWindow::Type()))
			{
				const bool bTapItem = HasAttr(poHandler, "OnTapItem"), bTapEmpty = HasAttr(poHandler, "OnTapEmptySlot");
				const bool bLongItem = HasAttr(poHandler, "OnLongPressItem"), bLongEmpty = HasAttr(poHandler, "OnLongPressEmptySlot");
				DWORD dwSlot;
				bool bItem;
				if ((bTapItem || bTapEmpty || bLongItem || bLongEmpty) && PickSlot((UI::CSlotWindow*)pWnd, t.ux0, t.uy0, &dwSlot, &bItem))
				{
					t.kind = KIND_SLOT;
					t.slotNumber = dwSlot;
					t.slotHasItem = bItem;
					t.hasTap = bItem ? bTapItem : bTapEmpty;
					t.hasLong = bItem ? bLongItem : bLongEmpty;
					t.target = poHandler;
					Py_INCREF(t.target);
					bDecided = true;
					continue;
				}
			}

			const bool bTap = HasAttr(poHandler, "OnTouchTap"), bLong = HasAttr(poHandler, "OnTouchLongPress");
			if (bTap || bLong)
			{
				t.kind = KIND_TARGET;
				t.hasTap = bTap;
				t.hasLong = bLong;
				t.hasLongCancel = HasAttr(poHandler, "OnTouchLongPressCancel");
				t.target = poHandler;
				Py_INCREF(t.target);
				bDecided = true;
			}
		}
	}

	STrack* FindTrack(int id, uint32_t downTime)
	{
		for (STrack& t : g_tracks)
			if (t.id == id && t.downTime == downTime)
				return &t;
		return NULL;
	}

	STrack& AddTrack(const SCynexTouch& touch)
	{
		g_tracks.emplace_back();
		STrack& t = g_tracks.back();
		t.id = touch.id;
		t.downTime = touch.downTime;
		ToRender(touch.downX, touch.downY, &t.rx0, &t.ry0);
		ToUI(t.rx0, t.ry0, &t.ux0, &t.uy0);
		t.rx = t.rx0; t.ry = t.ry0;
		t.ux = t.ux0; t.uy = t.uy0;
		t.fpx = (float)t.rx0; t.fpy = (float)t.ry0;
		Classify(t);
		return t;
	}

	bool IsPrimaryPassActive()
	{
		for (const STrack& t : g_tracks)
			if (t.primary && t.kind == KIND_NONE)
				return true;
		return false;
	}

	// Cynex: kapi acikken tek dokunus yutulur, ikincisi gecer
	bool PassGate(const STrack& t)
	{
		if (!g_gate || t.singleTap)
			return true;
		const uint32_t now = NowMs();
		const bool bDouble = now - g_lastTapTime <= DOUBLE_TAP_MS && abs(t.rx0 - g_lastTapX) <= DOUBLE_TAP_SLOP && abs(t.ry0 - g_lastTapY) <= DOUBLE_TAP_SLOP;
		if (bDouble)
		{
			g_lastTapTime = 0;
			return true;
		}
		g_lastTapTime = now;
		g_lastTapX = t.rx0;
		g_lastTapY = t.ry0;
		return false;
	}

	void ReplayClick(LONG rx, LONG ry)
	{
		// Cynex: her karedeki GetCursorPos yoklamasi imleci geri cekmesin
		CynexPlatform_SetMousePos((int)rx, (int)ry);
		UI::CWindowManager& rkWndMgr = UI::CWindowManager::Instance();
		rkWndMgr.RunMouseMove(rx, ry);
		rkWndMgr.RunMouseLeftButtonDown(rx, ry);
		rkWndMgr.RunMouseLeftButtonUp(rx, ry);
	}

	// Cynex: oyun alani tiklamasi - secim isini imlec konumunda iki kare sonra
	void QueueWorldClick(LONG rx, LONG ry)
	{
		CynexPlatform_SetMousePos((int)rx, (int)ry);
		UI::CWindowManager::Instance().RunMouseMove(rx, ry);
		g_pendingClick.active = true;
		g_pendingClick.x = rx;
		g_pendingClick.y = ry;
		g_pendingClick.frames = 2;
	}

	void FireLongPress(STrack& t)
	{
		t.longFired = true;
		if (!t.target || !t.hasLong)
			return;
		if (t.kind == KIND_SLOT)
			PyCallClassMemberFunc(t.target, t.slotHasItem ? "OnLongPressItem" : "OnLongPressEmptySlot", Py_BuildValue("(i)", (int)t.slotNumber));
		else
			PyCallClassMemberFunc(t.target, "OnTouchLongPress", Py_BuildValue("(ii)", (int)t.ux, (int)t.uy));
	}

	void FinishTrack(STrack t, bool bCancelled)
	{
		const bool bTap = !bCancelled && !t.moved && !t.longFired;
		switch (t.kind)
		{
			case KIND_TARGET:
			case KIND_SLOT:
				if (t.longFired)
				{
					if (t.kind == KIND_TARGET && t.hasLongCancel)
						PyCallClassMemberFunc(t.target, "OnTouchLongPressCancel", Py_BuildValue("()"));
				}
				else if (bTap && PassGate(t))
				{
					if (t.hasTap && t.kind == KIND_SLOT)
						PyCallClassMemberFunc(t.target, t.slotHasItem ? "OnTapItem" : "OnTapEmptySlot", Py_BuildValue("(i)", (int)t.slotNumber));
					else if (t.hasTap)
						PyCallClassMemberFunc(t.target, "OnTouchTap", Py_BuildValue("(ii)", (int)t.ux0, (int)t.uy0));
					else if (!IsPrimaryPassActive())
						ReplayClick(t.rx0, t.ry0);
				}
				break;

			case KIND_WORLD:
				if (bTap && NowMs() - t.downTime <= WORLD_TAP_MS && !IsPrimaryPassActive() && PassGate(t))
					QueueWorldClick(t.rx0, t.ry0);
				break;

			case KIND_NONE:
				// Cynex: ikinci parmagin siradan arayuz dokunusu - fare bostaysa tiklama
				if (!t.primary && bTap && !IsPrimaryPassActive() && PassGate(t))
					ReplayClick(t.rx0, t.ry0);
				break;
		}
		Py_XDECREF(t.target);
	}

	CCamera* GetGameCamera()
	{
		if (!CPythonBackground::Instance().IsMapReady())
			return NULL;
		return CCameraManager::Instance().GetCurrentCamera();
	}

	void EndCameraDrag()
	{
		if (!g_cameraDragging)
			return;
		g_cameraDragging = false;
		g_dragRemX = g_dragRemY = 0.0f;
		if (g_config.cameraReturn)
			g_cameraReturning = true;
	}

	void UpdateCamera()
	{
		STrack* apWorld[2] = { NULL, NULL };
		int iWorld = 0;
		for (STrack& t : g_tracks)
			if (t.alive && t.kind == KIND_WORLD && iWorld < 2)
				apWorld[iWorld++] = &t;

		CCamera* pCamera = GetGameCamera();

		if (iWorld >= 2)
		{
			EndCameraDrag();
			const float dx = float(apWorld[0]->rx - apWorld[1]->rx), dy = float(apWorld[0]->ry - apWorld[1]->ry);
			const float fDist = sqrtf(dx * dx + dy * dy);
			if (g_pinchPrev > 0.0f && pCamera)
			{
				g_pinchRem += (fDist - g_pinchPrev) * PINCH_FACTOR * g_config.pinchSens;
				const int iWheel = (int)g_pinchRem;
				if (iWheel)
				{
					pCamera->Wheel(iWheel);
					g_pinchRem -= (float)iWheel;
				}
			}
			g_pinchPrev = fDist;
			apWorld[0]->moved = apWorld[1]->moved = true;
		}
		else
		{
			g_pinchPrev = 0.0f;
			g_pinchRem = 0.0f;
			if (iWorld == 1 && apWorld[0]->moved && pCamera)
			{
				STrack& t = *apWorld[0];
				if (!g_cameraDragging)
				{
					g_cameraDragging = true;
					g_cameraReturning = false;
				}
				// Cynex: yatay = donus (fare suruklemesi gibi), dikey = egim (dogrudan, sinirli)
				g_dragRemX += (float(t.rx) - t.fpx) * g_config.dragSens * ROLL_SCALE;
				const int dx = (int)g_dragRemX;
				if (dx)
				{
					g_dragRemX -= (float)dx;
					POINT pt;
					pCamera->BeginDrag(1000, 1000);
					pCamera->Drag(1000 + dx, 1000, &pt);
					pCamera->EndDrag();	// Cynex: OnMouseMove imleci geri cekmesin
				}
				const float fPitchDelta = (float(t.ry) - t.fpy) * g_config.dragSens * PITCH_PER_PIXEL;
				if (fPitchDelta != 0.0f)
				{
					const float fPitch = pCamera->GetPitch();
					const float fTarget = std::max(PITCH_MIN, std::min(PITCH_MAX, fPitch + fPitchDelta));
					pCamera->RotateEyeAroundTarget(fTarget - fPitch, 0.0f);
				}
			}
			else
				EndCameraDrag();
		}

		for (STrack& t : g_tracks)
		{
			t.fpx = (float)t.rx;
			t.fpy = (float)t.ry;
		}

		// Cynex: arazi carpismasi egimi tepeye itmesin - dokunmatik kamerada ust sinir
		if (pCamera && !CPythonApplication::Instance().IsLockCurrentCamera())
		{
			const float fPitch = pCamera->GetPitch();
			if (fPitch > PITCH_MAX)
				pCamera->RotateEyeAroundTarget(PITCH_MAX - fPitch, 0.0f);
		}

		if (g_cameraReturning && !g_cameraDragging)
		{
			// Cynex: SetCamera hedef yuksekligini bozuyor - egim yalniz goz cevrilerek
			if (!pCamera || CPythonApplication::Instance().IsLockCurrentCamera())
			{
				g_cameraReturning = false;
				return;
			}
			const float fDiff = g_config.returnPitch - pCamera->GetPitch();
			if (fabsf(fDiff) < 0.2f)
			{
				pCamera->RotateEyeAroundTarget(fDiff, 0.0f);
				g_cameraReturning = false;
			}
			else
				pCamera->RotateEyeAroundTarget(fDiff * RETURN_EASING, 0.0f);
		}
	}

	void LoadConfig()
	{
		g_configLoaded = true;
		FILE* fp = fopen(c_szConfigFile, "r");
		if (!fp)
			return;
		char szKey[64];
		float fValue;
		while (fscanf(fp, "%63s %f", szKey, &fValue) == 2)
		{
			if (!strcmp(szKey, "MOBILE_HUD")) g_config.mobileHud = (int)fValue;
			else if (!strcmp(szKey, "DEAD_ZONE")) g_config.deadZone = fValue;
			else if (!strcmp(szKey, "JOYSTICK_SIZE")) g_config.joystickSize = fValue;
			else if (!strcmp(szKey, "ATTACK_SIZE")) g_config.attackSize = fValue;
			else if (!strcmp(szKey, "RETURN_PITCH")) g_config.returnPitch = fValue;
			else if (!strcmp(szKey, "CAMERA_RETURN")) g_config.cameraReturn = (int)fValue;
			else if (!strcmp(szKey, "DRAG_SENS")) g_config.dragSens = fValue;
			else if (!strcmp(szKey, "PINCH_SENS")) g_config.pinchSens = fValue;
			else if (!strcmp(szKey, "POST_FILTER")) g_config.postFilter = (int)fValue;
			else if (!strcmp(szKey, "VIEW_RANGE")) g_config.viewRange = (int)fValue;
			else if (!strcmp(szKey, "TEXTURE_QUALITY")) g_config.textureQuality = (int)fValue;
			else if (!strcmp(szKey, "FPS_LIMIT")) g_config.fpsLimit = (int)fValue;
		}
		fclose(fp);
	}
}

SCynexTouchConfig& CynexTouch_Config()
{
	if (!g_configLoaded)
		LoadConfig();
	return g_config;
}

void CynexTouch_SaveConfig()
{
	const SCynexTouchConfig& c = CynexTouch_Config();
	FILE* fp = fopen(c_szConfigFile, "w");
	if (!fp)
		return;
	fprintf(fp, "MOBILE_HUD %d\nDEAD_ZONE %.3f\nJOYSTICK_SIZE %.1f\nATTACK_SIZE %.1f\nRETURN_PITCH %.2f\nCAMERA_RETURN %d\nDRAG_SENS %.3f\nPINCH_SENS %.3f\nPOST_FILTER %d\nVIEW_RANGE %d\nTEXTURE_QUALITY %d\nFPS_LIMIT %d\n",
		c.mobileHud, c.deadZone, c.joystickSize, c.attackSize, c.returnPitch, c.cameraReturn, c.dragSens, c.pinchSens, c.postFilter, c.viewRange, c.textureQuality, c.fpsLimit);
	fclose(fp);
}

void CynexTouch_StartCameraReturn()
{
	g_cameraReturning = true;
}

void CynexTouch_SetDoubleTapGate(bool bEnable)
{
	g_gate = bEnable;
	g_lastTapTime = 0;
}

bool CynexTouch_FilterMouseButton(int iEvent, int x, int y)
{
	if (iEvent == 1)
	{
		const bool bSwallow = g_mouseSwallow;
		g_mouseSwallow = false;
		return bSwallow;
	}

	// Cynex: bu basis ilk parmaktan mi geldi? (gercek farede dokunma kaydi yok)
	const int iCount = CynexInput_GetTouchCount();
	const SCynexTouch* pBest = NULL;
	SCynexTouch kBest, kTouch;
	for (int i = 0; i < iCount; ++i)
	{
		if (!CynexInput_GetTouch(i, &kTouch))
			continue;
		LONG rx, ry;
		ToRender(kTouch.downX, kTouch.downY, &rx, &ry);
		if (abs(rx - x) > 2 || abs(ry - y) > 2)
			continue;
		// AKRAVON: parmak fare iletisinden once kalktiysa (iOS: dokunma ana is parcaciginda) dokunus zaten islendi
		// (OnTouchTap / ReplayClick / dunya tiklamasi). Fare iletisi de islenirse ayni dokunus iki kez calisir: ac-kapa.
		if (IsFinished(kTouch.id, kTouch.downTime))
		{
			g_mouseSwallow = true;
			return true;
		}
		STrack* pTrack = FindTrack(kTouch.id, kTouch.downTime);
		if (pTrack && pTrack->primary)
			continue;
		if (!pBest || kTouch.downTime >= kBest.downTime)
		{
			kBest = kTouch;
			pBest = &kBest;
		}
	}
	if (!pBest)
	{
		g_mouseSwallow = false;
		return false;
	}

	CynexTouch_Config();
	STrack* pTrack = FindTrack(kBest.id, kBest.downTime);
	if (!pTrack)
		pTrack = &AddTrack(kBest);
	pTrack->primary = true;
	pTrack->alive = true;
	g_mouseSwallow = pTrack->kind != KIND_NONE;
	return g_mouseSwallow;
}

void CynexTouch_Update()
{
	CynexTouch_Config();

	if (g_pendingClick.active && --g_pendingClick.frames <= 0)
	{
		g_pendingClick.active = false;
		ReplayClick(g_pendingClick.x, g_pendingClick.y);
	}

	for (STrack& t : g_tracks)
		t.alive = false;

	std::vector<std::pair<STrack, bool> > ended;
	const int iCount = CynexInput_GetTouchCount();
	SCynexTouch kTouch;

	// Cynex: listeden dusen bitmis parmaklari unut
	for (size_t i = 0; i < g_finished.size();)
	{
		bool bListed = false;
		for (int j = 0; j < iCount && !bListed; ++j)
			bListed = CynexInput_GetTouch(j, &kTouch) && kTouch.id == g_finished[i].first && kTouch.downTime == g_finished[i].second;
		if (bListed)
			++i;
		else
			g_finished.erase(g_finished.begin() + i);
	}

	for (int i = 0; i < iCount; ++i)
	{
		if (!CynexInput_GetTouch(i, &kTouch))
			continue;
		if (IsFinished(kTouch.id, kTouch.downTime))
			continue;
		STrack* pTrack = FindTrack(kTouch.id, kTouch.downTime);
		if (!pTrack)
			pTrack = &AddTrack(kTouch);

		ToRender(kTouch.x, kTouch.y, &pTrack->rx, &pTrack->ry);
		ToUI(pTrack->rx, pTrack->ry, &pTrack->ux, &pTrack->uy);
		if (!pTrack->moved && (abs(pTrack->rx - pTrack->rx0) > TAP_SLOP || abs(pTrack->ry - pTrack->ry0) > TAP_SLOP))
			pTrack->moved = true;
		pTrack->alive = kTouch.phase < 2;
	}

	// Cynex: kalkan ya da listeden dusen parmaklar
	const uint32_t now = NowMs();
	for (size_t i = 0; i < g_tracks.size();)
	{
		STrack& t = g_tracks[i];
		if (t.alive)
		{
			if (!t.longFired && !t.moved && (t.kind == KIND_TARGET || t.kind == KIND_SLOT) && now - t.downTime >= LONG_PRESS_MS)
				FireLongPress(t);
			++i;
			continue;
		}
		bool bCancelled = false;
		for (int j = 0; j < iCount; ++j)
		{
			if (CynexInput_GetTouch(j, &kTouch) && kTouch.id == t.id && kTouch.downTime == t.downTime)
			{
				bCancelled = kTouch.phase == 3;
				break;
			}
		}
		ended.push_back(std::make_pair(t, bCancelled));
		g_finished.push_back(std::make_pair(t.id, t.downTime));
		g_tracks.erase(g_tracks.begin() + i);
	}

	// Cynex: uzun basis olayi python'u cagirdi; listede kalan isaretciler gecerli
	UpdateCamera();

	for (auto& e : ended)
		FinishTrack(e.first, e.second);
}

int CynexTouch_GetActiveCount()
{
	int n = 0;
	SCynexTouch kTouch;
	const int iCount = CynexInput_GetTouchCount();
	for (int i = 0; i < iCount; ++i)
		if (CynexInput_GetTouch(i, &kTouch) && kTouch.phase < 2)
			++n;
	return n;
}

bool CynexTouch_GetActive(int index, int* piID, int* piX, int* piY, int* piRegion)
{
	SCynexTouch kTouch;
	const int iCount = CynexInput_GetTouchCount();
	for (int i = 0; i < iCount; ++i)
	{
		if (!CynexInput_GetTouch(i, &kTouch) || kTouch.phase >= 2)
			continue;
		if (index-- != 0)
			continue;

		LONG rx, ry, ux, uy;
		ToRender(kTouch.x, kTouch.y, &rx, &ry);
		ToUI(rx, ry, &ux, &uy);
		*piID = kTouch.id;
		*piX = (int)ux;
		*piY = (int)uy;

		const STrack* pTrack = FindTrack(kTouch.id, kTouch.downTime);
		if (pTrack)
			*piRegion = pTrack->region;
		else
		{
			// Cynex: python geri cagrisi sirasinda liste buyumesin - gecici siniflandirma
			STrack t;
			ToRender(kTouch.downX, kTouch.downY, &t.rx0, &t.ry0);
			ToUI(t.rx0, t.ry0, &t.ux0, &t.uy0);
			Classify(t);
			Py_XDECREF(t.target);
			*piRegion = t.region;
		}
		return true;
	}
	return false;
}
