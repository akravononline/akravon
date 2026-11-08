#pragma once

class CPythonSystem : public CSingleton<CPythonSystem>
{
	public:
		enum EWindow
		{
			WINDOW_STATUS,
			WINDOW_INVENTORY,
			WINDOW_ABILITY,
			WINDOW_SOCIETY,
			WINDOW_JOURNAL,
			WINDOW_COMMAND,

			WINDOW_QUICK,
			WINDOW_GAUGE,
			WINDOW_MINIMAP,
			WINDOW_CHAT,

			WINDOW_MAX_NUM,
		};

		enum
		{
			FREQUENCY_MAX_NUM  = 30,
			RESOLUTION_MAX_NUM = 100
		};

		typedef struct SResolution
		{
			DWORD	width;
			DWORD	height;
			DWORD	bpp;		// bits per pixel (high-color = 16bpp, true-color = 32bpp)

			DWORD	frequency[20];
			BYTE	frequency_count;
		} TResolution;

		typedef struct SWindowStatus
		{
			int		isVisible;
			int		isMinimized;

			int		ixPosition;
			int		iyPosition;
			int		iHeight;
		} TWindowStatus;

		typedef struct SConfig
		{
			DWORD			width;
			DWORD			height;
			DWORD			bpp;
			DWORD			frequency;

			bool			is_software_cursor;
			bool			is_object_culling;
			int				iDistance;
			int				iShadowLevel;
#ifdef ENABLE_FOV_OPTION
			FLOAT			iFOVLevel;
#endif
			FLOAT			music_volume;
			BYTE			voice_volume;

			int				gamma;

			int				isSaveID;
			char			SaveID[20];

			bool			bWindowed;
			bool			bDecompressDDS;
			bool			bNoSoundCard;
			bool			bUseDefaultIME;
			BYTE			bSoftwareTiling;
			bool			bViewChat;
			bool			bAlwaysShowName;
			bool			bShowDamage;
			bool			bShowSalesText;

			#ifdef ENABLE_FOG_FIX
			bool			bFogMode;
			#endif

			bool			bShowHighlight;

			#ifdef ENABLE_FPS
			BYTE				iFps;
			#endif

			#ifdef ENABLE_SHOW_NIGHT_SYSTEM
			bool			bNightMode;
			#endif

			#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
			bool			bShowMobAIFlag;
			#endif

			#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
			bool			bShowMobLevel;
			#endif

			#ifdef ENABLE_CHAT_STOP
			bool			bIsChatStop;
			#endif

			#ifdef ENABLE_HIDE_MOB_SYSTEM
			bool			bHidePets;
			bool			bHideMounts;
			#endif

			#ifdef ENABLE_AUTO_PICKUP_SYSTEM
			bool			bAutoPickup;
			bool			bAutoPickup2;
			bool			bAutoPickup3;
			#endif

			#ifdef ENABLE_SPECIAL_STORAGE
			bool			bStorageOpenWithInventory;
			#endif

			#ifdef ENABLE_DISCORD_STUFF
			bool			bMessage;
			#endif

			#ifdef ENABLE_NEW_GAMEOPTION
			bool			questLetter;
			bool			affectIcons;
			#endif

			#ifdef ENABLE_PERFORMANCE_REWORK
			bool			bExtremeFpsBoost;
			#endif

			#ifdef ENABLE_EFFECT_SCALE
			FLOAT			effect_scale;
			#endif

			#ifdef ENABLE_POLY_ALL_MOBS_DOG
			bool			bPolyDogMode;
			#endif

			bool			bYangDropMode;

		} TConfig;

	public:
		CPythonSystem();
		virtual ~CPythonSystem();

		void Clear();
		void SetInterfaceHandler(PyObject * poHandler);
		void DestroyInterfaceHandler();

		// Config
		void							SetDefaultConfig();
		bool							LoadConfig();
		bool							SaveConfig();
		void							ApplyConfig();
		void							SetConfig(TConfig * set_config);
		TConfig *						GetConfig();
		void							ChangeSystem();

		// Interface
		bool							LoadInterfaceStatus();
		void							SaveInterfaceStatus();
		bool							isInterfaceConfig();
		const TWindowStatus &			GetWindowStatusReference(int iIndex);

		DWORD							GetWidth();
		DWORD							GetHeight();
		DWORD							GetBPP();
		DWORD							GetFrequency();
		bool							IsSoftwareCursor();
		bool							IsWindowed();
		bool							IsViewChat();
		bool							IsAlwaysShowName();
		bool							IsShowDamage();
		bool							IsShowSalesText();
		bool							IsUseDefaultIME();
		bool							IsNoSoundCard();
		bool							IsAutoTiling();
		bool							IsSoftwareTiling();
		void							SetSoftwareTiling(bool isEnable);
		void							SetViewChatFlag(int iFlag);
		void							SetAlwaysShowNameFlag(int iFlag);
		void							SetShowDamageFlag(int iFlag);
		void							SetShowSalesTextFlag(int iFlag);
		void SetShowHighlight(int iFlag);
		bool IsShowHighlight();

