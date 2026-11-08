#include "StdAfx.h"
#include "../eterBase/Error.h"
#include "../eterlib/Camera.h"
#include "../eterlib/AttributeInstance.h"
#include "../gamelib/AreaTerrain.h"
#include "../EterGrnLib/Material.h"

#ifdef CEF_BROWSER
#include "CefWebBrowser.h"
#else
#include "../CWebBrowser/CWebBrowser.h"
#endif

#include "resource.h"
#include "PythonApplication.h"
#include "PythonCharacterManager.h"

#include "ProcessScanner.h"

#include "CheckLatestFiles.h"

#ifdef ENABLE_SWITCHBOT
#include "PythonSwitchbot.h"
#endif

#ifdef ENABLE_HILE_ENGEL
#include "xor.h"
#endif

#include "../Userinterface/Locale_inc.h"

extern void GrannyCreateSharedDeformBuffer();
extern void GrannyDestroySharedDeformBuffer();

float MIN_FOG = 2400.0f;
double g_specularSpd=0.007f;

CPythonApplication * CPythonApplication::ms_pInstance;

float c_fDefaultCameraRotateSpeed = 1.5f;
float c_fDefaultCameraPitchSpeed = 1.5f;
float c_fDefaultCameraZoomSpeed = 0.05f;

CPythonApplication::CPythonApplication() :
m_bCursorVisible(TRUE),
m_bLiarCursorOn(false),
m_iCursorMode(CURSOR_MODE_HARDWARE),
m_isWindowed(false),
m_isFrameSkipDisable(false),
m_poMouseHandler(NULL),
m_dwUpdateFPS(0),
m_dwRenderFPS(0),
m_fAveRenderTime(0.0f),
m_dwFaceCount(0),
m_fGlobalTime(0.0f),
m_fGlobalElapsedTime(0.0f),
m_dwLButtonDownTime(0),
m_dwLastIdleTime(0)
{
	#ifndef _DEBUG
	SetEterExceptionHandler();
	#endif

	CTimer::Instance().UseCustomTime();
	m_dwWidth = 800;
	m_dwHeight = 600;

	#ifdef ENABLE_DISCORD_STUFF
	m_bFarmIsBlocked = false;
	oldIcon = NULL;
	ZeroMemory(&NID, sizeof(NOTIFYICONDATA));
	#endif

	ms_pInstance = this;
	m_isWindowFullScreenEnable = FALSE;

	m_v3CenterPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_dwStartLocalTime = ELTimer_GetMSec();
	m_tServerTime = 0;
	m_tLocalStartTime = 0;

	m_iPort = 0;
	m_iFPS = 60;

	m_isActivateWnd = false;
	m_isMinimizedWnd = true;

	m_fRotationSpeed = 0.0f;
	m_fPitchSpeed = 0.0f;
	m_fZoomSpeed = 0.0f;

	m_fFaceSpd=0.0f;

	m_dwFaceAccCount=0;
	m_dwFaceAccTime=0;

	m_dwFaceSpdSum=0;
	m_dwFaceSpdCount=0;

	m_FlyingManager.SetMapManagerPtr(&m_pyBackground);

	m_iCursorNum = CURSOR_SHAPE_NORMAL;
	m_iContinuousCursorNum = CURSOR_SHAPE_NORMAL;

	m_isSpecialCameraMode = FALSE;
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed;

	m_iCameraMode = CAMERA_MODE_NORMAL;
	m_fBlendCameraStartTime = 0.0f;
	m_fBlendCameraBlendTime = 0.0f;

	m_iForceSightRange = -1;

	CCameraManager::Instance().AddCamera(EVENT_CAMERA_NUMBER);
	
#ifdef ENABLE_LOG_STATE
	memset(&m_szMainName, 0, sizeof(m_szMainName));
	m_bScanStatus = 0;
	m_bFoundForbiddenID = 0;
#endif

#ifdef ENABLE_HILE_ENGEL
	memset(&m_szMainName2, 0, sizeof(m_szMainName2));
	m_bScanStatus2 = 0;
	m_bFoundForbiddenID2 = 0;
#endif

}

CPythonApplication::~CPythonApplication()
{
}

void CPythonApplication::GetMousePosition(POINT* ppt)
{
	CMSApplication::GetMousePosition(ppt);
}

void CPythonApplication::SetMinFog(float fMinFog)
{
	MIN_FOG = fMinFog;
}

void CPythonApplication::SetFrameSkip(bool isEnable)
{
	if (isEnable)
		m_isFrameSkipDisable=false;
	else
		m_isFrameSkipDisable=true;
}

void CPythonApplication::NotifyHack(const char* c_szFormat, ...)
{
	char szBuf[1024];

	va_list args;
	va_start(args, c_szFormat);
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);
	m_pyNetworkStream.NotifyHack(szBuf);
}

void CPythonApplication::GetInfo(UINT eInfo, std::string* pstInfo)
{
	switch (eInfo)
	{
	case INFO_ACTOR:
		m_kChrMgr.GetInfo(pstInfo);
		break;
	case INFO_EFFECT:
		m_kEftMgr.GetInfo(pstInfo);
		break;
	case INFO_ITEM:
		m_pyItem.GetInfo(pstInfo);
		break;
	case INFO_TEXTTAIL:
		m_pyTextTail.GetInfo(pstInfo);
		break;
	}
}

void CPythonApplication::Abort()
{
	TraceError("============================================================================================================");
	TraceError("Abort!!!!\n\n");

	PostQuitMessage(0);
}

void CPythonApplication::Exit()
{
	PostQuitMessage(0);
}

bool PERF_CHECKER_RENDER_GAME = false;
#ifdef __ANDROID__
void CynexGfx_AfterWorld();
void CynexApp_SetSightRange(int iRange)
{
	CPythonApplication::Instance().SetForceSightRange(iRange);
}
#endif

#ifdef CYNEX_MOBILE
void CynexRenderFootprints();
#endif

