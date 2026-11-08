#include "StdAfx.h"
#include "Resource.h"
#include "PythonApplication.h"
#include "../EterLib/Camera.h"
#include "Locale.h"
#include "../GameLib/GameLibDefines.h"

extern bool PERF_CHECKER_RENDER_GAME;
extern D3DXCOLOR g_fSpecularColor;
extern BOOL bVisibleNotice = true;
extern BOOL bTestServerFlag = FALSE;
extern int TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE = 0;

PyObject* appShowWebPage(PyObject* poSelf, PyObject* poArgs)
{
	char* szWebPage;
	if (!PyTuple_GetString(poArgs, 0, &szWebPage))
		return Py_BuildException();

	PyObject* poRect=PyTuple_GetItem(poArgs, 1);
	if (!PyTuple_Check(poRect))
		return Py_BuildException();

	RECT rcWebPage;
	rcWebPage.left=PyInt_AsLong(PyTuple_GetItem(poRect, 0));
	rcWebPage.top=PyInt_AsLong(PyTuple_GetItem(poRect, 1));
	rcWebPage.right=PyInt_AsLong(PyTuple_GetItem(poRect, 2));
	rcWebPage.bottom=PyInt_AsLong(PyTuple_GetItem(poRect, 3));

	CPythonApplication::Instance().ShowWebPage(
		szWebPage,
		rcWebPage
	);
	return Py_BuildNone();

}

PyObject* appMoveWebPage(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poRect=PyTuple_GetItem(poArgs, 0);
	if (!PyTuple_Check(poRect))
		return Py_BuildException();

	RECT rcWebPage;
	rcWebPage.left=PyInt_AsLong(PyTuple_GetItem(poRect, 0));
	rcWebPage.top=PyInt_AsLong(PyTuple_GetItem(poRect, 1));
	rcWebPage.right=PyInt_AsLong(PyTuple_GetItem(poRect, 2));
	rcWebPage.bottom=PyInt_AsLong(PyTuple_GetItem(poRect, 3));

	CPythonApplication::Instance().MoveWebPage(rcWebPage);
	return Py_BuildNone();
}

PyObject* appHideWebPage(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().HideWebPage();
	return Py_BuildNone();
}


PyObject * appIsWebPageMode(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().IsWebPageMode());
}

PyObject* appEnablePerformanceTime(PyObject* poSelf, PyObject* poArgs)
{
	char* szMode;
	if (!PyTuple_GetString(poArgs, 0, &szMode))
		return Py_BuildException();

	int nEnable;
	if (!PyTuple_GetInteger(poArgs, 1, &nEnable))
		return Py_BuildException();

	bool isEnable=nEnable ? true : false;

	if (strcmp(szMode, "RENDER_GAME")==0)
		PERF_CHECKER_RENDER_GAME = isEnable;
	
	return Py_BuildNone();
}

/////////////////////////////////////////////////////

extern BOOL HAIR_COLOR_ENABLE;
extern BOOL USE_ARMOR_SPECULAR;
extern BOOL USE_WEAPON_SPECULAR;
extern BOOL SKILL_EFFECT_UPGRADE_ENABLE;
extern BOOL RIDE_HORSE_ENABLE;
extern double g_specularSpd;

// TEXTTAIL_LIVINGTIME_CONTROL
extern void TextTail_SetLivingTime(LONG livingTime);

PyObject* appSetTextTailLivingTime(PyObject* poSelf, PyObject* poArgs)
{
	float livingTime;
	if (!PyTuple_GetFloat(poArgs, 0, &livingTime))
		return Py_BuildException();

	TextTail_SetLivingTime(livingTime*1000);

	return Py_BuildNone();
}
// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

PyObject* appSetHairColorEnable(PyObject* poSelf, PyObject* poArgs)
{
	int nEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
		return Py_BuildException();

	HAIR_COLOR_ENABLE=nEnable;

	return Py_BuildNone();
}

PyObject* appSetArmorSpecularEnable(PyObject* poSelf, PyObject* poArgs)
{
	int nEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
		return Py_BuildException();

	USE_ARMOR_SPECULAR=nEnable;

	return Py_BuildNone();
}

PyObject* appSetWeaponSpecularEnable(PyObject* poSelf, PyObject* poArgs)
{
	int nEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
		return Py_BuildException();

	USE_WEAPON_SPECULAR=nEnable;

	return Py_BuildNone();
}

PyObject* appSetSkillEffectUpgradeEnable(PyObject* poSelf, PyObject* poArgs)
{
	int nEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
		return Py_BuildException();

	SKILL_EFFECT_UPGRADE_ENABLE=nEnable;

	return Py_BuildNone();
}

PyObject* SetTwoHandedWeaponAttSpeedDecreaseValue(PyObject* poSelf, PyObject* poArgs)
{
	int iValue;
	if (!PyTuple_GetInteger(poArgs, 0, &iValue))
		return Py_BuildException();

	TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE = iValue;

	return Py_BuildNone();
}

PyObject* appSetRideHorseEnable(PyObject* poSelf, PyObject* poArgs)
{
	int nEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
		return Py_BuildException();

	RIDE_HORSE_ENABLE=nEnable;

	return Py_BuildNone();
}

PyObject* appSetCameraMaxDistance(PyObject* poSelf, PyObject* poArgs)
{
	float fMax;
	if (!PyTuple_GetFloat(poArgs, 0, &fMax))
		return Py_BuildException();

	CCamera::SetCameraMaxDistance(fMax);
	return Py_BuildNone();
}

PyObject* appSetControlFP(PyObject* poSelf, PyObject* poArgs)
{
	_controlfp( _PC_24, _MCW_PC );
	return Py_BuildNone();
}

PyObject* appSetSpecularSpeed(PyObject* poSelf, PyObject* poArgs)
{
	float fSpeed;
	if (!PyTuple_GetFloat(poArgs, 0, &fSpeed))
		return Py_BuildException();

	g_specularSpd = fSpeed;

	return Py_BuildNone();
}

PyObject * appSetMinFog(PyObject * poSelf, PyObject * poArgs)
{
	float fMinFog;
	if (!PyTuple_GetFloat(poArgs, 0, &fMinFog))
		return Py_BuildException();

	CPythonApplication::Instance().SetMinFog(fMinFog);
	return Py_BuildNone();
}

PyObject* appSetFrameSkip(PyObject* poSelf, PyObject* poArgs)
{
	int nFrameSkip;
	if (!PyTuple_GetInteger(poArgs, 0, &nFrameSkip))
		return Py_BuildException();

	CPythonApplication::Instance().SetFrameSkip(nFrameSkip ? true : false);
	return Py_BuildNone();
}

// LOCALE

PyObject* appForceSetLocale(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	char* szLocalePath;
	if (!PyTuple_GetString(poArgs, 1, &szLocalePath))
		return Py_BuildException();

	LocaleService_ForceSetLocale(szName, szLocalePath);

	return Py_BuildNone();
}

PyObject* appGetLocaleServiceName(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("s", LocaleService_GetName());
}

//
bool LoadLocaleData(const char* localePath);

PyObject* appSetCHEONMA(PyObject* poSelf, PyObject* poArgs)
{
	int enable;
	if (!PyTuple_GetInteger(poArgs, 0, &enable))
		return Py_BuildException();

	LocaleService_SetCHEONMA(enable ? true : false);
	return Py_BuildNone();
}

PyObject* appIsCHEONMA(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", LocaleService_IsCHEONMA());
}

#include "../eterBase/tea.h"

PyObject* appLoadLocaleAddr(PyObject* poSelf, PyObject* poArgs)
{
	char* addrPath;
	if (!PyTuple_GetString(poArgs, 0, &addrPath))
		return Py_BuildException();

	FILE* fp = fopen(addrPath, "rb");
	if (!fp)
		return Py_BuildException();

	fseek(fp, 0, SEEK_END);

	int size = ftell(fp);
	char* enc = (char*)_alloca(size);
	fseek(fp, 0, SEEK_SET);
	fread(enc, size, 1, fp);
	fclose(fp);

	static const unsigned char key[16] = {
		0x82, 0x1b, 0x34, 0xae,
		0x12, 0x3b, 0xfb, 0x17,
		0xd7, 0x2c, 0x39, 0xae,
		0x41, 0x98, 0xf1, 0x63
	};

	char* buf = (char*)_alloca(size);
	//int decSize =
	tea_decrypt((DWORD*)buf, (const DWORD*)enc, (const DWORD*)key, size);
	unsigned int retSize = *(unsigned int*)buf;
	char* ret = buf + sizeof(unsigned int);
	return Py_BuildValue("s#", ret, retSize);
}

PyObject* appLoadLocaleData(PyObject* poSelf, PyObject* poArgs)
{
	char* localePath;
	if (!PyTuple_GetString(poArgs, 0, &localePath))
		return Py_BuildException();

	return Py_BuildValue("i", LoadLocaleData(localePath));
}

PyObject* appGetLocaleName(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("s", LocaleService_GetLocaleName());
}

PyObject* appGetLocalePath(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("s", LocaleService_GetLocalePath());
}
// END_OF_LOCALE

PyObject* appGetDefaultCodePage(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", LocaleService_GetCodePage());
}

#include <il/il.h>

PyObject* appGetImageInfo(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	BOOL canLoad=FALSE;
	ILuint uWidth=0;
	ILuint uHeight=0;

	ILuint uImg;
	ilGenImages(1, &uImg);
	ilBindImage(uImg);
	if (ilLoad(IL_TYPE_UNKNOWN, szFileName))
	{
		canLoad=TRUE;
		uWidth=ilGetInteger(IL_IMAGE_WIDTH);
		uHeight=ilGetInteger(IL_IMAGE_HEIGHT);
	}

	ilDeleteImages(1, &uImg);

	return Py_BuildValue("iii", canLoad, uWidth, uHeight);
}

#include "../EterPack/EterPackManager.h"

PyObject* appIsExistFile(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	bool isExist=CEterPackManager::Instance().isExist(szFileName);

	return Py_BuildValue("i", isExist);
}

PyObject* appGetFileList(PyObject* poSelf, PyObject* poArgs)
{
	char* szFilter;
	if (!PyTuple_GetString(poArgs, 0, &szFilter))
		return Py_BuildException();

	PyObject* poList=PyList_New(0);

	WIN32_FIND_DATA wfd;
	memset(&wfd, 0, sizeof(wfd));

	HANDLE hFind = FindFirstFile(szFilter, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			PyObject* poFileName=PyString_FromString(wfd.cFileName) ;
			PyList_Append(poList, poFileName);
		}
		while (FindNextFile(hFind, &wfd));


		FindClose(hFind);
	}

	return poList;
}


PyObject* appUpdateGame(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().UpdateGame();
	return Py_BuildNone();
}

PyObject* appRenderGame(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().RenderGame();
	return Py_BuildNone();
}



PyObject* appSetMouseHandler(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poHandler;
	if (!PyTuple_GetObject(poArgs, 0, &poHandler))
		return Py_BuildException();

	CPythonApplication::Instance().SetMouseHandler(poHandler);
	return Py_BuildNone();
}

