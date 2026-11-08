#include "StdAfx.h"
#ifdef CYNEX_MOBILE
extern "C" int CynexPlatform_GetDensityDpi(void);
#endif
#include "PythonSystem.h"
#include "PythonApplication.h"
#include "../GameLib/MapManager.h"

#define DEFAULT_VALUE_ALWAYS_SHOW_NAME		true

void CPythonSystem::SetInterfaceHandler(PyObject * poHandler)
{
//	if (m_poInterfaceHandler)
//		Py_DECREF(m_poInterfaceHandler);

	m_poInterfaceHandler = poHandler;

//	if (m_poInterfaceHandler)
//		Py_INCREF(m_poInterfaceHandler);
}

void CPythonSystem::DestroyInterfaceHandler()
{
	m_poInterfaceHandler = NULL;
}

void CPythonSystem::SaveWindowStatus(int iIndex, int iVisible, int iMinimized, int ix, int iy, int iHeight)
{
	m_WindowStatus[iIndex].isVisible	= iVisible;
	m_WindowStatus[iIndex].isMinimized	= iMinimized;
	m_WindowStatus[iIndex].ixPosition	= ix;
	m_WindowStatus[iIndex].iyPosition	= iy;
	m_WindowStatus[iIndex].iHeight		= iHeight;
}

void CPythonSystem::GetDisplaySettings()
{
	memset(m_ResolutionList, 0, sizeof(TResolution) * RESOLUTION_MAX_NUM);
	m_ResolutionCount = 0;

	LPDIRECT3D8 lpD3D = CPythonGraphic::Instance().GetD3D();

	D3DADAPTER_IDENTIFIER8 d3dAdapterIdentifier;
	D3DDISPLAYMODE d3ddmDesktop;

	lpD3D->GetAdapterIdentifier(0, D3DENUM_NO_WHQL_LEVEL, &d3dAdapterIdentifier);
	lpD3D->GetAdapterDisplayMode(0, &d3ddmDesktop);

	DWORD dwNumAdapterModes = lpD3D->GetAdapterModeCount(0);

	for (UINT iMode = 0; iMode < dwNumAdapterModes; iMode++)
	{
		D3DDISPLAYMODE DisplayMode;
		lpD3D->EnumAdapterModes(0, iMode, &DisplayMode);
		DWORD bpp = 0;

		if (DisplayMode.Width < 800 || DisplayMode.Height < 600)
			continue;

		if (DisplayMode.Format == D3DFMT_R5G6B5)
			bpp = 16;
		else if (DisplayMode.Format == D3DFMT_X8R8G8B8)
			bpp = 32;
		else
			continue;

		int check_res = false;

		for (int i = 0; !check_res && i < m_ResolutionCount; ++i)
		{
			if (m_ResolutionList[i].bpp != bpp ||
				m_ResolutionList[i].width != DisplayMode.Width ||
				m_ResolutionList[i].height != DisplayMode.Height)
				continue;

			int check_fre = false;

			for (int j = 0; j < m_ResolutionList[i].frequency_count; ++j)
			{
				if (m_ResolutionList[i].frequency[j] == DisplayMode.RefreshRate)
				{
					check_fre = true;
					break;
				}
			}

			if (!check_fre)
				if (m_ResolutionList[i].frequency_count < FREQUENCY_MAX_NUM)
					m_ResolutionList[i].frequency[m_ResolutionList[i].frequency_count++] = DisplayMode.RefreshRate;

			check_res = true;
		}

		if (!check_res)
		{
			if (m_ResolutionCount < RESOLUTION_MAX_NUM)
			{
				m_ResolutionList[m_ResolutionCount].width			= DisplayMode.Width;
				m_ResolutionList[m_ResolutionCount].height			= DisplayMode.Height;
				m_ResolutionList[m_ResolutionCount].bpp				= bpp;
				m_ResolutionList[m_ResolutionCount].frequency[0]	= DisplayMode.RefreshRate;
				m_ResolutionList[m_ResolutionCount].frequency_count	= 1;

				++m_ResolutionCount;
			}
		}
	}
}

int	CPythonSystem::GetResolutionCount()
{
	return m_ResolutionCount;
}

int CPythonSystem::GetFrequencyCount(int index)
{
	if (index >= m_ResolutionCount)
		return 0;

    return m_ResolutionList[index].frequency_count;
}

bool CPythonSystem::GetResolution(int index, OUT DWORD *width, OUT DWORD *height, OUT DWORD *bpp)
{
	if (index >= m_ResolutionCount)
		return false;

	*width = m_ResolutionList[index].width;
	*height = m_ResolutionList[index].height;
	*bpp = m_ResolutionList[index].bpp;
	return true;
}