void CPythonApplication::RenderGame()
{
	if (!PERF_CHECKER_RENDER_GAME)
	{
		#ifdef ENABLE_RENDER_TARGET
		m_kRenderTargetManager.RenderBackgrounds();
		#endif

		float fAspect=m_kWndMgr.GetAspect();
		float fFarClip=m_pyBackground.GetFarClip();

		#ifdef ENABLE_FOV_OPTION
		float fFOV = m_pySystem.GetFOVLevel();
		m_pyGraphic.SetPerspective(fFOV, fAspect, 100.0, fFarClip);
		#else
		m_pyGraphic.SetPerspective(30.0f, fAspect, 100.0, fFarClip);
		#endif

		CCullingManager::Instance().Process();

		m_kChrMgr.Deform();

		#ifdef ENABLE_RENDER_TARGET
		m_kRenderTargetManager.DeformModels();
		#endif

		m_pyBackground.RenderCharacterShadowToTexture();

		m_pyGraphic.SetGameRenderState();
		m_pyGraphic.PushState();

		{
			LONG lx, ly;
			m_kWndMgr.GetMousePosition(lx, ly);
			m_pyGraphic.SetCursorPosition(lx, ly);
		}

		m_pyBackground.RenderSky();

		m_pyBackground.RenderBeforeLensFlare();

		m_pyBackground.RenderCloud();

		m_pyBackground.BeginEnvironment();
		m_pyBackground.Render();

		m_pyBackground.SetCharacterDirLight();
		m_kChrMgr.Render();

		#ifdef ENABLE_RENDER_TARGET
		m_kRenderTargetManager.RenderModels();
		#endif

		m_pyBackground.SetBackgroundDirLight();
		m_pyBackground.RenderWater();
		m_pyBackground.RenderSnow();
		m_pyBackground.RenderEffect();

		m_pyBackground.EndEnvironment();

		#ifdef ENABLE_IKASHOP_RENEWAL
		m_pyOfflineshop.RenderEntities();
		#endif
		m_kEftMgr.Render();
		m_pyItem.Render();
#ifdef CYNEX_MOBILE
		CynexRenderFootprints();	// AKRAVON: haritadan secilen yolun ayak izleri (yerde)
#endif

		m_FlyingManager.Render();

		m_pyBackground.BeginEnvironment();
		m_pyBackground.RenderPCBlocker();
		m_pyBackground.EndEnvironment();

		m_pyBackground.RenderAfterLensFlare();

#ifdef __ANDROID__
		CynexGfx_AfterWorld();
#endif
		return;
	}

	DWORD t1=ELTimer_GetMSec();
	m_kChrMgr.Deform();
	#ifdef ENABLE_RENDER_TARGET
	m_kRenderTargetManager.DeformModels();
	#endif
	DWORD t2=ELTimer_GetMSec();
	DWORD t3=ELTimer_GetMSec();
	m_pyBackground.RenderCharacterShadowToTexture();
	DWORD t4=ELTimer_GetMSec();

	m_pyGraphic.SetGameRenderState();
	m_pyGraphic.PushState();

	#ifdef ENABLE_RENDER_TARGET
	m_kRenderTargetManager.RenderBackgrounds();
	#endif

	float fAspect=m_kWndMgr.GetAspect();
	float fFarClip=m_pyBackground.GetFarClip();

	#ifdef ENABLE_FOV_OPTION
	float fFOV = m_pySystem.GetFOVLevel();
	m_pyGraphic.SetPerspective(fFOV, fAspect, 100.0, fFarClip);
	#else
	m_pyGraphic.SetPerspective(30.0f, fAspect, 100.0, fFarClip);
	#endif

	DWORD t5=ELTimer_GetMSec();

	CCullingManager::Instance().Process();

	DWORD t6=ELTimer_GetMSec();

	{
		LONG lx, ly;
		m_kWndMgr.GetMousePosition(lx, ly);
		m_pyGraphic.SetCursorPosition(lx, ly);
	}

	m_pyBackground.RenderSky();
	DWORD t7=ELTimer_GetMSec();
	m_pyBackground.RenderBeforeLensFlare();
	DWORD t8=ELTimer_GetMSec();
	m_pyBackground.RenderCloud();
	DWORD t9=ELTimer_GetMSec();
	m_pyBackground.BeginEnvironment();
	m_pyBackground.Render();

	m_pyBackground.SetCharacterDirLight();
	DWORD t10=ELTimer_GetMSec();
	m_kChrMgr.Render();
	#ifdef ENABLE_RENDER_TARGET
	m_kRenderTargetManager.RenderModels();
	#endif
	DWORD t11=ELTimer_GetMSec();

	m_pyBackground.SetBackgroundDirLight();
	m_pyBackground.RenderWater();
	DWORD t12=ELTimer_GetMSec();
	m_pyBackground.RenderEffect();
	DWORD t13=ELTimer_GetMSec();
	m_pyBackground.EndEnvironment();

	m_kEftMgr.Render();
	DWORD t14=ELTimer_GetMSec();
	m_pyItem.Render();
	DWORD t15=ELTimer_GetMSec();
	m_FlyingManager.Render();
	DWORD t16=ELTimer_GetMSec();
	m_pyBackground.BeginEnvironment();
	m_pyBackground.RenderPCBlocker();
	m_pyBackground.EndEnvironment();
	DWORD t17=ELTimer_GetMSec();
	m_pyBackground.RenderAfterLensFlare();
	DWORD t18=ELTimer_GetMSec();
	DWORD tEnd=ELTimer_GetMSec();

	if (GetAsyncKeyState(VK_Z))
		STATEMANAGER.SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	if (tEnd-t1<3)
		return;

	static FILE* fp=fopen("perf_game_render.txt", "w");

	fprintf(fp, "GR.Total %d (Time %d)\n", tEnd-t1, ELTimer_GetMSec());
	fprintf(fp, "GR.DFM %d\n", t2-t1);
	fprintf(fp, "GR.EFT.UP %d\n", t3-t2);
	fprintf(fp, "GR.SHW %d\n", t4-t3);
	fprintf(fp, "GR.STT %d\n", t5-t4);
	fprintf(fp, "GR.CLL %d\n", t6-t5);
	fprintf(fp, "GR.BG.SKY %d\n", t7-t6);
	fprintf(fp, "GR.BG.LEN %d\n", t8-t7);
	fprintf(fp, "GR.BG.CLD %d\n", t9-t8);
	fprintf(fp, "GR.BG.MAIN %d\n", t10-t9);
	fprintf(fp, "GR.CHR %d\n",	t11-t10);
	fprintf(fp, "GR.BG.WTR %d\n", t12-t11);
	fprintf(fp, "GR.BG.EFT %d\n", t13-t12);
	fprintf(fp, "GR.EFT %d\n", t14-t13);
	fprintf(fp, "GR.ITM %d\n", t15-t14);
	fprintf(fp, "GR.FLY %d\n", t16-t15);
	fprintf(fp, "GR.BG.BLK %d\n", t17-t16);
	fprintf(fp, "GR.BG.LEN %d\n", t18-t17);

	fflush(fp);
}

void CPythonApplication::UpdateGame()
{
	DWORD t1=ELTimer_GetMSec();
	POINT ptMouse;
	GetMousePosition(&ptMouse);

	CGraphicTextInstance::Hyperlink_UpdateMousePos(ptMouse.x, ptMouse.y);

	DWORD t2=ELTimer_GetMSec();

	//if (m_isActivateWnd)
	{
		CScreen s;
		float fAspect = UI::CWindowManager::Instance().GetAspect();
		float fFarClip = CPythonBackground::Instance().GetFarClip();

		#ifdef ENABLE_FOV_OPTION
		float fFOV = CPythonSystem::Instance().GetFOVLevel();
		s.SetPerspective(fFOV, fAspect, 100.0f, fFarClip);
		#else
		s.SetPerspective(30.0f,fAspect, 100.0f, fFarClip);
		#endif
		s.BuildViewFrustum();
	}

	#ifdef ENABLE_RENDER_TARGET
	m_kRenderTargetManager.UpdateModels();
	#endif

	DWORD t3=ELTimer_GetMSec();
	TPixelPosition kPPosMainActor;
	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);

	DWORD t4=ELTimer_GetMSec();
	m_pyBackground.Update(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);

	DWORD t5=ELTimer_GetMSec();
	m_GameEventManager.SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
	m_GameEventManager.Update();

	#ifdef ENABLE_IKASHOP_RENEWAL
	m_pyOfflineshop.UpdateEntities();
	#endif

	DWORD t6=ELTimer_GetMSec();
	m_kChrMgr.Update();
	DWORD t7=ELTimer_GetMSec();
	m_kEftMgr.Update(); //@fixme029
	m_kEftMgr.UpdateSound();
	DWORD t8=ELTimer_GetMSec();
	m_FlyingManager.Update();
	DWORD t9=ELTimer_GetMSec();
	m_pyItem.Update(ptMouse);
	DWORD t10=ELTimer_GetMSec();
	m_pyPlayer.Update();
	DWORD t11=ELTimer_GetMSec();

	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);
	SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
	DWORD t12=ELTimer_GetMSec();

	if (PERF_CHECKER_RENDER_GAME)
	{
		if (t12-t1>5)
		{
			static FILE* fp=fopen("perf_game_update.txt", "w");

			fprintf(fp, "GU.Total %d (Time %d)\n", t12-t1, ELTimer_GetMSec());
			fprintf(fp, "GU.GMP %d\n", t2-t1);
			fprintf(fp, "GU.SCR %d\n", t3-t2);
			fprintf(fp, "GU.MPS %d\n", t4-t3);
			fprintf(fp, "GU.BG %d\n", t5-t4);
			fprintf(fp, "GU.GEM %d\n", t6-t5);
			fprintf(fp, "GU.CHR %d\n", t7-t6);
			fprintf(fp, "GU.EFT %d\n", t8-t7);
			fprintf(fp, "GU.FLY %d\n", t9-t8);
			fprintf(fp, "GU.ITM %d\n", t10-t9);
			fprintf(fp, "GU.PLR %d\n", t11-t10);
			fprintf(fp, "GU.POS %d\n", t12-t11);
			fflush(fp);
		}
	}
}