PyObject* appCreate(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	#ifdef ENABLE_DISCORD_STUFF
	char* szMiniName;
	#endif

	PyTuple_GetString(poArgs, 0, &szName);
	int width, height, Windowed;

	#ifdef ENABLE_DISCORD_STUFF
	PyTuple_GetString(poArgs, 1, &szMiniName);
	PyTuple_GetInteger(poArgs, 2, &width);
	PyTuple_GetInteger(poArgs, 3, &height);
	PyTuple_GetInteger(poArgs, 4, &Windowed);
	#else
	PyTuple_GetInteger(poArgs, 1, &width);
	PyTuple_GetInteger(poArgs, 2, &height);
	PyTuple_GetInteger(poArgs, 3, &Windowed);
	#endif

#ifdef __APPLE__
	fprintf(stderr, "[T] appCreate basladi\n"); fflush(stderr);
#endif
	#ifdef ENABLE_DISCORD_STUFF
	if (!CPythonApplication::Instance().Create(poSelf, szName, szMiniName, width, height, Windowed))
	#else
	if (!CPythonApplication::Instance().Create(poSelf, szName, width, height, Windowed))
	#endif
	{
#ifdef __APPLE__
		fprintf(stderr, "[T] appCreate BASARISIZ (Python hatasi firlatiliyor)\n"); fflush(stderr);
#endif
		return NULL;
	}

#ifdef __APPLE__
	fprintf(stderr, "[T] appCreate tamam\n"); fflush(stderr);
#endif
	return Py_BuildNone();
}

PyObject* appLoop(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().Loop();

	return Py_BuildNone();
}

PyObject* appGetInfo(PyObject* poSelf, PyObject* poArgs)
{
	int nInfo;
	if (!PyTuple_GetInteger(poArgs, 0, &nInfo))
		return Py_BuildException();

	std::string stInfo;
	CPythonApplication::Instance().GetInfo(nInfo, &stInfo);
	return Py_BuildValue("s", stInfo.c_str());
}

PyObject* appProcess(PyObject* poSelf, PyObject* poArgs)
{
	if (CPythonApplication::Instance().Process())
		return Py_BuildValue("i", 1);

	return Py_BuildValue("i", 0);
}

PyObject* appAbort(PyObject* poSelf, PyObject* poArgs)
{
#ifdef __APPLE__
	fprintf(stderr, "[T] appAbort cagrildi\n"); fflush(stderr);
#endif
	CPythonApplication::Instance().Abort();
	return Py_BuildNone();
}

PyObject* appExit(PyObject* poSelf, PyObject* poArgs)
{
#ifdef __APPLE__
	fprintf(stderr, "[T] appExit cagrildi\n"); fflush(stderr);
#endif
	CPythonApplication::Instance().Exit();
	return Py_BuildNone();
}

PyObject * appSetCamera(PyObject * poSelf, PyObject * poArgs)
{
	float Distance;
	if (!PyTuple_GetFloat(poArgs, 0, &Distance))
		return Py_BuildException();

	float Pitch;
	if (!PyTuple_GetFloat(poArgs, 1, &Pitch))
		return Py_BuildException();

	float Rotation;
	if (!PyTuple_GetFloat(poArgs, 2, &Rotation))
		return Py_BuildException();

	float fDestinationHeight;
	if (!PyTuple_GetFloat(poArgs, 3, &fDestinationHeight))
		return Py_BuildException();

	CPythonApplication::Instance().SetCamera(Distance, Pitch, Rotation, fDestinationHeight);
	return Py_BuildNone();
}

PyObject * appGetCamera(PyObject * poSelf, PyObject * poArgs)
{
	float Distance, Pitch, Rotation, DestinationHeight;
    CPythonApplication::Instance().GetCamera(&Distance, &Pitch, &Rotation, &DestinationHeight);

	return Py_BuildValue("ffff", Distance, Pitch, Rotation, DestinationHeight);
}

PyObject * appGetCameraPitch(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("f", CPythonApplication::Instance().GetPitch());
}

PyObject * appGetCameraRotation(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("f", CPythonApplication::Instance().GetRotation());
}

PyObject * appGetTime(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("f", CPythonApplication::Instance().GetGlobalTime());
}

PyObject * appGetGlobalTime(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetServerTime());
}

PyObject * appGetGlobalTimeStamp(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetServerTimeStamp());
}

PyObject * appGetUpdateFPS(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetUpdateFPS());
}

PyObject * appGetRenderFPS(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetRenderFPS());
}

PyObject * appRotateCamera(PyObject * poSelf, PyObject * poArgs)
{
	int iDirection;
	if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
		return Py_BuildException();
	CPythonApplication::Instance().RotateCamera(iDirection);
	return Py_BuildNone();
}

PyObject * appPitchCamera(PyObject * poSelf, PyObject * poArgs)
{
	int iDirection;
	if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
		return Py_BuildException();
	CPythonApplication::Instance().PitchCamera(iDirection);
	return Py_BuildNone();
}

PyObject * appZoomCamera(PyObject * poSelf, PyObject * poArgs)
{
	int iDirection;
	if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
		return Py_BuildException();
	CPythonApplication::Instance().ZoomCamera(iDirection);
	return Py_BuildNone();
}

PyObject * appMovieRotateCamera(PyObject * poSelf, PyObject * poArgs)
{
	int iDirection;
	if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
		return Py_BuildException();
	CPythonApplication::Instance().MovieRotateCamera(iDirection);
	return Py_BuildNone();
}

PyObject * appMoviePitchCamera(PyObject * poSelf, PyObject * poArgs)
{
	int iDirection;
	if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
		return Py_BuildException();
	CPythonApplication::Instance().MoviePitchCamera(iDirection);
	return Py_BuildNone();
}

PyObject * appMovieZoomCamera(PyObject * poSelf, PyObject * poArgs)
{
	int iDirection;
	if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
		return Py_BuildException();
	CPythonApplication::Instance().MovieZoomCamera(iDirection);
	return Py_BuildNone();
}

PyObject * appMovieResetCamera(PyObject * poSelf, PyObject * poArgs)
{
	CPythonApplication::Instance().MovieResetCamera();
	return Py_BuildNone();
}

PyObject * appGetFaceSpeed(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("f", CPythonApplication::Instance().GetFaceSpeed());
}

PyObject * appGetRenderTime(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("fi",
		CPythonApplication::Instance().GetAveRenderTime(),
		CPythonApplication::Instance().GetCurRenderTime());
}

PyObject * appGetUpdateTime(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetCurUpdateTime());
}

PyObject * appGetLoad(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetLoad());
}
PyObject * appGetFaceCount(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetFaceCount());
}

PyObject * appGetAvaiableTextureMememory(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CGraphicBase::GetAvailableTextureMemory());
}

PyObject * appSetFPS(PyObject * poSelf, PyObject * poArgs)
{
	int	iFPS;
	if (!PyTuple_GetInteger(poArgs, 0, &iFPS))
		return Py_BuildException();

	CPythonApplication::Instance().SetFPS(iFPS);

	return Py_BuildNone();
}

PyObject * appSetGlobalCenterPosition(PyObject * poSelf, PyObject * poArgs)
{
	int x;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();

	int y;
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();

	CPythonApplication::Instance().SetGlobalCenterPosition(x, y);
	return Py_BuildNone();
}


PyObject * appSetCenterPosition(PyObject * poSelf, PyObject * poArgs)
{
	float fx;
	if (!PyTuple_GetFloat(poArgs, 0, &fx))
		return Py_BuildException();

	float fy;
	if (!PyTuple_GetFloat(poArgs, 1, &fy))
		return Py_BuildException();

	float fz;
	if (!PyTuple_GetFloat(poArgs, 2, &fz))
		return Py_BuildException();

	CPythonApplication::Instance().SetCenterPosition(fx, -fy, fz);
	return Py_BuildNone();
}

PyObject * appGetCursorPosition(PyObject * poSelf, PyObject * poArgs)
{
	LONG lx, ly;
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.GetMousePosition(lx, ly);

	return Py_BuildValue("ii", lx, ly);
}

PyObject * appRunPythonFile(PyObject * poSelf, PyObject * poArgs)
{
	char *szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	bool ret = CPythonLauncher::Instance().RunFile(szFileName);
	return Py_BuildValue("i", ret);
}

PyObject * appIsPressed(PyObject * poSelf, PyObject * poArgs)
{
	int iKey;
	if (!PyTuple_GetInteger(poArgs, 0, &iKey))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonApplication::Instance().IsPressed(iKey));
}

PyObject * appSetCursor(PyObject * poSelf, PyObject * poArgs)
{
/*
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	if (!CPythonApplication::Instance().SetHardwareCursor(szName))
		return Py_BuildException("Wrong Cursor Name [%s]", szName);
*/
	int iCursorNum;
	if (!PyTuple_GetInteger(poArgs, 0, &iCursorNum))
		return Py_BuildException();

	if (!CPythonApplication::Instance().SetCursorNum(iCursorNum))
		return Py_BuildException("Wrong Cursor Name [%d]", iCursorNum);

	return Py_BuildNone();
}

PyObject * appGetCursor(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetCursorNum());
}

PyObject * appShowCursor(PyObject * poSelf, PyObject * poArgs)
{
	CPythonApplication::Instance().SetCursorVisible(TRUE);

	return Py_BuildNone();
}

PyObject * appHideCursor(PyObject * poSelf, PyObject * poArgs)
{
	CPythonApplication::Instance().SetCursorVisible(FALSE);

	return Py_BuildNone();
}

PyObject * appIsShowCursor(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", TRUE == CPythonApplication::Instance().GetCursorVisible());
}

PyObject * appIsLiarCursorOn(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", TRUE == CPythonApplication::Instance().GetLiarCursorOn());
}

PyObject * appSetSoftwareCursor(PyObject * poSelf, PyObject * poArgs)
{
	CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_SOFTWARE);
	return Py_BuildNone();
}

PyObject * appSetHardwareCursor(PyObject * poSelf, PyObject * poArgs)
{
	CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_HARDWARE);
	return Py_BuildNone();
}

PyObject * appSetConnectData(PyObject * poSelf, PyObject * poArgs)
{
	char * szIP;
	if (!PyTuple_GetString(poArgs, 0, &szIP))
		return Py_BuildException();

	int	iPort;
	if (!PyTuple_GetInteger(poArgs, 1, &iPort))
		return Py_BuildException();

	CPythonApplication::Instance().SetConnectData(szIP, iPort);

	return Py_BuildNone();
}

PyObject * appGetConnectData(PyObject * poSelf, PyObject * poArgs)
{
	std::string strIP;
	int iPort;

	CPythonApplication::Instance().GetConnectData(strIP, iPort);

	return Py_BuildValue("si", strIP.c_str(), iPort);
}

