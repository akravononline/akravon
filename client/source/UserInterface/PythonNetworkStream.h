#pragma once

#include "../eterLib/FuncObject.h"
#include "../eterlib/NetStream.h"
#include "../eterLib/NetPacketHeaderMap.h"

#include "InsultChecker.h"

#include "packet.h"

#ifdef ENABLE_SWITCHBOT
#include "PythonSwitchbot.h"
#endif

class CInstanceBase;
class CNetworkActorManager;
struct SNetworkActorData;
struct SNetworkUpdateActorData;

class CPythonNetworkStream : public CNetworkStream, public CSingleton<CPythonNetworkStream>
{
	public:
		enum
		{
			SERVER_COMMAND_LOG_OUT = 0,
			SERVER_COMMAND_RETURN_TO_SELECT_CHARACTER = 1,
			SERVER_COMMAND_QUIT = 2,

			MAX_ACCOUNT_PLAYER
		};

		enum
		{
			ERROR_NONE,
			ERROR_UNKNOWN,
			ERROR_CONNECT_MARK_SERVER,
			ERROR_LOAD_MARK,
			ERROR_MARK_WIDTH,
			ERROR_MARK_HEIGHT,

			// MARK_BUG_FIX
			ERROR_MARK_UPLOAD_NEED_RECONNECT,
			ERROR_MARK_CHECK_NEED_RECONNECT,
			// END_OF_MARK_BUG_FIX
		};

		enum
		{
			ACCOUNT_CHARACTER_SLOT_ID,
			ACCOUNT_CHARACTER_SLOT_NAME,
			ACCOUNT_CHARACTER_SLOT_RACE,
			ACCOUNT_CHARACTER_SLOT_LEVEL,
			ACCOUNT_CHARACTER_SLOT_STR,
			ACCOUNT_CHARACTER_SLOT_DEX,
			ACCOUNT_CHARACTER_SLOT_HTH,
			ACCOUNT_CHARACTER_SLOT_INT,
			ACCOUNT_CHARACTER_SLOT_PLAYTIME,
			ACCOUNT_CHARACTER_SLOT_FORM,
			ACCOUNT_CHARACTER_SLOT_ADDR,
			ACCOUNT_CHARACTER_SLOT_PORT,
			ACCOUNT_CHARACTER_SLOT_GUILD_ID,
			ACCOUNT_CHARACTER_SLOT_GUILD_NAME,
			ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG,
			ACCOUNT_CHARACTER_SLOT_HAIR,
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
			ACCOUNT_CHARACTER_SLOT_ACCE,
#endif
		};

		enum
		{
			PHASE_WINDOW_LOGO,
			PHASE_WINDOW_LOGIN,
			PHASE_WINDOW_SELECT,
			PHASE_WINDOW_CREATE,
			PHASE_WINDOW_LOAD,
			PHASE_WINDOW_GAME,
			PHASE_WINDOW_EMPIRE,
			PHASE_WINDOW_NUM,
		};

	public:
		CPythonNetworkStream();
		virtual ~CPythonNetworkStream();

		bool SendSpecial(int nLen, void * pvBuf);

		void StartGame();
		void Warp(LONG lGlobalX, LONG lGlobalY);

		void NotifyHack(const char* c_szMsg);
		void SetWaitFlag();

		#ifdef ENABLE_DISCORD_RPC
		void Discord_Start();
		void Discord_Close();
		void Discord_Update(const bool login, const bool select, const bool loading);
		const char* GetStaffRank();
		const char* GetRaceIcon();
		const char* GetRaceName();
		const char* GetEmpireIcon();
		const char* GetEmpireName();
		const char* GetMapName();
		#endif

		void SendEmoticon(UINT eEmoticon);

		void ExitApplication();
		void ExitGame();
		void LogOutGame();
		void AbsoluteExitGame();
		void AbsoluteExitApplication();

		void EnableChatInsultFilter(bool isEnable);
		bool IsChatInsultIn(const char* c_szMsg);
		bool IsInsultIn(const char* c_szMsg);

		DWORD GetGuildID();

		UINT UploadMark(const char* c_szImageFileName);
		UINT UploadSymbol(const char* c_szImageFileName);

		bool LoadInsultList(const char* c_szInsultListFileName);
		bool LoadConvertTable(DWORD dwEmpireID, const char* c_szFileName);

		UINT		GetAccountCharacterSlotDatau(UINT iSlot, UINT eType);
		const char* GetAccountCharacterSlotDataz(UINT iSlot, UINT eType);

		// SUPPORT_BGM
		const char*		GetFieldMusicFileName();
		float			GetFieldMusicVolume();
		// END_OF_SUPPORT_BGM

		bool IsSelectedEmpire();

		void ToggleGameDebugInfo();

		void SetMarkServer(const char* c_szAddr, UINT uPort);
		void ConnectLoginServer(const char* c_szAddr, UINT uPort);
		void ConnectGameServer(UINT iChrSlot);

		void SetLoginInfo(
			const char* c_szID
			, const char* c_szPassword
			#ifdef ENABLE_SERVER_PIN
			, const char* c_szPin
			#endif
			#ifdef ENABLE_PCBAN_SYSTEM
			, int c_iBan
			#endif
		);