void CPythonApplication::SkipRenderBuffering(DWORD dwSleepMSec)
{
	m_dwBufSleepSkipTime=ELTimer_GetMSec()+dwSleepMSec;
}

bool CPythonApplication::Process()
{
	#if defined(CHECK_LATEST_DATA_FILES)
	if (CheckLatestFiles_PollEvent())
		return false;
	#endif

	ELTimer_SetFrameMSec();

	// 	m_Profiler.Clear();
	DWORD dwStart = ELTimer_GetMSec();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	static DWORD	s_dwUpdateFrameCount = 0;
	static DWORD	s_dwRenderFrameCount = 0;
	static DWORD	s_dwFaceCount = 0;
	static UINT		s_uiLoad = 0;
	static DWORD	s_dwCheckTime = ELTimer_GetMSec();

	if (ELTimer_GetMSec() - s_dwCheckTime > 1000)
	{
		m_dwUpdateFPS		= s_dwUpdateFrameCount;
		m_dwRenderFPS		= s_dwRenderFrameCount;
		m_dwLoad			= s_uiLoad;

		m_dwFaceCount		= s_dwFaceCount / max(1, s_dwRenderFrameCount);

		s_dwCheckTime		= ELTimer_GetMSec();

		s_uiLoad = s_dwFaceCount = s_dwUpdateFrameCount = s_dwRenderFrameCount = 0;
	}

#ifdef ENABLE_LOG_STATE
	static DWORD	s_dwCheckTime2 = ELTimer_GetMSec();
	if (ELTimer_GetMSec() - s_dwCheckTime2 > 5000)
	{
		ScanWindowTitles();
		s_dwCheckTime2 = ELTimer_GetMSec();
	}
#endif

#ifdef ENABLE_HILE_ENGEL
	static DWORD	s_dwCheckTime3 = ELTimer_GetMSec();
	if (ELTimer_GetMSec() - s_dwCheckTime3 > 2000)
	{
		ScanWindowTitles2();
		s_dwCheckTime3 = ELTimer_GetMSec();
	}
#endif

	// Update Time
	static BOOL s_bFrameSkip = false;
	static UINT s_uiNextFrameTime = ELTimer_GetMSec();

#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime1=ELTimer_GetMSec();
#endif
	CTimer& rkTimer=CTimer::Instance();
	rkTimer.Advance();

	m_fGlobalTime = rkTimer.GetCurrentSecond();
	m_fGlobalElapsedTime = rkTimer.GetElapsedSecond();

	UINT uiFrameTime = rkTimer.GetElapsedMilliecond();
	s_uiNextFrameTime += uiFrameTime;

	DWORD updatestart = ELTimer_GetMSec();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime2=ELTimer_GetMSec();
#endif
	// Network I/O
	m_pyNetworkStream.Process();
	//m_pyNetworkDatagram.Process();

	m_kGuildMarkUploader.Process();

	m_kGuildMarkDownloader.Process();
	m_kAccountConnector.Process();
	DWORD cxNet = ELTimer_GetMSec();

#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime3=ELTimer_GetMSec();
#endif
	//////////////////////
	// Input Process
	// Keyboard
	UpdateKeyboard();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime4=ELTimer_GetMSec();
#endif
	// Mouse
	POINT Point;
	if (GetCursorPos(&Point))
	{
		ScreenToClient(m_hWnd, &Point);
		OnMouseMove(Point.x, Point.y);
	}
	//////////////////////
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime5=ELTimer_GetMSec();
#endif
	//if (m_isActivateWnd)
	__UpdateCamera();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime6=ELTimer_GetMSec();
#endif
	// Update Game Playing
	CResourceManager::Instance().Update();
	DWORD cxRes = ELTimer_GetMSec();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime7=ELTimer_GetMSec();
#endif
	OnCameraUpdate();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime8=ELTimer_GetMSec();
#endif
	OnMouseUpdate();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime9=ELTimer_GetMSec();
#endif
	OnUIUpdate();

#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime10=ELTimer_GetMSec();

	if (dwUpdateTime10-dwUpdateTime1>10)
	{
		static FILE* fp=fopen("perf_app_update.txt", "w");

		fprintf(fp, "AU.Total %d (Time %d)\n", dwUpdateTime9-dwUpdateTime1, ELTimer_GetMSec());
		fprintf(fp, "AU.TU %d\n", dwUpdateTime2-dwUpdateTime1);
		fprintf(fp, "AU.NU %d\n", dwUpdateTime3-dwUpdateTime2);
		fprintf(fp, "AU.KU %d\n", dwUpdateTime4-dwUpdateTime3);
		fprintf(fp, "AU.MP %d\n", dwUpdateTime5-dwUpdateTime4);
		fprintf(fp, "AU.CP %d\n", dwUpdateTime6-dwUpdateTime5);
		fprintf(fp, "AU.RU %d\n", dwUpdateTime7-dwUpdateTime6);
		fprintf(fp, "AU.CU %d\n", dwUpdateTime8-dwUpdateTime7);
		fprintf(fp, "AU.MU %d\n", dwUpdateTime9-dwUpdateTime8);
		fprintf(fp, "AU.UU %d\n", dwUpdateTime10-dwUpdateTime9);
		fprintf(fp, "----------------------------------\n");
		fflush(fp);
	}
#endif

	m_dwCurUpdateTime = ELTimer_GetMSec() - updatestart;
	{
		// Cynex: kare suresi dagilimi (logcat -s CynexOyun | grep perf)
		static DWORD s_t0 = 0, s_n = 0, s_net = 0, s_res = 0, s_rest = 0;
		DWORD cxNow = ELTimer_GetMSec();
		s_net += cxNet - updatestart; s_res += cxRes - cxNet; s_rest += cxNow - cxRes; ++s_n;
		if (cxNow - s_t0 > 3000)
		{
			TraceError("perf kare=%u ag=%u giris+kaynak=%u kamera+ui=%u ms/kare cizim=%.1f", s_n, s_net / s_n, s_res / s_n, s_rest / s_n, m_fAveRenderTime);
			s_t0 = cxNow; s_n = s_net = s_res = s_rest = 0;
		}
	}

	DWORD dwCurrentTime = ELTimer_GetMSec();
	BOOL  bCurrentLateUpdate = FALSE;

	s_bFrameSkip = false;

	if (dwCurrentTime > s_uiNextFrameTime)
	{
		int dt = dwCurrentTime - s_uiNextFrameTime;
		int nAdjustTime = ((float)dt / (float)uiFrameTime) * uiFrameTime;

		if ( dt >= 500 )
		{
			s_uiNextFrameTime += nAdjustTime;
			printf("FrameSkip Adjusting... %d\n",nAdjustTime);
			CTimer::Instance().Adjust(nAdjustTime);
		}
		s_bFrameSkip = true;
		bCurrentLateUpdate = TRUE;
	}

	if (m_isMinimizedWnd)
		CEffectManager::Instance().Update();

	if (m_isFrameSkipDisable && !m_isMinimizedWnd)
		s_bFrameSkip = false;

	if (!s_bFrameSkip)
	{
		CGrannyMaterial::TranslateSpecularMatrix(g_specularSpd, g_specularSpd, 0.0f);

		DWORD dwRenderStartTime = ELTimer_GetMSec();

		bool canRender = true;

		if (m_isMinimizedWnd)
		{
			canRender = false;
		}
		else
		{
			#ifdef ENABLE_FIX_MOBS_LAG
			if (DEVICE_STATE_OK != CheckDeviceState())
			{
				canRender = false;

				CPythonBackground& rkBG = CPythonBackground::Instance();
				#ifdef ENABLE_RENDER_TARGET
				CRenderTargetManager::Instance().ReleaseRenderTargetTextures();
				#endif

				if (m_pyGraphic.RestoreDevice())
				{
					#ifdef ENABLE_RENDER_TARGET
					CRenderTargetManager::Instance().CreateRenderTargetTextures();
					#endif
					rkBG.CreateCharacterShadowTexture();
				}
				else
				{
					canRender = false;
				}
			}
			#else
			if (m_pyGraphic.IsLostDevice())
			{
				CPythonBackground& rkBG = CPythonBackground::Instance();
				rkBG.ReleaseCharacterShadowTexture();

				#ifdef ENABLE_RENDER_TARGET
				CRenderTargetManager::Instance().ReleaseRenderTargetTextures();
				#endif

				if (m_pyGraphic.RestoreDevice())
				{
					#ifdef ENABLE_RENDER_TARGET
					CRenderTargetManager::Instance().CreateRenderTargetTextures();
					#endif
					rkBG.CreateCharacterShadowTexture();
				}
				else
				{
					canRender = false;
				}
			}
			#endif
		}

		if (!IsActive())
		{
			SkipRenderBuffering(3000);
		}

		if (!canRender)
		{
			SkipRenderBuffering(3000);
		}
		else
		{
			// RestoreLostDevice
			CCullingManager::Instance().Update();
			DWORD cxR0 = ELTimer_GetMSec();
			if (m_pyGraphic.Begin())
			{

				m_pyGraphic.ClearDepthBuffer();

#ifdef _DEBUG
				m_pyGraphic.SetClearColor(0.3f, 0.3f, 0.3f);
				m_pyGraphic.Clear();
#endif

				/////////////////////
				// Interface
				m_pyGraphic.SetInterfaceRenderState();

				OnUIRender();
				OnMouseRender();
				DWORD cxR1 = ELTimer_GetMSec();
				/////////////////////

				m_pyGraphic.End();
				DWORD cxR2 = ELTimer_GetMSec();

				m_pyGraphic.Show();
				{
					static DWORD s_p0 = 0, s_pn = 0, s_a = 0, s_b = 0, s_c = 0;
					DWORD cxR3 = ELTimer_GetMSec();
					s_a += cxR1 - cxR0; s_b += cxR2 - cxR1; s_c += cxR3 - cxR2; ++s_pn;
					if (cxR3 - s_p0 > 3000)
					{
						TraceError("perf cizim kare=%u cizim=%u bitis=%u ekrana_ver=%u ms/kare", s_pn, s_a / s_pn, s_b / s_pn, s_c / s_pn);
						s_p0 = cxR3; s_pn = s_a = s_b = s_c = 0;
					}
				}

				DWORD dwRenderEndTime = ELTimer_GetMSec();

				static DWORD s_dwRenderCheckTime = dwRenderEndTime;
				static DWORD s_dwRenderRangeTime = 0;
				static DWORD s_dwRenderRangeFrame = 0;

				m_dwCurRenderTime = dwRenderEndTime - dwRenderStartTime;
				s_dwRenderRangeTime += m_dwCurRenderTime;
				++s_dwRenderRangeFrame;

				if (dwRenderEndTime-s_dwRenderCheckTime>1000)
				{
					m_fAveRenderTime=float(double(s_dwRenderRangeTime)/double(s_dwRenderRangeFrame));

					s_dwRenderCheckTime=ELTimer_GetMSec();
					s_dwRenderRangeTime=0;
					s_dwRenderRangeFrame=0;
				}

				DWORD dwCurFaceCount=m_pyGraphic.GetFaceCount();
				m_pyGraphic.ResetFaceCount();
				s_dwFaceCount += dwCurFaceCount;

				if (dwCurFaceCount > 5000)
				{
					if (dwRenderEndTime > m_dwBufSleepSkipTime)
					{
						static float s_fBufRenderTime = 0.0f;

						float fCurRenderTime = m_dwCurRenderTime;

						if (fCurRenderTime > s_fBufRenderTime)
						{
							float fRatio = fMAX(0.5f, (fCurRenderTime - s_fBufRenderTime) / 30.0f);
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + (fCurRenderTime + 5) * fRatio) / 100.0f;
						}
						else
						{
							float fRatio = 0.5f;
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + fCurRenderTime * fRatio) / 100.0f;
						}

						if (s_fBufRenderTime > 100.0f)
							s_fBufRenderTime = 100.0f;

						DWORD dwBufRenderTime = s_fBufRenderTime;

						if (m_isWindowed)
						{
							if (dwBufRenderTime>58)
								dwBufRenderTime=64;
							else if (dwBufRenderTime>42)
								dwBufRenderTime=48;
							else if (dwBufRenderTime>26)
								dwBufRenderTime=32;
							else if (dwBufRenderTime>10)
								dwBufRenderTime=16;
							else
								dwBufRenderTime=8;
						}

						m_fAveRenderTime=s_fBufRenderTime;
					}

					m_dwFaceAccCount += dwCurFaceCount;
					m_dwFaceAccTime += m_dwCurRenderTime;

					m_fFaceSpd=(m_dwFaceAccCount/m_dwFaceAccTime);

#ifdef __ANDROID__
					// Adaptive sight range shrinks the sky dome toward the camera on mobile; keep it fixed.
					if (-1 == m_iForceSightRange)
					{
						m_pyBackground.SetViewDistanceSet(0, 25600.0f);
					}
#else
					if (-1 == m_iForceSightRange)
					{
						static float s_fAveRenderTime = 16.0f;
						float fRatio=0.3f;
						s_fAveRenderTime=(s_fAveRenderTime*(100.0f-fRatio)+max(16.0f, m_dwCurRenderTime)*fRatio)/100.0f;


						float fFar=25600.0f;
						float fNear=MIN_FOG;
						double dbAvePow=double(1000.0f/s_fAveRenderTime);
						double dbMaxPow=60.0;
						float fDistance=max(fNear+(fFar-fNear)*(dbAvePow)/dbMaxPow, fNear);
						m_pyBackground.SetViewDistanceSet(0, fDistance);
					}
#endif
					else
					{
						m_pyBackground.SetViewDistanceSet(0, float(m_iForceSightRange));
					}
				}
				else
				{
					m_pyBackground.SetViewDistanceSet(0, 25600.0f);
				}

				++s_dwRenderFrameCount;
			}
		}
	}



	int rest = s_uiNextFrameTime - ELTimer_GetMSec();

	if (rest > 0 && !bCurrentLateUpdate )
	{
		s_uiLoad -= rest;
		Sleep(rest);
	}

	++s_dwUpdateFrameCount;

	s_uiLoad += ELTimer_GetMSec() - dwStart;
	//m_Profiler.ProfileByScreen();
	return true;
}