bool CPythonSystem::GetFrequency(int index, int freq_index, OUT DWORD *frequncy)
{
	if (index >= m_ResolutionCount)
		return false;

	if (freq_index >= m_ResolutionList[index].frequency_count)
		return false;

	*frequncy = m_ResolutionList[index].frequency[freq_index];
	return true;
}

int	CPythonSystem::GetResolutionIndex(DWORD width, DWORD height, DWORD bit)
{
	DWORD re_width, re_height, re_bit;
	int i = 0;

	while (GetResolution(i, &re_width, &re_height, &re_bit))
	{
		if (re_width == width)
			if (re_height == height)
				if (re_bit == bit)
					return i;
		i++;
	}

	return 0;
}

int	CPythonSystem::GetFrequencyIndex(int res_index, DWORD frequency)
{
	DWORD re_frequency;
	int i = 0;

	while (GetFrequency(res_index, i, &re_frequency))
	{
		if (re_frequency == frequency)
			return i;

		i++;
	}

	return 0;
}

DWORD CPythonSystem::GetWidth()
{
	return m_Config.width;
}

DWORD CPythonSystem::GetHeight()
{
	return m_Config.height;
}
DWORD CPythonSystem::GetBPP()
{
	return m_Config.bpp;
}
DWORD CPythonSystem::GetFrequency()
{
	return m_Config.frequency;
}

bool CPythonSystem::IsNoSoundCard()
{
	return m_Config.bNoSoundCard;
}

bool CPythonSystem::IsSoftwareCursor()
{
	return m_Config.is_software_cursor;
}

float CPythonSystem::GetMusicVolume()
{
	return m_Config.music_volume;
}

int CPythonSystem::GetSoundVolume()
{
	return m_Config.voice_volume;
}

void CPythonSystem::SetMusicVolume(float fVolume)
{
	m_Config.music_volume = fVolume;
}

void CPythonSystem::SetSoundVolumef(float fVolume)
{
	m_Config.voice_volume = int(5 * fVolume);
}

int CPythonSystem::GetDistance()
{
	return m_Config.iDistance;
}

int CPythonSystem::GetShadowLevel()
{
	return m_Config.iShadowLevel;
}

void CPythonSystem::SetShadowLevel(unsigned int level)
{
	m_Config.iShadowLevel = MIN(level, 5);
	CPythonBackground::instance().RefreshShadowLevel();
}

int CPythonSystem::IsSaveID()
{
	return m_Config.isSaveID;
}

const char * CPythonSystem::GetSaveID()
{
	return m_Config.SaveID;
}

bool CPythonSystem::isViewCulling()
{
	return m_Config.is_object_culling;
}

#ifdef ENABLE_PERFORMANCE_REWORK
void CPythonSystem::SetExtremeFpsBoost(bool bVal)
{
	m_Config.bExtremeFpsBoost = bVal;
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.TouchViewBoundDistance(bVal);
}
#endif

#ifdef ENABLE_EFFECT_SCALE
float CPythonSystem::GetEffectScale()
{
	return m_Config.effect_scale;
}

void CPythonSystem::SetEffectScale(float fScale)
{
	m_Config.effect_scale = fScale;
}
#endif

void CPythonSystem::SetSaveID(int iValue, const char * c_szSaveID)
{
	if (iValue != 1)
		return;

	m_Config.isSaveID = iValue;
	strncpy(m_Config.SaveID, c_szSaveID, sizeof(m_Config.SaveID) - 1);
}

CPythonSystem::TConfig * CPythonSystem::GetConfig()
{
	return &m_Config;
}

void CPythonSystem::SetConfig(TConfig * pNewConfig)
{
	m_Config = *pNewConfig;
}