		std::string GetLoginID() const { return m_stID; }

		void SetLoginKey(DWORD dwLoginKey);
		void ClearLoginInfo( void );

		void SetHandler(PyObject* poHandler);
		void SetPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd);
		void ClearPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd);
		void SetServerCommandParserWindow(PyObject* poPhaseWnd);

		bool SendSyncPositionElementPacket(DWORD dwVictimVID, DWORD dwVictimX, DWORD dwVictimY);

		bool SendAttackPacket(UINT uMotAttack, DWORD dwVIDVictim);

		bool SendCharacterStatePacket(const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg);
		bool SendUseSkillPacket(DWORD dwSkillIndex, DWORD dwTargetVID=0);
		bool SendTargetPacket(DWORD dwVID);

		// OLDCODE:
		bool SendCharacterStartWalkingPacket(float fRotation, LONG lx, LONG ly);
		bool SendCharacterEndWalkingPacket(float fRotation, LONG lx, LONG ly);
		bool SendCharacterCheckWalkingPacket(float fRotation, LONG lx, LONG ly);

		bool SendCharacterPositionPacket(BYTE iPosition);

		bool SendItemUsePacket(TItemPos pos);
		bool SendItemUseToItemPacket(TItemPos source_pos, TItemPos target_pos);

		#ifdef ENABLE_DROP_DIALOG_EXTENDED_SYSTEM
		bool SendItemDeletePacket(TItemPos item_pos);
		bool SendItemSellPacket(TItemPos item_pos, bool is_auto);
		#else
		bool SendItemDropPacket(TItemPos pos, DWORD elk);
		bool SendItemDropPacketNew(TItemPos pos, DWORD elk, DWORD count);
		#endif

		#ifdef ENABLE_NEW_MAX_COUNT
		bool SendItemMovePacket(TItemPos pos, TItemPos change_pos, short num);
		#else
		bool SendItemMovePacket(TItemPos pos, TItemPos change_pos, BYTE num);
		#endif

		bool SendItemPickUpPacket(DWORD vid);

		bool SendQuickSlotAddPacket(BYTE wpos, BYTE type, BYTE pos);
		bool SendQuickSlotDelPacket(BYTE wpos);
		bool SendQuickSlotMovePacket(BYTE wpos, BYTE change_pos);

		bool SendPointResetPacket();

		#ifdef ENABLE_FAST_OPEN_CHEST
		bool SendInstantChestOpenPacket(TItemPos iPos);
		#endif

		#ifdef ENABLE_BLACK_MARKET_SYSTEM
		bool SendOpenBlackMarketPacket();
		bool SendItemBlackMarketPacket(TItemPos pos);
		#endif

		// Shop
		bool SendShopEndPacket();
		bool SendShopBuyPacket(BYTE byCount);
		bool SendShopSellPacket(BYTE bySlot);

	#ifdef ENABLE_SPECIAL_STORAGE
		#ifdef ENABLE_NEW_MAX_COUNT
		bool SendShopSellPacketNew(BYTE bySlot, short byCount, BYTE byType);
		#else
		bool SendShopSellPacketNew(BYTE bySlot, BYTE byCount, BYTE byType);
		#endif
	#else
		#ifdef ENABLE_NEW_MAX_COUNT
		bool SendShopSellPacketNew(BYTE bySlot, short byCount);
		#else
		bool SendShopSellPacketNew(BYTE bySlot, BYTE byCount);
		#endif
	#endif

		// Exchange
		bool SendExchangeStartPacket(DWORD vid);
		bool SendExchangeItemAddPacket(TItemPos ItemPos, BYTE byDisplayPos);

		#ifdef ENABLE_FULL_YANG
		bool SendExchangeElkAddPacket(uint64_t elk);
		#else
		bool SendExchangeElkAddPacket(DWORD elk);
		#endif

		bool SendExchangeItemDelPacket(BYTE pos);
		bool SendExchangeAcceptPacket();
		bool SendExchangeExitPacket();

		// Quest
		bool SendScriptAnswerPacket(int iAnswer);
		bool SendScriptButtonPacket(unsigned int iIndex);
		bool SendAnswerMakeGuildPacket(const char * c_szName);
		bool SendQuestInputStringPacket(const char * c_szString);
		bool SendQuestConfirmPacket(BYTE byAnswer, DWORD dwPID);

		// Event
		bool SendOnClickPacket(DWORD vid);

		// Fly
		bool SendFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition& kPPosTarget);
		bool SendAddFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition& kPPosTarget);
		bool SendShootPacket(UINT uSkill);

		// Command
		bool ClientCommand(const char * c_szCommand);
		void ServerCommand(const char * c_szCommand);

		// Emoticon
		void RegisterEmoticonString(const char * pcEmoticonString);

		// Party
		bool SendPartyInvitePacket(DWORD dwVID);
		bool SendPartyInviteAnswerPacket(DWORD dwLeaderVID, BYTE byAccept);
		bool SendPartyRemovePacket(DWORD dwPID);
		bool SendPartySetStatePacket(DWORD dwVID, BYTE byState, BYTE byFlag);
		bool SendPartyUseSkillPacket(BYTE bySkillIndex, DWORD dwVID);
		bool SendPartyParameterPacket(BYTE byDistributeMode);

		// SafeBox
		bool SendSafeBoxMoneyPacket(BYTE byState, DWORD dwMoney);
		bool SendSafeBoxCheckinPacket(TItemPos InventoryPos, BYTE bySafeBoxPos);
		bool SendSafeBoxCheckoutPacket(BYTE bySafeBoxPos, TItemPos InventoryPos);

		#ifdef ENABLE_NEW_MAX_COUNT
		bool SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, short byCount);
		#else
		bool SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, BYTE byCount);
		#endif

		// Mall
		bool SendMallCheckoutPacket(BYTE byMallPos, TItemPos InventoryPos);
		#if defined(__BL_LUCKY_BOX__)
		bool SendLuckyBoxActionPacket(BYTE bAction);
		bool RecvLuckyBoxPacket();
		#endif
		// Guild
		bool SendGuildAddMemberPacket(DWORD dwVID);
		bool SendGuildRemoveMemberPacket(DWORD dwPID);
		bool SendGuildChangeGradeNamePacket(BYTE byGradeNumber, const char * c_szName);
		bool SendGuildChangeGradeAuthorityPacket(BYTE byGradeNumber, BYTE byAuthority);
		bool SendGuildOfferPacket(DWORD dwExperience);
		bool SendGuildPostCommentPacket(const char * c_szMessage);
		bool SendGuildDeleteCommentPacket(DWORD dwIndex);
		bool SendGuildRefreshCommentsPacket(DWORD dwHighestIndex);
		bool SendGuildChangeMemberGradePacket(DWORD dwPID, BYTE byGrade);
		bool SendGuildUseSkillPacket(DWORD dwSkillID, DWORD dwTargetVID);
		bool SendGuildChangeMemberGeneralPacket(DWORD dwPID, BYTE byFlag);
		bool SendGuildInvitePacket(DWORD dwVID);
		bool SendGuildInviteAnswerPacket(DWORD dwGuildID, BYTE byAnswer);
		bool SendGuildChargeGSPPacket(DWORD dwMoney);
		bool SendGuildDepositMoneyPacket(DWORD dwMoney);
		bool SendGuildWithdrawMoneyPacket(DWORD dwMoney);

		// Mall
		bool RecvMallOpenPacket();
		bool RecvMallItemSetPacket();
		bool RecvMallItemDelPacket();

	#ifdef ENABLE_IKASHOP_RENEWAL
		bool RecvOfflineshopPacket();

		bool RecvOfflineshopShopOpen();
		bool RecvOfflineshopShopOpenOwner();
		bool RecvOfflineshopShopOpenOwnerNoShop();
		bool RecvOfflineshopShopExpiredGuesting();
		bool RecvOfflineshopShopRemoveItem(bool owner);

		bool RecvOfflineshopShopGuestEditItem();
		bool RecvOfflineshopShopOwnerEditItem();

		bool RecvOfflineshopShopFilterResult();

		#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		bool RecvOfflineshopShopFilterNameResult();
		#endif

		#ifdef ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE
		bool RecvOfflineshopAvarageAndMinMaxPrice();
		#endif

		bool RecvOfflineshopOfferList();
		bool RecvOfflineshopShopSafeboxRefresh();
		bool RecvOfflineshopShopSafeboxRemoveItem();
		bool RecvOfflineshopShopSafeboxAddItem();

		bool RecvOfflineshopAuctionList();
		bool RecvOfflineshopOpenMyAuction();
		bool RecvOfflineshopOpenMyAuctionNoAuction();
		bool RecvOfflineshopOpenAuction();

		bool RecvOfflineshopPopupMessage();

		bool RecvOfflineshopBoardCounters();

		#ifdef ENABLE_IKASHOP_ENTITIES
		bool RecvOfflineshopInsertEntity();
		bool RecvOfflineshopRemoveEntity();

		void SendIkarusShopOnClickShopEntity(DWORD dwPickedShopVID);

		bool RecvOfflineshopSearchResultDelete();
		#endif

		void SendIkarusShopCreateShop(const ikashop::TShopInfo& shopInfo);
		void SendIkarusShopForceCloseShop();

		void SendIkarusShopRequestShopList();

		void SendIkarusShopOpenShop(DWORD dwOwnerID);
		void SendIkarusShopOpenShopOwner();

		void SendIkarusShopBuyItem(DWORD dwOwnerID, DWORD dwItemID, bool isSearch, long long TotalPriceSeen); //patch seen price check

		void SendIkarusShopAddItem(ikashop::TShopItemInfo& itemInfo);
		void SendIkarusShopRemoveItem(DWORD dwItemID);
		void SendOfflineShopEditItem(DWORD dwItemID, const ikashop::TPriceInfo& price);

		void SendIkarusShopFilterRequest(const ikashop::TFilterInfo& filter);
		void SendIkarusShopSearchFillRequest();

		#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		void SendIkarusShopFilterNameRequest(const ikashop::TFilterName& filter);
		void ShopFilterNameClear();
		#endif

		#ifdef ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE
		void SendIkarusShopAvarageAndMinMaxPrice(const uint32_t vnum);
		#endif

		void SendIkarusShopOfferCreate(const ikashop::TOfferInfo& offer);
		void SendIkarusShopOfferAccept(DWORD dwOfferID);
		void SendIkarusShopOfferCancel(DWORD dwOfferID, DWORD dwOwnerID);
		void SendIkarusShopOfferListRequest();

		void SendIkarusShopSafeboxOpen();
		void SendIkarusShopSafeboxGetItem(DWORD dwItemID);
		void SendIkarusShopSafeboxGetValutes();
		void SendIkarusShopSafeboxClose();

		//AUCTION
		void SendIkarusShopAuctionListRequest();
		void SendIkarusShopAuctionOpen(DWORD dwOwnerID);
		void SendIkarusShopAuctionAddOffer(DWORD dwOwnerID, const ikashop::TPriceInfo& price);
		void SendIkarusShopAuctionExitFrom(DWORD dwOwnerID);
		void SendIkarusShopAuctionCreate(const TItemPos& pos, const ikashop::TPriceInfo& price);
		void SendIkarusShopMyAuctionOpen();
		void SendIkarusShopMyAuctionClose();
		void SendIkarusShopMyAuctionCancel();

		void SendIkarusMyShopCloseBoard();
		void SendIkarusOffersListClose();
		void SendIkarusCloseShopGuestBoard();
	#endif

		// Lover
		bool RecvLoverInfoPacket();
		bool RecvLovePointUpdatePacket();

		// Dig
		bool RecvDigMotionPacket();

		// Fishing
		bool SendFishingPacket(int iRotation);
		bool SendGiveItemPacket(DWORD dwTargetVID, TItemPos ItemPos, int iItemCount);

		// Private Shop
		bool SendBuildPrivateShopPacket(const char * c_szName, const std::vector<TShopItemTable> & c_rSellingItemStock);

		// Refine
		bool SendRefinePacket(BYTE byPos, BYTE byType);
		bool SendSelectItemPacket(DWORD dwItemPos);

		// Client Version
		bool SendClientVersionPacket();

		// CRC Report
		bool __SendCRCReportPacket();

		bool SendDragonSoulRefinePacket(BYTE bRefineType, TItemPos* pos);

		// Handshake
		bool RecvHandshakePacket();
		bool RecvHandshakeOKPacket();

		bool RecvHybridCryptKeyPacket();
		bool RecvHybridCryptSDBPacket();

		#ifdef _IMPROVED_PACKET_ENCRYPTION_
		bool RecvKeyAgreementPacket();
		bool RecvKeyAgreementCompletedPacket();
		#endif

		// ETC
		DWORD GetMainActorVID();
		DWORD GetMainActorRace();
		DWORD GetMainActorEmpire();
		DWORD GetMainActorSkillGroup();
		void SetEmpireID(DWORD dwEmpireID);
		DWORD GetEmpireID();
		void __TEST_SetSkillGroupFake(int iIndex);

		#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		bool	SendAcceClosePacket();
		bool	SendAcceAddPacket(TItemPos tPos, BYTE bPos);
		bool	SendAcceRemovePacket(BYTE bPos);
		bool	SendAcceRefinePacket();
		#endif

		#ifdef ENABLE_SHOW_CHEST_DROP
		bool	SendChestDropInfo(int iInventoryCell);
		bool	SendChestDropVnumInfo(int iVnum);
		bool 	RecvChestDropInfo();
		#endif

		#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
		bool CubeRenewalMakeItem(int index_item, int count_item, int index_item_improve);
		bool CubeRenewalClose();
		bool RecvCubeRenewalPacket();
		#endif

		#ifdef ENABLE_EVENT_MANAGER
		bool RecvEventManager();
		#endif

		#ifdef ENABLE_SKILL_RENEWAL
		bool RecvSkillBookCountPacket();
		#endif

		#ifdef ENABLE_OFFER_SYSTEM
		bool RecvOfferListPacket();
		#endif

		#ifdef ENABLE_EXCHANGE_LOG
		bool RecvExchangeLog();
		#endif

		#ifdef ENABLE_RONARK_SYSTEM
		bool RecvRonarkInfoPacket();
		#endif

		#ifdef ENABLE_AUTO_ITEM_SELL_SYSTEM
		bool SendAutoItemSell(BYTE bSubHeader, DWORD dwValue);
		bool RecvAutoItemSellPacket();
		#endif

		#ifdef ENABLE_DRAGON_COIN_SHOP_SYSTEM
		bool SendItemShopPacket(BYTE subHeader, BYTE itemCount, DWORD itemID, BYTE itemPriceType, char* szCategoryStr);
		bool RecvItemShopGeneralPacket();
		#endif

		#ifdef ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM
		bool RecvItemShopGeneralAdminPacket();

		bool SendItemShopOpenAdminPagePacket();

		bool SendItemShopCateAddPacket(TGCIShopCateAdmin table);
		bool SendItemShopCateDeletePacket(TGCIShopCateAdmin table);

		bool SendItemShopSubCateAddPacket(TGCIShopCateAdmin table);
		bool SendItemShopSubCateDeletePacket(TGCIShopCateAdmin table);

		bool SendItemShopRenamePacket(TGCIShopCateAdmin table);

		bool SendItemShopItemAddPacket(TGCIShopItemAdmin table);
		bool SendItemShopItemUpdatePacket(TGCIShopItemAdmin table);
		bool SendItemShopItemDeletePacket(DWORD dwItemID);
		#endif

		#ifdef ENABLE_BAKIM_DUYURU
		bool RecvMaintenanceInfo();
		#endif

		#ifdef ENABLE_TANAKA_EVENT
		bool RecvTanakaEventPacket();
		#endif

		#ifdef ENABLE_TELEPORTION_WIZARD_SYSTEM
		bool RecvTpWizardInfoPacket();
		#endif

		#ifdef ENABLE_CHANGELOOK_SYSTEM
		bool	RecvChangeLookPacket();
		bool	SendClClosePacket();
		bool	SendClAddPacket(TItemPos tPos, BYTE bPos);
		bool	SendClRemovePacket(BYTE bPos);
		bool	SendClRefinePacket();
		#endif

		#ifdef BOSS_DMG_RANKING
		bool	RecvBossRankingPacket();
		#endif

		#ifdef ENABLE_BLACK_MARKET_SYSTEM
		bool	RecvBlackMarketPacket();
		#endif

		#ifdef ENABLE_REWARD_SYSTEM
		bool	RecvRewardInfoPacket();
		#endif

		#ifdef ENABLE_GUILD_STATISTICS_SYSTEM
		bool	RecvGuildStatisticsPacket();
		#endif

	#ifdef ENABLE_BATTLE_PASS
	public:
		bool SendBattlePassAction(BYTE bAction, int32_t iArg);

	protected:
		bool RecvBattlePassPacket();
		bool RecvBattlePassUpdatePacket();
	#endif

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	public:
		void SetOffLinePhase();
		void SetHandShakePhase();
		void SetLoginPhase();
		void SetSelectPhase();
		void SetLoadingPhase();
		void SetGamePhase();
		void ClosePhase();

		// Login Phase
		bool SendLoginPacket(const char * c_szName, const char * c_szPassword);
		bool SendLoginPacketNew(const char * c_szName);
		bool SendDirectEnterPacket(const char * c_szName, const char * c_szPassword, UINT uChrSlot);

		bool SendEnterGame();

		// Select Phase
		bool SendSelectEmpirePacket(DWORD dwEmpireID);
		bool SendSelectCharacterPacket(BYTE account_Index);
		bool SendChangeNamePacket(BYTE index, const char *name);
		bool SendCreateCharacterPacket(BYTE index, const char *name, BYTE job, BYTE shape, BYTE byStat1, BYTE byStat2, BYTE byStat3, BYTE byStat4);
		bool SendDestroyCharacterPacket(BYTE index, const char * szPrivateCode);

		// Main Game Phase
		bool SendC2CPacket(DWORD dwSize, void * pData);
		bool SendChatPacket(const char * c_szChat, BYTE byType = CHAT_TYPE_TALKING);
		bool SendWhisperPacket(const char * name, const char * c_szChat);
		bool SendMobileMessagePacket(const char * name, const char * c_szChat);
		bool SendMessengerAddByVIDPacket(DWORD vid);
		bool SendMessengerAddByNamePacket(const char * c_szName);
		bool SendMessengerRemovePacket(const char * c_szKey, const char * c_szName);

		#ifdef ENABLE_DISCORD_STUFF
		std::string GetPhaseName() { return m_strPhase; }
		#endif

		#ifdef ENABLE_MESSENGER_BLOCK
		bool SendMessengerAddBlockByVIDPacket(DWORD vid);
		bool SendMessengerAddBlockByNamePacket(const char * c_szName);
		bool SendMessengerRemoveBlockPacket(const char * c_szKey, const char * c_szName);
		#endif

		#ifdef ENABLE_PM_ALL_SEND_SYSTEM
		bool SendBulkWhisperPacket(const char * c_szContent);
		#endif

		#ifdef ENABLE_WHEEL_OF_FORTUNE
		bool RecvWheelOfFortunePacket();
		#endif

		#ifdef ENABLE_PUBLISHER_SYSTEM
		bool RecvPublisherInfoPacket();
		#endif

	protected:
		bool OnProcess();
		void OffLinePhase();
		void HandShakePhase();
		void LoginPhase();
		void SelectPhase();
		void LoadingPhase();
		void GamePhase();

		bool __IsNotPing();

		void __DownloadMark();
		void __DownloadSymbol(const std::vector<DWORD> & c_rkVec_dwGuildID);

		void __PlayInventoryItemUseSound(TItemPos uSlotPos);
		void __PlayInventoryItemDropSound(TItemPos uSlotPos);
		//void __PlayShopItemDropSound(UINT uSlotPos);
		void __PlaySafeBoxItemDropSound(UINT uSlotPos);
		void __PlayMallItemDropSound(UINT uSlotPos);

		bool __CanActMainInstance();

		enum REFRESH_WINDOW_TYPE
		{
			RefreshStatus = (1 << 0),
			RefreshAlignmentWindow = (1 << 1),
			RefreshCharacterWindow = (1 << 2),
			RefreshEquipmentWindow = (1 << 3),
			RefreshInventoryWindow = (1 << 4),
			RefreshExchangeWindow = (1 << 5),
			RefreshSkillWindow = (1 << 6),
			RefreshSafeboxWindow  = (1 << 7),
			RefreshMessengerWindow = (1 << 8),
			RefreshGuildWindowInfoPage = (1 << 9),
			RefreshGuildWindowBoardPage = (1 << 10),
			RefreshGuildWindowMemberPage = (1 << 11),
			RefreshGuildWindowMemberPageGradeComboBox = (1 << 12),
			RefreshGuildWindowSkillPage = (1 << 13),
			RefreshGuildWindowGradePage = (1 << 14),
			RefreshTargetBoard = (1 << 15),
			RefreshMallWindow = (1 << 16),
		};

		void __RefreshStatus();
		void __RefreshAlignmentWindow();
		void __RefreshCharacterWindow();
		void __RefreshEquipmentWindow();
		void __RefreshInventoryWindow();
		void __RefreshExchangeWindow();
		void __RefreshSkillWindow();
		void __RefreshSafeboxWindow();
		void __RefreshMessengerWindow();
		void __RefreshGuildWindowInfoPage();
		void __RefreshGuildWindowBoardPage();
		void __RefreshGuildWindowMemberPage();
		void __RefreshGuildWindowMemberPageGradeComboBox();
		void __RefreshGuildWindowSkillPage();
		void __RefreshGuildWindowGradePage();
		void __RefreshTargetBoardByVID(DWORD dwVID);
		void __RefreshTargetBoardByName(const char * c_szName);
		void __RefreshTargetBoard();
		void __RefreshMallWindow();

		bool __SendHack(const char* c_szMsg);

	public:
		bool Rapor(const char* c_szMsg);


	protected:
		bool RecvObserverAddPacket();
		bool RecvObserverRemovePacket();
		bool RecvObserverMovePacket();

		// Common
		bool RecvErrorPacket(int header);
		bool RecvPingPacket();
		bool RecvDefaultPacket(int header);
		bool RecvPhasePacket();

		// Login Phase
		bool __RecvLoginSuccessPacket3();
		bool __RecvLoginSuccessPacket4();
		bool __RecvLoginFailurePacket();
		bool __RecvEmpirePacket();
		bool __RecvLoginKeyPacket();

		// Select Phase
		bool __RecvPlayerCreateSuccessPacket();
		bool __RecvPlayerCreateFailurePacket();
		bool __RecvPlayerDestroySuccessPacket();
		bool __RecvPlayerDestroyFailurePacket();
		bool __RecvPreserveItemPacket();
		bool __RecvPlayerPoints();
		bool __RecvChangeName();