void CPythonApplication::UpdateClientRect()
{
	RECT rcApp;
	GetClientRect(&rcApp);
	OnSizeChange(rcApp.right - rcApp.left, rcApp.bottom - rcApp.top);
}

void CPythonApplication::SetMouseHandler(PyObject* poMouseHandler)
{
	m_poMouseHandler = poMouseHandler;
}

int CPythonApplication::CheckDeviceState()
{
	CGraphicDevice::EDeviceState e_deviceState = m_grpDevice.GetDeviceState();

	switch (e_deviceState)
	{
	case CGraphicDevice::DEVICESTATE_NULL:
		return DEVICE_STATE_FALSE;

	case CGraphicDevice::DEVICESTATE_BROKEN:
		return DEVICE_STATE_SKIP;

	#ifdef ENABLE_FIX_MOBS_LAG
	case CGraphicDevice::DEVICESTATE_NEEDS_RESET:
		m_pyBackground.ReleaseCharacterShadowTexture();
		#ifdef ENABLE_RENDER_TARGET
		CRenderTargetManager::Instance().ReleaseRenderTargetTextures();
		#endif

		Trace("DEVICESTATE_NEEDS_RESET - attempting");
		if (!m_grpDevice.Reset())
		{
			return DEVICE_STATE_SKIP;
		}

		#ifdef ENABLE_RENDER_TARGET
		CRenderTargetManager::Instance().CreateRenderTargetTextures();
		#endif
		m_pyBackground.CreateCharacterShadowTexture();
		break;

	case CGraphicDevice::DEVICESTATE_OK: break;
	default:;
	#else
	case CGraphicDevice::DEVICESTATE_NEEDS_RESET:
		if (!m_grpDevice.Reset())
			return DEVICE_STATE_SKIP;
		#ifdef ENABLE_RENDER_TARGET
		CRenderTargetManager::Instance().CreateRenderTargetTextures();
		#endif
		break;
	#endif
	}

	return DEVICE_STATE_OK;
}