void CPythonSystem::SetDefaultConfig()
{
	memset(&m_Config, 0, sizeof(m_Config));

	m_Config.width				= 1024;
	m_Config.height				= 768;
	m_Config.bpp				= 32;

	#if defined( LOCALE_SERVICE_WE_JAPAN )
	m_Config.bWindowed			= true;
	#else
	m_Config.bWindowed			= false;
	#endif

	m_Config.is_software_cursor	= false;
	m_Config.is_object_culling	= true;
	m_Config.iDistance			= 3;

	m_Config.gamma				= 3;
	m_Config.music_volume		= 1.0f;
	m_Config.voice_volume		= 5;

	m_Config.bDecompressDDS		= 0;
	m_Config.bSoftwareTiling	= 0;
	m_Config.iShadowLevel		= 3;
	m_Config.bViewChat			= true;
	m_Config.bShowHighlight		= true;
	m_Config.bAlwaysShowName	= DEFAULT_VALUE_ALWAYS_SHOW_NAME;
	m_Config.bShowDamage		= true;
	m_Config.bShowSalesText		= true;

	#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
	m_Config.bShowMobAIFlag		= true;
	#endif

	#ifdef ENABLE_FPS
	m_Config.iFps				= 2;
	#endif

	#ifdef ENABLE_EFFECT_SCALE
	m_Config.effect_scale		= 3500.0f;
	#endif

	#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
	m_Config.bShowMobLevel		= false;
	#endif

	#ifdef ENABLE_HIDE_MOB_SYSTEM
	m_Config.bHidePets			= false;
	m_Config.bHideMounts		= false;
	#endif

	#ifdef ENABLE_CHAT_STOP
	m_Config.bIsChatStop		= false;
	#endif

	#ifdef ENABLE_FOG_FIX
	m_Config.bFogMode			= false;
	CMapManager::SetFogAllowed(false);
	#endif

	#ifdef ENABLE_AUTO_PICKUP_SYSTEM
	m_Config.bAutoPickup		= true;
	m_Config.bAutoPickup2		= true;
	m_Config.bAutoPickup3		= true;
	#endif

	#ifdef ENABLE_SPECIAL_STORAGE
	m_Config.bStorageOpenWithInventory = false;
	#endif

	#ifdef ENABLE_FOV_OPTION
	m_Config.iFOVLevel			= 30.0f;
	#endif

	#ifdef ENABLE_DISCORD_STUFF
	m_Config.bMessage			= false;//true
	#endif

	#ifdef ENABLE_NEW_GAMEOPTION
	m_Config.questLetter		= true;
	m_Config.affectIcons		= true;
	#endif

	#ifdef ENABLE_PERFORMANCE_REWORK
	m_Config.bExtremeFpsBoost	= 0;
	#endif

	#ifdef ENABLE_POLY_ALL_MOBS_DOG
	m_Config.bPolyDogMode		= false;
	#endif

	m_Config.bYangDropMode		= false;
}

bool CPythonSystem::IsWindowed()
{
	return m_Config.bWindowed;
}

bool CPythonSystem::IsViewChat()
{
	return m_Config.bViewChat;
}