PyObject * appGetRandom(PyObject * poSelf, PyObject * poArgs)
{
	int from;
	if (!PyTuple_GetInteger(poArgs, 0, &from))
		return Py_BuildException();

	int	to;
	if (!PyTuple_GetInteger(poArgs, 1, &to))
		return Py_BuildException();

	if (from > to)
	{
		int tmp = from;
		from = to;
		to = tmp;
	}

	return Py_BuildValue("i", random_range(from, to));
}

PyObject * appGetRotatingDirection(PyObject * poSelf, PyObject * poArgs)
{
	float fSource;
	if (!PyTuple_GetFloat(poArgs, 0, &fSource))
		return Py_BuildException();
	float fTarget;
	if (!PyTuple_GetFloat(poArgs, 1, &fTarget))
		return Py_BuildException();

	return Py_BuildValue("i", GetRotatingDirection(fSource, fTarget));
}

PyObject * appGetDegreeDifference(PyObject * poSelf, PyObject * poArgs)
{
	float fSource;
	if (!PyTuple_GetFloat(poArgs, 0, &fSource))
		return Py_BuildException();
	float fTarget;
	if (!PyTuple_GetFloat(poArgs, 1, &fTarget))
		return Py_BuildException();

	return Py_BuildValue("f", GetDegreeDifference(fSource, fTarget));
}

PyObject * appSleep(PyObject * poSelf, PyObject * poArgs)
{
	int	iTime;
	if (!PyTuple_GetInteger(poArgs, 0, &iTime))
		return Py_BuildException();

	Sleep(iTime);

	return Py_BuildNone();
}

PyObject * appSetDefaultFontName(PyObject * poSelf, PyObject * poArgs)
{
	char * szFontName;
	if (!PyTuple_GetString(poArgs, 0, &szFontName))
		return Py_BuildException();

	// DEFAULT_FONT
	DefaultFont_SetName(szFontName);
	// END_OF_DEFAULT_FONT

	return Py_BuildNone();
}

PyObject * appSetGuildSymbolPath(PyObject * poSelf, PyObject * poArgs)
{
	char * szPathName;
	if (!PyTuple_GetString(poArgs, 0, &szPathName))
		return Py_BuildException();

	SetGuildSymbolPath(szPathName);

	return Py_BuildNone();
}

PyObject * appEnableSpecialCameraMode(PyObject * poSelf, PyObject * poArgs)
{
	CPythonApplication::Instance().EnableSpecialCameraMode();
	return Py_BuildNone();
}

PyObject * appSetCameraSpeed(PyObject * poSelf, PyObject * poArgs)
{
	int iPercentage;
	if (!PyTuple_GetInteger(poArgs, 0, &iPercentage))
		return Py_BuildException();

	CPythonApplication::Instance().SetCameraSpeed(iPercentage);

	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
	if (pCamera)
		pCamera->SetResistance(float(iPercentage) / 100.0f);
	return Py_BuildNone();
}

PyObject * appIsFileExist(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	return Py_BuildValue("i", -1 != _access(szFileName, 0));
}

PyObject * appSetCameraSetting(PyObject * poSelf, PyObject * poArgs)
{
	int ix;
	if (!PyTuple_GetInteger(poArgs, 0, &ix))
		return Py_BuildException();
	int iy;
	if (!PyTuple_GetInteger(poArgs, 1, &iy))
		return Py_BuildException();
	int iz;
	if (!PyTuple_GetInteger(poArgs, 2, &iz))
		return Py_BuildException();

	int iZoom;
	if (!PyTuple_GetInteger(poArgs, 3, &iZoom))
		return Py_BuildException();
	int iRotation;
	if (!PyTuple_GetInteger(poArgs, 4, &iRotation))
		return Py_BuildException();
	int iPitch;
	if (!PyTuple_GetInteger(poArgs, 5, &iPitch))
		return Py_BuildException();

	CPythonApplication::SCameraSetting CameraSetting;
	ZeroMemory(&CameraSetting, sizeof(CameraSetting));
	CameraSetting.v3CenterPosition.x = float(ix);
	CameraSetting.v3CenterPosition.y = float(iy);
	CameraSetting.v3CenterPosition.z = float(iz);
	CameraSetting.fZoom = float(iZoom);
	CameraSetting.fRotation = float(iRotation);
	CameraSetting.fPitch = float(iPitch);
	CPythonApplication::Instance().SetEventCamera(CameraSetting);
	return Py_BuildNone();
}

PyObject * appSaveCameraSetting(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CPythonApplication::Instance().SaveCameraSetting(szFileName);
	return Py_BuildNone();
}

PyObject * appLoadCameraSetting(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	bool bResult = CPythonApplication::Instance().LoadCameraSetting(szFileName);
	return Py_BuildValue("i", bResult);
}

PyObject * appSetDefaultCamera(PyObject * poSelf, PyObject * poArgs)
{
	CPythonApplication::Instance().SetDefaultCamera();
	return Py_BuildNone();
}

PyObject * appSetSightRange(PyObject * poSelf, PyObject * poArgs)
{
	int iRange;
	if (!PyTuple_GetInteger(poArgs, 0, &iRange))
		return Py_BuildException();

	CPythonApplication::Instance().SetForceSightRange(iRange);
	return Py_BuildNone();
}

extern int g_iAccumulationTime;

PyObject * apptestGetAccumulationTime(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", g_iAccumulationTime);
}

PyObject * apptestResetAccumulationTime(PyObject * poSelf, PyObject * poArgs)
{
	g_iAccumulationTime = 0;
	return Py_BuildNone();
}

PyObject * apptestSetSpecularColor(PyObject * poSelf, PyObject * poArgs)
{
	float fr;
	if (!PyTuple_GetFloat(poArgs, 0, &fr))
		return Py_BuildException();
	float fg;
	if (!PyTuple_GetFloat(poArgs, 1, &fg))
		return Py_BuildException();
	float fb;
	if (!PyTuple_GetFloat(poArgs, 2, &fb))
		return Py_BuildException();
	g_fSpecularColor = D3DXCOLOR(fr, fg, fb, 1.0f);
	return Py_BuildNone();
}

PyObject * appSetVisibleNotice(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();
	bVisibleNotice = iFlag;
	return Py_BuildNone();
}

PyObject * appIsVisibleNotice(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", bVisibleNotice);
}

PyObject * appEnableTestServerFlag(PyObject * poSelf, PyObject * poArgs)
{
	bTestServerFlag = TRUE;
	return Py_BuildNone();
}

PyObject * appIsEnableTestServerFlag(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", bTestServerFlag);
}

class CTextLineLoader
{
	public:
		CTextLineLoader(const char * c_szFileName)
		{
			const VOID* pvData;
			CMappedFile kFile;
			if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
				return;

			m_kTextFileLoader.Bind(kFile.Size(), pvData);
		}

		DWORD GetLineCount()
		{
			return m_kTextFileLoader.GetLineCount();
		}

		const char * GetLine(DWORD dwIndex)
		{
			if (dwIndex >= GetLineCount())
				return "";

			return m_kTextFileLoader.GetLineString(dwIndex).c_str();
		}

	protected:
		CMemoryTextFileLoader m_kTextFileLoader;
};

PyObject * appOpenTextFile(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CTextLineLoader * pTextLineLoader = new CTextLineLoader(szFileName);

	return Py_BuildValue("i", pTextLineLoader);	// Cynex: 64bit tutamac (Py_BuildValue sarmalayicisi)
}

PyObject * appCloseTextFile(PyObject * poSelf, PyObject * poArgs)
{
	long long iHandle;
	if (!PyTuple_GetLongLong(poArgs, 0, &iHandle))
		return Py_BuildException();

	CTextLineLoader * pTextFileLoader = (CTextLineLoader *)iHandle;
	delete pTextFileLoader;

	return Py_BuildNone();
}

PyObject * appGetTextFileLineCount(PyObject * poSelf, PyObject * poArgs)
{
	long long iHandle;
	if (!PyTuple_GetLongLong(poArgs, 0, &iHandle))
		return Py_BuildException();

	CTextLineLoader * pTextFileLoader = (CTextLineLoader *)iHandle;
	return Py_BuildValue("i", pTextFileLoader->GetLineCount());
}

PyObject * appGetTextFileLine(PyObject * poSelf, PyObject * poArgs)
{
	long long iHandle;
	if (!PyTuple_GetLongLong(poArgs, 0, &iHandle))
		return Py_BuildException();
	int iLineIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iLineIndex))
		return Py_BuildException();

	CTextLineLoader * pTextFileLoader = (CTextLineLoader *)iHandle;
	return Py_BuildValue("s", pTextFileLoader->GetLine(iLineIndex));
}

PyObject * appSetGuildMarkPath(PyObject * poSelf, PyObject * poArgs)
{
	char * path;
	if (!PyTuple_GetString(poArgs, 0, &path))
		return Py_BuildException();

    char newPath[256];
    char * ext = strstr(path, ".tga");

    if (ext)
    {
		int extPos = ext - path;
        strncpy(newPath, path, extPos);
        newPath[extPos] = '\0';
    }
    else
        strncpy(newPath, path, sizeof(newPath)-1);

	CGuildMarkManager::Instance().SetMarkPathPrefix(newPath);
	return Py_BuildNone();
}

PyObject* appIsDevStage(PyObject* poSelf, PyObject* poArgs)
{
	int nIsDevelopmentStage = 0;
#if defined(LOCALE_SERVICE_STAGE_DEVELOPMENT)
	nIsDevelopmentStage = 1;
#endif
	return Py_BuildValue("i", nIsDevelopmentStage);
}

PyObject* appIsTestStage(PyObject* poSelf, PyObject* poArgs)
{
	int nIsTestStage = 0;
#if defined(LOCALE_SERVICE_STAGE_TEST)
	nIsTestStage = 1;
#endif
	return Py_BuildValue("i", nIsTestStage);
}

PyObject* appIsLiveStage(PyObject* poSelf, PyObject* poArgs)
{
	int nIsLiveStage = 0;
#if !defined(LOCALE_SERVICE_STAGE_TEST) && !defined(LOCALE_SERVICE_STAGE_DEVELOPMENT)
	nIsLiveStage = 1;
#endif
	return Py_BuildValue("i", nIsLiveStage);
}

PyObject* appLogoOpen(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	int nIsSuccess = 1; //CPythonApplication::Instance().OnLogoOpen(szName);
	CMovieMan::Instance().PlayLogo(szName);

	return Py_BuildValue("i", nIsSuccess);
}

PyObject* appLogoUpdate(PyObject* poSelf, PyObject* poArgs)
{
	int nIsRun = 0; //CPythonApplication::Instance().OnLogoUpdate();
	return Py_BuildValue("i", nIsRun);
}

PyObject* appLogoRender(PyObject* poSelf, PyObject* poArgs)
{
	//CPythonApplication::Instance().OnLogoRender();
	return Py_BuildNone();
}

PyObject* appLogoClose(PyObject* poSelf, PyObject* poArgs)
{
	//CPythonApplication::Instance().OnLogoClose();
	return Py_BuildNone();
}