bool CPythonApplication::CreateDevice(int width, int height, int Windowed, int bit /* = 32*/, int frequency /* = 0*/)

{
	int iRet;

	m_grpDevice.InitBackBufferCount(2);
	m_grpDevice.RegisterWarningString(CGraphicDevice::CREATE_BAD_DRIVER, ApplicationStringTable_GetStringz(IDS_WARN_BAD_DRIVER, "WARN_BAD_DRIVER"));
	m_grpDevice.RegisterWarningString(CGraphicDevice::CREATE_NO_TNL, ApplicationStringTable_GetStringz(IDS_WARN_NO_TNL, "WARN_NO_TNL"));

	iRet = m_grpDevice.Create(GetWindowHandle(), width, height, Windowed ? true : false, bit,frequency);

	switch (iRet)
	{
	case CGraphicDevice::CREATE_OK:
		return true;

	case CGraphicDevice::CREATE_REFRESHRATE:
		return true;

	case CGraphicDevice::CREATE_ENUM:
	case CGraphicDevice::CREATE_DETECT:
		SET_EXCEPTION(CREATE_NO_APPROPRIATE_DEVICE);
		TraceError("CreateDevice: Enum & Detect failed");
		return false;

	case CGraphicDevice::CREATE_NO_DIRECTX:
		SET_EXCEPTION(CREATE_NO_DIRECTX);
		TraceError("CreateDevice: DirectX 8.1 or greater required to run game");
		return false;

	case CGraphicDevice::CREATE_DEVICE:
		SET_EXCEPTION(CREATE_DEVICE);
		TraceError("CreateDevice: GraphicDevice create failed");
		return false;

	case CGraphicDevice::CREATE_FORMAT:
		SET_EXCEPTION(CREATE_FORMAT);
		TraceError("CreateDevice: Change the screen format");
		return false;

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps failed");
		TraceError("CreateDevice: GetDevCaps failed");
		return false;

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS2:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps2 failed");
		TraceError("CreateDevice: GetDevCaps2 failed");
		return false;

	default:
		if (iRet & CGraphicDevice::CREATE_OK)
		{
			if (iRet & CGraphicDevice::CREATE_NO_TNL)
			{
				CGrannyLODController::SetMinLODMode(true);
				//LogBox(ApplicationStringTable_GetStringz(IDS_WARN_NO_TNL), NULL, GetWindowHandle());
			}
			return true;
		}

		SET_EXCEPTION(UNKNOWN_ERROR);
		TraceError("CreateDevice: Unknown Error!");
		return false;
	}
}

void CPythonApplication::Loop()
{
#ifdef __APPLE__
	fprintf(stderr, "[T] Loop basladi\n"); fflush(stderr);
	unsigned int cynexIter = 0;
#endif
	while (1)
	{
		if (IsMessage())
		{
			if (!MessageProcess())
			{
#ifdef __APPLE__
				fprintf(stderr, "[T] Loop cikis: MessageProcess false (tur=%u)\n", cynexIter); fflush(stderr);
#endif
				break;
			}
		}
		else
		{
			if (!Process())
			{
#ifdef __APPLE__
				fprintf(stderr, "[T] Loop cikis: Process false (tur=%u)\n", cynexIter); fflush(stderr);
#endif
				break;
			}

			m_dwLastIdleTime=ELTimer_GetMSec();
		}
#ifdef __APPLE__
		if (++cynexIter == 1 || cynexIter == 120)
			{ fprintf(stderr, "[T] Loop tur %u\n", cynexIter); fflush(stderr); }
#endif
	}
}