#ifdef ENABLE_FULL_YANG
		bool __RecvPlayerGold();
#endif

		// Loading Phase
		bool RecvMainCharacter();
		bool RecvMainCharacter2_EMPIRE();
		bool RecvMainCharacter3_BGM();
		bool RecvMainCharacter4_BGM_VOL();

		void __SetFieldMusicFileName(const char* musicName);
		void __SetFieldMusicFileInfo(const char* musicName, float vol);
		// END_OF_SUPPORT_BGM

		// Main Game Phase
		bool RecvWarpPacket();

		#ifdef ENABLE_ENTITY_PRELOADING
		bool RecvPreloadEntitiesPacket();
		#endif

		bool RecvPVPPacket();
		bool RecvDuelStartPacket();
		bool RecvGlobalTimePacket();
		bool RecvCharacterAppendPacket();
		bool RecvCharacterAdditionalInfo();
		bool RecvCharacterAppendPacketNew();
		bool RecvCharacterUpdatePacket();
		bool RecvCharacterDeletePacket();
		bool RecvCharacterAffectResetPacket();
		bool RecvChatPacket();
		bool RecvOwnerShipPacket();
		bool RecvSyncPositionPacket();
		bool RecvWhisperPacket();
		bool RecvPointChange();						// Alarm to python
		bool RecvChangeSpeedPacket();

		bool RecvStunPacket();
		bool RecvDeadPacket();
		bool RecvCharacterMovePacket();

		bool RecvItemDelPacket();					// Alarm to python
		bool RecvItemSetPacket();					// Alarm to python
		bool RecvItemUsePacket();					// Alarm to python
		bool RecvItemUpdatePacket();				// Alarm to python
		bool RecvItemGroundAddPacket();
		bool RecvItemGroundDelPacket();
		bool RecvItemOwnership();

		bool RecvQuickSlotAddPacket();				// Alarm to python
		bool RecvQuickSlotDelPacket();				// Alarm to python
		bool RecvQuickSlotMovePacket();				// Alarm to python

		bool RecvCharacterPositionPacket();
		bool RecvMotionPacket();

		bool RecvShopPacket();
		bool RecvShopSignPacket();
		bool RecvExchangePacket();

		// Quest
		bool RecvScriptPacket();
		bool RecvQuestInfoPacket();
		bool RecvQuestConfirmPacket();
		bool RecvRequestMakeGuild();

		// Skill
		bool RecvSkillLevel();
		bool RecvSkillLevelNew();
		bool RecvSkillCoolTimeEnd();

		// Target
		bool RecvTargetPacket();
		bool RecvViewEquipPacket();
		bool RecvDamageInfoPacket();

		// Mount
		bool RecvMountPacket();

		// Fly
		bool RecvCreateFlyPacket();
		bool RecvFlyTargetingPacket();
		bool RecvAddFlyTargetingPacket();
	
		//Target