void CPythonSystem::SetViewChatFlag(int iFlag)
{
	m_Config.bViewChat = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAlwaysShowName()
{
	return m_Config.bAlwaysShowName;
}

void CPythonSystem::SetAlwaysShowNameFlag(int iFlag)
{
	m_Config.bAlwaysShowName = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowHighlight()
{
	return m_Config.bShowHighlight;
}

void CPythonSystem::SetShowHighlight(int iFlag)
{
	m_Config.bShowHighlight = iFlag == 1 ? true : false;
}


bool CPythonSystem::IsShowDamage()
{
	return m_Config.bShowDamage;
}

void CPythonSystem::SetShowDamageFlag(int iFlag)
{
	m_Config.bShowDamage = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowSalesText()
{
	return m_Config.bShowSalesText;
}

void CPythonSystem::SetShowSalesTextFlag(int iFlag)
{
	m_Config.bShowSalesText = iFlag == 1 ? true : false;
}

#ifdef ENABLE_SHOW_NIGHT_SYSTEM
bool CPythonSystem::GetNightMode()
{
	return m_Config.bNightMode;
}

void CPythonSystem::SetNightMode(int iFlag)
{
	m_Config.bNightMode = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_FOV_OPTION
void CPythonSystem::SetFOVLevel(float fFOV)
{
	m_Config.iFOVLevel = fMINMAX(30.0f, fFOV, 120.0f);
}
float CPythonSystem::GetFOVLevel()
{
	return m_Config.iFOVLevel;
}
#endif

#ifdef ENABLE_FOG_FIX
void CPythonSystem::SetFogMode(int iFlag)
{
	m_Config.bFogMode = iFlag == 1 ? true : false;
	CMapManager::SetFogAllowed(m_Config.bFogMode);
}

bool CPythonSystem::IsFogMode()
{
	return m_Config.bFogMode;
}
#endif

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
bool CPythonSystem::IsShowMobAIFlag()
{
	return m_Config.bShowMobAIFlag;
}

void CPythonSystem::SetShowMobAIFlagFlag(int iFlag)
{
	m_Config.bShowMobAIFlag = iFlag == 1 ? true : false;
}
#endif

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
bool CPythonSystem::IsShowMobLevel()
{
	return m_Config.bShowMobLevel;
}

void CPythonSystem::SetShowMobLevelFlag(int iFlag)
{
	m_Config.bShowMobLevel = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_POLY_ALL_MOBS_DOG
void CPythonSystem::SetPolyDogMode(int iFlag)
{
	m_Config.bPolyDogMode = (iFlag == 1) ? true : false;
}

bool CPythonSystem::IsPolyDogMode()
{
	return m_Config.bPolyDogMode;
}
#endif

void CPythonSystem::SetYangDropMode(int iFlag)
{
	m_Config.bYangDropMode = (iFlag == 1) ? true : false;
}

bool CPythonSystem::IsYangDropMode()
{
	return m_Config.bYangDropMode;
}

bool CPythonSystem::IsAutoTiling()
{
	if (m_Config.bSoftwareTiling == 0)
		return true;

	return false;
}

void CPythonSystem::SetSoftwareTiling(bool isEnable)
{
	if (isEnable)
		m_Config.bSoftwareTiling=1;
	else
		m_Config.bSoftwareTiling=2;
}

bool CPythonSystem::IsSoftwareTiling()
{
	if (m_Config.bSoftwareTiling==1)
		return true;

	return false;
}

bool CPythonSystem::IsUseDefaultIME()
{
	return m_Config.bUseDefaultIME;
}

#ifdef ENABLE_AUTO_PICKUP_SYSTEM
void CPythonSystem::SetAutoPickup(int iFlag)
{
	m_Config.bAutoPickup = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAutoPickup()
{
	return m_Config.bAutoPickup;
}

void CPythonSystem::SetAutoPickup2(int iFlag)
{
	m_Config.bAutoPickup2 = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAutoPickup2()
{
	return m_Config.bAutoPickup2;
}

void CPythonSystem::SetAutoPickup3(int iFlag)
{
	m_Config.bAutoPickup3 = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAutoPickup3()
{
	return m_Config.bAutoPickup3;
}
#endif

#ifdef ENABLE_SPECIAL_STORAGE
bool CPythonSystem::IsStorageOpenWithInventory()
{
	return m_Config.bStorageOpenWithInventory;
}

void CPythonSystem::SetStorageOpenWithInventory(int iFlag)
{
	m_Config.bStorageOpenWithInventory = iFlag == 1 ? true : false;
}
#endif

#ifdef CYNEX_MOBILE
static bool s_akravonUserRes = false;
#endif

bool CPythonSystem::LoadConfig()
{
	FILE * fp = NULL;

	if (NULL == (fp = fopen("metin2.cfg", "rt")))
		return false;

	char buf[256];
	char command[256];
	char value[256];

	while (fgets(buf, 256, fp))
	{
		if (sscanf(buf, " %s %s\n", command, value) == EOF)
			break;

		if (!stricmp(command, "WIDTH"))
			m_Config.width = atoi(value);
#ifdef CYNEX_MOBILE
		else if (!stricmp(command, "AKRAVON_RES"))
			s_akravonUserRes = true;	// AKRAVON: kullanici ayarlardan cozunurluk secti
#endif
		else if (!stricmp(command, "HEIGHT"))
			m_Config.height = atoi(value);
		else if (!stricmp(command, "BPP"))
			m_Config.bpp = atoi(value);
		else if (!stricmp(command, "FREQUENCY"))
			m_Config.frequency = atoi(value);
		else if (!stricmp(command, "SOFTWARE_CURSOR"))
			m_Config.is_software_cursor = atoi(value) ? true : false;
		else if (!stricmp(command, "OBJECT_CULLING"))
			m_Config.is_object_culling = atoi(value) ? true : false;
		else if (!stricmp(command, "VISIBILITY"))
			m_Config.iDistance = atoi(value);
		else if (!stricmp(command, "MUSIC_VOLUME")) {
			if(strchr(value, '.') == 0) { // Old compatiability
				m_Config.music_volume = pow(10.0f, (-1.0f + (((float) atoi(value)) / 5.0f)));
				if(atoi(value) == 0)
					m_Config.music_volume = 0.0f;
			} else
				m_Config.music_volume = atof(value);
		} else if (!stricmp(command, "VOICE_VOLUME"))
			m_Config.voice_volume = (char) atoi(value);
		else if (!stricmp(command, "GAMMA"))
			m_Config.gamma = atoi(value);
		else if (!stricmp(command, "IS_SAVE_ID"))
			m_Config.isSaveID = atoi(value);
		else if (!stricmp(command, "SAVE_ID"))
			strncpy(m_Config.SaveID, value, 20);
		else if (!stricmp(command, "WINDOWED"))
		{
			m_Config.bWindowed = atoi(value) == 1 ? true : false;
		}
		else if (!stricmp(command, "USE_DEFAULT_IME"))
			m_Config.bUseDefaultIME = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SOFTWARE_TILING"))
			m_Config.bSoftwareTiling = atoi(value);
		else if (!stricmp(command, "SHADOW_LEVEL"))
			m_Config.iShadowLevel = atoi(value);
		else if (!stricmp(command, "DECOMPRESSED_TEXTURE"))
			m_Config.bDecompressDDS = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "NO_SOUND_CARD"))
			m_Config.bNoSoundCard = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "VIEW_CHAT"))
			m_Config.bViewChat = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "ALWAYS_VIEW_NAME"))
			m_Config.bAlwaysShowName = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_DAMAGE"))
			m_Config.bShowDamage = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_SALESTEXT"))
			m_Config.bShowSalesText = atoi(value) == 1 ? true : false;
		#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
		else if (!stricmp(command, "SHOW_MOBAIFLAG"))
			m_Config.bShowMobAIFlag = atoi(value) == 1 ? true : false;
		#endif
		else if (!stricmp(command, "SHOW_HIGHLIGHT"))
			m_Config.bShowHighlight = atoi(value) == 1 ? true : false;
		#ifdef ENABLE_FPS
		else if (!stricmp(command, "FRAME_VALUE"))
			m_Config.iFps = atoi(value);
		#endif
		#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
		else if (!stricmp(command, "SHOW_MOBLEVEL"))
			m_Config.bShowMobLevel = atoi(value) == 1 ? true : false;
		#endif
		#ifdef ENABLE_FOG_FIX
		else if (!stricmp(command, "FOG_MODE_ON"))
		{
			m_Config.bFogMode = atoi(value) == 1 ? true : false;
			CMapManager::SetFogAllowed(m_Config.bFogMode);
		}
		#endif
		#ifdef ENABLE_FOV_OPTION
		else if (!stricmp(command, "FIELD_OF_VIEW"))
			m_Config.iFOVLevel = atoi(value);
		#endif
		#ifdef ENABLE_SHOW_NIGHT_SYSTEM
		else if (!stricmp(command, "NIGHT_MODE"))
			m_Config.bNightMode = atoi(value) == 1 ? true : false;
		#endif
		#if defined(__BL_HIDE_EFFECT__)
		else if (!stricmp(command, "HIDE_BUFF_EFFECT"))
			CEffectManager::Instance().SetEffectOption(CEffectManager::EFFECT_OPTION_BUFF, (atoi(value) == 1));
		else if (!stricmp(command, "HIDE_SKILL_EFFECT"))
			CEffectManager::Instance().SetEffectOption(CEffectManager::EFFECT_OPTION_SKILL, (atoi(value) == 1));
		#endif
		#ifdef ENABLE_CHAT_STOP
		else if (!stricmp(command, "CHAT_STOP"))
			m_Config.bIsChatStop = atoi(value) == 1 ? true : false;
		#endif
		#ifdef ENABLE_HIDE_MOB_SYSTEM
		else if (!stricmp(command, "HIDE_PETS"))
			m_Config.bHidePets = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "HIDE_MOUNTS"))
			m_Config.bHideMounts = atoi(value) == 1 ? true : false;
		#endif
		#ifdef ENABLE_AUTO_PICKUP_SYSTEM
		else if (!stricmp(command, "AUTO_PICKUP_METIN"))
			m_Config.bAutoPickup = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "AUTO_PICKUP_MONSTER"))
			m_Config.bAutoPickup2 = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "AUTO_PICKUP_BOSS"))
			m_Config.bAutoPickup3 = atoi(value) == 1 ? true : false;
		#endif
		#ifdef ENABLE_SPECIAL_STORAGE
		else if (!stricmp(command, "STORAGE_OPEN_WITH_INVENTORY"))
			m_Config.bStorageOpenWithInventory = atoi(value) == 1 ? true : false;
		#endif
		#ifdef ENABLE_DISCORD_STUFF
		else if (!stricmp(command, "special_message"))
			m_Config.bMessage = atoi(value) == 1 ? true : false;
		#endif
		#ifdef ENABLE_NEW_GAMEOPTION
		else if (!stricmp(command, "QUEST_LETTER"))
			m_Config.questLetter = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "AFFECT_ICONS"))
			m_Config.affectIcons = atoi(value) == 1 ? true : false;
		#endif
		#ifdef ENABLE_PERFORMANCE_REWORK
		else if (!stricmp(command, "PERFORMANCE_REWORK")) { m_Config.bExtremeFpsBoost = atoi(value) == 1 ? true : false; }
		#endif
		#ifdef ENABLE_EFFECT_SCALE
		else if (!stricmp(command, "EFFECT_SCALE_NEW"))
		{
			if (strchr(value, '.') == 0)
			{
				m_Config.effect_scale = static_cast<float>(atoi(value));
				if (atoi(value) == 0)
					m_Config.effect_scale = 0.0f;
			}
			else
				m_Config.effect_scale = atof(value);
		}
		#endif
		#ifdef ENABLE_POLY_ALL_MOBS_DOG
		else if (!stricmp(command, "POLY_DOG_MODE"))
			m_Config.bPolyDogMode = (atoi(value) == 1) ? true : false;
		#endif
		else if (!stricmp(command, "YANG_DROP_MODE"))
			m_Config.bYangDropMode = (atoi(value) == 1) ? true : false;
	}