//#define ENABLE_LOAD_ITEM_LIST_FROM_ROOT
bool LoadLocaleData(const char* localePath)
{
	CPythonNonPlayer&	rkNPCMgr	= CPythonNonPlayer::Instance();
	CItemManager&		rkItemMgr	= CItemManager::Instance();
	CPythonSkill&		rkSkillMgr	= CPythonSkill::Instance();
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();

	char szItemList[256];
	char szItemProto[256];
	char szItemDesc[256];
	char szMobProto[256];
	char szSkillDescFileName[256];
	char szSkillTableFileName[256];
	char szInsultList[256];
	snprintf(szItemList, sizeof(szItemList), "locale/common/item_list.txt");
	snprintf(szItemProto,	sizeof(szItemProto),	"%s/item_proto",	localePath);
	snprintf(szItemDesc,	sizeof(szItemDesc),	"%s/itemdesc.txt",	localePath);
	snprintf(szMobProto,	sizeof(szMobProto),	"%s/mob_proto",		localePath);
	snprintf(szSkillDescFileName, sizeof(szSkillDescFileName),	"%s/SkillDesc.txt", localePath);
	snprintf(szSkillTableFileName, sizeof(szSkillTableFileName), "locale/common/SkillTable.txt");
	snprintf(szInsultList,	sizeof(szInsultList),	"%s/insult.txt", localePath);

	rkNPCMgr.Destroy();
	rkItemMgr.Destroy();
	rkSkillMgr.Destroy();

	if (!rkItemMgr.LoadItemList(szItemList))
	{
		TraceError("LoadLocaleData - LoadItemList(%s) Error", szItemList);
	}

	if (!rkItemMgr.LoadItemTable(szItemProto))
	{
		TraceError("LoadLocaleData - LoadItemProto(%s) Error", szItemProto);
		return false;
	}

	if (!rkItemMgr.LoadItemDesc(szItemDesc))
	{
		Tracenf("LoadLocaleData - LoadItemDesc(%s) Error", szItemDesc);
	}

	if (!rkNPCMgr.LoadNonPlayerData(szMobProto))
	{
		TraceError("LoadLocaleData - LoadMobProto(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillDesc(szSkillDescFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillDesc(%s) Error", szSkillDescFileName); //@warme670
		return false;
	}

	if (!rkSkillMgr.RegisterSkillTable(szSkillTableFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillTable(%s) Error", szSkillTableFileName); //@warme670
		return false;
	}

	if (!rkNetStream.LoadInsultList(szInsultList))
	{
		Tracenf("CPythonApplication - CPythonNetworkStream::LoadInsultList(%s)", szInsultList);
	}

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	char szItemScale[256]{};
	snprintf(szItemScale, sizeof(szItemScale), "locale/common/item_scale.txt");

	if (!rkItemMgr.LoadItemScale(szItemScale))
	{
		Tracenf("LoadLocaleData: error while loading %s.", szItemScale);
		return false;
	}
#endif

	return true;
}

unsigned __GetWindowMode(bool windowed)
{
	if (windowed)
		return WS_OVERLAPPED | WS_CAPTION |   WS_SYSMENU | WS_MINIMIZEBOX;

	return WS_POPUP;
}

#ifdef ENABLE_DISCORD_STUFF
bool CPythonApplication::Create(PyObject* poSelf, const char* c_szName, const char* c_szMiniName, int width, int height, int Windowed)
#else
bool CPythonApplication::Create(PyObject* poSelf, const char* c_szName, int width, int height, int Windowed)
#endif
{
	Windowed = CPythonSystem::Instance().IsWindowed() ? 1 : 0;

	bool bAnotherWindow = false;

	if (FindWindow(NULL, c_szName))
		bAnotherWindow = true;

	m_dwWidth = width;
	m_dwHeight = height;

	// Window
	UINT WindowMode = __GetWindowMode(Windowed ? true : false);

	#ifdef ENABLE_DISCORD_STUFF
	oldIcon = LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_METIN2));
	if (!CMSWindow::Create(c_szName, 4, 0, WindowMode, oldIcon, IDC_CURSOR_NORMAL))
	#else
	if (!CMSWindow::Create(c_szName, 4, 0, WindowMode, ::LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_METIN2)), IDC_CURSOR_NORMAL))
	#endif
	{
		TraceError("CMSWindow::Create failed");
		SET_EXCEPTION(CREATE_WINDOW);
		return false;
	}

	if (m_pySystem.IsUseDefaultIME())
		CPythonIME::Instance().UseDefaultIME();

	#ifdef ENABLE_DISCORD_RPC
	m_pyNetworkStream.Discord_Start();
	#endif

	if (!m_pySystem.IsWindowed())
	{
		m_isWindowed = false;
		m_isWindowFullScreenEnable = TRUE;
		__SetFullScreenWindow(GetWindowHandle(), width, height, m_pySystem.GetBPP());

		Windowed = true;
	}
	else
	{
		AdjustSize(m_pySystem.GetWidth(), m_pySystem.GetHeight());

		if (Windowed)
		{
			m_isWindowed = true;
			// @fixme031 BEGIN
			RECT rc{};
			GetClientRect(&rc);
			const auto windowWidth = rc.right - rc.left;
			const auto windowHeight = (rc.bottom - rc.top);
			RECT rc2{};
			GetWindowRect(&rc2);
			const auto windowWidth2 = rc2.right - rc2.left;
			const auto windowHeight2 = (rc2.bottom - rc2.top);
			const auto windowWidthDiff = windowWidth2 - windowWidth;
			const auto windowHeightDiff = windowHeight2 - windowHeight;
			const auto dropshadowSize = (windowWidthDiff / 2 != 0) ? (windowWidthDiff / 2 - 1) : 0;
			constexpr auto taskbarSize = 73;
			constexpr auto titlebarSize = 10;
			if (bAnotherWindow)
				CMSApplication::SetPosition(GetScreenWidth() - windowWidth - dropshadowSize, GetScreenHeight() - windowHeight - taskbarSize);
			else
				SetPosition(-dropshadowSize, (m_pySystem.GetHeight() >= 1000) ? -titlebarSize : 0);
			// @fixme031 END
		}
		else
		{
			m_isWindowed = false;
			SetPosition(0, 0);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Cursor
	if (!CreateCursors())
	{
		TraceError("CMSWindow::Cursors Create Error");
		SET_EXCEPTION("CREATE_CURSOR");
		return false;
	}

	if (!m_pySystem.IsNoSoundCard())
	{
		// Sound
		if (!m_SoundManager.Create())
		{
			// LogBox(ApplicationStringTable_GetStringz(IDS_WARN_NO_SOUND_DEVICE));
		}
	}

	extern bool GRAPHICS_CAPS_SOFTWARE_TILING;

	if (!m_pySystem.IsAutoTiling())
		GRAPHICS_CAPS_SOFTWARE_TILING = m_pySystem.IsSoftwareTiling();

	// Device
	if (!CreateDevice(m_pySystem.GetWidth(), m_pySystem.GetHeight(), Windowed, m_pySystem.GetBPP(), m_pySystem.GetFrequency()))
		return false;

	GrannyCreateSharedDeformBuffer();

	if (m_pySystem.IsAutoTiling())
	{
		if (m_grpDevice.IsFastTNL())
			m_pyBackground.ReserveSoftwareTilingEnable(false);
		else
			m_pyBackground.ReserveSoftwareTilingEnable(true);
	}
	else
		m_pyBackground.ReserveSoftwareTilingEnable(m_pySystem.IsSoftwareTiling());

	SetVisibleMode(true);

	if (m_isWindowFullScreenEnable)
		SetWindowPos(GetWindowHandle(), HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);

	if (!InitializeKeyboard(GetWindowHandle()))
		return false;

	m_pySystem.GetDisplaySettings();

	// Mouse
	if (m_pySystem.IsSoftwareCursor())
		SetCursorMode(CURSOR_MODE_SOFTWARE);
	else
		SetCursorMode(CURSOR_MODE_HARDWARE);

	// Network
	if (!m_netDevice.Create())
	{
		//PyErr_SetString(PyExc_RuntimeError, "NetDevice::Create failed");
		TraceError("NetDevice::Create failed");
		SET_EXCEPTION("CREATE_NETWORK");
		return false;
	}

	if (!m_grpDevice.IsFastTNL())
		CGrannyLODController::SetMinLODMode(true);

	m_pyItem.Create();

	// Other Modules
	DefaultFont_Startup();

	CPythonIME::Instance().Create(GetWindowHandle());
	CPythonIME::Instance().SetText("", 0);
	CPythonTextTail::Instance().Initialize();

	// Light Manager
	m_LightManager.Initialize();

	CGraphicImageInstance::CreateSystem(32);

	#ifdef ENABLE_DISCORD_STUFF
	NID.cbSize = sizeof(NID);
	NID.hIcon = oldIcon;
	NID.hWnd = GetWindowHandle();
	NID.uFlags = NIF_ICON | NIF_TIP;
	strcpy(NID.szTip, c_szMiniName);
	NID.uCallbackMessage = WM_APP + 1;
	NID.hBalloonIcon = NID.hIcon;
	Shell_NotifyIcon(NIM_ADD, &NID);
	#endif

	STICKYKEYS sStickKeys;
	memset(&sStickKeys, 0, sizeof(sStickKeys));
	sStickKeys.cbSize = sizeof(sStickKeys);
	SystemParametersInfo( SPI_GETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
	m_dwStickyKeysFlag = sStickKeys.dwFlags;

	sStickKeys.dwFlags &= ~(SKF_AVAILABLE|SKF_HOTKEYACTIVE);
	SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );

	// SphereMap
	CGrannyMaterial::CreateSphereMap(0, "d:/ymir work/special/spheremap.jpg");
	CGrannyMaterial::CreateSphereMap(1, "d:/ymir work/special/spheremap01.jpg");
	return true;
}

void CPythonApplication::SetGlobalCenterPosition(LONG x, LONG y)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	rkBG.GlobalPositionToLocalPosition(x, y);

	float z = CPythonBackground::Instance().GetHeight(x, y);

	CPythonApplication::Instance().SetCenterPosition(x, y, z);
}

void CPythonApplication::SetCenterPosition(float fx, float fy, float fz)
{
	m_v3CenterPosition.x = +fx;
	m_v3CenterPosition.y = -fy;
	m_v3CenterPosition.z = +fz;
}

void CPythonApplication::GetCenterPosition(TPixelPosition * pPixelPosition)
{
	pPixelPosition->x = +m_v3CenterPosition.x;
	pPixelPosition->y = -m_v3CenterPosition.y;
	pPixelPosition->z = +m_v3CenterPosition.z;
}

void CPythonApplication::SetServerTime(time_t tTime)
{
	m_dwStartLocalTime	= ELTimer_GetMSec();
	m_tServerTime		= tTime;
	m_tLocalStartTime	= time(0);
}

time_t CPythonApplication::GetServerTime()
{
	return (ELTimer_GetMSec() - m_dwStartLocalTime) + m_tServerTime;
}

time_t CPythonApplication::GetServerTimeStamp()
{
	return (time(0) - m_tLocalStartTime) + m_tServerTime;
}

float CPythonApplication::GetGlobalTime()
{
	return m_fGlobalTime;
}

float CPythonApplication::GetGlobalElapsedTime()
{
	return m_fGlobalElapsedTime;
}

void CPythonApplication::SetFPS(int iFPS)
{
	m_iFPS = iFPS;
}

int CPythonApplication::GetWidth()
{
	return m_dwWidth;
}

int CPythonApplication::GetHeight()
{
	return m_dwHeight;
}

void CPythonApplication::SetConnectData(const char * c_szIP, int iPort)
{
	m_strIP = c_szIP;
	m_iPort = iPort;
}

void CPythonApplication::GetConnectData(std::string & rstIP, int & riPort)
{
	rstIP	= m_strIP;
	riPort	= m_iPort;
}

