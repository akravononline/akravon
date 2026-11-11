// Cynex: mobil arayuzun bekledigi python fonksiyonlari (app, player, systemSetting, wndMgr)
#include "../../../client/source/UserInterface/StdAfx.h"
#include "../../../client/source/UserInterface/PythonPlayer.h"
#include "../../platform/CynexPlatform.h"
#include "../CynexMobileUI.h"
#include "CynexTouch_mobile.h"

namespace
{
	const float TOUCH_SIZE_MIN = 80.0f;
	const float TOUCH_SIZE_MAX = 560.0f;

	float Clamp(float v, float lo, float hi)
	{
		return v < lo ? lo : (v > hi ? hi : v);
	}

	// Cynex: boy 0 = otomatik, aksi halde sinir icinde
	float ClampTouchSize(float v)
	{
		return v <= 0.0f ? 0.0f : Clamp(v, TOUCH_SIZE_MIN, TOUCH_SIZE_MAX);
	}

	// ------------------------------------------------------------------ app
	PyObject* appIsMobile(PyObject* poSelf, PyObject* poArgs)
	{
		return Py_BuildValue("i", 1);
	}

	PyObject* appGetTouchCount(PyObject* poSelf, PyObject* poArgs)
	{
		return Py_BuildValue("i", CynexTouch_GetActiveCount());
	}

	// Cynex: (kimlik, x, y) arayuz koordinati; yoksa None
	PyObject* appGetTouchPoint(PyObject* poSelf, PyObject* poArgs)
	{
		int iIndex;
		if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
			return Py_BuildException();
		int iID, iX, iY, iRegion;
		if (!CynexTouch_GetActive(iIndex, &iID, &iX, &iY, &iRegion))
			return Py_BuildNone();
		return Py_BuildValue("(iii)", iID, iX, iY);
	}

	// Cynex: (kimlik, x, y, bolge) - bolge: 0 oyun alani, 1 arayuz, 2 dokunmayi kendisi okuyan pencere
	PyObject* appGetTouchPointEx(PyObject* poSelf, PyObject* poArgs)
	{
		int iIndex;
		if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
			return Py_BuildException();
		int iID, iX, iY, iRegion;
		if (!CynexTouch_GetActive(iIndex, &iID, &iX, &iY, &iRegion))
			return Py_BuildNone();
		return Py_BuildValue("(iiii)", iID, iX, iY, iRegion);
	}

	PyObject* appSetIkiDokunusKapisi(PyObject* poSelf, PyObject* poArgs)
	{
		int iEnable = 0;
		PyTuple_GetInteger(poArgs, 0, &iEnable);
		CynexTouch_SetDoubleTapGate(iEnable != 0);
		return Py_BuildNone();
	}

	PyObject* appVibrate(PyObject* poSelf, PyObject* poArgs)
	{
		int iMs = 30;
		PyTuple_GetInteger(poArgs, 0, &iMs);
		if (iMs > 0)
			CynexPlatform_Vibrate(iMs);
		return Py_BuildNone();
	}

	PyObject* appOpenURL(PyObject* poSelf, PyObject* poArgs)
	{
		char* szURL;
		if (!PyTuple_GetString(poArgs, 0, &szURL))
			return Py_BuildException();
		CynexPlatform_OpenURL(szURL);
		return Py_BuildNone();
	}

	// AKRAVON: galeriden gorsel sec: app.PickImage(kind, width, height, dest) ; app.PickImageState() -> 0 bosta, 1 suruyor, 2 tamam, 3 iptal
	PyObject* appPickImage(PyObject* poSelf, PyObject* poArgs)
	{
		int iKind = 0, iWidth = 16, iHeight = 12;
		char* szDest;
		if (!PyTuple_GetInteger(poArgs, 0, &iKind) || !PyTuple_GetInteger(poArgs, 1, &iWidth) || !PyTuple_GetInteger(poArgs, 2, &iHeight) || !PyTuple_GetString(poArgs, 3, &szDest))
			return Py_BuildException();
		CynexPlatform_PickImage(iKind, iWidth, iHeight, szDest);
		return Py_BuildNone();
	}

	PyObject* appPickImageState(PyObject* poSelf, PyObject* poArgs)
	{
		return Py_BuildValue("i", CynexPlatform_PickImageState());
	}

	PyObject* appShowKeyboard(PyObject* poSelf, PyObject* poArgs)
	{
		int iShow = 1;
		PyTuple_GetInteger(poArgs, 0, &iShow);
		CynexPlatform_ShowKeyboard(iShow);
		return Py_BuildNone();
	}

	// Cynex: platformda bildirim kanali yok - bos
	PyObject* appShowNotification(PyObject* poSelf, PyObject* poArgs)
	{
		return Py_BuildNone();
	}