#ifdef ENABLE_SEND_TARGET_INFO
		bool RecvTargetInfoPacket();

		public:
			bool SendTargetInfoLoadPacket(DWORD dwVID);

		protected:
#endif
		
		// Messenger
		bool RecvMessenger();

		// Guild
		bool RecvGuild();

		// Party
		bool RecvPartyInvite();
		bool RecvPartyAdd();
		bool RecvPartyUpdate();
		bool RecvPartyRemove();
		bool RecvPartyLink();
		bool RecvPartyUnlink();
		bool RecvPartyParameter();

		// SafeBox
		bool RecvSafeBoxSetPacket();
		bool RecvSafeBoxDelPacket();
		bool RecvSafeBoxWrongPasswordPacket();
		bool RecvSafeBoxSizePacket();
		bool RecvSafeBoxMoneyChangePacket();

		// Fishing
		bool RecvFishing();

		// Dungeon
		bool RecvDungeon();

		// Time
		bool RecvTimePacket();

		// WalkMode
		bool RecvWalkModePacket();

		// ChangeSkillGroup
		bool RecvChangeSkillGroupPacket();

		// Refine
		bool RecvRefineInformationPacket();
		bool RecvRefineInformationPacketNew();

		// Use Potion
		bool RecvSpecialEffect();

		bool RecvSpecificEffect();

		bool RecvDragonSoulRefine();

		// MiniMap Info
		bool RecvNPCList();
		bool RecvLandPacket();
		bool RecvTargetCreatePacket();
		bool RecvTargetCreatePacketNew();
		bool RecvTargetUpdatePacket();
		bool RecvTargetDeletePacket();

		// Affect
		bool RecvAffectAddPacket();
		bool RecvAffectRemovePacket();

		// Channel
		bool RecvChannelPacket();

		#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		bool RecvAccePacket(bool bReturn = false);
		#endif

		#ifdef ENABLE_FULL_YANG
		bool RecvGoldChange();
		#endif

		#ifdef __RANKING_SYSTEM__
		bool	RecvRankUpdate();
		#endif

		#ifdef ENABLE_PM_ALL_SEND_SYSTEM
		bool RecvBulkWhisperPacket();
		#endif