void CPythonApplication::EnableSpecialCameraMode()
{
	m_isSpecialCameraMode = TRUE;
}

void CPythonApplication::SetCameraSpeed(int iPercentage)
{
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed * float(iPercentage) / 100.0f;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed * float(iPercentage) / 100.0f;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed * float(iPercentage) / 100.0f;
}

void CPythonApplication::SetForceSightRange(int iRange)
{
	m_iForceSightRange = iRange;
}

void CPythonApplication::Clear()
{
	m_pySystem.Clear();
}

#ifdef ENABLE_FLASH_APPLICATION
#include <Windows.h>
void CPythonApplication::FlashApplication()
{
	HWND hWnd = GetWindowHandle();
	FLASHWINFO fi;
	fi.cbSize = sizeof(FLASHWINFO);
	fi.hwnd = hWnd;
	fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
	fi.uCount = 0;
	fi.dwTimeout = 0;
	FlashWindowEx(&fi);
}
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
void CPythonApplication::MultiFarmBlockIcon(BYTE bStatus)
{
	#ifdef ENABLE_DISCORD_STUFF
	m_bFarmIsBlocked = !bStatus;
	ChangeIcon();
	#else
	HICON exeIcon = bStatus ? LoadIcon(ms_hInstance, MAKEINTRESOURCE(IDI_METIN2)) : LoadIcon(ms_hInstance, MAKEINTRESOURCE(BLOCK_METIN2));
	SendMessage(GetWindowHandle(), WM_SETICON, ICON_BIG, (LPARAM)exeIcon);
	#endif
}
#endif

void CPythonApplication::Destroy()
{
	#ifdef CEF_BROWSER
	CefWebBrowser_Destroy();
	#else
	WebBrowser_Destroy();
	#endif

	#ifdef ENABLE_IKASHOP_RENEWAL
	CPythonIkarusShop::instance().DeleteEntities();
	#endif

	// SphereMap
	CGrannyMaterial::DestroySphereMap();

	#ifdef ENABLE_DISCORD_STUFF
	if (oldIcon != NULL)
		DestroyIcon(oldIcon);
	oldIcon = NULL;

	if (NID.hIcon != NULL)
		DestroyIcon(NID.hIcon);
	NID.hIcon = NULL;
	Shell_NotifyIcon(NIM_DELETE, &NID);
	ZeroMemory(&NID, sizeof(NOTIFYICONDATA));
	#endif

	m_kWndMgr.Destroy();

	CPythonSystem::Instance().SaveConfig();

	DestroyCollisionInstanceSystem();

	m_pySystem.SaveInterfaceStatus();

	m_pyEventManager.Destroy();
	m_FlyingManager.Destroy();

	m_pyMiniMap.Destroy();

	m_pyTextTail.Destroy();
	m_pyChat.Destroy();
	m_kChrMgr.Destroy();
	#ifdef ENABLE_RENDER_TARGET
	m_kRenderTargetManager.Destroy();
	#endif
	m_RaceManager.Destroy();

	m_pyItem.Destroy();
	m_kItemMgr.Destroy();

	m_pyBackground.Destroy();

	m_kEftMgr.Destroy();
	m_LightManager.Destroy();

	// DEFAULT_FONT
	DefaultFont_Cleanup();
	// END_OF_DEFAULT_FONT

	GrannyDestroySharedDeformBuffer();

	m_pyGraphic.Destroy();



	#ifdef ENABLE_DISCORD_RPC
	m_pyNetworkStream.Discord_Close();
	#endif

	m_pyRes.Destroy();
	m_kGuildMarkDownloader.Disconnect();

	CGrannyModelInstance::DestroySystem();
	CGraphicImageInstance::DestroySystem();

	m_SoundManager.Destroy();
	m_grpDevice.Destroy();

	CAttributeInstance::DestroySystem();
	CTextFileLoader::DestroySystem();
	DestroyCursors();

	CMSApplication::Destroy();

	STICKYKEYS sStickKeys;
	memset(&sStickKeys, 0, sizeof(sStickKeys));
	sStickKeys.cbSize = sizeof(sStickKeys);
	sStickKeys.dwFlags = m_dwStickyKeysFlag;
	SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
}

#ifdef ENABLE_DISCORD_STUFF
void CPythonApplication::ChangeIcon()
{
	if (oldIcon != NULL)
		DestroyIcon(oldIcon);
	oldIcon = NULL;

	const DWORD count = CPythonChat::Instance().GetNewChatCount();
	if (count == 0)
		oldIcon = LoadIcon(ms_hInstance, MAKEINTRESOURCE(m_bFarmIsBlocked ? BLOCK_METIN2 : IDI_METIN2));
	else if (count == 1)
		oldIcon = LoadIcon(ms_hInstance, MAKEINTRESOURCE(m_bFarmIsBlocked ? BLOCK_METIN2_1 : IDI_METIN2_1));
	else if (count == 2)
		oldIcon = LoadIcon(ms_hInstance, MAKEINTRESOURCE(m_bFarmIsBlocked ? BLOCK_METIN2_2 :IDI_METIN2_2));
	else if (count == 3)
		oldIcon = LoadIcon(ms_hInstance, MAKEINTRESOURCE(m_bFarmIsBlocked ? BLOCK_METIN2_3 :IDI_METIN2_3));
	else if (count == 4)
		oldIcon = LoadIcon(ms_hInstance, MAKEINTRESOURCE(m_bFarmIsBlocked ? BLOCK_METIN2_4 :IDI_METIN2_4));
	else if (count == 5)
		oldIcon = LoadIcon(ms_hInstance, MAKEINTRESOURCE(m_bFarmIsBlocked ? BLOCK_METIN2_5 :IDI_METIN2_5));
	else if (count == 6)
		oldIcon = LoadIcon(ms_hInstance, MAKEINTRESOURCE(m_bFarmIsBlocked ? BLOCK_METIN2_6 :IDI_METIN2_6));
	else if (count == 7)
		oldIcon = LoadIcon(ms_hInstance, MAKEINTRESOURCE(m_bFarmIsBlocked ? BLOCK_METIN2_7 :IDI_METIN2_7));
	else if (count == 8)
		oldIcon = LoadIcon(ms_hInstance, MAKEINTRESOURCE(m_bFarmIsBlocked ? BLOCK_METIN2_8 :IDI_METIN2_8));
	else if (count == 9)
		oldIcon = LoadIcon(ms_hInstance, MAKEINTRESOURCE(m_bFarmIsBlocked ? BLOCK_METIN2_9 :IDI_METIN2_9));
	else if (count > 9)
		oldIcon = LoadIcon(ms_hInstance, MAKEINTRESOURCE(m_bFarmIsBlocked ? BLOCK_METIN2_10 :IDI_METIN2_10));

	if (oldIcon == NULL)
		return;

	NID.uFlags = NIF_ICON;
	NID.hIcon = oldIcon;
	Shell_NotifyIcon(NIM_MODIFY, &NID);

	SendMessage(GetWindowHandle(), WM_SETICON, ICON_BIG, (LPARAM)oldIcon);
}

void CPythonApplication::SetTitle(const char* szTitle, const char* szMiniTitle)
{
	CMSWindow::SetText(szTitle);
	NID.uFlags = NIF_ICON | NIF_TIP;
	strcpy(NID.szTip, szMiniTitle);
	Shell_NotifyIcon(NIM_MODIFY, &NID);
}

void CPythonApplication::SetMessage(const char* szMsgTitle, const char* szMsgText)
{
	if (!CPythonSystem::Instance().GetStuffMessage())
		return;

	NID.uFlags = NIF_ICON | NIF_MESSAGE | NIF_INFO | NIF_TIP;
	strcpy(NID.szInfoTitle, szMsgTitle);
	strcpy(NID.szInfo, szMsgText);
	Shell_NotifyIconA(NIM_MODIFY, &NID);
}
#endif