	PyObject* appCancelNotification(PyObject* poSelf, PyObject* poArgs)
	{
		return Py_BuildNone();
	}

	// ------------------------------------------------------------------ player
	PyObject* playerMoveToDirection(PyObject* poSelf, PyObject* poArgs)
	{
		float fRot;
		if (!PyTuple_GetFloat(poArgs, 0, &fRot))
			return Py_BuildException();
		CPythonPlayer::Instance().CynexMoveToDirection(fRot);
		return Py_BuildNone();
	}

	// Cynex: barbaros'ta serbest kamera kipi yok
	PyObject* playerIsCameraMode(PyObject* poSelf, PyObject* poArgs)
	{
		return Py_BuildValue("i", 0);
	}

	// ------------------------------------------------------------------ systemSetting
	PyObject* ReturnFloat(float v) { return Py_BuildValue("f", v); }

	bool ArgFloat(PyObject* poArgs, float* pv)
	{
		if (PyTuple_GetFloat(poArgs, 0, pv))
			return true;
		int i;
		if (PyTuple_GetInteger(poArgs, 0, &i))
		{
			*pv = (float)i;
			return true;
		}
		return false;
	}

#define CYNEX_FLOAT_SETTING(getName, setName, field, lo, hi) \
	PyObject* getName(PyObject* poSelf, PyObject* poArgs) { return ReturnFloat(CynexTouch_Config().field); } \
	PyObject* setName(PyObject* poSelf, PyObject* poArgs) \
	{ \
		float v; \
		if (!ArgFloat(poArgs, &v)) \
			return Py_BuildException(); \
		CynexTouch_Config().field = Clamp(v, lo, hi); \
		CynexTouch_SaveConfig(); \
		return Py_BuildNone(); \
	}

	CYNEX_FLOAT_SETTING(sysGetJoystickDeadZone, sysSetJoystickDeadZone, deadZone, 0.0f, 0.9f)
	PyObject* sysGetCameraReturnPitch(PyObject* poSelf, PyObject* poArgs) { return ReturnFloat(CynexTouch_Config().returnPitch); }

	// Cynex: yeni egim hemen gorunsun
	PyObject* sysSetCameraReturnPitch(PyObject* poSelf, PyObject* poArgs)
	{
		float v;
		if (!ArgFloat(poArgs, &v))
			return Py_BuildException();
		CynexTouch_Config().returnPitch = Clamp(v, -10.0f, 80.0f);
		CynexTouch_SaveConfig();
		CynexTouch_StartCameraReturn();
		return Py_BuildNone();
	}
	CYNEX_FLOAT_SETTING(sysGetCameraDragSens, sysSetCameraDragSens, dragSens, 0.1f, 5.0f)
	CYNEX_FLOAT_SETTING(sysGetPinchZoomSens, sysSetPinchZoomSens, pinchSens, 0.1f, 5.0f)
#undef CYNEX_FLOAT_SETTING

	PyObject* sysGetJoystickSize(PyObject* poSelf, PyObject* poArgs) { return ReturnFloat(CynexTouch_Config().joystickSize); }
	PyObject* sysGetAttackButtonSize(PyObject* poSelf, PyObject* poArgs) { return ReturnFloat(CynexTouch_Config().attackSize); }

	PyObject* sysSetJoystickSize(PyObject* poSelf, PyObject* poArgs)
	{
		float v;
		if (!ArgFloat(poArgs, &v))
			return Py_BuildException();
		CynexTouch_Config().joystickSize = ClampTouchSize(v);
		CynexTouch_SaveConfig();
		return Py_BuildNone();
	}

	PyObject* sysSetAttackButtonSize(PyObject* poSelf, PyObject* poArgs)
	{
		float v;
		if (!ArgFloat(poArgs, &v))
			return Py_BuildException();
		CynexTouch_Config().attackSize = ClampTouchSize(v);
		CynexTouch_SaveConfig();
		return Py_BuildNone();
	}

	PyObject* sysGetTouchSizeLimits(PyObject* poSelf, PyObject* poArgs)
	{
		return Py_BuildValue("(ff)", TOUCH_SIZE_MIN, TOUCH_SIZE_MAX);
	}

	PyObject* sysIsMobileHud(PyObject* poSelf, PyObject* poArgs)
	{
		return Py_BuildValue("i", CynexTouch_Config().mobileHud ? 1 : 0);
	}

	PyObject* sysSetMobileHud(PyObject* poSelf, PyObject* poArgs)
	{
		int iEnable;
		if (!PyTuple_GetInteger(poArgs, 0, &iEnable))
			return Py_BuildException();
		CynexTouch_Config().mobileHud = iEnable ? 1 : 0;
		CynexTouch_SaveConfig();
		return Py_BuildNone();
	}