#ifdef ENABLE_RENDER_TARGET
PyObject* appIsCanOpenRender(PyObject* poSelf, PyObject* poArgs)
{
	if(CPythonApplication::Instance().IsPressed(DIK_LSHIFT) && CPythonApplication::Instance().IsPressed(DIK_X))
		return Py_BuildValue("i", 1);
	return Py_BuildValue("i", 0);
}
#endif

#ifdef ENABLE_FLASH_APPLICATION
PyObject* appFlashApplication(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().FlashApplication();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
PyObject* appSetMultiFarmExeIcon(PyObject* poSelf, PyObject* poArgs)
{
	int iStatus;
	if (!PyTuple_GetInteger(poArgs, 0, &iStatus))
		return Py_BuildException();
	CPythonApplication::Instance().MultiFarmBlockIcon(iStatus);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_DISCORD_STUFF
PyObject* appSetTitle(PyObject* poSelf, PyObject* poArgs)
{
	char* szTitle;
	if (!PyTuple_GetString(poArgs, 0, &szTitle))
		return Py_BuildException();
	char* szMiniTitle;
	if (!PyTuple_GetString(poArgs, 1, &szMiniTitle))
		return Py_BuildException();
	CPythonApplication::Instance().SetTitle(szTitle, szMiniTitle);
	return Py_BuildNone();
}

PyObject* appSetMessage(PyObject* poSelf, PyObject* poArgs)
{
	char* szTitle;
	if (!PyTuple_GetString(poArgs, 0, &szTitle))
		return Py_BuildException();
	char* szMessage;
	if (!PyTuple_GetString(poArgs, 1, &szMessage))
		return Py_BuildException();
	CPythonApplication::Instance().SetMessage(szTitle, szMessage);
	return Py_BuildNone();
}

PyObject* appWindowStatus(PyObject* poSelf, PyObject* poArgs)
{
	int iStatus;
	if (!PyTuple_GetInteger(poArgs, 0, &iStatus))
		return Py_BuildException();
	CPythonApplication::Instance().SetVisibleMode(iStatus == 1 ? true : false);
	return Py_BuildNone();
}

PyObject* appIsVisible(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().IsVisible());
}
#endif

#ifdef ENABLE_BAKIM_DUYURU
PyObject * appGetLocalTime(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("f", ELTimer_GetMSec() / 1000.0f);
}
#endif


void initapp()
{
	static PyMethodDef s_methods[] =
	{
		{ "IsDevStage",					appIsDevStage,					METH_VARARGS },
		{ "IsTestStage",				appIsTestStage,					METH_VARARGS },
		{ "IsLiveStage",				appIsLiveStage,					METH_VARARGS },

		// TEXTTAIL_LIVINGTIME_CONTROL
		{ "SetTextTailLivingTime",		appSetTextTailLivingTime,		METH_VARARGS },
		// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

		{ "SetHairColorEnable",			appSetHairColorEnable,			METH_VARARGS },
		{ "EnablePerformanceTime",		appEnablePerformanceTime,		METH_VARARGS },

		{ "SetArmorSpecularEnable",		appSetArmorSpecularEnable,		METH_VARARGS },
		{ "SetWeaponSpecularEnable",	appSetWeaponSpecularEnable,		METH_VARARGS },
		{ "SetSkillEffectUpgradeEnable",appSetSkillEffectUpgradeEnable,	METH_VARARGS },
		{ "SetTwoHandedWeaponAttSpeedDecreaseValue", SetTwoHandedWeaponAttSpeedDecreaseValue, METH_VARARGS },
		{ "SetRideHorseEnable",			appSetRideHorseEnable,			METH_VARARGS },

		{ "SetCameraMaxDistance",		appSetCameraMaxDistance,		METH_VARARGS },
		{ "SetMinFog",					appSetMinFog,					METH_VARARGS },
		{ "SetFrameSkip",				appSetFrameSkip,				METH_VARARGS },
		{ "GetImageInfo",				appGetImageInfo,				METH_VARARGS },
		{ "GetInfo",					appGetInfo,						METH_VARARGS },
		{ "UpdateGame",					appUpdateGame,					METH_VARARGS },
		{ "RenderGame",					appRenderGame,					METH_VARARGS },
		{ "Loop",						appLoop,						METH_VARARGS },
		{ "Create",						appCreate,						METH_VARARGS },
		{ "Process",					appProcess,						METH_VARARGS },
		{ "Exit",						appExit,						METH_VARARGS },
		{ "Abort",						appAbort,						METH_VARARGS },
		{ "SetMouseHandler",			appSetMouseHandler,				METH_VARARGS },
		{ "IsExistFile",				appIsExistFile,					METH_VARARGS },
		{ "GetFileList",				appGetFileList,					METH_VARARGS },

		{ "SetCamera",					appSetCamera,					METH_VARARGS },
		{ "GetCamera",					appGetCamera,					METH_VARARGS },
		{ "GetCameraPitch",				appGetCameraPitch,				METH_VARARGS },
		{ "GetCameraRotation",			appGetCameraRotation,			METH_VARARGS },
		{ "GetTime",					appGetTime,						METH_VARARGS },
		{ "GetGlobalTime",				appGetGlobalTime,				METH_VARARGS },
		{ "GetGlobalTimeStamp",			appGetGlobalTimeStamp,			METH_VARARGS },
		{ "GetUpdateFPS",				appGetUpdateFPS,				METH_VARARGS },
		{ "GetRenderFPS",				appGetRenderFPS,				METH_VARARGS },

		#ifdef ENABLE_BAKIM_DUYURU
		{ "GetLocalTime",				appGetLocalTime,				METH_VARARGS },
		#endif

		{ "RotateCamera",				appRotateCamera,				METH_VARARGS },
		{ "PitchCamera",				appPitchCamera,					METH_VARARGS },
		{ "ZoomCamera",					appZoomCamera,					METH_VARARGS },
		{ "MovieRotateCamera",			appMovieRotateCamera,			METH_VARARGS },
		{ "MoviePitchCamera",			appMoviePitchCamera,			METH_VARARGS },
		{ "MovieZoomCamera",			appMovieZoomCamera,				METH_VARARGS },
		{ "MovieResetCamera",			appMovieResetCamera,			METH_VARARGS },

		{ "GetAvailableTextureMemory",	appGetAvaiableTextureMememory,	METH_VARARGS },
		{ "GetRenderTime",				appGetRenderTime,				METH_VARARGS },
		{ "GetUpdateTime",				appGetUpdateTime,				METH_VARARGS },
		{ "GetLoad",					appGetLoad,						METH_VARARGS },
		{ "GetFaceSpeed",				appGetFaceSpeed,				METH_VARARGS },
		{ "GetFaceCount",				appGetFaceCount,				METH_VARARGS },
		{ "SetFPS",						appSetFPS,						METH_VARARGS },
		{ "SetGlobalCenterPosition",	appSetGlobalCenterPosition,		METH_VARARGS },
		{ "SetCenterPosition",			appSetCenterPosition,			METH_VARARGS },
		{ "GetCursorPosition",			appGetCursorPosition,			METH_VARARGS },

		{ "GetRandom",					appGetRandom,					METH_VARARGS },
		{ "RunPythonFile",				appRunPythonFile,				METH_VARARGS },
		{ "IsWebPageMode",				appIsWebPageMode,				METH_VARARGS },
		{ "ShowWebPage",				appShowWebPage,					METH_VARARGS },
		{ "MoveWebPage",				appMoveWebPage,					METH_VARARGS },
		{ "HideWebPage",				appHideWebPage,					METH_VARARGS },
		{ "IsPressed",					appIsPressed,					METH_VARARGS },
		{ "SetCursor",					appSetCursor,					METH_VARARGS },
		{ "GetCursor",					appGetCursor,					METH_VARARGS },
		{ "ShowCursor",					appShowCursor,					METH_VARARGS },
		{ "HideCursor",					appHideCursor,					METH_VARARGS },
		{ "IsShowCursor",				appIsShowCursor,				METH_VARARGS },
		{ "IsLiarCursorOn",				appIsLiarCursorOn,				METH_VARARGS },
		{ "SetSoftwareCursor",			appSetSoftwareCursor,			METH_VARARGS },
		{ "SetHardwareCursor",			appSetHardwareCursor,			METH_VARARGS },

		{ "SetConnectData",				appSetConnectData,				METH_VARARGS },
		{ "GetConnectData",				appGetConnectData,				METH_VARARGS },

		{ "GetRotatingDirection",		appGetRotatingDirection,		METH_VARARGS },
		{ "GetDegreeDifference",		appGetDegreeDifference,			METH_VARARGS },
		{ "Sleep",						appSleep,						METH_VARARGS },
		{ "SetDefaultFontName",			appSetDefaultFontName,			METH_VARARGS },
		{ "SetGuildSymbolPath",			appSetGuildSymbolPath,			METH_VARARGS },

		{ "EnableSpecialCameraMode",	appEnableSpecialCameraMode,		METH_VARARGS },
		{ "SetCameraSpeed",				appSetCameraSpeed,				METH_VARARGS },

		{ "SaveCameraSetting",			appSaveCameraSetting,			METH_VARARGS },
		{ "LoadCameraSetting",			appLoadCameraSetting,			METH_VARARGS },
		{ "SetDefaultCamera",			appSetDefaultCamera,			METH_VARARGS },
		{ "SetCameraSetting",			appSetCameraSetting,			METH_VARARGS },

		{ "SetSightRange",				appSetSightRange,				METH_VARARGS },

		{ "IsFileExist",				appIsFileExist,					METH_VARARGS },
		{ "OpenTextFile",				appOpenTextFile,				METH_VARARGS },
		{ "CloseTextFile",				appCloseTextFile,				METH_VARARGS },
		{ "GetTextFileLineCount",		appGetTextFileLineCount,		METH_VARARGS },
		{ "GetTextFileLine",			appGetTextFileLine,				METH_VARARGS },

		// LOCALE
		{ "GetLocaleServiceName",		appGetLocaleServiceName,		METH_VARARGS },
		{ "GetLocaleName",				appGetLocaleName,				METH_VARARGS },
		{ "GetLocalePath",				appGetLocalePath,				METH_VARARGS },
		{ "ForceSetLocale",				appForceSetLocale,				METH_VARARGS },
		// END_OF_LOCALE

		// CHEONMA
		{ "LoadLocaleAddr",				appLoadLocaleAddr,				METH_VARARGS },
		{ "LoadLocaleData",				appLoadLocaleData,				METH_VARARGS },
		{ "SetCHEONMA",					appSetCHEONMA,					METH_VARARGS },
		{ "IsCHEONMA",					appIsCHEONMA,					METH_VARARGS },
		// END_OF_CHEONMA

		{ "GetDefaultCodePage",			appGetDefaultCodePage,			METH_VARARGS },
		{ "SetControlFP",				appSetControlFP,				METH_VARARGS },
		{ "SetSpecularSpeed",			appSetSpecularSpeed,			METH_VARARGS },

		{ "testGetAccumulationTime",	apptestGetAccumulationTime,		METH_VARARGS },
		{ "testResetAccumulationTime",	apptestResetAccumulationTime,	METH_VARARGS },
		{ "testSetSpecularColor",		apptestSetSpecularColor,		METH_VARARGS },

		{ "SetVisibleNotice",			appSetVisibleNotice,			METH_VARARGS },
		{ "IsVisibleNotice",			appIsVisibleNotice,				METH_VARARGS },
		{ "EnableTestServerFlag",		appEnableTestServerFlag,		METH_VARARGS },
		{ "IsEnableTestServerFlag",		appIsEnableTestServerFlag,		METH_VARARGS },

		{ "SetGuildMarkPath",			appSetGuildMarkPath,			METH_VARARGS },

		{ "OnLogoUpdate",				appLogoUpdate,					METH_VARARGS },
		{ "OnLogoRender",				appLogoRender,					METH_VARARGS },
		{ "OnLogoOpen",					appLogoOpen,					METH_VARARGS },
		{ "OnLogoClose",				appLogoClose,					METH_VARARGS },

		#ifdef ENABLE_RENDER_TARGET
		{ "IsCanOpenRender",			appIsCanOpenRender,				METH_VARARGS },
		#endif

		#ifdef ENABLE_FLASH_APPLICATION
		{ "FlashApplication",			appFlashApplication,			METH_VARARGS },
		#endif

		#ifdef ENABLE_MULTI_FARM_BLOCK
		{ "SetMultiFarmExeIcon", 		appSetMultiFarmExeIcon,			METH_VARARGS },
		#endif

		#ifdef ENABLE_DISCORD_STUFF
		{ "WindowStatus",				appWindowStatus,				METH_VARARGS },
		{ "IsVisible",					appIsVisible,					METH_VARARGS },
		{ "SetTitle", 					appSetTitle,					METH_VARARGS },
		{ "SetMessage", 				appSetMessage,					METH_VARARGS },
		#endif

		{ NULL, NULL },
	};

	PyObject * poModule = Py_InitModule("app", s_methods);

	PyModule_AddIntConstant(poModule, "INFO_ITEM",		CPythonApplication::INFO_ITEM);
	PyModule_AddIntConstant(poModule, "INFO_ACTOR",		CPythonApplication::INFO_ACTOR);
	PyModule_AddIntConstant(poModule, "INFO_EFFECT",	CPythonApplication::INFO_EFFECT);
	PyModule_AddIntConstant(poModule, "INFO_TEXTTAIL",	CPythonApplication::INFO_TEXTTAIL);

	PyModule_AddIntConstant(poModule, "DEGREE_DIRECTION_SAME",		DEGREE_DIRECTION_SAME);
	PyModule_AddIntConstant(poModule, "DEGREE_DIRECTION_RIGHT",		DEGREE_DIRECTION_RIGHT);
	PyModule_AddIntConstant(poModule, "DEGREE_DIRECTION_LEFT",		DEGREE_DIRECTION_LEFT);

	PyModule_AddIntConstant(poModule, "VK_LEFT",	     VK_LEFT);
	PyModule_AddIntConstant(poModule, "VK_RIGHT",	     VK_RIGHT);
	PyModule_AddIntConstant(poModule, "VK_UP",		     VK_UP);
	PyModule_AddIntConstant(poModule, "VK_DOWN",	     VK_DOWN);
	PyModule_AddIntConstant(poModule, "VK_HOME",	     VK_HOME);
	PyModule_AddIntConstant(poModule, "VK_END",          VK_END);
	PyModule_AddIntConstant(poModule, "VK_DELETE",	     VK_DELETE);

	PyModule_AddIntConstant(poModule, "DIK_ESCAPE",      DIK_ESCAPE);
	PyModule_AddIntConstant(poModule, "DIK_ESC",         DIK_ESCAPE);
	PyModule_AddIntConstant(poModule, "DIK_1",           DIK_1);
	PyModule_AddIntConstant(poModule, "DIK_2",           DIK_2);
	PyModule_AddIntConstant(poModule, "DIK_3",           DIK_3);
	PyModule_AddIntConstant(poModule, "DIK_4",           DIK_4);
	PyModule_AddIntConstant(poModule, "DIK_5",           DIK_5);
	PyModule_AddIntConstant(poModule, "DIK_6",           DIK_6);
	PyModule_AddIntConstant(poModule, "DIK_7",           DIK_7);
	PyModule_AddIntConstant(poModule, "DIK_8",           DIK_8);
	PyModule_AddIntConstant(poModule, "DIK_9",           DIK_9);
	PyModule_AddIntConstant(poModule, "DIK_0",           DIK_0);
	PyModule_AddIntConstant(poModule, "DIK_MINUS",       DIK_MINUS);        /* - on main keyboard */
	PyModule_AddIntConstant(poModule, "DIK_EQUALS",      DIK_EQUALS);
	PyModule_AddIntConstant(poModule, "DIK_BACK",        DIK_BACK);           /* backspace */
	PyModule_AddIntConstant(poModule, "DIK_TAB",         DIK_TAB);
	PyModule_AddIntConstant(poModule, "DIK_Q",           DIK_Q);
	PyModule_AddIntConstant(poModule, "DIK_W",           DIK_W);
	PyModule_AddIntConstant(poModule, "DIK_E",           DIK_E);
	PyModule_AddIntConstant(poModule, "DIK_R",           DIK_R);
	PyModule_AddIntConstant(poModule, "DIK_T",           DIK_T);
	PyModule_AddIntConstant(poModule, "DIK_Y",           DIK_Y);
	PyModule_AddIntConstant(poModule, "DIK_U",           DIK_U);
	PyModule_AddIntConstant(poModule, "DIK_I",           DIK_I);
	PyModule_AddIntConstant(poModule, "DIK_O",           DIK_O);
	PyModule_AddIntConstant(poModule, "DIK_P",           DIK_P);
	PyModule_AddIntConstant(poModule, "DIK_LBRACKET",    DIK_LBRACKET);
	PyModule_AddIntConstant(poModule, "DIK_RBRACKET",    DIK_RBRACKET);
	PyModule_AddIntConstant(poModule, "DIK_RETURN",      DIK_RETURN);         /* Enter on main keyboard */
	PyModule_AddIntConstant(poModule, "DIK_LCONTROL",    DIK_LCONTROL);
	PyModule_AddIntConstant(poModule, "DIK_A",           DIK_A);
	PyModule_AddIntConstant(poModule, "DIK_S",           DIK_S);
	PyModule_AddIntConstant(poModule, "DIK_D",           DIK_D);
	PyModule_AddIntConstant(poModule, "DIK_F",           DIK_F);
	PyModule_AddIntConstant(poModule, "DIK_G",           DIK_G);
	PyModule_AddIntConstant(poModule, "DIK_H",           DIK_H);
	PyModule_AddIntConstant(poModule, "DIK_J",           DIK_J);
	PyModule_AddIntConstant(poModule, "DIK_K",           DIK_K);
	PyModule_AddIntConstant(poModule, "DIK_L",           DIK_L);
	PyModule_AddIntConstant(poModule, "DIK_SEMICOLON",   DIK_SEMICOLON);
	PyModule_AddIntConstant(poModule, "DIK_APOSTROPHE",  DIK_APOSTROPHE);
	PyModule_AddIntConstant(poModule, "DIK_GRAVE",       DIK_GRAVE);          /* accent grave */
	PyModule_AddIntConstant(poModule, "DIK_LSHIFT",      DIK_LSHIFT);
	PyModule_AddIntConstant(poModule, "DIK_BACKSLASH",   DIK_BACKSLASH);
	PyModule_AddIntConstant(poModule, "DIK_Z",           DIK_Z);
	PyModule_AddIntConstant(poModule, "DIK_X",           DIK_X);
	PyModule_AddIntConstant(poModule, "DIK_C",           DIK_C);
	PyModule_AddIntConstant(poModule, "DIK_V",           DIK_V);
	PyModule_AddIntConstant(poModule, "DIK_B",           DIK_B);
	PyModule_AddIntConstant(poModule, "DIK_N",           DIK_N);
	PyModule_AddIntConstant(poModule, "DIK_M",           DIK_M);
	PyModule_AddIntConstant(poModule, "DIK_COMMA",       DIK_COMMA);
	PyModule_AddIntConstant(poModule, "DIK_PERIOD",      DIK_PERIOD);         /* . on main keyboard */
	PyModule_AddIntConstant(poModule, "DIK_SLASH",       DIK_SLASH);          /* / on main keyboard */
	PyModule_AddIntConstant(poModule, "DIK_RSHIFT",      DIK_RSHIFT);
	PyModule_AddIntConstant(poModule, "DIK_MULTIPLY",    DIK_MULTIPLY);       /* * on numeric keypad */
	PyModule_AddIntConstant(poModule, "DIK_LALT",        DIK_LMENU);          /* left Alt */
	PyModule_AddIntConstant(poModule, "DIK_SPACE",       DIK_SPACE);
	PyModule_AddIntConstant(poModule, "DIK_CAPITAL",     DIK_CAPITAL);
	PyModule_AddIntConstant(poModule, "DIK_F1",          DIK_F1);
	PyModule_AddIntConstant(poModule, "DIK_F2",          DIK_F2);
	PyModule_AddIntConstant(poModule, "DIK_F3",          DIK_F3);
	PyModule_AddIntConstant(poModule, "DIK_F4",          DIK_F4);
	PyModule_AddIntConstant(poModule, "DIK_F5",          DIK_F5);
	PyModule_AddIntConstant(poModule, "DIK_F6",          DIK_F6);
	PyModule_AddIntConstant(poModule, "DIK_F7",          DIK_F7);
	PyModule_AddIntConstant(poModule, "DIK_F8",          DIK_F8);
	PyModule_AddIntConstant(poModule, "DIK_F9",          DIK_F9);
	PyModule_AddIntConstant(poModule, "DIK_F10",         DIK_F10);
	PyModule_AddIntConstant(poModule, "DIK_NUMLOCK",     DIK_NUMLOCK);
	PyModule_AddIntConstant(poModule, "DIK_SCROLL",      DIK_SCROLL);         /* Scroll Lock */
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD7",     DIK_NUMPAD7);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD8",     DIK_NUMPAD8);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD9",     DIK_NUMPAD9);
	PyModule_AddIntConstant(poModule, "DIK_SUBTRACT",    DIK_SUBTRACT);       /* - on numeric keypad */
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD4",     DIK_NUMPAD4);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD5",     DIK_NUMPAD5);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD6",     DIK_NUMPAD6);
	PyModule_AddIntConstant(poModule, "DIK_ADD",         DIK_ADD);            /* + on numeric keypad */
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD1",     DIK_NUMPAD1);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD2",     DIK_NUMPAD2);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD3",     DIK_NUMPAD3);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD0",     DIK_NUMPAD0);
	PyModule_AddIntConstant(poModule, "DIK_DECIMAL",     DIK_DECIMAL);        /* . on numeric keypad */
	PyModule_AddIntConstant(poModule, "DIK_F11",         DIK_F11);
	PyModule_AddIntConstant(poModule, "DIK_F12",         DIK_F12);
	PyModule_AddIntConstant(poModule, "DIK_NEXTTRACK",   DIK_NEXTTRACK);      /* Next Track */
	PyModule_AddIntConstant(poModule, "DIK_NUMPADENTER", DIK_NUMPADENTER);    /* Enter on numeric keypad */
	PyModule_AddIntConstant(poModule, "DIK_RCONTROL",    DIK_RCONTROL);
	PyModule_AddIntConstant(poModule, "DIK_MUTE",        DIK_MUTE);           /* Mute */
	PyModule_AddIntConstant(poModule, "DIK_CALCULATOR",  DIK_CALCULATOR);     /* Calculator */
	PyModule_AddIntConstant(poModule, "DIK_PLAYPAUSE",   DIK_PLAYPAUSE);      /* Play / Pause */
	PyModule_AddIntConstant(poModule, "DIK_MEDIASTOP",   DIK_MEDIASTOP);      /* Media Stop */
	PyModule_AddIntConstant(poModule, "DIK_VOLUMEDOWN",  DIK_VOLUMEDOWN);     /* Volume - */
	PyModule_AddIntConstant(poModule, "DIK_VOLUMEUP",    DIK_VOLUMEUP);       /* Volume + */
	PyModule_AddIntConstant(poModule, "DIK_WEBHOME",     DIK_WEBHOME);        /* Web home */
	PyModule_AddIntConstant(poModule, "DIK_NUMPADCOMMA", DIK_NUMPADCOMMA);    /* , on numeric keypad (NEC PC98) */
	PyModule_AddIntConstant(poModule, "DIK_DIVIDE",      DIK_DIVIDE);         /* / on numeric keypad */
	PyModule_AddIntConstant(poModule, "DIK_SYSRQ",       DIK_SYSRQ);
	PyModule_AddIntConstant(poModule, "DIK_RALT",        DIK_RMENU);          /* right Alt */
	PyModule_AddIntConstant(poModule, "DIK_PAUSE",       DIK_PAUSE);          /* Pause */
	PyModule_AddIntConstant(poModule, "DIK_HOME",        DIK_HOME);           /* Home on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_UP",          DIK_UP);             /* UpArrow on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_PGUP",        DIK_PRIOR);          /* PgUp on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_LEFT",        DIK_LEFT);           /* LeftArrow on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_RIGHT",       DIK_RIGHT);          /* RightArrow on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_END",         DIK_END);            /* End on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_DOWN",        DIK_DOWN);           /* DownArrow on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_PGDN",        DIK_NEXT);           /* PgDn on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_INSERT",      DIK_INSERT);         /* Insert on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_DELETE",      DIK_DELETE);         /* Delete on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_LWIN",        DIK_LWIN);           /* Left Windows key */
	PyModule_AddIntConstant(poModule, "DIK_RWIN",        DIK_RWIN);           /* Right Windows key */
	PyModule_AddIntConstant(poModule, "DIK_APPS",        DIK_APPS);           /* AppMenu key */

	// Cursor
	PyModule_AddIntConstant(poModule, "NORMAL",			CPythonApplication::CURSOR_SHAPE_NORMAL);
	PyModule_AddIntConstant(poModule, "ATTACK",			CPythonApplication::CURSOR_SHAPE_ATTACK);
	PyModule_AddIntConstant(poModule, "TARGET",			CPythonApplication::CURSOR_SHAPE_TARGET);
	PyModule_AddIntConstant(poModule, "TALK",			CPythonApplication::CURSOR_SHAPE_TALK);
	PyModule_AddIntConstant(poModule, "CANT_GO",		CPythonApplication::CURSOR_SHAPE_CANT_GO);
	PyModule_AddIntConstant(poModule, "PICK",			CPythonApplication::CURSOR_SHAPE_PICK);

	PyModule_AddIntConstant(poModule, "DOOR",			CPythonApplication::CURSOR_SHAPE_DOOR);
	PyModule_AddIntConstant(poModule, "CHAIR",			CPythonApplication::CURSOR_SHAPE_CHAIR);
	PyModule_AddIntConstant(poModule, "MAGIC",			CPythonApplication::CURSOR_SHAPE_MAGIC);
	PyModule_AddIntConstant(poModule, "BUY",			CPythonApplication::CURSOR_SHAPE_BUY);
	PyModule_AddIntConstant(poModule, "SELL",			CPythonApplication::CURSOR_SHAPE_SELL);

	PyModule_AddIntConstant(poModule, "CAMERA_ROTATE",	CPythonApplication::CURSOR_SHAPE_CAMERA_ROTATE);
	PyModule_AddIntConstant(poModule, "HSIZE",			CPythonApplication::CURSOR_SHAPE_HSIZE);
	PyModule_AddIntConstant(poModule, "VSIZE",			CPythonApplication::CURSOR_SHAPE_VSIZE);
	PyModule_AddIntConstant(poModule, "HVSIZE",			CPythonApplication::CURSOR_SHAPE_HVSIZE);

	PyModule_AddIntConstant(poModule, "CAMERA_TO_POSITIVE",		CPythonApplication::CAMERA_TO_POSITIVE);
	PyModule_AddIntConstant(poModule, "CAMERA_TO_NEGATIVE",		CPythonApplication::CAMERA_TO_NEGITIVE);
	PyModule_AddIntConstant(poModule, "CAMERA_STOP",			CPythonApplication::CAMERA_STOP);