#ifdef ENABLE_LOG_STATE
std::string forbiddenList[2] = { "İlan Ekle", "İlanlarım" };

void CPythonApplication::ScanWindowTitles()
{
	std::string strPhase = CPythonNetworkStream::Instance().GetPhase();
	if (m_bScanStatus == 2)
	{
		std::string strLoginID = static_cast<std::string>(m_szMainName);
		if (strLoginID != CPythonNetworkStream::Instance().GetLoginID())
		{
			memset(&m_szMainName, 0, sizeof(m_szMainName));
			m_bScanStatus = 0;
			m_bFoundForbiddenID = 0;
		}
		return;
	}

	if (m_bScanStatus == 1)
	{
		if (strPhase != "Game")
			return;

		std::string catchCause = "LIST_SIZES_NOT_EQUAL";

		if (m_bFoundForbiddenID < _countof(forbiddenList))
			catchCause = forbiddenList[m_bFoundForbiddenID];

		char szComm[256];
		snprintf(szComm, sizeof(szComm), "/itemcilogkaydet %d", m_bFoundForbiddenID);
		CPythonNetworkStream::Instance().SendChatPacket(szComm);

		m_bScanStatus = 2;
		snprintf(m_szMainName, sizeof(m_szMainName), CPythonNetworkStream::Instance().GetLoginID().c_str());
		return;
	}

	for (HWND hwnd = GetTopWindow(NULL); hwnd != NULL; hwnd = GetNextWindow(hwnd, GW_HWNDNEXT))
	{
		if (!IsWindowVisible(hwnd))
			continue;

		int iLength = GetWindowTextLength(hwnd);
		if (iLength == 0)
			continue;

		char* wndTitle = new char[iLength + 1];
		GetWindowText(hwnd, wndTitle, iLength + 1);

		if (wndTitle == "Program Manager")
			continue;

		std::string strTitle = static_cast<std::string>(wndTitle);

		for (uint8_t i = 0; i < _countof(forbiddenList); i++)
		{
			if (strTitle.find(forbiddenList[i]) != std::string::npos)
			{
				if (strPhase == "Game")
				{
					/*char szComm[256];
					snprintf(szComm, sizeof(szComm), "/trader_found %d", i);
					CPythonNetworkStream::Instance().SendChatPacket(szComm);*/

					std::string catchCause = "LIST_SIZES_NOT_EQUAL";

					if (i < _countof(forbiddenList))
						catchCause = forbiddenList[i];

					char szComm[256];
					snprintf(szComm, sizeof(szComm), "/itemcilogkaydet %d", i);
					CPythonNetworkStream::Instance().SendChatPacket(szComm);

					m_bScanStatus = 2;
					m_bFoundForbiddenID = i;
					snprintf(m_szMainName, sizeof(m_szMainName), CPythonNetworkStream::Instance().GetLoginID().c_str());
				}
				else
				{
					m_bScanStatus = 1;
					m_bFoundForbiddenID = i;
				}
				break;
			}
		}
		delete[] wndTitle;
	}
}
#endif

#ifdef ENABLE_HILE_ENGEL
std::string forbiddenList2[7] = { XorStr<0x3A,14,0x32BC88D5>("\x69\x4A\x55\x49\x4B\x4B\x60\x03\x2D\x37\x64\x33\x77" + 0x32BC88D5).s, XorStr<0xCF,4,0x4DC6F358>("\x8F\xF3\x91" + 0x4DC6F358).s, XorStr<0x7D,11,0x6F10F153>("\x30\x1B\x12\xF4\xEE\xF0\xA3\xC6\xEA\xF2" + 0x6F10F153).s, XorStr<0x37,9,0x650EF0CB>("\x73\x5D\x41\x77\x5A\x1C\x4B\x0F" + 0x650EF0CB).s, XorStr<0x18,13,0xB04EE601>("\x5B\x71\x7F\x7A\x68\x3D\x5B\x71\x47\x48\x4C\x46" + 0xB04EE601).s, XorStr<0xFE,18,0xD19E35B8>("\xBD\x8A\x6C\x21\x4F\x62\x77\x71\x63\x75\x28\x4A\x62\x6E\x6D\x79\x7D" + 0xD19E35B8).s, "|2}2" };
std::string hileisimi[7] = { "Sqitut_Bot_v1_Rapor", "RextBot_Rapor1", "MemtorBot_Rapor", "DexMa_Rapor", "Cheat_Engine_Rapor", "CulMasterCheats_Rapor", "uiBot_Rapor" };

void CPythonApplication::ScanWindowTitles2()
{
	std::string strPhase = CPythonNetworkStream::Instance().GetPhase();
	if (m_bScanStatus2 == 2)
	{
		std::string strLoginID = static_cast<std::string>(m_szMainName2);
		if (strLoginID != CPythonNetworkStream::Instance().GetLoginID())
		{
			memset(&m_szMainName2, 0, sizeof(m_szMainName2));
			m_bScanStatus2 = 0;
			m_bFoundForbiddenID2 = 0;
		}
		return;
	}

	if (m_bScanStatus2 == 1)
	{
		if (strPhase != "Game")
			return;

		std::string catchCause = "LIST_SIZES_NOT_EQUAL";

		if (m_bFoundForbiddenID2 < _countof(forbiddenList2))
			catchCause = hileisimi[m_bFoundForbiddenID2];

		CPythonNetworkStream::Instance().Rapor(catchCause.c_str());

		m_bScanStatus2 = 2;
		snprintf(m_szMainName2, sizeof(m_szMainName2), CPythonNetworkStream::Instance().GetLoginID().c_str());
		return;
	}

	for (HWND hwnd = GetTopWindow(NULL); hwnd != NULL; hwnd = GetNextWindow(hwnd, GW_HWNDNEXT))
	{
		if (!IsWindowVisible(hwnd))
			continue;

		int iLength = GetWindowTextLength(hwnd);
		if (iLength == 0)
			continue;

		char* wndTitle = new char[iLength + 1];
		GetWindowText(hwnd, wndTitle, iLength + 1);

		if (wndTitle == "Program Manager")
			continue;

		std::string strTitle = static_cast<std::string>(wndTitle);

		for (uint8_t i = 0; i < _countof(forbiddenList2); i++)
		{
			if (strTitle.find(forbiddenList2[i]) != std::string::npos)
			{
				if (strPhase == "Game")
				{
					std::string catchCause = "LIST_SIZES_NOT_EQUAL";

					if (i < _countof(forbiddenList2))
						catchCause = hileisimi[i];

					CPythonNetworkStream::Instance().Rapor(catchCause.c_str());

					m_bScanStatus2 = 2;
					m_bFoundForbiddenID2 = i;
					snprintf(m_szMainName2, sizeof(m_szMainName2), CPythonNetworkStream::Instance().GetLoginID().c_str());
				}
				else
				{
					m_bScanStatus2 = 1;
					m_bFoundForbiddenID2 = i;
				}
				break;
			}
		}
		delete[] wndTitle;
	}
}
#endif

#ifdef ENABLE_FPS
void CPythonApplication::SetAppFPS(BYTE index)
{
	std::map<float, std::pair<BYTE, BYTE>> m_Data =
	{
		{30.0,{32,1}}, //30FPS
		{60.0,{16,1}}, //60FPS
		{90.0,{11,0}}, //90FPS
		{120.0,{8,1}}, //120FPS
		{150.0,{6,1}}, //150FPS
		{180.0,{5,1}}, //180FPS
		{220.0,{4,1}}, //220FPS
	};

	if (index >= m_Data.size())
		return;

	auto it = m_Data.begin();
	std::advance(it, index);

	float fps = it->first;

	g_fGameFPS = fps;
	g_fGameIndex = index;

	ANIFPS_MIN = g_fGameFPS;
	ANIFPS_MAX = g_fGameFPS*2;

	float value = fps * (0.007f / 220.0f);
	g_specularSpd = (value < 0.007f) ? 0.007f : value;
	CTimer::Instance().SetUpdateTime(it->second.first, it->second.second);
}
#endif
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