#ifdef CYNEX_MOBILE
	// AKRAVON: ilk acilista (secim yoksa) cihazin kendi cozunurlugu - her telefonda keskin
	if (!s_akravonUserRes)
	{
		// Cynex: oyun her zaman yatay; acilista yuzey kisa sure dikey (portre) olcululursa uzun kenar genislik sayilir
		const int rawW = GetSystemMetrics(SM_CXFULLSCREEN), rawH = GetSystemMetrics(SM_CYFULLSCREEN);
		const int sw = rawW > rawH ? rawW : rawH, sh = rawW > rawH ? rawH : rawW;
		if (sw >= 800 && sh >= 600)
		{
			// AKRAVON: arayuz mantiksal 900 piksel yukseklikte tasarlandi; daha yuksek ekranlarda ayni fiziksel boyut icin
			// mantiksal cozunurluk 900'e sabitlenir, cizim yerel cozunurlukte (keskin) kalir. Daha dusuk ekran oldugu gibi.
			// Telefonda fiziksel ekran kucuk: ekran yuksekligi dp cinsinden 600dp'de 900, daha kucukte orantili azalir (alt sinir 600)
			// -> arayuz ayni dokunma boyutunda kalir (emulator 900dp=600dp: degismez, telefon ~411dp: 616).
			int kDesignHeight = 900;
			{
				const int dpi = CynexPlatform_GetDensityDpi();
				if (dpi > 0)
				{
					const int dpH = sh * 160 / dpi;
					kDesignHeight = dpH * 3 / 2;
					if (kDesignHeight < 600) kDesignHeight = 600;
					if (kDesignHeight > 900) kDesignHeight = 900;
				}
			}
			if (sh > kDesignHeight)
			{
				m_Config.height = kDesignHeight;
				m_Config.width = ((sw * kDesignHeight / sh) + 1) / 2 * 2;
			}
			else
			{
				m_Config.width = sw;
				m_Config.height = sh;
			}
		}
	}