		#ifdef ENABLE_FOG_FIX
		void							SetFogMode(int iFlag);
		bool							IsFogMode();
		#endif

		#ifdef ENABLE_FPS
		void							SetFPS(BYTE iFlag);
		BYTE							GetFPS() { return m_Config.iFps; }
		#endif

		#if defined(ENABLE_SHOW_NIGHT_SYSTEM)
		void							SetNightMode(int iFlag);
		bool							GetNightMode();
		#endif

		#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
		bool							IsShowMobAIFlag();
		void							SetShowMobAIFlagFlag(int iFlag);
		#endif

		#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
		bool							IsShowMobLevel();
		void							SetShowMobLevelFlag(int iFlag);
		#endif

		#ifdef ENABLE_CHAT_STOP
		void							SetChatStop(bool iFlag)								{ m_Config.bIsChatStop = iFlag; }
		bool							IsChatStop()										{ return m_Config.bIsChatStop; }
		#endif

		#ifdef ENABLE_HIDE_MOB_SYSTEM
		bool							IsHidePets()									{ return m_Config.bHidePets; }
		void							SetHidePets(bool iFlag)							{ m_Config.bHidePets = iFlag; }
		bool							IsHideMounts()										{ return m_Config.bHideMounts; }
		void							SetHideMounts(bool iFlag)							{ m_Config.bHideMounts = iFlag; }
		#endif

		#ifdef ENABLE_AUTO_PICKUP_SYSTEM
		void							SetAutoPickup(int iFlag);
		bool							IsAutoPickup();

		void							SetAutoPickup2(int iFlag);
		bool							IsAutoPickup2();

		void							SetAutoPickup3(int iFlag);
		bool							IsAutoPickup3();
		#endif

		#ifdef ENABLE_SPECIAL_STORAGE
		bool							IsStorageOpenWithInventory();
		void							SetStorageOpenWithInventory(int iFlag);
		#endif

		#ifdef ENABLE_EFFECT_SCALE
		float							GetEffectScale();
		void							SetEffectScale(float fScale);
		#endif

		#ifdef ENABLE_POLY_ALL_MOBS_DOG
		void							SetPolyDogMode(int iFlag);
		bool							IsPolyDogMode();
		#endif

		void							SetYangDropMode(int iFlag);
		bool							IsYangDropMode();

		// Window
		void							SaveWindowStatus(int iIndex, int iVisible, int iMinimized, int ix, int iy, int iHeight);

		// SaveID
		int								IsSaveID();
		const char *					GetSaveID();
		void							SetSaveID(int iValue, const char * c_szSaveID);

		/// Display
		void							GetDisplaySettings();

		int								GetResolutionCount();
		int								GetFrequencyCount(int index);
		bool							GetResolution(int index, OUT DWORD *width, OUT DWORD *height, OUT DWORD *bpp);
		bool							GetFrequency(int index, int freq_index, OUT DWORD *frequncy);
		int								GetResolutionIndex(DWORD width, DWORD height, DWORD bpp);
		int								GetFrequencyIndex(int res_index, DWORD frequency);
		bool							isViewCulling();

		// Sound
		float							GetMusicVolume();
		int								GetSoundVolume();
		void							SetMusicVolume(float fVolume);
		void							SetSoundVolumef(float fVolume);

		int								GetDistance();
		int								GetShadowLevel();
		void							SetShadowLevel(unsigned int level);

		#ifdef ENABLE_FOV_OPTION
		float							GetFOVLevel();
		void							SetFOVLevel(float fFOV);
		#endif

		#ifdef ENABLE_NEW_GAMEOPTION
		void							SetQuestLetter(bool flag) { m_Config.questLetter = flag; }
		bool							GetQuestLetter() { return m_Config.questLetter; }
		void							SetAffectIcons(bool flag) { m_Config.affectIcons = flag; }
		bool							GetAffectIcons() { return m_Config.affectIcons; }
		#endif

		#ifdef ENABLE_DISCORD_STUFF
		bool							GetStuffMessage() { return m_Config.bMessage; }
		void							SetStuffMessage(bool flag) { m_Config.bMessage = flag; }
		#endif

		#ifdef ENABLE_PERFORMANCE_REWORK
		void							SetExtremeFpsBoost(bool bVal);
		bool							GetExtremeFpsBoost() { return m_Config.bExtremeFpsBoost; }
		#endif

	protected:
		TResolution						m_ResolutionList[RESOLUTION_MAX_NUM];
		int								m_ResolutionCount;

		TConfig							m_Config;
		TConfig							m_OldConfig;

		bool							m_isInterfaceConfig;
		PyObject *						m_poInterfaceHandler;
		TWindowStatus					m_WindowStatus[WINDOW_MAX_NUM];
};
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