	PyObject* sysIsCameraReturn(PyObject* poSelf, PyObject* poArgs)
	{
		return Py_BuildValue("i", CynexTouch_Config().cameraReturn ? 1 : 0);
	}

	PyObject* sysSetCameraReturn(PyObject* poSelf, PyObject* poArgs)
	{
		int iEnable;
		if (!PyTuple_GetInteger(poArgs, 0, &iEnable))
			return Py_BuildException();
		CynexTouch_Config().cameraReturn = iEnable ? 1 : 0;
		CynexTouch_SaveConfig();
		return Py_BuildNone();
	}

	PyObject* sysGetPostFilter(PyObject* poSelf, PyObject* poArgs) { return Py_BuildValue("i", CynexTouch_Config().postFilter); }
	PyObject* sysGetViewRange(PyObject* poSelf, PyObject* poArgs) { return Py_BuildValue("i", CynexTouch_Config().viewRange); }

	PyObject* sysSetPostFilter(PyObject* poSelf, PyObject* poArgs)
	{
		int v;
		if (!PyTuple_GetInteger(poArgs, 0, &v))
			return Py_BuildException();
		CynexTouch_Config().postFilter = v < 0 ? 0 : (v > 2 ? 2 : v);
		CynexTouch_SaveConfig();
		return Py_BuildNone();
	}

	PyObject* sysGetTextureQuality(PyObject* poSelf, PyObject* poArgs) { return Py_BuildValue("i", CynexTouch_Config().textureQuality); }
	PyObject* sysGetFpsLimit(PyObject* poSelf, PyObject* poArgs) { return Py_BuildValue("i", CynexTouch_Config().fpsLimit); }

	PyObject* sysSetTextureQuality(PyObject* poSelf, PyObject* poArgs)
	{
		int v;
		if (!PyTuple_GetInteger(poArgs, 0, &v))
			return Py_BuildException();
		CynexTouch_Config().textureQuality = v < 0 ? 0 : (v > 2 ? 2 : v);
		CynexTouch_SaveConfig();
		return Py_BuildNone();
	}

	PyObject* sysSetFpsLimit(PyObject* poSelf, PyObject* poArgs)
	{
		int v;
		if (!PyTuple_GetInteger(poArgs, 0, &v))
			return Py_BuildException();
		CynexTouch_Config().fpsLimit = v <= 30 ? 30 : 60;
		CynexTouch_SaveConfig();
		return Py_BuildNone();
	}

	PyObject* sysGetDisplaySize(PyObject* poSelf, PyObject* poArgs);

	PyObject* sysSetViewRange(PyObject* poSelf, PyObject* poArgs)
	{
		int v;
		if (!PyTuple_GetInteger(poArgs, 0, &v))
			return Py_BuildException();
		CynexTouch_Config().viewRange = v < 0 ? 0 : (v > 2 ? 2 : v);
		CynexTouch_SaveConfig();
		return Py_BuildNone();
	}
}

void CynexGfx_PostFilter(int mode);
void CynexApp_SetSightRange(int iRange);
void CynexGfx_SetTextureQuality(int quality);
void CynexGfx_SetFpsLimit(int fps);
int CynexGfx_DisplayWidth();
const char* CynexGfx_GpuName();
int CynexGfx_DisplayHeight();

namespace
{
	PyObject* sysGetGpuName(PyObject* poSelf, PyObject* poArgs)
	{
		return Py_BuildValue("s", CynexGfx_GpuName());
	}

	PyObject* sysGetSafeInsets(PyObject* poSelf, PyObject* poArgs)
	{
		int v[4];
		CynexPlatform_GetSafeInsets(v);
		return Py_BuildValue("(iiii)", v[0], v[1], v[2], v[3]);
	}

	PyObject* sysGetDisplaySize(PyObject* poSelf, PyObject* poArgs)
	{
		return Py_BuildValue("(ii)", CynexGfx_DisplayWidth(), CynexGfx_DisplayHeight());
	}
}

// AKRAVON: dunya cizildikten sonra, arayuzden once
void CynexGfx_AfterWorld()
{
	static int s_iAppliedRange = -1;
	const SCynexTouchConfig& c = CynexTouch_Config();
	if (c.viewRange != s_iAppliedRange)
	{
		s_iAppliedRange = c.viewRange;
		CynexApp_SetSightRange(c.viewRange == 2 ? 12800 : (c.viewRange == 1 ? 19200 : -1));
	}
	CynexGfx_SetTextureQuality(c.textureQuality);
	CynexGfx_SetFpsLimit(c.fpsLimit);
	CynexGfx_PostFilter(c.postFilter);
}