#endif
	if (m_Config.bWindowed)
	{
		unsigned screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
		unsigned screen_height = GetSystemMetrics(SM_CYFULLSCREEN);

		if (m_Config.width >= screen_width)
		{
			m_Config.width = screen_width;
		}
		if (m_Config.height >= screen_height)
		{
			m_Config.height = screen_height;
		}
	}

	if (m_Config.width < 800) // 800x600 Fix
		m_Config.width = 800;

	if (m_Config.height < 600)  // 800x600 Fix
		m_Config.height = 600;

	m_OldConfig = m_Config;

	fclose(fp);

//	Tracef("LoadConfig: Resolution: %dx%d %dBPP %dHZ Software Cursor: %d, Music/Voice Volume: %d/%d Gamma: %d\n",
//		m_Config.width,
//		m_Config.height,
//		m_Config.bpp,
//		m_Config.frequency,
//		m_Config.is_software_cursor,
//		m_Config.music_volume,
//		m_Config.voice_volume,
//		m_Config.gamma);

	return true;
}

bool CPythonSystem::SaveConfig()
{
	FILE *fp;

	if (NULL == (fp = fopen("metin2.cfg", "wt")))
		return false;

	fprintf(fp, "WIDTH						%d\n"
				"HEIGHT						%d\n"
				"BPP						%d\n"
				"FREQUENCY					%d\n"
				"SOFTWARE_CURSOR			%d\n"
				"OBJECT_CULLING				%d\n"
				"VISIBILITY					%d\n"
				"MUSIC_VOLUME				%.3f\n"
				"VOICE_VOLUME				%d\n"
				"GAMMA						%d\n"
				"IS_SAVE_ID					%d\n"
				"SAVE_ID					%s\n"
				"DECOMPRESSED_TEXTURE		%d\n",
				m_Config.width,
				m_Config.height,
				m_Config.bpp,
				m_Config.frequency,
				m_Config.is_software_cursor,
				m_Config.is_object_culling,
				m_Config.iDistance,
				m_Config.music_volume,
				m_Config.voice_volume,
				m_Config.gamma,
				m_Config.isSaveID,
				m_Config.SaveID,
				m_Config.bDecompressDDS);

	if (m_Config.bWindowed == 1)
		fprintf(fp, "WINDOWED				%d\n", m_Config.bWindowed);

	if (m_Config.bViewChat == 0)
		fprintf(fp, "VIEW_CHAT				%d\n", m_Config.bViewChat);

	if (m_Config.bAlwaysShowName != DEFAULT_VALUE_ALWAYS_SHOW_NAME)
		fprintf(fp, "ALWAYS_VIEW_NAME		%d\n", m_Config.bAlwaysShowName);

	if (m_Config.bShowDamage == 0)
		fprintf(fp, "SHOW_DAMAGE		%d\n", m_Config.bShowDamage);

	if (m_Config.bShowSalesText == 0)
		fprintf(fp, "SHOW_SALESTEXT		%d\n", m_Config.bShowSalesText);

	#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
	if (m_Config.bShowMobAIFlag == 0)
		fprintf(fp, "SHOW_MOBAIFLAG		%d\n", m_Config.bShowMobAIFlag);
	#endif

	#ifdef ENABLE_HIDE_MOB_SYSTEM
	fprintf(fp, "HIDE_PETS				%d\n", m_Config.bHidePets);
	fprintf(fp, "HIDE_MOUNTS			%d\n", m_Config.bHideMounts);
	#endif

	#ifdef ENABLE_EFFECT_SCALE
	fprintf(fp, "EFFECT_SCALE_NEW			%.1f\n", m_Config.effect_scale);
	#endif

	#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
	if (m_Config.bShowMobLevel == 0)
		fprintf(fp, "SHOW_MOBLEVEL		%d\n", m_Config.bShowMobLevel);
	#endif

	#ifdef ENABLE_CHAT_STOP
	fprintf(fp, "CHAT_STOP				%d\n", m_Config.bIsChatStop);
	#endif

	#ifdef ENABLE_SHOW_NIGHT_SYSTEM
	fprintf(fp, "NIGHT_MODE			%d\n", m_Config.bNightMode);
	#endif

	#ifdef ENABLE_FOG_FIX
	if (m_Config.bFogMode == 0)
		fprintf(fp, "FOG_MODE_ON		%d\n", m_Config.bFogMode);
	#endif

	#ifdef ENABLE_AUTO_PICKUP_SYSTEM
	fprintf(fp, "AUTO_PICKUP_METIN			%d\n", m_Config.bAutoPickup);
	fprintf(fp, "AUTO_PICKUP_MONSTER		%d\n", m_Config.bAutoPickup2);
	fprintf(fp, "AUTO_PICKUP_BOSS			%d\n", m_Config.bAutoPickup3);
	#endif

	#ifdef ENABLE_SPECIAL_STORAGE
	fprintf(fp, "STORAGE_OPEN_WITH_INVENTORY			%d\n", m_Config.bStorageOpenWithInventory);
	#endif

	#ifdef ENABLE_FOV_OPTION
	fprintf(fp, "FIELD_OF_VIEW		%.1f\n", m_Config.iFOVLevel);
	#endif

	#if defined(__BL_HIDE_EFFECT__)
	fprintf(fp, "HIDE_BUFF_EFFECT		%d\n", CEffectManager::Instance().GetEffectOption(CEffectManager::EFFECT_OPTION_BUFF));
	fprintf(fp, "HIDE_SKILL_EFFECT		%d\n", CEffectManager::Instance().GetEffectOption(CEffectManager::EFFECT_OPTION_SKILL));
	#endif

	#ifdef ENABLE_DISCORD_STUFF
	fprintf(fp, "special_message\t%d\n", m_Config.bMessage);
	#endif

	#ifdef ENABLE_NEW_GAMEOPTION
	fprintf(fp, "QUEST_LETTER				%d\n", m_Config.questLetter);
	fprintf(fp, "AFFECT_ICONS				%d\n", m_Config.affectIcons);
	#endif

	#ifdef ENABLE_FPS
	fprintf(fp, "FRAME_VALUE			%d\n", m_Config.iFps);
	#endif

	fprintf(fp, "SHOW_HIGHLIGHT		%d\n", m_Config.bShowHighlight);

	fprintf(fp, "USE_DEFAULT_IME		%d\n", m_Config.bUseDefaultIME);
	fprintf(fp, "SOFTWARE_TILING		%d\n", m_Config.bSoftwareTiling);
	fprintf(fp, "SHADOW_LEVEL			%d\n", m_Config.iShadowLevel);

	#ifdef ENABLE_POLY_ALL_MOBS_DOG
	fprintf(fp, "POLY_DOG_MODE			%d\n", m_Config.bPolyDogMode);
	#endif

	fprintf(fp, "YANG_DROP_MODE			%d\n", m_Config.bYangDropMode);

	fprintf(fp, "\n");

	#ifdef ENABLE_PERFORMANCE_REWORK
	fprintf(fp, "PERFORMANCE_REWORK	%d\n", m_Config.bExtremeFpsBoost);
	#endif

	fclose(fp);
	return true;
}

