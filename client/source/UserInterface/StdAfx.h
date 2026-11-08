#pragma once

#pragma warning(disable:4702)
#pragma warning(disable:4100)
#pragma warning(disable:4201)
#pragma warning(disable:4511)
#pragma warning(disable:4663)
#pragma warning(disable:4018)
#pragma warning(disable:4245)

#pragma warning(disable:4267)
#pragma warning(disable:4838)
#pragma warning(disable:4996)

#if _MSC_VER >= 1400
//if don't use below, time_t is 64bit
#define _USE_32BIT_TIME_T
#endif
#include <intrin.h>

#include <iterator>
#include "../eterLib/StdAfx.h"
#include "../eterPythonLib/StdAfx.h"
#include "../gameLib/StdAfx.h"
#include "../scriptLib/StdAfx.h"
#include "../milesLib/StdAfx.h"
#include "../EffectLib/StdAfx.h"
#include "../PRTerrainLib/StdAfx.h"
#include "../SpeedTreeLib/StdAfx.h"

#ifndef __D3DRM_H__
#define __D3DRM_H__
#endif

#include <dshow.h>
#include <qedit.h>

#include "Locale.h"


#include "GameType.h"
extern DWORD __DEFAULT_CODE_PAGE__;

#define APP_NAME	"Metin 2"

enum
{
	POINT_MAX_NUM = 255,
	CHARACTER_NAME_MAX_LEN = 24,
#if defined(LOCALE_SERVICE_JAPAN)
	PLAYER_NAME_MAX_LEN = 16,
#else
	PLAYER_NAME_MAX_LEN = 12,
#endif
#ifdef ENABLE_IKASHOP_RENEWAL
	OFFLINE_SHOP_NAME_MAX_LEN = 64,
	#ifdef ENABLE_CHEQUE_SYSTEM
	YANG_PER_CHEQUE = 100'000'000,
	#endif
#endif
};

void initudp();
void initapp();
void initime();
void initsystemSetting();
void initchr();
void initchrmgr();
void initChat();
void initTextTail();
void initime();
void initItem();
void initNonPlayer();
void initnet();
void initPlayer();
void initSectionDisplayer();
void initServerStateChecker();
void initTrade();
void initMiniMap();
void initProfiler();
void initEvent();
void initeffect();
void initsnd();
void initeventmgr();
void initBackground();
void initwndMgr();
void initshop();
void initpack();
void initskill();
void initfly();
void initquest();
void initsafebox();
void initguild();
void initMessenger();
#ifdef ENABLE_IKASHOP_RENEWAL
void initikashop();
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
void initAcce();
#endif
#ifdef ENABLE_SWITCHBOT
void initSwitchbot();
#endif
#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
void intcuberenewal();
#endif
#ifdef ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM
void initprivateItemShop();
#endif
#ifdef ENABLE_RENDER_TARGET
void initRenderTarget();
#endif
#ifdef ENABLE_CPP_PSM
void initplayersettingsmodule();
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
void initChangeLook();
#endif

extern const std::string& ApplicationStringTable_GetString(DWORD dwID);
extern const std::string& ApplicationStringTable_GetString(DWORD dwID, LPCSTR szKey);

extern const char* ApplicationStringTable_GetStringz(DWORD dwID);
extern const char* ApplicationStringTable_GetStringz(DWORD dwID, LPCSTR szKey);

extern void ApplicationSetErrorString(const char* szErrorString);

#ifdef __AUTO_HUNT__
extern float GetDistanceNew(const TPixelPosition& PixelPosition, const TPixelPosition& c_rPixelPosition);
extern void split_argument(const char* argument, std::vector<std::string>& vecArgs, const char* arg = " ");
#endif

#ifdef ENABLE_DUNGEON_BOSS_ICON_IN_MAP
extern bool GetDungeonNPC(DWORD v);
extern bool IsBoss(DWORD race);
#endif


//martysama0134's d3fef13446d3498b8b5c1aceb4c44240