// Cynex: klavye yon tuslari ile ayni denetimler, yon serbest aci
bool CPythonPlayer::CynexMoveToDirection(float fDirRot)
{
	if (NEW_CancelFishing())
		return false;
	if (!__CanMove())
		return false;
	return NEW_MoveToDirection(fmod(fDirRot + 360.0f, 360.0f));
}

void CynexMobile_InitPythonModules()
{
	static PyMethodDef s_appMethods[] =
	{
		{ "IsMobile",				appIsMobile,				METH_VARARGS },
		{ "GetTouchCount",			appGetTouchCount,			METH_VARARGS },
		{ "GetTouchPoint",			appGetTouchPoint,			METH_VARARGS },
		{ "GetTouchPointEx",		appGetTouchPointEx,			METH_VARARGS },
		{ "SetIkiDokunusKapisi",	appSetIkiDokunusKapisi,		METH_VARARGS },
		{ "Vibrate",				appVibrate,					METH_VARARGS },
		{ "OpenURL",				appOpenURL,					METH_VARARGS },
		{ "ShowKeyboard",			appShowKeyboard,			METH_VARARGS },
		{ "PickImage",				appPickImage,				METH_VARARGS },
		{ "PickImageState",			appPickImageState,			METH_VARARGS },
		{ "ShowNotification",		appShowNotification,		METH_VARARGS },
		{ "CancelNotification",		appCancelNotification,		METH_VARARGS },
		{ NULL, NULL, 0 },
	};
	PyObject* poApp = Py_InitModule("app", s_appMethods);
	PyModule_AddIntConstant(poApp, "IS_MOBILE", 1);
	PyModule_AddIntConstant(poApp, "TOUCH_API_VERSION", 2);

	static PyMethodDef s_playerMethods[] =
	{
		{ "MoveToDirection",		playerMoveToDirection,		METH_VARARGS },
		{ "IsCameraMode",			playerIsCameraMode,			METH_VARARGS },
		{ NULL, NULL, 0 },
	};
	Py_InitModule("player", s_playerMethods);

	static PyMethodDef s_systemMethods[] =
	{
		{ "IsMobileHud",			sysIsMobileHud,				METH_VARARGS },
		{ "SetMobileHud",			sysSetMobileHud,			METH_VARARGS },
		{ "GetJoystickDeadZone",	sysGetJoystickDeadZone,		METH_VARARGS },
		{ "SetJoystickDeadZone",	sysSetJoystickDeadZone,		METH_VARARGS },
		{ "GetJoystickSize",		sysGetJoystickSize,			METH_VARARGS },
		{ "SetJoystickSize",		sysSetJoystickSize,			METH_VARARGS },
		{ "GetAttackButtonSize",	sysGetAttackButtonSize,		METH_VARARGS },
		{ "SetAttackButtonSize",	sysSetAttackButtonSize,		METH_VARARGS },
		{ "GetTouchSizeLimits",		sysGetTouchSizeLimits,		METH_VARARGS },
		{ "GetCameraReturnPitch",	sysGetCameraReturnPitch,	METH_VARARGS },
		{ "SetCameraReturnPitch",	sysSetCameraReturnPitch,	METH_VARARGS },
		{ "IsCameraReturn",			sysIsCameraReturn,			METH_VARARGS },
		{ "SetCameraReturn",		sysSetCameraReturn,			METH_VARARGS },
		{ "GetCameraDragSens",		sysGetCameraDragSens,		METH_VARARGS },
		{ "SetCameraDragSens",		sysSetCameraDragSens,		METH_VARARGS },
		{ "GetPinchZoomSens",		sysGetPinchZoomSens,		METH_VARARGS },
		{ "SetPinchZoomSens",		sysSetPinchZoomSens,		METH_VARARGS },
		{ "GetPostFilter",			sysGetPostFilter,			METH_VARARGS },
		{ "SetPostFilter",			sysSetPostFilter,			METH_VARARGS },
		{ "GetViewRange",			sysGetViewRange,			METH_VARARGS },
		{ "SetViewRange",			sysSetViewRange,			METH_VARARGS },
		{ "GetTextureQuality",		sysGetTextureQuality,		METH_VARARGS },
		{ "SetTextureQuality",		sysSetTextureQuality,		METH_VARARGS },
		{ "GetFpsLimit",			sysGetFpsLimit,				METH_VARARGS },
		{ "SetFpsLimit",			sysSetFpsLimit,				METH_VARARGS },
		{ "GetDisplaySize",			sysGetDisplaySize,			METH_VARARGS },
		{ "GetSafeInsets",			sysGetSafeInsets,			METH_VARARGS },
		{ "GetGpuName",				sysGetGpuName,				METH_VARARGS },
		{ NULL, NULL, 0 },
	};
	Py_InitModule("systemSetting", s_systemMethods);

	CynexWndMgr_InitMobile();
}