bool CPythonSystem::LoadInterfaceStatus()
{
	FILE * File;
	File = fopen("interface.cfg", "rb");

	if (!File)
		return false;

	fread(m_WindowStatus, 1, sizeof(TWindowStatus) * WINDOW_MAX_NUM, File);
	fclose(File);
	return true;
}

void CPythonSystem::SaveInterfaceStatus()
{
	if (!m_poInterfaceHandler)
		return;

	PyCallClassMemberFunc(m_poInterfaceHandler, "OnSaveInterfaceStatus", Py_BuildValue("()"));

	FILE * File;

	File = fopen("interface.cfg", "wb");

	if (!File)
	{
		TraceError("Cannot open interface.cfg");
		return;
	}

	fwrite(m_WindowStatus, 1, sizeof(TWindowStatus) * WINDOW_MAX_NUM, File);
	fclose(File);
}

bool CPythonSystem::isInterfaceConfig()
{
	return m_isInterfaceConfig;
}

const CPythonSystem::TWindowStatus & CPythonSystem::GetWindowStatusReference(int iIndex)
{
	return m_WindowStatus[iIndex];
}

#ifdef ENABLE_FPS
void CPythonSystem::SetFPS(BYTE iFlag)
{
	m_Config.iFps = iFlag;
	CPythonApplication::Instance().SetAppFPS(iFlag);
} 
#endif