#ifdef SPIN_WHEEL
	bool RecvSpinWheelPacket();
	bool RecvSpinWheelRewardsPacket();
#endif

	protected:
		bool ParseEmoticon(const char * pChatMsg, DWORD * pdwEmoticon);

		void OnConnectFailure();
		void OnScriptEventStart(int iSkin, int iIndex);

		void OnRemoteDisconnect();
		void OnDisconnect();

		void SetGameOnline();
		void SetGameOffline();
		BOOL IsGameOnline();

	protected:
		bool CheckPacket(TPacketHeader * pRetHeader);

		void __InitializeGamePhase();
		void __InitializeMarkAuth();
		void __GlobalPositionToLocalPosition(LONG& rGlobalX, LONG& rGlobalY);
		void __LocalPositionToGlobalPosition(LONG& rLocalX, LONG& rLocalY);

		bool __IsPlayerAttacking();
		bool __IsEquipItemInSlot(TItemPos Cell);

		void __ShowMapName(LONG lLocalX, LONG lLocalY);

		void __LeaveOfflinePhase() {}
		void __LeaveHandshakePhase() {}
		void __LeaveLoginPhase() {}
		void __LeaveSelectPhase() {}
		void __LeaveLoadingPhase() {}
		void __LeaveGamePhase();

		void __ClearNetworkActorManager();

		void __ClearSelectCharacterData();

		// DELETEME
		//void __SendWarpPacket();

		void __ConvertEmpireText(DWORD dwEmpireID, char* szText);

		void __RecvCharacterAppendPacket(SNetworkActorData * pkNetActorData);
		void __RecvCharacterUpdatePacket(SNetworkUpdateActorData * pkNetUpdateActorData);

		void __FilterInsult(char* szLine, UINT uLineLen);

		void __SetGuildID(DWORD id);

	protected:
		TPacketGCHandshake m_HandshakeData;
		DWORD m_dwChangingPhaseTime;
		DWORD m_dwBindupRetryCount;
		DWORD m_dwMainActorVID;
		DWORD m_dwMainActorRace;
		DWORD m_dwMainActorEmpire;
		DWORD m_dwMainActorSkillGroup;
		BOOL m_isGameOnline;
		BOOL m_isStartGame;

		DWORD m_dwGuildID;

		DWORD m_dwEmpireID;

		struct SServerTimeSync
		{
			DWORD m_dwChangeServerTime;
			DWORD m_dwChangeClientTime;
		} m_kServerTimeSync;

		void __ServerTimeSync_Initialize();
		//DWORD m_dwBaseServerTime;
		//DWORD m_dwBaseClientTime;

		#ifdef ENABLE_CYPOR_ENGEL
		DWORD m_dwLastStatus;
		#endif

		DWORD m_dwLastGamePingTime;

		std::string	m_stID;
		std::string	m_stPassword;
		#ifdef ENABLE_SERVER_PIN
		std::string	m_stPin;
		#endif
		#ifdef ENABLE_PCBAN_SYSTEM
		int			m_intBan;
		#endif
		std::string	m_strLastCommand;
		std::string	m_strPhase;
		DWORD		m_dwLoginKey;
		BOOL		m_isWaitLoginKey;

		std::string m_stMarkIP;

		CFuncObject<CPythonNetworkStream>	m_phaseProcessFunc;
		CFuncObject<CPythonNetworkStream>	m_phaseLeaveFunc;

		PyObject*							m_poHandler;
		PyObject*							m_apoPhaseWnd[PHASE_WINDOW_NUM];
		PyObject*							m_poSerCommandParserWnd;

		TSimplePlayerInformation			m_akSimplePlayerInfo[PLAYER_PER_ACCOUNT4];
		DWORD								m_adwGuildID[PLAYER_PER_ACCOUNT4];
		std::string							m_astrGuildName[PLAYER_PER_ACCOUNT4];
		bool m_bSimplePlayerInfo;

		CRef<CNetworkActorManager>			m_rokNetActorMgr;

		bool m_isRefreshStatus;
		bool m_isRefreshCharacterWnd;
		bool m_isRefreshEquipmentWnd;
		bool m_isRefreshInventoryWnd;
		bool m_isRefreshExchangeWnd;
		bool m_isRefreshSkillWnd;
		bool m_isRefreshSafeboxWnd;
		bool m_isRefreshMallWnd;
		bool m_isRefreshMessengerWnd;
		bool m_isRefreshGuildWndInfoPage;
		bool m_isRefreshGuildWndBoardPage;
		bool m_isRefreshGuildWndMemberPage;
		bool m_isRefreshGuildWndMemberPageGradeComboBox;
		bool m_isRefreshGuildWndSkillPage;
		bool m_isRefreshGuildWndGradePage;

		// Emoticon
		std::vector<std::string> m_EmoticonStringVector;

		struct STextConvertTable
		{
			char acUpper[26];
			char acLower[26];
			BYTE aacHan[5000][2];
		} m_aTextConvTable[3];



		struct SMarkAuth
		{
			CNetworkAddress m_kNetAddr;
			DWORD m_dwHandle;
			DWORD m_dwRandomKey;
		} m_kMarkAuth;



		DWORD m_dwSelectedCharacterIndex;

		CInsultChecker m_kInsultChecker;

		bool m_isEnableChatInsultFilter;
		bool m_bComboSkillFlag;

		std::deque<std::string> m_kQue_stHack;

	private:
		struct SDirectEnterMode
		{
			bool m_isSet;
#ifdef ENABLE_NEW_LOADING
			bool m_isWarp;
#endif

			DWORD m_dwChrSlotIndex;
		} m_kDirectEnterMode;

		void __DirectEnterMode_Initialize();