#ifdef ENABLE_RENEWAL_AFFECT_SHOWER
	PyModule_AddIntConstant(poModule, "ENABLE_RENEWAL_AFFECT_SHOWER", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_RENEWAL_AFFECT_SHOWER", 0);
#endif

#ifdef ENABLE_FPS
	PyModule_AddIntConstant(poModule, "ENABLE_FPS", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_FPS", 0);
#endif

#ifdef ENABLE_SHOW_NIGHT_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_SHOW_NIGHT_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SHOW_NIGHT_SYSTEM", 0);
#endif

#ifdef ENABLE_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_COSTUME_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_COSTUME_SYSTEM",	0);
#endif

#ifdef ENABLE_ENERGY_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_ENERGY_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ENERGY_SYSTEM",	0);
#endif

#if defined(__BL_HIDE_EFFECT__)
	PyModule_AddIntConstant(poModule, "__BL_HIDE_EFFECT__", true);
#else
	PyModule_AddIntConstant(poModule, "__BL_HIDE_EFFECT__", false);
#endif

#ifdef ENABLE_DRAGON_SOUL_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_SOUL_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_SOUL_SYSTEM",	0);
#endif

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_EQUIPMENT_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_EQUIPMENT_SYSTEM",	0);
#endif

#ifdef ENABLE_FOG_FIX
	PyModule_AddIntConstant(poModule, "ENABLE_FOG_FIX",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_FOG_FIX",	0);
#endif

#ifdef ENABLE_SOULBIND_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_SOULBIND_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SOULBIND_SYSTEM",	0);
#endif

#ifdef ENABLE_PLAYER_PER_ACCOUNT5
	PyModule_AddIntConstant(poModule, "ENABLE_PLAYER_PER_ACCOUNT5",	1);
	PyModule_AddIntConstant(poModule, "PLAYER_PER_ACCOUNT",	PLAYER_PER_ACCOUNT5);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PLAYER_PER_ACCOUNT5",	0);
	PyModule_AddIntConstant(poModule, "PLAYER_PER_ACCOUNT",	PLAYER_PER_ACCOUNT4);
#endif

#ifdef ENABLE_WOLFMAN_CHARACTER
	PyModule_AddIntConstant(poModule, "ENABLE_WOLFMAN_CHARACTER",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_WOLFMAN_CHARACTER",	0);
#endif

#ifdef ENABLE_QUIVER_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_QUIVER_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_QUIVER_SYSTEM",	0);
#endif

#ifdef ENABLE_LEVEL_IN_TRADE
	PyModule_AddIntConstant(poModule, "ENABLE_LEVEL_IN_TRADE",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_LEVEL_IN_TRADE",	0);
#endif

#ifdef ENABLE_678TH_SKILL
	PyModule_AddIntConstant(poModule, "ENABLE_678TH_SKILL",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_678TH_SKILL",	0);
#endif

#ifdef LOGIN_COUNT_DOWN_UI_MODIFY
	PyModule_AddIntConstant(poModule, "LOGIN_COUNT_DOWN_UI_MODIFY",	1);
#else
	PyModule_AddIntConstant(poModule, "LOGIN_COUNT_DOWN_UI_MODIFY",	0);
#endif

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_MOUNT_COSTUME_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MOUNT_COSTUME_SYSTEM",	0);
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_ACCE_SYSTEM",	1);
	PyModule_AddIntConstant(poModule, "ENABLE_ACCE_COSTUME_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ACCE_SYSTEM",	0);
	PyModule_AddIntConstant(poModule, "ENABLE_ACCE_COSTUME_SYSTEM",	0);
#endif

#ifdef USE_ACCE_ABSORB_WITH_NO_NEGATIVE_BONUS
	PyModule_AddIntConstant(poModule, "USE_ACCE_ABSORB_WITH_NO_NEGATIVE_BONUS", 1);
#else
	PyModule_AddIntConstant(poModule, "USE_ACCE_ABSORB_WITH_NO_NEGATIVE_BONUS", 0);
#endif

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	PyModule_AddIntConstant(poModule, "ENABLE_ENVIRONMENT_EFFECT_OPTION",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ENVIRONMENT_EFFECT_OPTION",	0);
#endif

#ifdef WJ_SHOW_MOB_INFO
	PyModule_AddIntConstant(poModule, "WJ_SHOW_MOB_INFO",	1);
#else
	PyModule_AddIntConstant(poModule, "WJ_SHOW_MOB_INFO",	0);
#endif

#ifdef NEW_SELECT_CHARACTER
	PyModule_AddIntConstant(poModule, "NEW_SELECT_CHARACTER",	1);
#else
	PyModule_AddIntConstant(poModule, "NEW_SELECT_CHARACTER",	0);
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_GROWTH_PET_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GROWTH_PET_SYSTEM",	0);
#endif

#ifdef ENABLE_AUTO_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_AUTO_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_AUTO_SYSTEM",	0);
#endif

#ifdef ENABLE_MONSTER_CARD
	PyModule_AddIntConstant(poModule, "ENABLE_MONSTER_CARD",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MONSTER_CARD",	0);
#endif

#ifdef ENABLE_HELP_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_HELP_RENEWAL",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_HELP_RENEWAL",	0);
#endif

#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_MAGIC_REDUCTION_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MAGIC_REDUCTION_SYSTEM",	0);
#endif

#ifdef ENABLE_DICE_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_DICE_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DICE_SYSTEM",	0);
#endif

#ifdef ENABLE_CHEQUE_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_CHEQUE_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CHEQUE_SYSTEM",	0);
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_EXTEND_INVEN_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EXTEND_INVEN_SYSTEM",	0);
#endif

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_WEAPON_COSTUME_SYSTEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_WEAPON_COSTUME_SYSTEM",	0);
#endif

#ifdef ENABLE_LVL115_ARMOR_EFFECT
	PyModule_AddIntConstant(poModule, "ENABLE_LVL115_ARMOR_EFFECT",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_LVL115_ARMOR_EFFECT",	0);
#endif

#ifdef ENABLE_USE_COSTUME_ATTR
	PyModule_AddIntConstant(poModule, "ENABLE_USE_COSTUME_ATTR",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_USE_COSTUME_ATTR",	0);
#endif

#ifdef ENABLE_SLOT_WINDOW_EX
	PyModule_AddIntConstant(poModule, "ENABLE_SLOT_WINDOW_EX",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SLOT_WINDOW_EX",	0);
#endif

#ifdef ENABLE_USE_COSTUME_ATTR
	PyModule_AddIntConstant(poModule, "ENABLE_USE_COSTUME_ATTR",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_USE_COSTUME_ATTR",	0);
#endif

#ifdef USE_OPENID
	PyModule_AddIntConstant(poModule, "USE_OPENID",	1);
	if (openid_test)
		PyModule_AddIntConstant(poModule, "OPENID_TEST",	1);
	else
		PyModule_AddIntConstant(poModule, "OPENID_TEST",	0);
#else
	PyModule_AddIntConstant(poModule, "USE_OPENID",	0);
	PyModule_AddIntConstant(poModule, "OPENID_TEST",	0);
#endif /* USE_OPENID */

#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
	PyModule_AddIntConstant(poModule, "ENABLE_HIGHLIGHT_NEW_ITEM",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_HIGHLIGHT_NEW_ITEM",	0);
#endif

#ifdef ENABLE_EMOJI_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_EMOJI_SYSTEM",   1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EMOJI_SYSTEM",   0);
#endif

#ifdef ENABLE_PET_SYSTEM_EX
	PyModule_AddIntConstant(poModule, "ENABLE_PET_SYSTEM_EX",   1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PET_SYSTEM_EX",   0);
#endif

#ifdef ENABLE_SHOW_CHEST_DROP
	PyModule_AddIntConstant(poModule, "ENABLE_SHOW_CHEST_DROP",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SHOW_CHEST_DROP",	0);
#endif

#ifdef ENABLE_CLEAR_CHAT
	PyModule_AddIntConstant(poModule, "ENABLE_CLEAR_CHAT", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CLEAR_CHAT", 0);
#endif

#ifdef ENABLE_CHAT_STOP
	PyModule_AddIntConstant(poModule, "ENABLE_CHAT_STOP", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CHAT_STOP", 0);
#endif

#ifdef ENABLE_STATUS_UP_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_STATUS_UP_RENEWAL", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_STATUS_UP_RENEWAL", 0);
#endif

#ifdef ENABLE_REFINE_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_REFINE_RENEWAL",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_REFINE_RENEWAL",	0);
#endif

#ifdef ENABLE_DETAILS_INTERFACE
	PyModule_AddIntConstant(poModule, "ENABLE_DETAILS_INTERFACE", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DETAILS_INTERFACE", 0);
#endif

#ifdef ENABLE_FAST_ATTACH_ITEMS_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_FAST_ATTACH_ITEMS_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_FAST_ATTACH_ITEMS_SYSTEM", 0);
#endif

#ifdef ENABLE_SEND_TARGET_INFO
	PyModule_AddIntConstant(poModule, "ENABLE_SEND_TARGET_INFO", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SEND_TARGET_INFO", 0);
#endif

#ifdef ENABLE_AUTO_PICKUP_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_AUTO_PICKUP_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_AUTO_PICKUP_SYSTEM", 0);
#endif

#ifdef ENABLE_MOUSEWHEEL_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_MOUSEWHEEL_EVENT", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MOUSEWHEEL_EVENT", 0);
#endif

#ifdef ENABLE_MOUSEWHEEL_EVENT_OLD
	PyModule_AddIntConstant(poModule, "ENABLE_MOUSEWHEEL_EVENT_OLD", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MOUSEWHEEL_EVENT_OLD", 0);
#endif

#ifdef ENABLE_FLASH_APPLICATION
	PyModule_AddIntConstant(poModule, "ENABLE_FLASH_APPLICATION", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_FLASH_APPLICATION", 0);
#endif

#ifdef ENABLE_CHANNEL_INFO_UPDATE
	PyModule_AddIntConstant(poModule, "ENABLE_CHANNEL_INFO_UPDATE", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CHANNEL_INFO_UPDATE", 0);
#endif

#ifdef ENABLE_SPECIAL_STORAGE
	PyModule_AddIntConstant(poModule, "ENABLE_SPECIAL_STORAGE", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SPECIAL_STORAGE", 0);
#endif

#ifdef ENABLE_IMAGE_CLIP_RECT
	PyModule_AddIntConstant(poModule, "ENABLE_IMAGE_CLIP_RECT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_IMAGE_CLIP_RECT", false);
#endif

#ifdef ENABLE_SWITCHBOT
	PyModule_AddIntConstant(poModule, "ENABLE_SWITCHBOT", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SWITCHBOT", 0);
#endif

#ifdef ENABLE_DROP_DIALOG_EXTENDED_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_DROP_DIALOG_EXTENDED_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DROP_DIALOG_EXTENDED_SYSTEM", 0);
#endif

#ifdef ENABLE_ITEM_DELETE_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_ITEM_DELETE_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ITEM_DELETE_SYSTEM", 0);
#endif

#ifdef ENABLE_BOSS_EFFECT_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_BOSS_EFFECT_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_BOSS_EFFECT_SYSTEM", 0);
#endif

#ifdef ENABLE_PERMA_SET
	PyModule_AddIntConstant(poModule, "ENABLE_PERMA_SET", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PERMA_SET", 0);
#endif

#ifdef ENABLE_ANTI_EXP
	PyModule_AddIntConstant(poModule, "ENABLE_ANTI_EXP", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ANTI_EXP", 0);
#endif

#ifdef ENABLE_MESSENGER_BLOCK
	PyModule_AddIntConstant(poModule, "ENABLE_MESSENGER_BLOCK",	1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MESSENGER_BLOCK",	0);
#endif

#ifdef ENABLE_MESSENGER_TEAM
	PyModule_AddIntConstant(poModule, "ENABLE_MESSENGER_TEAM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MESSENGER_TEAM", 0);
#endif

#ifdef ENABLE_FOV_OPTION
	PyModule_AddIntConstant(poModule, "ENABLE_FOV_OPTION", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_FOV_OPTION", 0);
#endif

#ifdef ENABLE_FAST_ATTACH_ITEMS_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_FAST_ATTACH_ITEMS_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_FAST_ATTACH_ITEMS_SYSTEM", 0);
#endif

#ifdef ENABLE_KOSTUM_SAC_BINEK_PET_EFSUNLAMA
	PyModule_AddIntConstant(poModule, "ENABLE_KOSTUM_SAC_BINEK_PET_EFSUNLAMA", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_KOSTUM_SAC_BINEK_PET_EFSUNLAMA", 0);
#endif

#ifdef ENABLE_STYLE_ATTRIBUTE_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_STYLE_ATTRIBUTE_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_STYLE_ATTRIBUTE_SYSTEM", 0);
#endif

#ifdef ENABLE_GIVE_TIME_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_GIVE_TIME_COSTUME_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GIVE_TIME_COSTUME_SYSTEM", 0);
#endif

#ifdef ENABLE_COSTUME_PET_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_COSTUME_PET_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_COSTUME_PET_SYSTEM", 0);
#endif

#ifdef ENABLE_EXCHANGE_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_EXCHANGE_RENEWAL", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EXCHANGE_RENEWAL", 0);
#endif

#ifdef ENABLE_PASIF_SKILL
	PyModule_AddIntConstant(poModule, "ENABLE_PASIF_SKILL", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PASIF_SKILL", 0);
#endif

#ifdef ENABLE_NEW_ATTRIBUTE_TYPES
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_ATTRIBUTE_TYPES", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_ATTRIBUTE_TYPES", 0);
#endif

#ifdef ENABLE_HIDE_COSTUME_PART
	PyModule_AddIntConstant(poModule, "ENABLE_HIDE_COSTUME_PART", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_HIDE_COSTUME_PART", 0);
#endif

#ifdef ENABLE_EXTRA_SOCKET_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_EXTRA_SOCKET_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EXTRA_SOCKET_SYSTEM", 0);
#endif

#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
	PyModule_AddIntConstant(poModule, "ENABLE_CUBE_RENEWAL_WORLDARD", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CUBE_RENEWAL_WORLDARD", 0);
#endif

#ifdef ENABLE_SMITH_EFFECT_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_SMITH_EFFECT_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SMITH_EFFECT_SYSTEM", 0);
#endif

#ifdef __RANKING_SYSTEM__
	PyModule_AddIntConstant(poModule, "__RANKING_SYSTEM__", 1);
#else
	PyModule_AddIntConstant(poModule, "__RANKING_SYSTEM__", 0);
#endif

#ifdef ENABLE_WHEEL_OF_FORTUNE
	PyModule_AddIntConstant(poModule, "ENABLE_WHEEL_OF_FORTUNE", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_WHEEL_OF_FORTUNE", 0);
#endif

#ifdef ENABLE_RUZGARIN_AYAKKABI
	PyModule_AddIntConstant(poModule, "ENABLE_RUZGARIN_AYAKKABI", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_RUZGARIN_AYAKKABI", 0);
#endif

#ifdef ENABLE_BIYOLOG
	PyModule_AddIntConstant(poModule, "ENABLE_BIYOLOG", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_BIYOLOG", 0);
#endif

#ifdef ENABLE_NEW_GAMEOPTION
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_GAMEOPTION", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_GAMEOPTION", 0);
#endif

#ifdef ENABLE_DISCORD_STUFF
	PyModule_AddIntConstant(poModule, "ENABLE_DISCORD_STUFF", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DISCORD_STUFF", 0);
#endif

#ifdef __WHISPER_FUTURES__
	PyModule_AddIntConstant(poModule, "__WHISPER_FUTURES__", 1);
#else
	PyModule_AddIntConstant(poModule, "__WHISPER_FUTURES__", 0);
#endif

#ifdef ENABLE_EVENT_MANAGER
	PyModule_AddIntConstant(poModule, "ENABLE_EVENT_MANAGER", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EVENT_MANAGER", 0);
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
	PyModule_AddIntConstant(poModule, "ENABLE_MULTI_FARM_BLOCK", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MULTI_FARM_BLOCK", 0);
#endif

#ifdef ENABLE_MULTI_HUNT_BLOCK
	PyModule_AddIntConstant(poModule, "ENABLE_MULTI_HUNT_BLOCK", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MULTI_HUNT_BLOCK", 0);
#endif

#ifdef ENABLE_MULTI_FISH_BLOCK
	PyModule_AddIntConstant(poModule, "ENABLE_MULTI_FISH_BLOCK", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MULTI_FISH_BLOCK", 0);
#endif

#ifdef ENABLE_SKILL_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_SKILL_RENEWAL", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SKILL_RENEWAL", 0);
#endif

#ifdef ENABLE_SERVER_PIN
	PyModule_AddIntConstant(poModule, "ENABLE_PIN", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PIN", 0);
#endif

#ifdef ENABLE_NEW_LOADING
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_LOADING", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_LOADING", 0);
#endif

#ifdef ENABLE_BAKIM_DUYURU
	PyModule_AddIntConstant(poModule, "ENABLE_BAKIM_DUYURU", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_BAKIM_DUYURU", 0);
#endif

#ifdef ENABLE_DRAGON_COIN_SHOP_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_COIN_SHOP_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_COIN_SHOP_SYSTEM", 0);
#endif

#ifdef ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM", 0);
#endif

#ifdef ENABLE_DRAGON_COIN_EM_MARKET
	PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_COIN_EM_MARKET", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_COIN_EM_MARKET", 0);
#endif

#ifdef ENABLE_PM_ALL_SEND_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_PM_ALL_SEND_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PM_ALL_SEND_SYSTEM", 0);
#endif

#ifdef __ARE_YOU_IN_OR_NOT__
	PyModule_AddIntConstant(poModule, "__ARE_YOU_IN_OR_NOT__", 1);
#else
	PyModule_AddIntConstant(poModule, "__ARE_YOU_IN_OR_NOT__", 0);
#endif


#ifdef __AUTO_SKILL_READER__
	PyModule_AddIntConstant(poModule, "__AUTO_SKILL_READER__", 1);
#else
	PyModule_AddIntConstant(poModule, "__AUTO_SKILL_READER__", 0);
#endif

#ifdef __AUTO_SELL__
	PyModule_AddIntConstant(poModule, "__AUTO_SELL__", 1);
#else
	PyModule_AddIntConstant(poModule, "__AUTO_SELL__", 0);
#endif

#ifdef ENABLE_OFFER_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_OFFER_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_OFFER_SYSTEM", 0);
#endif

#ifdef ENABLE_EFFECT_SCALE
	PyModule_AddIntConstant(poModule, "ENABLE_EFFECT_SCALE", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EFFECT_SCALE", 0);
#endif

#ifdef ENABLE_EXCHANGE_LOG
	PyModule_AddIntConstant(poModule, "ENABLE_EXCHANGE_LOG", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EXCHANGE_LOG", 0);
#endif

#ifdef __RENEWAL_NOTICE__
	PyModule_AddIntConstant(poModule, "__RENEWAL_NOTICE__", 1);
#else
	PyModule_AddIntConstant(poModule, "__RENEWAL_NOTICE__", 0);
#endif

#ifdef ENABLE_REFINE_LUCK
	PyModule_AddIntConstant(poModule, "ENABLE_REFINE_LUCK", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_REFINE_LUCK", 0);
#endif

#ifdef ENABLE_KADIM_SILAH
	PyModule_AddIntConstant(poModule, "ENABLE_KADIM_SILAH", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_KADIM_SILAH", 0);
#endif

#ifdef ENABLE_SKILL_TIME
	PyModule_AddIntConstant(poModule, "ENABLE_SKILL_TIME", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SKILL_TIME", 0);
#endif

#ifdef __RENEWAL_BRAVE_CAPE__
	PyModule_AddIntConstant(poModule, "__RENEWAL_BRAVE_CAPE__", 1);
#else
	PyModule_AddIntConstant(poModule, "__RENEWAL_BRAVE_CAPE__", 0);
#endif

#ifdef ENABLE_GUILD_WAR_OPTION_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_WAR_OPTION_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_WAR_OPTION_SYSTEM", 0);
#endif

#ifdef ENABLE_TANAKA_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_TANAKA_EVENT", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_TANAKA_EVENT", 0);
#endif

#ifdef ENABLE_BATTLE_PASS
	PyModule_AddIntConstant(poModule, "ENABLE_BATTLE_PASS", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_BATTLE_PASS", 0);
#endif

#ifdef ENABLE_TRADABLE_ICON
	PyModule_AddIntConstant(poModule, "ENABLE_TRADABLE_ICON", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_TRADABLE_ICON", 0);
#endif

#ifdef ENABLE_TRADABLE_ICON
	PyModule_AddIntConstant(poModule, "WJ_ENABLE_TRADABLE_ICON", 1);
#else
	PyModule_AddIntConstant(poModule, "WJ_ENABLE_TRADABLE_ICON", 0);
#endif

#ifdef ENABLE_TELEPORTION_WIZARD_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_TELEPORTION_WIZARD_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_TELEPORTION_WIZARD_SYSTEM", 0);
#endif

#ifdef ENABLE_CAPTCHA_SELECT_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_CAPTCHA_SELECT_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CAPTCHA_SELECT_SYSTEM", 0);
#endif

#ifdef ENABLE_RENDER_TARGET
	PyModule_AddIntConstant(poModule, "ENABLE_RENDER_TARGET", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_RENDER_TARGET", 0);
#endif

#ifdef ENABLE_CPP_PSM
	PyModule_AddIntConstant(poModule, "ENABLE_CPP_PSM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CPP_PSM", 0);
#endif

#ifdef ENABLE_IKASHOP_RENEWAL
	PyModule_AddIntConstant(poModule, "EXTEND_IKASHOP_PRO", 0);
	PyModule_AddIntConstant(poModule, "EXTEND_IKASHOP_ULTIMATE", 0);
	PyModule_AddIntConstant(poModule, "ENABLE_IKASHOP_RENEWAL", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_IKASHOP_RENEWAL", 0);
#endif

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_SHOP_SEARCH_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SHOP_SEARCH_SYSTEM", 0);
#endif

#ifdef ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE
	PyModule_AddIntConstant(poModule, "ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE", 0);
#endif

#ifdef __AUTO_HUNT__
	PyModule_AddIntConstant(poModule, "__AUTO_HUNT__", 1);
#else
	PyModule_AddIntConstant(poModule, "__AUTO_HUNT__", 0);
#endif

#ifdef ENABLE_EVOLUTION_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_EVOLUTION_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EVOLUTION_SYSTEM", 0);
#endif

#ifdef ENABLE_CHANGELOOK_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGELOOK_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGELOOK_SYSTEM", 0);
#endif

#ifdef ENABLE_LOCKED_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_LOCKED_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_LOCKED_SYSTEM", 0);
#endif

#ifdef __BACK_DUNGEON__
	PyModule_AddIntConstant(poModule, "__BACK_DUNGEON__", 1);
#else
	PyModule_AddIntConstant(poModule, "__BACK_DUNGEON__", 0);
#endif

#ifdef NEW_MOB_TIMER
	PyModule_AddIntConstant(poModule, "NEW_MOB_TIMER", 1);
#else
	PyModule_AddIntConstant(poModule, "NEW_MOB_TIMER", 0);
#endif

#ifdef BOSS_DMG_RANKING
	PyModule_AddIntConstant(poModule, "BOSS_RANKING", 1);
#else
	PyModule_AddIntConstant(poModule, "BOSS_RANKING", 0);
#endif

#ifdef ENABLE_PERMA_BLEND
	PyModule_AddIntConstant(poModule, "ENABLE_PERMA_BLEND", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PERMA_BLEND", 0);
#endif

#ifdef ENABLE_FARM_BLEND
	PyModule_AddIntConstant(poModule, "ENABLE_FARM_BLEND", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_FARM_BLEND", 0);
#endif

#ifdef ENABLE_COSTUME_SET_BONUS
	PyModule_AddIntConstant(poModule, "ENABLE_COSTUME_SET_BONUS", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_COSTUME_SET_BONUS", 0);
#endif

#ifdef ENABLE_POLY_ALL_MOBS_DOG
	PyModule_AddIntConstant(poModule, "ENABLE_POLY_ALL_MOBS_DOG", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_POLY_ALL_MOBS_DOG", 0);
#endif

#ifdef ENABLE_ALIGNMENT_BONUS
	PyModule_AddIntConstant(poModule, "ENABLE_ALIGNMENT_BONUS", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ALIGNMENT_BONUS", 0);
#endif

#ifdef ENABLE_BLACK_MARKET_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_BLACK_MARKET_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_BLACK_MARKET_SYSTEM", 0);
#endif

#ifdef ENABLE_DEFENCE_SHIELD
	PyModule_AddIntConstant(poModule, "ENABLE_DEFENCE_SHIELD", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DEFENCE_SHIELD", 0);
#endif

#ifdef ENABLE_REWARD_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_REWARD_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_REWARD_SYSTEM", 0);
#endif

#ifdef ENABLE_REWARD_SYSTEM
	PyModule_AddIntConstant(poModule, "EXCHANGE_CHECKIN_UPDATE", 1);
#else
	PyModule_AddIntConstant(poModule, "EXCHANGE_CHECKIN_UPDATE", 0);
#endif

#ifdef ENABLE_STANDING_MOUNT
	PyModule_AddIntConstant(poModule, "ENABLE_STANDING_MOUNT", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_STANDING_MOUNT", 0);
#endif

#ifdef ENABLE_GUILD_STATISTICS_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_STATISTICS_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_STATISTICS_SYSTEM", 0);
#endif

#if defined(__BL_LUCKY_BOX__)
	PyModule_AddIntConstant(poModule, "__BL_LUCKY_BOX__", true);
#else
	PyModule_AddIntConstant(poModule, "__BL_LUCKY_BOX__", false);
#endif

#ifdef ENABLE_PUBLISHER_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_PUBLISHER_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PUBLISHER_SYSTEM", 0);
#endif

#ifdef ENABLE_DUNGEON_BOSS_ICON_IN_MAP
	PyModule_AddIntConstant(poModule, "ENABLE_DUNGEON_BOSS_ICON_IN_MAP", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DUNGEON_BOSS_ICON_IN_MAP", 0);
#endif

#ifdef ENABLE_OKEY_KART_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_OKEY_KART_EVENT", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_OKEY_KART_EVENT", 0);
#endif

#ifdef ENABLE_RONARK_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_RONARK_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_RONARK_SYSTEM", 0);
#endif

#ifdef ENABLE_AUTO_ITEM_SELL_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_AUTO_ITEM_SELL_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_AUTO_ITEM_SELL_SYSTEM", 0);
#endif
#ifdef SPIN_WHEEL
	PyModule_AddIntConstant(poModule, "SPIN_WHEEL", 1);
#else
	PyModule_AddIntConstant(poModule, "SPIN_WHEEL", 0);
#endif
#if defined(__BL_CLIP_MASK__)
	PyModule_AddIntConstant(poModule, "__BL_CLIP_MASK__", true);
#else
	PyModule_AddIntConstant(poModule, "__BL_CLIP_MASK__", false);
#endif

}
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