void CPythonSystem::ApplyConfig()
{
	if (m_OldConfig.gamma != m_Config.gamma)
	{
		float val = 1.0f;

		switch (m_Config.gamma)
		{
			case 0: val = 0.4f;	break;
			case 1: val = 0.7f; break;
			case 2: val = 1.0f; break;
			case 3: val = 1.2f; break;
			case 4: val = 1.4f; break;
		}

		CPythonGraphic::Instance().SetGamma(val);
	}

	if (m_OldConfig.is_software_cursor != m_Config.is_software_cursor)
	{
		if (m_Config.is_software_cursor)
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_SOFTWARE);
		else
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_HARDWARE);
	}

	m_OldConfig = m_Config;

	ChangeSystem();
}

void CPythonSystem::ChangeSystem()
{
	// Shadow
	/*
	if (m_Config.is_shadow)
		CScreen::SetShadowFlag(true);
	else
		CScreen::SetShadowFlag(false);
	*/
	CSoundManager& rkSndMgr = CSoundManager::Instance();
	/*
	float fMusicVolume;
	if (0 == m_Config.music_volume)
		fMusicVolume = 0.0f;
	else
		fMusicVolume= (float)pow(10.0f, (-1.0f + (float)m_Config.music_volume / 5.0f));
		*/
	rkSndMgr.SetMusicVolume(m_Config.music_volume);

	/*
	float fVoiceVolume;
	if (0 == m_Config.voice_volume)
		fVoiceVolume = 0.0f;
	else
		fVoiceVolume = (float)pow(10.0f, (-1.0f + (float)m_Config.voice_volume / 5.0f));
	*/
	rkSndMgr.SetSoundVolumeGrade(m_Config.voice_volume);
}

void CPythonSystem::Clear()
{
	SetInterfaceHandler(NULL);
}

CPythonSystem::CPythonSystem()
{
	memset(&m_Config, 0, sizeof(TConfig));

	m_poInterfaceHandler = NULL;

	SetDefaultConfig();

	LoadConfig();

	ChangeSystem();

	if (LoadInterfaceStatus())
		m_isInterfaceConfig = true;
	else
		m_isInterfaceConfig = false;
}

CPythonSystem::~CPythonSystem()
{
	assert(m_poInterfaceHandler==NULL && "CPythonSystem MUST CLEAR!");
}
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