#ifdef ENABLE_NEW_LOADING
		void __DirectEnterMode_Set(UINT uChrSlotIndex, bool bisWarp = false);
		bool __DirectEnterMode_IsWarp();
#else
		void __DirectEnterMode_Set(UINT uChrSlotIndex);
#endif
		bool __DirectEnterMode_IsSet();

	public:
		DWORD EXPORT_GetBettingGuildWarValue(const char* c_szValueName);
		std::string GetPhase() { return m_strPhase; };

	private:
		struct SBettingGuildWar
		{
			DWORD m_dwBettingMoney;
			DWORD m_dwObserverCount;
		} m_kBettingGuildWar;

		CInstanceBase * m_pInstTarget;

		void __BettingGuildWar_Initialize();
		void __BettingGuildWar_SetObserverCount(UINT uObserverCount);
		void __BettingGuildWar_SetBettingMoney(UINT uBettingMoney);

#ifdef ENABLE_SWITCHBOT
	public:
		bool RecvSwitchbotPacket();

		bool SendSwitchbotStartPacket(BYTE slot, std::vector<CPythonSwitchbot::TSwitchbotAttributeAlternativeTable> alternatives);
		bool SendSwitchbotStopPacket(BYTE slot);
#endif

#ifdef ENABLE_STYLE_ATTRIBUTE_SYSTEM
	public:
		bool SendItemNewAttributePacket(TItemPos source_pos, TItemPos target_pos, BYTE * bValues);
#endif

#ifdef ENABLE_HIDE_COSTUME_PART
	public:
		bool ChangeCostumeVisible(BYTE type, bool status);
		bool UpdateCostumeVisibleSettings();
#endif
};
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240

