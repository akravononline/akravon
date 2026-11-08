#include "StdAfx.h"
#include "PythonNetworkStream.h"
#ifdef ENABLE_RASCAL_ANTICHEAT
#include "rascal.h"
#endif

#include "Packet.h"

#include "PythonGuild.h"
#include "PythonCharacterManager.h"
#include "PythonPlayer.h"
#include "PythonBackground.h"
#include "PythonMiniMap.h"
#include "PythonTextTail.h"
#include "PythonItem.h"
#include "PythonChat.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonQuest.h"
#include "PythonEventManager.h"
#include "PythonMessenger.h"
#include "PythonApplication.h"

#include "../EterPack/EterPackManager.h"
#include "../gamelib/ItemManager.h"

#include "AbstractApplication.h"
#include "AbstractCharacterManager.h"
#include "InstanceBase.h"

#include "ProcessCRC.h"

#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
#include "PythonCubeRenewal.h"
#endif

BOOL gs_bEmpireLanuageEnable = TRUE;

void CPythonNetworkStream::__RefreshAlignmentWindow()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshAlignment", Py_BuildValue("()"));
}

void CPythonNetworkStream::__RefreshTargetBoardByVID(DWORD dwVID)
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoardByVID", Py_BuildValue("(i)", dwVID));
}

void CPythonNetworkStream::__RefreshTargetBoardByName(const char * c_szName)
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoardByName", Py_BuildValue("(s)", c_szName));
}

void CPythonNetworkStream::__RefreshTargetBoard()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoard", Py_BuildValue("()"));
}

void CPythonNetworkStream::__RefreshGuildWindowGradePage()
{
	m_isRefreshGuildWndGradePage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowSkillPage()
{
	m_isRefreshGuildWndSkillPage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPageGradeComboBox()
{
	m_isRefreshGuildWndMemberPageGradeComboBox=true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPage()
{
	m_isRefreshGuildWndMemberPage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowBoardPage()
{
	m_isRefreshGuildWndBoardPage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowInfoPage()
{
	m_isRefreshGuildWndInfoPage=true;
}

void CPythonNetworkStream::__RefreshMessengerWindow()
{
	m_isRefreshMessengerWnd=true;
}

void CPythonNetworkStream::__RefreshSafeboxWindow()
{
	m_isRefreshSafeboxWnd=true;
}

void CPythonNetworkStream::__RefreshMallWindow()
{
	m_isRefreshMallWnd=true;
}

void CPythonNetworkStream::__RefreshSkillWindow()
{
	m_isRefreshSkillWnd=true;
}

void CPythonNetworkStream::__RefreshExchangeWindow()
{
	m_isRefreshExchangeWnd=true;
}

void CPythonNetworkStream::__RefreshStatus()
{
	m_isRefreshStatus=true;
}

void CPythonNetworkStream::__RefreshCharacterWindow()
{
	m_isRefreshCharacterWnd=true;
}

void CPythonNetworkStream::__RefreshInventoryWindow()
{
	m_isRefreshInventoryWnd=true;
}

void CPythonNetworkStream::__RefreshEquipmentWindow()
{
	m_isRefreshEquipmentWnd=true;
}

void CPythonNetworkStream::__SetGuildID(DWORD id)
{
	if (m_dwGuildID != id)
	{
		m_dwGuildID = id;
		IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();

		for (int i = 0; i < PLAYER_PER_ACCOUNT4; ++i)
			if (!strncmp(m_akSimplePlayerInfo[i].szName, rkPlayer.GetName(), CHARACTER_NAME_MAX_LEN))
			{
				m_adwGuildID[i] = id;

				std::string  guildName;
				if (CPythonGuild::Instance().GetGuildName(id, &guildName))
				{
					m_astrGuildName[i] = guildName;
				}
				else
				{
					m_astrGuildName[i] = "";
				}
			}
	}
}

struct PERF_PacketInfo
{
	DWORD dwCount;
	DWORD dwTime;

	PERF_PacketInfo()
	{
		dwCount=0;
		dwTime=0;
	}
};


#ifdef __PERFORMANCE_CHECK__

class PERF_PacketTimeAnalyzer
{
	public:
		~PERF_PacketTimeAnalyzer()
		{
			FILE* fp=fopen("perf_dispatch_packet_result.txt", "w");

			for (std::map<DWORD, PERF_PacketInfo>::iterator i=m_kMap_kPacketInfo.begin(); i!=m_kMap_kPacketInfo.end(); ++i)
			{
				if (i->second.dwTime>0)
					fprintf(fp, "header %d: count %d, time %d, tpc %d\n", i->first, i->second.dwCount, i->second.dwTime, i->second.dwTime/i->second.dwCount);
			}
			fclose(fp);
		}

	public:
		std::map<DWORD, PERF_PacketInfo> m_kMap_kPacketInfo;
};

PERF_PacketTimeAnalyzer gs_kPacketTimeAnalyzer;

#endif

// Game Phase ---------------------------------------------------------------------------
void CPythonNetworkStream::GamePhase()
{
	if (!m_kQue_stHack.empty())
	{
		Rapor(m_kQue_stHack.front().c_str());
		m_kQue_stHack.pop_front();
	}

	TPacketHeader header = 0;
	bool ret = true;

	#ifdef __PERFORMANCE_CHECK__
	DWORD timeBeginDispatch=timeGetTime();

	static std::map<DWORD, PERF_PacketInfo> kMap_kPacketInfo;
	kMap_kPacketInfo.clear();
	#endif

	// const DWORD MAX_RECV_COUNT = 8; // @warme669 //Ms5.8p23
	const DWORD MAX_RECV_COUNT = 12; // @warme669
	const DWORD SAFE_RECV_BUFSIZE = 8192;
	DWORD dwRecvCount = 0;

	while (ret)
	{
		if (dwRecvCount++ >= MAX_RECV_COUNT-1 && GetRecvBufferSize() < SAFE_RECV_BUFSIZE && m_strPhase == "Game")
			break;

		if (!CheckPacket(&header))
			break;

		#ifdef __PERFORMANCE_CHECK__
		DWORD timeBeginPacket=timeGetTime();
		#endif

		#if defined(_DEBUG) && defined(ENABLE_PRINT_RECV_PACKET_DEBUG)
		Tracenf("RECV HEADER : %u , phase %s ", header, m_strPhase.c_str());
		#endif

		switch (header)
		{
			case HEADER_GC_OBSERVER_ADD:
				ret = RecvObserverAddPacket();
				break;

			case HEADER_GC_OBSERVER_REMOVE:
				ret = RecvObserverRemovePacket();
				break;

			case HEADER_GC_OBSERVER_MOVE:
				ret = RecvObserverMovePacket();
				break;

			case HEADER_GC_WARP:
				ret = RecvWarpPacket();
				break;

			case HEADER_GC_PHASE:
				ret = RecvPhasePacket();
				return;
				break;

			case HEADER_GC_PVP:
				ret = RecvPVPPacket();
				break;

			case HEADER_GC_DUEL_START:
				ret = RecvDuelStartPacket();
				break;

			case HEADER_GC_CHARACTER_ADD:
 				ret = RecvCharacterAppendPacket();
				break;

			case HEADER_GC_CHAR_ADDITIONAL_INFO:
				ret = RecvCharacterAdditionalInfo();
				break;

			case HEADER_GC_CHARACTER_ADD2:
				ret = RecvCharacterAppendPacketNew();
				break;

			case HEADER_GC_CHARACTER_UPDATE:
				ret = RecvCharacterUpdatePacket();
				break;

			case HEADER_GC_CHARACTER_DEL:
				ret = RecvCharacterDeletePacket();
				break;

			case HEADER_GC_CHAT:
				ret = RecvChatPacket();
				break;

			case HEADER_GC_SYNC_POSITION:
				ret = RecvSyncPositionPacket();
				break;

			case HEADER_GC_OWNERSHIP:
				ret = RecvOwnerShipPacket();
				break;

			case HEADER_GC_WHISPER:
				ret = RecvWhisperPacket();
				break;

			case HEADER_GC_CHARACTER_MOVE:
				ret = RecvCharacterMovePacket();
				break;

			// Position
			case HEADER_GC_CHARACTER_POSITION:
				ret = RecvCharacterPositionPacket();
				break;

			// Battle Packet
			case HEADER_GC_STUN:
				ret = RecvStunPacket();
				break;

			case HEADER_GC_DEAD:
				ret = RecvDeadPacket();
				break;

			case HEADER_GC_PLAYER_POINT_CHANGE:
				ret = RecvPointChange();
				break;

			// item packet.
			case HEADER_GC_ITEM_DEL:
				ret = RecvItemDelPacket();
				break;

			case HEADER_GC_ITEM_SET:
				ret = RecvItemSetPacket();
				break;

			case HEADER_GC_ITEM_USE:
				ret = RecvItemUsePacket();
				break;

			case HEADER_GC_ITEM_UPDATE:
				ret = RecvItemUpdatePacket();
				break;

			case HEADER_GC_ITEM_GROUND_ADD:
				ret = RecvItemGroundAddPacket();
				break;

			case HEADER_GC_ITEM_GROUND_DEL:
				ret = RecvItemGroundDelPacket();
				break;

			case HEADER_GC_ITEM_OWNERSHIP:
				ret = RecvItemOwnership();
				break;

			case HEADER_GC_QUICKSLOT_ADD:
				ret = RecvQuickSlotAddPacket();
				break;

			case HEADER_GC_QUICKSLOT_DEL:
				ret = RecvQuickSlotDelPacket();
				break;

			case HEADER_GC_QUICKSLOT_SWAP:
				ret = RecvQuickSlotMovePacket();
				break;

			case HEADER_GC_MOTION:
				ret = RecvMotionPacket();
				break;

			case HEADER_GC_SHOP:
				ret = RecvShopPacket();
				break;

			case HEADER_GC_SHOP_SIGN:
				ret = RecvShopSignPacket();
				break;

			case HEADER_GC_EXCHANGE:
				ret = RecvExchangePacket();
				break;

			case HEADER_GC_QUEST_INFO:
				ret = RecvQuestInfoPacket();
				break;

			case HEADER_GC_REQUEST_MAKE_GUILD:
				ret = RecvRequestMakeGuild();
				break;

			case HEADER_GC_PING:
				ret = RecvPingPacket();
				break;

			case HEADER_GC_SCRIPT:
				ret = RecvScriptPacket();
				break;

			case HEADER_GC_QUEST_CONFIRM:
				ret = RecvQuestConfirmPacket();
				break;

			case HEADER_GC_TARGET:
				ret = RecvTargetPacket();
				break;

			case HEADER_GC_DAMAGE_INFO:
				ret = RecvDamageInfoPacket();
				break;

			case HEADER_GC_MOUNT:
				ret = RecvMountPacket();
				break;

			case HEADER_GC_CHANGE_SPEED:
				ret = RecvChangeSpeedPacket();
				break;

			case HEADER_GC_PLAYER_POINTS:
				ret = __RecvPlayerPoints();
				break;

			case HEADER_GC_CREATE_FLY:
				ret = RecvCreateFlyPacket();
				break;

			case HEADER_GC_FLY_TARGETING:
				ret = RecvFlyTargetingPacket();
				break;

			case HEADER_GC_ADD_FLY_TARGETING:
				ret = RecvAddFlyTargetingPacket();
				break;

			case HEADER_GC_SKILL_LEVEL:
				ret = RecvSkillLevel();
				break;

			case HEADER_GC_SKILL_LEVEL_NEW:
				ret = RecvSkillLevelNew();
				break;

			case HEADER_GC_MESSENGER:
				ret = RecvMessenger();
				break;

			case HEADER_GC_GUILD:
				ret = RecvGuild();
				break;

			case HEADER_GC_PARTY_INVITE:
				ret = RecvPartyInvite();
				break;

			case HEADER_GC_PARTY_ADD:
				ret = RecvPartyAdd();
				break;

			case HEADER_GC_PARTY_UPDATE:
				ret = RecvPartyUpdate();
				break;

			case HEADER_GC_PARTY_REMOVE:
				ret = RecvPartyRemove();
				break;

			case HEADER_GC_PARTY_LINK:
				ret = RecvPartyLink();
				break;

			case HEADER_GC_PARTY_UNLINK:
				ret = RecvPartyUnlink();
				break;

			case HEADER_GC_PARTY_PARAMETER:
				ret = RecvPartyParameter();
				break;

			case HEADER_GC_SAFEBOX_SET:
				ret = RecvSafeBoxSetPacket();
				break;

			case HEADER_GC_SAFEBOX_DEL:
				ret = RecvSafeBoxDelPacket();
				break;

			case HEADER_GC_SAFEBOX_WRONG_PASSWORD:
				ret = RecvSafeBoxWrongPasswordPacket();
				break;

			case HEADER_GC_SAFEBOX_SIZE:
				ret = RecvSafeBoxSizePacket();
				break;

			case HEADER_GC_SAFEBOX_MONEY_CHANGE:
				ret = RecvSafeBoxMoneyChangePacket();
				break;

			case HEADER_GC_FISHING:
				ret = RecvFishing();
				break;

			case HEADER_GC_DUNGEON:
				ret = RecvDungeon();
				break;

			case HEADER_GC_TIME:
				ret = RecvTimePacket();
				break;

			case HEADER_GC_WALK_MODE:
				ret = RecvWalkModePacket();
				break;

			case HEADER_GC_CHANGE_SKILL_GROUP:
				ret = RecvChangeSkillGroupPacket();
				break;

			case HEADER_GC_REFINE_INFORMATION:
				ret = RecvRefineInformationPacket();
				break;

			case HEADER_GC_REFINE_INFORMATION_NEW:
				ret = RecvRefineInformationPacketNew();
				break;

			case HEADER_GC_SEPCIAL_EFFECT:
				ret = RecvSpecialEffect();
				break;

			case HEADER_GC_NPC_POSITION:
				ret = RecvNPCList();
				break;

			case HEADER_GC_CHANNEL:
				ret = RecvChannelPacket();
				break;

			case HEADER_GC_VIEW_EQUIP:
				ret = RecvViewEquipPacket();
				break;

			case HEADER_GC_LAND_LIST:
				ret = RecvLandPacket();
				break;

			//case HEADER_GC_TARGET_CREATE:
			//	ret = RecvTargetCreatePacket();
			//	break;

			case HEADER_GC_TARGET_CREATE_NEW:
				ret = RecvTargetCreatePacketNew();
				break;

			case HEADER_GC_TARGET_UPDATE:
				ret = RecvTargetUpdatePacket();
				break;

			case HEADER_GC_TARGET_DELETE:
				ret = RecvTargetDeletePacket();
				break;

			case HEADER_GC_AFFECT_ADD:
				ret = RecvAffectAddPacket();
				break;

			case HEADER_GC_AFFECT_REMOVE:
				ret = RecvAffectRemovePacket();
				break;

			case HEADER_GC_MALL_OPEN:
				ret = RecvMallOpenPacket();
				break;

			case HEADER_GC_MALL_SET:
				ret = RecvMallItemSetPacket();
				break;

			case HEADER_GC_MALL_DEL:
				ret = RecvMallItemDelPacket();
				break;

			case HEADER_GC_LOVER_INFO:
				ret = RecvLoverInfoPacket();
				break;

			case HEADER_GC_LOVE_POINT_UPDATE:
				ret = RecvLovePointUpdatePacket();
				break;

			case HEADER_GC_DIG_MOTION:
				ret = RecvDigMotionPacket();
				break;

			case HEADER_GC_HANDSHAKE:
				RecvHandshakePacket();
				return;
				break;

			case HEADER_GC_HANDSHAKE_OK:
				RecvHandshakeOKPacket();
				return;
				break;

			case HEADER_GC_HYBRIDCRYPT_KEYS:
				RecvHybridCryptKeyPacket();
				return;
				break;

			case HEADER_GC_HYBRIDCRYPT_SDB:
				RecvHybridCryptSDBPacket();
				return;
				break;

			#ifdef _IMPROVED_PACKET_ENCRYPTION_
			case HEADER_GC_KEY_AGREEMENT:
				RecvKeyAgreementPacket();
				return;
				break;

			case HEADER_GC_KEY_AGREEMENT_COMPLETED:
				RecvKeyAgreementCompletedPacket();
				return;
				break;
			#endif

			case HEADER_GC_SPECIFIC_EFFECT:
				ret = RecvSpecificEffect();
				break;

			case HEADER_GC_DRAGON_SOUL_REFINE:
				ret = RecvDragonSoulRefine();
				break;

			#if defined(__BL_LUCKY_BOX__)
			case HEADER_GC_LUCKY_BOX:
				ret = RecvLuckyBoxPacket();
				break;
			#endif

			#ifdef ENABLE_ACCE_COSTUME_SYSTEM
			case HEADER_GC_ACCE:
				ret = RecvAccePacket();
				break;
			#endif

			#ifdef ENABLE_IKASHOP_RENEWAL
			case HEADER_GC_NEW_OFFLINESHOP:
				ret = RecvOfflineshopPacket();
				break;
			#endif

			#ifdef __RANKING_SYSTEM__
			case HEADER_GC_RANKING_UPDATE:
				ret = RecvRankUpdate();
				break;
			#endif

			#ifdef ENABLE_BAKIM_DUYURU
			case HEADER_GC_ENABLE_BAKIM_DUYURU:
				ret = RecvMaintenanceInfo();
				break;
			#endif

			#ifdef ENABLE_SEND_TARGET_INFO
			case HEADER_GC_TARGET_INFO:
				ret = RecvTargetInfoPacket();
				break;
			#endif

			#ifdef ENABLE_SHOW_CHEST_DROP
			case HEADER_GC_CHEST_DROP_INFO:
				ret = RecvChestDropInfo();
				break;
			#endif

			#ifdef ENABLE_SWITCHBOT
			case HEADER_GC_SWITCHBOT:
				ret = RecvSwitchbotPacket();
				break;
			#endif

			#ifdef ENABLE_HIDE_COSTUME_PART
			case HEADER_GC_UPDATE_COSTUME_VISIBILITY_STATUS:
				ret = UpdateCostumeVisibleSettings();
				break;
			#endif

			#ifdef ENABLE_FULL_YANG
			case HEADER_GC_GOLD_CHANGE:
				if (RecvGoldChange())
					return;
				break;

			case HEADER_GC_GOLD:
				ret = __RecvPlayerGold();
				break;
			#endif

			#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
			case HEADER_GC_CUBE_RENEWAL:
				ret = RecvCubeRenewalPacket();
				break;
			#endif

			#ifdef ENABLE_EVENT_MANAGER
			case HEADER_GC_EVENT_MANAGER:
				ret = RecvEventManager();
				break;
			#endif

			#ifdef ENABLE_SKILL_RENEWAL
			case HEADER_GC_SKILL_BOOK_COUNT:
				ret = RecvSkillBookCountPacket();
				break;
			#endif

			#ifdef ENABLE_DRAGON_COIN_SHOP_SYSTEM
			case HEADER_GC_ITEMSHOP_GENERAL:
				ret = RecvItemShopGeneralPacket();
				break;
			#endif

			#ifdef ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM
			case HEADER_GC_ITEMSHOP_ADMIN_GENERAL:
				ret = RecvItemShopGeneralAdminPacket();
				break;
			#endif

			#ifdef ENABLE_EXCHANGE_LOG
			case HEADER_GC_EXCHANGE_LOG:
				ret = RecvExchangeLog();
				break;
			#endif

			#ifdef ENABLE_PM_ALL_SEND_SYSTEM
			case HEADER_GC_BULK_WHISPER:
				ret = RecvBulkWhisperPacket();
				break;
			#endif

			#ifdef ENABLE_OFFER_SYSTEM
			case HEADER_GC_OFFER:
				ret = RecvOfferListPacket();
				break;
			#endif

			#ifdef ENABLE_ENTITY_PRELOADING
			case HEADER_GC_PRELOAD_ENTITIES:
				ret = RecvPreloadEntitiesPacket();
				break;
			#endif

			#ifdef ENABLE_WHEEL_OF_FORTUNE
			case HEADER_GC_WHEEL_OF_FORTUNE:
				ret = RecvWheelOfFortunePacket();
				break;
			#endif

			#ifdef ENABLE_TANAKA_EVENT
			case HEADER_GC_TANAKA_EVENT:
				ret = RecvTanakaEventPacket();
				break;
			#endif
#ifdef SPIN_WHEEL
		case HEADER_GC_SPIN_WHEEL:
			ret = RecvSpinWheelPacket();
			break;
		case HEADER_GC_SPINWHEEL_REWARD:
			ret = RecvSpinWheelRewardsPacket();
			break;
#endif


			#ifdef ENABLE_BATTLE_PASS
			case HEADER_GC_BATTLE_PASS_OPEN:
				ret = RecvBattlePassPacket();
				break;

			case HEADER_GC_BATTLE_PASS_UPDATE:
				ret = RecvBattlePassUpdatePacket();
				break;
			#endif

			#ifdef ENABLE_TELEPORTION_WIZARD_SYSTEM
			case HEADER_GC_TELEPORTION_WIZARD_INFO:
				ret = RecvTpWizardInfoPacket();
				break;
			#endif

			#ifdef ENABLE_CHANGELOOK_SYSTEM
			case HEADER_GC_CL:
				ret = RecvChangeLookPacket();
				break;
			#endif

			#ifdef BOSS_DMG_RANKING
			case HEADER_GC_BOSS_RANKING:
				ret = RecvBossRankingPacket();
				break;
			#endif

			case HEADER_GC_AFFECT_RESET:
				ret = RecvCharacterAffectResetPacket();
				break;

			#ifdef ENABLE_BLACK_MARKET_SYSTEM
			case HEADER_GC_BLACKMARKET:
				ret = RecvBlackMarketPacket();
				break;
			#endif

			#ifdef ENABLE_REWARD_SYSTEM
			case HEADER_GC_REWARD_INFO:
				ret = RecvRewardInfoPacket();
				break;
			#endif

			#ifdef ENABLE_GUILD_STATISTICS_SYSTEM
			case HEADER_GC_GUILD_STATISTICS:
				ret = RecvGuildStatisticsPacket();
				break;
			#endif


			#ifdef ENABLE_PUBLISHER_SYSTEM
			case HEADER_GC_PUBLISHER_INFO:
				ret = RecvPublisherInfoPacket();
				break;
			#endif

			#ifdef ENABLE_RONARK_SYSTEM
			case HEADER_GC_RONARK_INFO:
				ret = RecvRonarkInfoPacket();
				break;
			#endif

			#ifdef ENABLE_AUTO_ITEM_SELL_SYSTEM
			case HEADER_GC_AUTO_ITEM_SELL:
				ret = RecvAutoItemSellPacket();
				break;
			#endif

			default:
				ret = RecvDefaultPacket(header);
				break;
		}

		#ifdef __PERFORMANCE_CHECK__
		DWORD timeEndPacket=timeGetTime();

		{
			PERF_PacketInfo& rkPacketInfo=kMap_kPacketInfo[header];
			rkPacketInfo.dwCount++;
			rkPacketInfo.dwTime+=timeEndPacket-timeBeginPacket;
		}

		{
			PERF_PacketInfo& rkPacketInfo=gs_kPacketTimeAnalyzer.m_kMap_kPacketInfo[header];
			rkPacketInfo.dwCount++;
			rkPacketInfo.dwTime+=timeEndPacket-timeBeginPacket;
		}
		#endif
	}

	#ifdef __PERFORMANCE_CHECK__
	DWORD timeEndDispatch=timeGetTime();

	if (timeEndDispatch-timeBeginDispatch>2)
	{
		static FILE* fp=fopen("perf_dispatch_packet.txt", "w");

		fprintf(fp, "delay %d\n", timeEndDispatch-timeBeginDispatch);
		for (std::map<DWORD, PERF_PacketInfo>::iterator i=kMap_kPacketInfo.begin(); i!=kMap_kPacketInfo.end(); ++i)
		{
			if (i->second.dwTime>0)
				fprintf(fp, "header %d: count %d, time %d\n", i->first, i->second.dwCount, i->second.dwTime);
		}
		fputs("=====================================================\n", fp);
		fflush(fp);
	}
	#endif

	if (!ret)
		RecvErrorPacket(header);

	static DWORD s_nextRefreshTime = ELTimer_GetMSec();

	DWORD curTime = ELTimer_GetMSec();
	if (s_nextRefreshTime > curTime)
		return;

	if (m_isRefreshCharacterWnd)
	{
		m_isRefreshCharacterWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharacter", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshEquipmentWnd)
	{
		m_isRefreshEquipmentWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshEquipment", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshInventoryWnd)
	{
		m_isRefreshInventoryWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshInventory", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshExchangeWnd)
	{
		m_isRefreshExchangeWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshExchange", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshSkillWnd)
	{
		m_isRefreshSkillWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSkill", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshSafeboxWnd)
	{
		m_isRefreshSafeboxWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSafebox", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshMallWnd)
	{
		m_isRefreshMallWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMall", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshStatus)
	{
		m_isRefreshStatus=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshStatus", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshMessengerWnd)
	{
		m_isRefreshMessengerWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMessenger", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndInfoPage)
	{
		m_isRefreshGuildWndInfoPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildInfoPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndBoardPage)
	{
		m_isRefreshGuildWndBoardPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildBoardPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndMemberPage)
	{
		m_isRefreshGuildWndMemberPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildMemberPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndMemberPageGradeComboBox)
	{
		m_isRefreshGuildWndMemberPageGradeComboBox=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildMemberPageGradeComboBox", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndSkillPage)
	{
		m_isRefreshGuildWndSkillPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildSkillPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndGradePage)
	{
		m_isRefreshGuildWndGradePage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGradePage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}
}

void CPythonNetworkStream::__InitializeGamePhase()
{
	__ServerTimeSync_Initialize();

	m_isRefreshStatus=false;
	m_isRefreshCharacterWnd=false;
	m_isRefreshEquipmentWnd=false;
	m_isRefreshInventoryWnd=false;
	m_isRefreshExchangeWnd=false;
	m_isRefreshSkillWnd=false;
	m_isRefreshSafeboxWnd=false;
	m_isRefreshMallWnd=false;
	m_isRefreshMessengerWnd=false;
	m_isRefreshGuildWndInfoPage=false;
	m_isRefreshGuildWndBoardPage=false;
	m_isRefreshGuildWndMemberPage=false;
	m_isRefreshGuildWndMemberPageGradeComboBox=false;
	m_isRefreshGuildWndSkillPage=false;
	m_isRefreshGuildWndGradePage=false;

	m_EmoticonStringVector.clear();

	m_pInstTarget = NULL;
}

void CPythonNetworkStream::Warp(LONG lGlobalX, LONG lGlobalY)
{
	CPythonBackground& rkBgMgr=CPythonBackground::Instance();
	rkBgMgr.Destroy();
	rkBgMgr.Create();
	rkBgMgr.Warp(lGlobalX, lGlobalY);
	//rkBgMgr.SetShadowLevel(CPythonBackground::SHADOW_ALL);
	rkBgMgr.RefreshShadowLevel();

	LONG lLocalX = lGlobalX;
	LONG lLocalY = lGlobalY;
	__GlobalPositionToLocalPosition(lLocalX, lLocalY);
	float fHeight = CPythonBackground::Instance().GetHeight(float(lLocalX), float(lLocalY));

	IAbstractApplication& rkApp=IAbstractApplication::GetSingleton();
	rkApp.SetCenterPosition(float(lLocalX), float(lLocalY), fHeight);

	__ShowMapName(lLocalX, lLocalY);
}

void CPythonNetworkStream::__ShowMapName(LONG lLocalX, LONG lLocalY)
{
	const std::string & c_rstrMapFileName = CPythonBackground::Instance().GetWarpMapName();
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ShowMapName", Py_BuildValue("(sii)", c_rstrMapFileName.c_str(), lLocalX, lLocalY));
}

void CPythonNetworkStream::__LeaveGamePhase()
{
	CInstanceBase::ClearPVPKeySystem();

	__ClearNetworkActorManager();

	m_bComboSkillFlag = FALSE;

	IAbstractCharacterManager& rkChrMgr=IAbstractCharacterManager::GetSingleton();
	rkChrMgr.Destroy();

	CPythonItem& rkItemMgr=CPythonItem::Instance();
	rkItemMgr.Destroy();
#ifdef ENABLE_IKASHOP_RENEWAL
#ifdef ENABLE_IKASHOP_ENTITIES
	CPythonIkarusShop::instance().DeleteEntities();
#endif
#endif

}

#ifdef ENABLE_CYPOR_ENGEL
#include <windows.h>
#include <string>

bool WrongPathYmir(const std::string& dirName_in)
{
	DWORD type = GetFileAttributesA(dirName_in.c_str());

	if (type == INVALID_FILE_ATTRIBUTES)
		return false;

	if (type & FILE_ATTRIBUTE_DIRECTORY)
		return true;

	return false;
}
#endif

#ifdef ENABLE_HIYUS_FIX
/*SandBoxie*/
void SandBox()
{
	CHAR _SandboxDll[] = { 'S', 'b', 'i', 'e', 'D', 'l', 'l', '.', 'd', 'l', 'l', 0x0 };
	HMODULE sandbox = GetModuleHandleA(_SandboxDll);

	if (sandbox)
	{
		CHAR __warn[] = { 'S', 'a', 'n', 'd', 'b', 'o', 'x', '4', ' ', 'D', 'e', 't', 'e', 'c', 't', 'e', 'd', '!', 0x0 };
		CPythonNetworkStream::Instance().Rapor(__warn);
	}
}
/*SandBoxie*/

/*VPC*/
DWORD __forceinline IsInsideVPC_exceptionFilter(LPEXCEPTION_POINTERS ep)
{
	PCONTEXT ctx = ep->ContextRecord;

	ctx->Ebx = -1;
	ctx->Eip += 4;
	return EXCEPTION_CONTINUE_EXECUTION;
}

bool IsInsideVPC()
{
	bool rc = false;

	__try
	{
		_asm push ebx
		_asm mov  ebx, 0
		_asm mov  eax, 1

		_asm __emit 0Fh
		_asm __emit 3Fh
		_asm __emit 07h
		_asm __emit 0Bh

		_asm test ebx, ebx
		_asm setz[rc]
			_asm pop ebx
	}

	__except (IsInsideVPC_exceptionFilter(GetExceptionInformation()))
	{
	}

	return rc;
}

void AntiVPC()
{
	CHAR __vmcheckdll[] = { 'v', 'm', 'c', 'h', 'e', 'c', 'k', '.', 'd', 'l', 'l', 0x0 }; // vmcheck.dll
	if (IsInsideVPC() || GetModuleHandleA(__vmcheckdll))
	{
		CHAR __warn[] = { 'V', 'i', 'r', 't', 'u', 'a', 'l', 'P', 'C', ' ', 'D', 'e', 't', 'e', 'c', 't', 'e', 'd', '!', 0x0 };
		CPythonNetworkStream::Instance().Rapor(__warn);
	}
}
/*VPC*/

/*VMWare*/
bool IsInsideVMWare()
{
	bool rc = true;

	__try
	{
		__asm
		{
			push   edx
			push   ecx
			push   ebx

			mov    eax, 'VMXh'
			mov    ebx, 0 // any value but not the MAGIC VALUE
			mov    ecx, 10 // get VMWare version
			mov    edx, 'VX' // port number

			in     eax, dx // read port
						   // on return EAX returns the VERSION
						   cmp    ebx, 'VMXh' // is it a reply from VMWare?
						   setz[rc] // set return value

						   pop    ebx
						   pop    ecx
						   pop    edx
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		rc = false;
	}

	return rc;
}

void AntiVMware()
{
	if (IsInsideVMWare())
	{
		CHAR __warn[] = { 'V', 'm', 'w', 'a', 'r', 'e', ' ', 'D', 'e', 't', 'e', 'c', 't', 'e', 'd', '!', 0x0 };
		CPythonNetworkStream::Instance().Rapor(__warn);
	}
}
/*VMWare*/
#endif

void CPythonNetworkStream::SetGamePhase()
{
	if ("Game"!=m_strPhase)
		m_phaseLeaveFunc.Run();

	Tracen("");
	Tracen("## Network - Game Phase ##");
	Tracen("");

	m_strPhase = "Game";

#ifdef ENABLE_PERFORMANCE_REWORK
	CPythonCharacterManager::Instance().TouchViewBoundDistance(CPythonSystem::Instance().GetExtremeFpsBoost());
#endif

	m_dwChangingPhaseTime = ELTimer_GetMSec();
	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::GamePhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveGamePhase);


	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.SetMainCharacterIndex(GetMainActorVID());

	__RefreshStatus();

#ifdef ENABLE_HIYUS_FIX
	if (m_dwLastStatus == 0)
	{
		/*SandBoxie*/
		SandBox();
		/*SandBoxie*/

		/*VPC*/
		AntiVPC();
		/*VPC*/

		/*VMWare*/
		AntiVMware();
		/*VMWare*/

		m_dwLastStatus = 1;
	}
#endif

#ifdef ENABLE_CYPOR_ENGEL
	if (m_dwLastStatus == 0)
	{
		if (WrongPathYmir(std::string("C") + std::string(":\\") + std::string("Cypor")))
		{
			std::string rapor = "Cypor";
			CPythonNetworkStream::Instance().Rapor(rapor.c_str());
			m_dwLastStatus = 1;
		}
	}
#endif

#ifdef ENABLE_SIKITUT_ENGEL
		if (WrongPathYmir(std::string("C") + std::string(":\\") + std::string("Sqitut")))
		{
			std::string rapor2 = "Sqitut";
			CPythonNetworkStream::Instance().Rapor(rapor2.c_str());
			m_dwLastStatus = 1;
		}
#endif

#ifdef ENABLE_HILE_ENGEL
	if (g_iAntiCheat == 1)
	{
		std::string rapor3162 = "HLBOT";
		CPythonNetworkStream::Instance().Rapor(rapor3162.c_str());
		g_iAntiCheat = 0;
	}
#endif
}

#ifdef ENABLE_SEND_TARGET_INFO
bool CPythonNetworkStream::SendTargetInfoLoadPacket(DWORD dwVID)
{
	TPacketCGTargetInfoLoad TargetInfoLoadPacket;
	TargetInfoLoadPacket.header = HEADER_CG_TARGET_INFO_LOAD;
	TargetInfoLoadPacket.dwVID = dwVID;

	if (!Send(sizeof(TargetInfoLoadPacket), &TargetInfoLoadPacket))
		return false;

	return SendSequence();
}
#endif

#ifdef ENABLE_SHOW_CHEST_DROP
bool CPythonNetworkStream::SendChestDropInfo(int iInventoryCell)
{
	TPacketCGChestDropInfo packet;
	packet.header = HEADER_CG_CHEST_DROP_INFO;
	packet.subheader = CDI_SUB_HEADER_INV;
	packet.arg = iInventoryCell;

	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendChestDropVnumInfo(int iVnum)
{
	TPacketCGChestDropInfo packet;
	packet.header = HEADER_CG_CHEST_DROP_INFO;
	packet.subheader = CDI_SUB_HEADER_VNUM;
	packet.arg = iVnum;

	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvChestDropInfo()
{
	TPacketGCChestDropInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	packet.wSize -= sizeof(packet);
	while (packet.wSize > 0)
	{
		TChestDropInfoTable kTab;
		if (!Recv(sizeof(kTab), &kTab))
			return false;

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddChestDropInfo", Py_BuildValue("(iiiii)", packet.dwChestVnum, kTab.bPageIndex, kTab.bSlotIndex, kTab.dwItemVnum, kTab.bItemCount));
		packet.wSize -= sizeof(kTab);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RefreshChestDropInfo", Py_BuildValue("(i)", packet.dwChestVnum));
	return true;
}
#endif

bool CPythonNetworkStream::RecvObserverAddPacket()
{
	TPacketGCObserverAdd kObserverAddPacket;
	if (!Recv(sizeof(kObserverAddPacket), &kObserverAddPacket))
		return false;

	CPythonMiniMap::Instance().AddObserver(
		kObserverAddPacket.vid,
		kObserverAddPacket.x*100.0f,
		kObserverAddPacket.y*100.0f);

	return true;
}

bool CPythonNetworkStream::RecvObserverRemovePacket()
{
	TPacketGCObserverAdd kObserverRemovePacket;
	if (!Recv(sizeof(kObserverRemovePacket), &kObserverRemovePacket))
		return false;

	CPythonMiniMap::Instance().RemoveObserver(
		kObserverRemovePacket.vid
	);

	return true;
}

bool CPythonNetworkStream::RecvObserverMovePacket()
{
	TPacketGCObserverMove kObserverMovePacket;
	if (!Recv(sizeof(kObserverMovePacket), &kObserverMovePacket))
		return false;

	CPythonMiniMap::Instance().MoveObserver(
		kObserverMovePacket.vid,
		kObserverMovePacket.x*100.0f,
		kObserverMovePacket.y*100.0f);

	return true;
}


bool CPythonNetworkStream::RecvWarpPacket()
{
	TPacketGCWarp kWarpPacket;
	if (!Recv(sizeof(kWarpPacket), &kWarpPacket))
		return false;

	#ifdef ENABLE_NEW_LOADING
	__DirectEnterMode_Set(m_dwSelectedCharacterIndex, true);
	#else
	__DirectEnterMode_Set(m_dwSelectedCharacterIndex);
	#endif

	CNetworkStream::Connect((DWORD)kWarpPacket.lAddr, kWarpPacket.wPort);
	return true;
}

bool CPythonNetworkStream::RecvDuelStartPacket()
{
	TPacketGCDuelStart kDuelStartPacket;
	if (!Recv(sizeof(kDuelStartPacket), &kDuelStartPacket))
		return false;

	DWORD count = (kDuelStartPacket.wSize - sizeof(kDuelStartPacket))/sizeof(DWORD);

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
	{
		TraceError("CPythonNetworkStream::RecvDuelStartPacket - MainCharacter is NULL");
		return false;
	}
	DWORD dwVIDSrc = pkInstMain->GetVirtualID();
	DWORD dwVIDDest;

	for ( DWORD i = 0; i < count; i++)
	{
		Recv(sizeof(dwVIDDest),&dwVIDDest);
		CInstanceBase::InsertDUELKey(dwVIDSrc,dwVIDDest);
	}

	if (count == 0)
		pkInstMain->SetDuelMode(CInstanceBase::DUEL_CANNOTATTACK);
	else
		pkInstMain->SetDuelMode(CInstanceBase::DUEL_START);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));

	rkChrMgr.RefreshAllPCTextTail();

	return true;
}

bool CPythonNetworkStream::RecvPVPPacket()
{
	TPacketGCPVP kPVPPacket;
	if (!Recv(sizeof(kPVPPacket), &kPVPPacket))
		return false;

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();

	switch (kPVPPacket.bMode)
	{
		case PVP_MODE_AGREE:
			rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);

			if (rkPlayer.IsMainCharacterIndex(kPVPPacket.dwVIDDst))
				rkPlayer.RememberChallengeInstance(kPVPPacket.dwVIDSrc);

			if (rkPlayer.IsMainCharacterIndex(kPVPPacket.dwVIDSrc))
				rkPlayer.RememberCantFightInstance(kPVPPacket.dwVIDDst);
			break;
		case PVP_MODE_REVENGE:
		{
			rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);

			DWORD dwKiller = kPVPPacket.dwVIDSrc;
			DWORD dwVictim = kPVPPacket.dwVIDDst;

			if (rkPlayer.IsMainCharacterIndex(dwVictim))
				rkPlayer.RememberRevengeInstance(dwKiller);

			if (rkPlayer.IsMainCharacterIndex(dwKiller))
				rkPlayer.RememberCantFightInstance(dwVictim);
			break;
		}

		case PVP_MODE_FIGHT:
			rkChrMgr.InsertPVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDSrc);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDDst);
			break;
		case PVP_MODE_NONE:
			rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDSrc);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDDst);
			break;
	}

	__RefreshTargetBoardByVID(kPVPPacket.dwVIDSrc);
	__RefreshTargetBoardByVID(kPVPPacket.dwVIDDst);

	return true;
}

// DELETEME
/*
void CPythonNetworkStream::__SendWarpPacket()
{
	TPacketCGWarp kWarpPacket;
	kWarpPacket.bHeader=HEADER_GC_WARP;
	if (!Send(sizeof(kWarpPacket), &kWarpPacket))
	{
		return;
	}
}
*/

bool CPythonNetworkStream::Rapor(const char* c_szMsg)
{
	Tracen(c_szMsg);
	
	TPacketCGHack kPacketHack;
	kPacketHack.bHeader=HEADER_CG_HACK;
	strncpy(kPacketHack.szBuf, c_szMsg, sizeof(kPacketHack.szBuf)-1);

	if (!Send(sizeof(kPacketHack), &kPacketHack))
		return false;

	return SendSequence();
}


void CPythonNetworkStream::NotifyHack(const char* c_szMsg)
{
	if (!m_kQue_stHack.empty())
		if (c_szMsg==m_kQue_stHack.back())
			return;

	m_kQue_stHack.push_back(c_szMsg);
}

bool CPythonNetworkStream::__SendHack(const char* c_szMsg)
{
	Tracen(c_szMsg);

	TPacketCGHack kPacketHack;
	kPacketHack.bHeader=HEADER_CG_HACK;
	strncpy(kPacketHack.szBuf, c_szMsg, sizeof(kPacketHack.szBuf)-1);

	if (!Send(sizeof(kPacketHack), &kPacketHack))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerAddByVIDPacket(DWORD vid)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BY_VID;
	if (!Send(sizeof(packet), &packet))
		return false;
	if (!Send(sizeof(vid), &vid))
		return false;
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerAddByNamePacket(const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BY_NAME;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szName[CHARACTER_NAME_MAX_LEN];
	strncpy(szName, c_szName, CHARACTER_NAME_MAX_LEN-1);
	szName[CHARACTER_NAME_MAX_LEN-1] = '\0';

	if (!Send(sizeof(szName), &szName))
		return false;
	Tracef(" SendMessengerAddByNamePacket : %s\n", c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerRemovePacket(const char * c_szKey, const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_REMOVE;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szKey[CHARACTER_NAME_MAX_LEN];
	strncpy(szKey, c_szKey, CHARACTER_NAME_MAX_LEN-1);
	if (!Send(sizeof(szKey), &szKey))
		return false;
	__RefreshTargetBoardByName(c_szName);
	return SendSequence();
}

#ifdef ENABLE_MESSENGER_BLOCK
bool CPythonNetworkStream::SendMessengerAddBlockByVIDPacket(DWORD vid)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_VID;
	if (!Send(sizeof(packet), &packet))
		return false;
	if (!Send(sizeof(vid), &vid))
		return false;
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerAddBlockByNamePacket(const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_NAME;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szName[CHARACTER_NAME_MAX_LEN];
	strncpy(szName, c_szName, CHARACTER_NAME_MAX_LEN-1);
	szName[CHARACTER_NAME_MAX_LEN-1] = '\0'; // #720: ??? ?? ?? ?? ????? ?? ??

	if (!Send(sizeof(szName), &szName))
		return false;
	Tracef(" SendMessengerAddBlockByNamePacket : %s\n", c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerRemoveBlockPacket(const char * c_szKey, const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_REMOVE_BLOCK;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szKey[CHARACTER_NAME_MAX_LEN];
	strncpy(szKey, c_szKey, CHARACTER_NAME_MAX_LEN-1);
	if (!Send(sizeof(szKey), &szKey))
		return false;
	__RefreshTargetBoardByName(c_szName);
	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendCharacterStatePacket(const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg)
{
	if (!__CanActMainInstance())
		return true;

	if (fDstRot < 0.0f)
		fDstRot = 360 + fDstRot;
	else if (fDstRot > 360.0f)
		fDstRot = fmodf(fDstRot, 360.0f);

	TPacketCGMove kStatePacket;
	kStatePacket.bHeader = HEADER_CG_CHARACTER_MOVE;
	kStatePacket.bFunc = eFunc;
	kStatePacket.bArg = uArg;
	kStatePacket.bRot = fDstRot/5.0f;
	kStatePacket.lX = LONG(c_rkPPosDst.x);
	
    
    kStatePacket.lY = LONG(c_rkPPosDst.y);
	{
		CInstanceBase * pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();
		if (eFunc == 3 && uArg > 21){ return true; }
		 
		static UINT c_packet = 0;
		static DWORD t_packet = 0;
		DWORD t = timeGetTime();
		if (t_packet + 500 < t) {
			c_packet = 0;
			t_packet = t;
		}
		c_packet++;
		if (c_packet > 8){ return true; }
		 
		static UINT l_func = 0;
		static DWORD t_f = 0;
		 
		static float f_x = 0.0f;
		static float f_y = 0.0f;
		const D3DXVECTOR3 & cpos = pkInst->GetGraphicThingInstancePtr()->GetPosition();
		if (f_x == cpos.x && f_y == cpos.y && t - t_f < 1000 && l_func == eFunc && (eFunc < 3)) {
			return true;
		}
		f_x = cpos.x;
		f_y = cpos.y;
		 
		if (l_func == 0 && eFunc == 0 && t_f + 10 > t){ return true; }
		l_func = eFunc;
		t_f = t;
		if (cpos.x == 0.0f && cpos.y == -0.0f) {
			kStatePacket.lX = LONG(c_rkPPosDst.x);
			kStatePacket.lY = LONG(c_rkPPosDst.y);
		}else{
			kStatePacket.lX = LONG(cpos.x);
			kStatePacket.lY = LONG(cpos.y < 0.0f ? cpos.y * -1.0f : cpos.y);
		}
	}

	{
		CInstanceBase * pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();
		if (eFunc == 3 && uArg > 21){ return true; }
		 
		static UINT c_packet = 0;
		static DWORD t_packet = 0;
		DWORD t = timeGetTime();
		if (t_packet + 500 < t) {
			c_packet = 0;
			t_packet = t;
		}
		c_packet++;
		if (c_packet > 8){ return true; }
		 
		static UINT l_func = 0;
		static DWORD t_f = 0;
		 
		static float f_x = 0.0f;
		static float f_y = 0.0f;
		const D3DXVECTOR3 & cpos = pkInst->GetGraphicThingInstancePtr()->GetPosition();
		if (f_x == cpos.x && f_y == cpos.y && t - t_f < 1000 && l_func == eFunc && (eFunc < 3)) {
			return true;
		}
		f_x = cpos.x;
		f_y = cpos.y;
		 
		if (l_func == 0 && eFunc == 0 && t_f + 10 > t){ return true; }
		l_func = eFunc;
		t_f = t;
		if (cpos.x == 0.0f && cpos.y == -0.0f) {
			kStatePacket.lX = LONG(c_rkPPosDst.x);
			kStatePacket.lY = LONG(c_rkPPosDst.y);
		}else{
			kStatePacket.lX = LONG(cpos.x);
			kStatePacket.lY = LONG(cpos.y < 0.0f ? cpos.y * -1.0f : cpos.y);
		}
	}

	kStatePacket.dwTime = ELTimer_GetServerMSec();

	assert(kStatePacket.lX >= 0 && kStatePacket.lX < 204800);

	__LocalPositionToGlobalPosition(kStatePacket.lX, kStatePacket.lY);


	if (!Send(sizeof(kStatePacket), &kStatePacket))
	{
		Tracenf("CPythonNetworkStream::SendCharacterStatePacket(dwCmdTime=%u, fDstPos=(%f, %f), fDstRot=%f, eFunc=%d uArg=%d) - PACKET SEND ERROR",
			kStatePacket.dwTime,
			float(kStatePacket.lX),
			float(kStatePacket.lY),
			fDstRot,
			kStatePacket.bFunc,
			kStatePacket.bArg);
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendUseSkillPacket(DWORD dwSkillIndex, DWORD dwTargetVID)
{
	TPacketCGUseSkill UseSkillPacket;
	UseSkillPacket.bHeader = HEADER_CG_USE_SKILL;
	UseSkillPacket.dwVnum = dwSkillIndex;
	UseSkillPacket.dwTargetVID = dwTargetVID;
	if (!Send(sizeof(TPacketCGUseSkill), &UseSkillPacket))
	{
		Tracen("CPythonNetworkStream::SendUseSkillPacket - SEND PACKET ERROR");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChatPacket(const char * c_szChat, BYTE byType)
{
	if ("Game" != m_strPhase)//Fix
		return true;

	if (strlen(c_szChat) == 0)
		return true;

	if (strlen(c_szChat) >= 512)
		return true;

	if (c_szChat[0] == '/')
	{
		if (1 == strlen(c_szChat))
		{
			if (!m_strLastCommand.empty())
				c_szChat = m_strLastCommand.c_str();
		}
		else
		{
			m_strLastCommand = c_szChat;
		}
	}

	if (ClientCommand(c_szChat))
		return true;

	int iTextLen = strlen(c_szChat) + 1;
	TPacketCGChat ChatPacket;
	ChatPacket.header = HEADER_CG_CHAT;
	ChatPacket.length = sizeof(ChatPacket) + iTextLen;
	ChatPacket.type = byType;

	if (!Send(sizeof(ChatPacket), &ChatPacket))
		return false;

	if (!Send(iTextLen, c_szChat))
		return false;

	return SendSequence();
}

//////////////////////////////////////////////////////////////////////////
// Emoticon
void CPythonNetworkStream::RegisterEmoticonString(const char * pcEmoticonString)
{
	if (m_EmoticonStringVector.size() >= CInstanceBase::EMOTICON_NUM)
	{
		TraceError("Can't register emoticon string... vector is full (size:%d)", m_EmoticonStringVector.size() );
		return;
	}
	m_EmoticonStringVector.push_back(pcEmoticonString);
}

bool CPythonNetworkStream::ParseEmoticon(const char * pChatMsg, DWORD * pdwEmoticon)
{
	for (DWORD dwEmoticonIndex = 0; dwEmoticonIndex < m_EmoticonStringVector.size() ; ++dwEmoticonIndex)
	{
		if (strlen(pChatMsg) > m_EmoticonStringVector[dwEmoticonIndex].size())
			continue;

		const char * pcFind = strstr(pChatMsg, m_EmoticonStringVector[dwEmoticonIndex].c_str());

		if (pcFind != pChatMsg)
			continue;

		*pdwEmoticon = dwEmoticonIndex;

		return true;
	}

	return false;
}
// Emoticon
//////////////////////////////////////////////////////////////////////////

void CPythonNetworkStream::__ConvertEmpireText(DWORD dwEmpireID, char* szText)
{
	if (dwEmpireID<1 || dwEmpireID>3)
		return;

	UINT uHanPos;

	STextConvertTable& rkTextConvTable=m_aTextConvTable[dwEmpireID-1];

	BYTE* pbText=(BYTE*)szText;
	while (*pbText)
	{
		if (*pbText & 0x80)
		{
			if (pbText[0]>=0xb0 && pbText[0]<=0xc8 && pbText[1]>=0xa1 && pbText[1]<=0xfe)
			{
				uHanPos=(pbText[0]-0xb0)*(0xfe-0xa1+1)+(pbText[1]-0xa1);
				pbText[0]=rkTextConvTable.aacHan[uHanPos][0];
				pbText[1]=rkTextConvTable.aacHan[uHanPos][1];
			}
			pbText+=2;
		}
		else
		{
			if (*pbText>='a' && *pbText<='z')
			{
				*pbText=rkTextConvTable.acLower[*pbText-'a'];
			}
			else if (*pbText>='A' && *pbText<='Z')
			{
				*pbText=rkTextConvTable.acUpper[*pbText-'A'];
			}
			pbText++;
		}
	}
}

bool CPythonNetworkStream::RecvChatPacket()
{
	TPacketGCChat kChat;
    char buf[1024 + 1];
	char line[1024 + 1];

	if (!Recv(sizeof(kChat), &kChat))
		return false;

	UINT uChatSize=kChat.size - sizeof(kChat);

	if (!Recv(uChatSize, buf))
		return false;

	buf[uChatSize]='\0';

	if (LocaleService_IsEUROPE() && GetDefaultCodePage() == 1256)
	{
		char * p = strchr(buf, ':');
		if (p && p[1] == ' ')
			p[1] = 0x08;
	}

	if (kChat.type >= CHAT_TYPE_MAX_NUM)
		return true;

	if (CHAT_TYPE_COMMAND == kChat.type)
	{
		ServerCommand(buf);
		return true;
	}

	if (kChat.dwVID != 0)
	{
		CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
		CInstanceBase * pkInstChatter = rkChrMgr.GetInstancePtr(kChat.dwVID);
		if (NULL == pkInstChatter)
			return true;

		switch (kChat.type)
		{
		case CHAT_TYPE_TALKING:
		case CHAT_TYPE_PARTY:
		case CHAT_TYPE_GUILD:
		case CHAT_TYPE_SHOUT:
		case CHAT_TYPE_WHISPER:
			{
				char * p = strchr(buf, ':');

				if (p)
					p += 2;
				else
					p = buf;

				DWORD dwEmoticon;

				if (ParseEmoticon(p, &dwEmoticon))
				{
					pkInstChatter->SetEmoticon(dwEmoticon);
					return true;
				}
				else
				{
					if (gs_bEmpireLanuageEnable)
					{
						CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
						if (pkInstMain)
							if (!pkInstMain->IsSameEmpire(*pkInstChatter))
								__ConvertEmpireText(pkInstChatter->GetEmpireID(), p);
					}

					if (m_isEnableChatInsultFilter)
					{
						if (false == pkInstChatter->IsNPC() && false == pkInstChatter->IsEnemy())
						{
							__FilterInsult(p, strlen(p));
						}
					}

					_snprintf(line, sizeof(line), "%s", p);
				}
			}
			break;
		case CHAT_TYPE_COMMAND:
		case CHAT_TYPE_INFO:
		case CHAT_TYPE_NOTICE:
		case CHAT_TYPE_BIG_NOTICE:
		// case CHAT_TYPE_UNK_10:
#ifdef ENABLE_DICE_SYSTEM
		case CHAT_TYPE_DICE_INFO:
#endif
		case CHAT_TYPE_MAX_NUM:
		default:
			_snprintf(line, sizeof(line), "%s", buf);
			break;
		}

		if (CHAT_TYPE_SHOUT != kChat.type)
		{
			CPythonTextTail::Instance().RegisterChatTail(kChat.dwVID, line);
		}

		if (pkInstChatter->IsPC())
			CPythonChat::Instance().AppendChat(kChat.type, buf);
	}
	else
	{
		if (CHAT_TYPE_NOTICE == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetTipMessage", Py_BuildValue("(s)", buf));
		}
		else if (CHAT_TYPE_BIG_NOTICE == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetBigMessage", Py_BuildValue("(s)", buf));
		}
		else if (CHAT_TYPE_SHOUT == kChat.type)
		{
			char * p = strchr(buf, ':');

			if (p)
			{
				if (m_isEnableChatInsultFilter)
					__FilterInsult(p, strlen(p));
			}
		}

		CPythonChat::Instance().AppendChat(kChat.type, buf);

	}
	return true;
}

bool CPythonNetworkStream::RecvWhisperPacket()
{
	TPacketGCWhisper whisperPacket;
    char buf[512 + 1];

	if (!Recv(sizeof(whisperPacket), &whisperPacket))
		return false;

	assert(whisperPacket.wSize - sizeof(whisperPacket) < 512);

	if (!Recv(whisperPacket.wSize - sizeof(whisperPacket), &buf))
		return false;

	buf[whisperPacket.wSize - sizeof(whisperPacket)] = '\0';

	static char line[256];
	if (CPythonChat::WHISPER_TYPE_CHAT == whisperPacket.bType || CPythonChat::WHISPER_TYPE_GM == whisperPacket.bType)
	{
		_snprintf(line, sizeof(line), "%s : %s", whisperPacket.szNameFrom, buf);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisper", Py_BuildValue("(iss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, line));
	}
	else if (CPythonChat::WHISPER_TYPE_SYSTEM == whisperPacket.bType || CPythonChat::WHISPER_TYPE_ERROR == whisperPacket.bType)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperSystemMessage", Py_BuildValue("(iss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, buf));
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperError", Py_BuildValue("(iss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, buf));
	}

	return true;
}

bool CPythonNetworkStream::SendWhisperPacket(const char * name, const char * c_szChat)
{
	if (strlen(c_szChat) >= 255)
		return true;

	int iTextLen = strlen(c_szChat) + 1;
	TPacketCGWhisper WhisperPacket;
	WhisperPacket.bHeader = HEADER_CG_WHISPER;
	WhisperPacket.wSize = sizeof(WhisperPacket) + iTextLen;

	strncpy(WhisperPacket.szNameTo, name, sizeof(WhisperPacket.szNameTo) - 1);

	if (!Send(sizeof(WhisperPacket), &WhisperPacket))
		return false;

	if (!Send(iTextLen, c_szChat))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMobileMessagePacket(const char * name, const char * c_szChat)
{
	int iTextLen = strlen(c_szChat) + 1;
	TPacketCGSMS SMSPacket;
	SMSPacket.bHeader = HEADER_CG_SMS;
	SMSPacket.wSize = sizeof(SMSPacket) + iTextLen;

	strncpy(SMSPacket.szNameTo, name, sizeof(SMSPacket.szNameTo) - 1);

	if (!Send(sizeof(SMSPacket), &SMSPacket))
		return false;

	if (!Send(iTextLen, c_szChat))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvPointChange()
{
	TPacketGCPointChange PointChange;
	if (!Recv(sizeof(TPacketGCPointChange), &PointChange))
	{
		Tracen("Recv Point Change Packet Error");
		return false;
	}

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.ShowPointEffect(PointChange.Type, PointChange.dwVID);

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (pInstance && PointChange.dwVID == pInstance->GetVirtualID())
	{
		CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
		rkPlayer.SetStatus(PointChange.Type, PointChange.value);

		switch (PointChange.Type)
		{
			case POINT_STAT_RESET_COUNT:
				__RefreshStatus();
				break;
			case POINT_LEVEL:
			case POINT_ST:
			case POINT_DX:
			case POINT_HT:
			case POINT_IQ:
				__RefreshStatus();
				__RefreshSkillWindow();
				break;
			case POINT_SKILL:
			case POINT_SUB_SKILL:
			case POINT_HORSE_SKILL:
				__RefreshSkillWindow();
				break;
			case POINT_ENERGY:
				if (PointChange.value == 0)
				{
					rkPlayer.SetStatus(POINT_ENERGY_END_TIME, 0);
				}
				__RefreshStatus();
				break;
			default:
				__RefreshStatus();
				break;
		}

		if (POINT_GOLD == PointChange.Type)
		{
			if (PointChange.amount > 0)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickMoney", Py_BuildValue("(i)", PointChange.amount));
			}
		}
	}
	#ifdef ENABLE_TEXT_LEVEL_REFRESH
	else
	{
		// the /advance command will provide no global refresh! it sends the pointchange only to the specific player and not all
		pInstance = CPythonCharacterManager::Instance().GetInstancePtr(PointChange.dwVID);
		if (pInstance && PointChange.Type == POINT_LEVEL)
		{
			pInstance->SetLevel(PointChange.value);
			pInstance->UpdateTextTailLevel(PointChange.value);
		}
	}
	#endif

	return true;
}

bool CPythonNetworkStream::RecvStunPacket()
{
	TPacketGCStun StunPacket;
	if (!Recv(sizeof(StunPacket), &StunPacket))
	{
		Tracen("CPythonNetworkStream::RecvStunPacket Error");
		return false;
	}

	//Tracef("RecvStunPacket %d\n", StunPacket.vid);

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase * pkInstSel = rkChrMgr.GetInstancePtr(StunPacket.vid);

	if (pkInstSel)
	{
		if (CPythonCharacterManager::Instance().GetMainInstancePtr()==pkInstSel)
			pkInstSel->Die();
		else
			pkInstSel->Stun();
	}

	return true;
}

bool CPythonNetworkStream::RecvDeadPacket()
{
	TPacketGCDead DeadPacket;
	if (!Recv(sizeof(DeadPacket), &DeadPacket))
	{
		Tracen("CPythonNetworkStream::RecvDeadPacket Error");
		return false;
	}

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase * pkChrInstSel = rkChrMgr.GetInstancePtr(DeadPacket.vid);
	if (pkChrInstSel)
	{
		CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();
		if (pkInstMain == pkChrInstSel)
		{
			Tracenf("On MainActor");
			if (false == pkInstMain->GetDuelMode())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnGameOver", Py_BuildValue("()"));
			}
			CPythonPlayer::Instance().NotifyDeadMainCharacter();
		}

		pkChrInstSel->Die();

		#ifdef __AUTO_HUNT__
		if (CPythonPlayer::Instance().GetAutoHuntStatus())
		{
			if (CPythonPlayer::Instance().GetTargetVID() == DeadPacket.vid)
			{
				if (pkChrInstSel->IsStone() && CPythonPlayer::Instance().GetAutoHuntStone())
					CPythonPlayer::Instance().AutoHuntSetStoneCount(CPythonPlayer::Instance().GetAutoHuntStoneCount() + 1);

				if (pkChrInstSel->IsEnemy() && CPythonPlayer::Instance().GetAutoHuntMob())
					CPythonPlayer::Instance().AutoHuntSetEnemyCount(CPythonPlayer::Instance().GetAutoHuntEnemyCount() + 1);

				CPythonPlayer::Instance().AutoHuntStop();
				CPythonPlayer::Instance().AutoHuntRefreshStartPoint();
			}
		}
		#endif
	}

	return true;
}

bool CPythonNetworkStream::SendCharacterPositionPacket(BYTE iPosition)
{
	TPacketCGPosition PositionPacket;

	PositionPacket.header = HEADER_CG_CHARACTER_POSITION;
	PositionPacket.position = iPosition;

	if (!Send(sizeof(TPacketCGPosition), &PositionPacket))
	{
		Tracen("Send Character Position Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendOnClickPacket(DWORD vid)
{
	TPacketCGOnClick OnClickPacket;
	OnClickPacket.header	= HEADER_CG_ON_CLICK;
	OnClickPacket.vid		= vid;

	if (!Send(sizeof(OnClickPacket), &OnClickPacket))
	{
		Tracen("Send On_Click Packet Error");
		return false;
	}

	Tracef("SendOnClickPacket\n");
	return SendSequence();
}

bool CPythonNetworkStream::RecvCharacterPositionPacket()
{
	TPacketGCPosition PositionPacket;

	if (!Recv(sizeof(TPacketGCPosition), &PositionPacket))
		return false;

	CInstanceBase * pChrInstance = CPythonCharacterManager::Instance().GetInstancePtr(PositionPacket.vid);

	if (!pChrInstance)
		return true;

	//pChrInstance->UpdatePosition(PositionPacket.position);

	return true;
}

bool CPythonNetworkStream::RecvMotionPacket()
{
	TPacketGCMotion MotionPacket;

	if (!Recv(sizeof(TPacketGCMotion), &MotionPacket))
		return false;

	CInstanceBase * pMainInstance = CPythonCharacterManager::Instance().GetInstancePtr(MotionPacket.vid);
	CInstanceBase * pVictimInstance = NULL;

	if (0 != MotionPacket.victim_vid)
		pVictimInstance = CPythonCharacterManager::Instance().GetInstancePtr(MotionPacket.victim_vid);

	if (!pMainInstance)
		return false;

	return true;
}

bool CPythonNetworkStream::RecvShopPacket()
{
	std::vector<char> vecBuffer;
	vecBuffer.clear();

	TPacketGCShop packet_shop;
	if (!Recv(sizeof(packet_shop), &packet_shop))
		return false;

	int iSize = packet_shop.size - sizeof(packet_shop);
	if (iSize > 0)
	{
		vecBuffer.resize(iSize);
		if (!Recv(iSize, &vecBuffer[0]))
			return false;
	}

	switch (packet_shop.subheader)
	{
		case SHOP_SUBHEADER_GC_START:
			{
				CPythonShop::Instance().Clear();

				DWORD dwVID = *(DWORD *)&vecBuffer[0];

				TPacketGCShopStart * pShopStartPacket = (TPacketGCShopStart *)&vecBuffer[4];
				for (BYTE iItemIndex = 0; iItemIndex < SHOP_HOST_ITEM_MAX_NUM; ++iItemIndex)
				{
					CPythonShop::Instance().SetItemData(iItemIndex, pShopStartPacket->items[iItemIndex]);
				}

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop", Py_BuildValue("(i)", dwVID));
			}
			break;

		case SHOP_SUBHEADER_GC_START_EX:
			{
				CPythonShop::Instance().Clear();

				TPacketGCShopStartEx * pShopStartPacket = (TPacketGCShopStartEx *)&vecBuffer[0];
				size_t read_point = sizeof(TPacketGCShopStartEx);

				DWORD dwVID = pShopStartPacket->owner_vid;
				BYTE shop_tab_count = pShopStartPacket->shop_tab_count;

				CPythonShop::instance().SetTabCount(shop_tab_count);

				for (size_t i = 0; i < shop_tab_count; i++)
				{
					TPacketGCShopStartEx::TSubPacketShopTab* pPackTab = (TPacketGCShopStartEx::TSubPacketShopTab*)&vecBuffer[read_point];
					read_point += sizeof(TPacketGCShopStartEx::TSubPacketShopTab);

					CPythonShop::instance().SetTabCoinType(i, pPackTab->coin_type);
					CPythonShop::instance().SetTabName(i, pPackTab->name);

					struct packet_shop_item* item = &pPackTab->items[0];

					for (BYTE j = 0; j < SHOP_HOST_ITEM_MAX_NUM; j++)
					{
						TShopItemData* itemData = (item + j);
						CPythonShop::Instance().SetItemData(i, j, *itemData);
					}
				}

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop", Py_BuildValue("(i)", dwVID));
			}
			break;


		case SHOP_SUBHEADER_GC_END:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndShop", Py_BuildValue("()"));
			break;

		case SHOP_SUBHEADER_GC_UPDATE_ITEM:
			{
				TPacketGCShopUpdateItem * pShopUpdateItemPacket = (TPacketGCShopUpdateItem *)&vecBuffer[0];
				CPythonShop::Instance().SetItemData(pShopUpdateItemPacket->pos, pShopUpdateItemPacket->item);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshShop", Py_BuildValue("()"));
			}
			break;

		case SHOP_SUBHEADER_GC_UPDATE_PRICE:
#ifdef ENABLE_FULL_YANG
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetShopSellingPrice", Py_BuildValue("(K)", *(uint64_t*)&vecBuffer[0]));
#else
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetShopSellingPrice", Py_BuildValue("(i)", *(int *)&vecBuffer[0]));
#endif
			break;

		case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY"));
			break;

		case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY_EX"));
			break;

		case SHOP_SUBHEADER_GC_SOLDOUT:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SOLDOUT"));
			break;

		case SHOP_SUBHEADER_GC_INVENTORY_FULL:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVENTORY_FULL"));
			break;

		case SHOP_SUBHEADER_GC_INVALID_POS:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVALID_POS"));
			break;

		default:
			TraceError("CPythonNetworkStream::RecvShopPacket: Unknown subheader\n");
			break;
	}

	return true;
}

bool CPythonNetworkStream::RecvExchangePacket()
{
	TPacketGCExchange exchange_packet;

	if (!Recv(sizeof(exchange_packet), &exchange_packet))
		return false;

	switch (exchange_packet.subheader)
	{
		case EXCHANGE_SUBHEADER_GC_START:
			CPythonExchange::Instance().Clear();
			CPythonExchange::Instance().Start();
			CPythonExchange::Instance().SetSelfName(CPythonPlayer::Instance().GetName());
			#ifdef ENABLE_LEVEL_IN_TRADE
			//CPythonExchange::Instance().SetSelfLevel(CPythonPlayer::Instance().GetLevel());
			CPythonExchange::Instance().SetSelfLevel(CPythonPlayer::Instance().GetStatus(POINT_LEVEL));
			#endif

			{
				CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(exchange_packet.arg1);

				if (pCharacterInstance)
				{
					CPythonExchange::Instance().SetTargetName(pCharacterInstance->GetNameString());
					#ifdef ENABLE_LEVEL_IN_TRADE
					CPythonExchange::Instance().SetTargetLevel(pCharacterInstance->GetLevel());
					#endif
				}
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartExchange", Py_BuildValue("()"));
			break;

		case EXCHANGE_SUBHEADER_GC_ITEM_ADD:
			if (exchange_packet.is_me)
			{
				int iSlotIndex = exchange_packet.arg2.cell;
				#ifdef ENABLE_NEW_MAX_COUNT
				CPythonExchange::Instance().SetItemToSelf(iSlotIndex, exchange_packet.arg1, (short) exchange_packet.arg3);
				#else
				CPythonExchange::Instance().SetItemToSelf(iSlotIndex, exchange_packet.arg1, (BYTE) exchange_packet.arg3);
				#endif

				for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
					CPythonExchange::Instance().SetItemMetinSocketToSelf(iSlotIndex, i, exchange_packet.alValues[i]);

				for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
					CPythonExchange::Instance().SetItemAttributeToSelf(iSlotIndex, j, exchange_packet.aAttr[j].bType, exchange_packet.aAttr[j].sValue);

				#ifdef ENABLE_TRADABLE_ICON
				PyObject *args = PyTuple_New(3);
				PyTuple_SetItem(args, 0, PyInt_FromLong(exchange_packet.arg4.window_type));
				PyTuple_SetItem(args, 1, PyInt_FromLong(exchange_packet.arg4.cell));
				PyTuple_SetItem(args, 2, PyInt_FromLong(iSlotIndex));
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddItemToExchange", args);
				#endif

				#ifdef ENABLE_EVOLUTION_SYSTEM
				CPythonExchange::Instance().SetItemEvolutionToSelf(iSlotIndex, exchange_packet.iEvolution);
				#endif
				#ifdef ENABLE_CHANGELOOK_SYSTEM
				CPythonExchange::Instance().SetItemTransmutation(iSlotIndex, exchange_packet.dwTransmutation, true);
				#endif
			}
			else
			{
				int iSlotIndex = exchange_packet.arg2.cell;

				#ifdef ENABLE_NEW_MAX_COUNT
				CPythonExchange::Instance().SetItemToTarget(iSlotIndex, exchange_packet.arg1, (short) exchange_packet.arg3);
				#else
				CPythonExchange::Instance().SetItemToTarget(iSlotIndex, exchange_packet.arg1, (BYTE) exchange_packet.arg3);
				#endif

				for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
					CPythonExchange::Instance().SetItemMetinSocketToTarget(iSlotIndex, i, exchange_packet.alValues[i]);

				for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
					CPythonExchange::Instance().SetItemAttributeToTarget(iSlotIndex, j, exchange_packet.aAttr[j].bType, exchange_packet.aAttr[j].sValue);

				#ifdef ENABLE_EVOLUTION_SYSTEM
				CPythonExchange::Instance().SetItemEvolutionToTarget(iSlotIndex, exchange_packet.iEvolution);
				#endif
				#ifdef ENABLE_CHANGELOOK_SYSTEM
				CPythonExchange::Instance().SetItemTransmutation(iSlotIndex, exchange_packet.dwTransmutation, false);
				#endif
			}

			__RefreshExchangeWindow();
			__RefreshInventoryWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_ITEM_DEL:
			if (exchange_packet.is_me)
			{
#ifdef ENABLE_NEW_MAX_COUNT
				CPythonExchange::Instance().DelItemOfSelf((short) exchange_packet.arg1);
#else
				CPythonExchange::Instance().DelItemOfSelf((BYTE) exchange_packet.arg1);
#endif
			}
			else
			{
#ifdef ENABLE_NEW_MAX_COUNT
				CPythonExchange::Instance().DelItemOfTarget((short) exchange_packet.arg1);
#else
				CPythonExchange::Instance().DelItemOfTarget((BYTE) exchange_packet.arg1);
#endif
			}
			__RefreshExchangeWindow();
			__RefreshInventoryWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_ELK_ADD:
			if (exchange_packet.is_me)
				CPythonExchange::Instance().SetElkToSelf(exchange_packet.arg1);
			else
				CPythonExchange::Instance().SetElkToTarget(exchange_packet.arg1);

			__RefreshExchangeWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_ACCEPT:
			if (exchange_packet.is_me)
			{
				CPythonExchange::Instance().SetAcceptToSelf((BYTE) exchange_packet.arg1);
			}
			else
			{
				CPythonExchange::Instance().SetAcceptToTarget((BYTE) exchange_packet.arg1);
			}
			__RefreshExchangeWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_END:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndExchange", Py_BuildValue("()"));
			__RefreshInventoryWindow();
			CPythonExchange::Instance().End();
			break;

		case EXCHANGE_SUBHEADER_GC_ALREADY:
			Tracef("trade_already");
			break;

		case EXCHANGE_SUBHEADER_GC_LESS_ELK:
			Tracef("trade_less_elk");
			break;
	};

	return true;
}

bool CPythonNetworkStream::RecvQuestInfoPacket()
{
	TPacketGCQuestInfo QuestInfo;

	if (!Peek(sizeof(TPacketGCQuestInfo), &QuestInfo))
	{
		Tracen("Recv Quest Info Packet Error #1");
		return false;
	}

	if (!PeekNoFetch(QuestInfo.size))
	{
		Tracen("Recv Quest Info Packet Error #2");
		return false;
	}

	RecvNoFetch(sizeof(TPacketGCQuestInfo));

	const BYTE & c_rFlag = QuestInfo.flag;

	enum
	{
		QUEST_PACKET_TYPE_NONE,
		QUEST_PACKET_TYPE_BEGIN,
		QUEST_PACKET_TYPE_UPDATE,
		QUEST_PACKET_TYPE_END,
	};

	BYTE byQuestPacketType = QUEST_PACKET_TYPE_NONE;

	if (0 != (c_rFlag & QUEST_SEND_IS_BEGIN))
	{
		BYTE isBegin;
		if (!Recv(sizeof(isBegin), &isBegin))
			return false;

		if (isBegin)
			byQuestPacketType = QUEST_PACKET_TYPE_BEGIN;
		else
			byQuestPacketType = QUEST_PACKET_TYPE_END;
	}
	else
	{
		byQuestPacketType = QUEST_PACKET_TYPE_UPDATE;
	}

	// Recv Data Start
	char szTitle[30 + 1] = "";
	char szClockName[16 + 1] = "";
	int iClockValue = 0;
	char szCounterName[16 + 1] = "";
	int iCounterValue = 0;
	char szIconFileName[24 + 1] = "";

	if (0 != (c_rFlag & QUEST_SEND_TITLE))
	{
		if (!Recv(sizeof(szTitle), &szTitle))
			return false;

		szTitle[30]='\0';
	}
	if (0 != (c_rFlag & QUEST_SEND_CLOCK_NAME))
	{
		if (!Recv(sizeof(szClockName), &szClockName))
			return false;

		szClockName[16]='\0';
	}
	if (0 != (c_rFlag & QUEST_SEND_CLOCK_VALUE))
	{
		if (!Recv(sizeof(iClockValue), &iClockValue))
			return false;
	}
	if (0 != (c_rFlag & QUEST_SEND_COUNTER_NAME))
	{
		if (!Recv(sizeof(szCounterName), &szCounterName))
			return false;

		szCounterName[16]='\0';
	}
	if (0 != (c_rFlag & QUEST_SEND_COUNTER_VALUE))
	{
		if (!Recv(sizeof(iCounterValue), &iCounterValue))
			return false;
	}
	if (0 != (c_rFlag & QUEST_SEND_ICON_FILE))
	{
		if (!Recv(sizeof(szIconFileName), &szIconFileName))
			return false;

		szIconFileName[24]='\0';
	}
	// Recv Data End

	CPythonQuest& rkQuest=CPythonQuest::Instance();

	// Process Start
	if (QUEST_PACKET_TYPE_END == byQuestPacketType)
	{
		rkQuest.DeleteQuestInstance(QuestInfo.index);
	}
	else if (QUEST_PACKET_TYPE_UPDATE == byQuestPacketType)
	{
		if (!rkQuest.IsQuest(QuestInfo.index))
		{
			rkQuest.MakeQuest(QuestInfo.index);
		}

		if (strlen(szTitle) > 0)
			rkQuest.SetQuestTitle(QuestInfo.index, szTitle);
		if (strlen(szClockName) > 0)
			rkQuest.SetQuestClockName(QuestInfo.index, szClockName);
		if (strlen(szCounterName) > 0)
			rkQuest.SetQuestCounterName(QuestInfo.index, szCounterName);
		if (strlen(szIconFileName) > 0)
			rkQuest.SetQuestIconFileName(QuestInfo.index, szIconFileName);

		if (c_rFlag & QUEST_SEND_CLOCK_VALUE)
			rkQuest.SetQuestClockValue(QuestInfo.index, iClockValue);
		if (c_rFlag & QUEST_SEND_COUNTER_VALUE)
			rkQuest.SetQuestCounterValue(QuestInfo.index, iCounterValue);
	}
	else if (QUEST_PACKET_TYPE_BEGIN == byQuestPacketType)
	{
		CPythonQuest::SQuestInstance QuestInstance;
		QuestInstance.dwIndex = QuestInfo.index;
		QuestInstance.strTitle = szTitle;
		QuestInstance.strClockName = szClockName;
		QuestInstance.iClockValue = iClockValue;
		QuestInstance.strCounterName = szCounterName;
		QuestInstance.iCounterValue = iCounterValue;
		QuestInstance.strIconFileName = szIconFileName;
		CPythonQuest::Instance().RegisterQuestInstance(QuestInstance);
	}
	// Process Start End

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshQuest", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::RecvQuestConfirmPacket()
{
	TPacketGCQuestConfirm kQuestConfirmPacket;
	if (!Recv(sizeof(kQuestConfirmPacket), &kQuestConfirmPacket))
	{
		Tracen("RecvQuestConfirmPacket Error");
		return false;
	}

	PyObject * poArg = Py_BuildValue("(sii)", kQuestConfirmPacket.msg, kQuestConfirmPacket.timeout, kQuestConfirmPacket.requestPID);
 	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OnQuestConfirm", poArg);
	return true;
}

bool CPythonNetworkStream::RecvRequestMakeGuild()
{
	TPacketGCBlank blank;
	if (!Recv(sizeof(blank), &blank))
	{
		Tracen("RecvRequestMakeGuild Packet Error");
		return false;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AskGuildName", Py_BuildValue("()"));

	return true;
}

void CPythonNetworkStream::ToggleGameDebugInfo()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ToggleDebugInfo", Py_BuildValue("()"));
}

bool CPythonNetworkStream::SendExchangeStartPacket(DWORD vid)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_START;
	packet.arg1			= vid;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_start_packet Error\n");
		return false;
	}

	Tracef("send_trade_start_packet   vid %d \n", vid);
	return SendSequence();
}

#ifdef ENABLE_FULL_YANG
bool CPythonNetworkStream::SendExchangeElkAddPacket(uint64_t elk)
#else
bool CPythonNetworkStream::SendExchangeElkAddPacket(DWORD elk)
#endif
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header = HEADER_CG_EXCHANGE;
	packet.subheader = EXCHANGE_SUBHEADER_CG_ELK_ADD;
	packet.arg1 = elk;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_elk_add_packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeItemAddPacket(TItemPos ItemPos, BYTE byDisplayPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ITEM_ADD;
	packet.Pos			= ItemPos;
	packet.arg2			= byDisplayPos;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_item_add_packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeItemDelPacket(BYTE pos)
{
	assert(!"Can't be called function - CPythonNetworkStream::SendExchangeItemDelPacket");
	return true;

	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ITEM_DEL;
	packet.arg1			= pos;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_item_del_packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeAcceptPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ACCEPT;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_accept_packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeExitPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_CANCEL;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_exit_packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendPointResetPacket()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartPointReset", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::__IsPlayerAttacking()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
		return false;

	if (!pkInstMain->IsAttacking())
		return false;

	return true;
}

bool CPythonNetworkStream::RecvScriptPacket()
{
	TPacketGCScript ScriptPacket;

	if (!Recv(sizeof(TPacketGCScript), &ScriptPacket))
	{
		TraceError("RecvScriptPacket_RecvError");
		return false;
	}

	if (ScriptPacket.size < sizeof(TPacketGCScript))
	{
		TraceError("RecvScriptPacket_SizeError");
		return false;
	}

	ScriptPacket.size -= sizeof(TPacketGCScript);

	static string str;
	str = "";
	str.resize(ScriptPacket.size+1);

	if (!Recv(ScriptPacket.size, &str[0]))
		return false;

	str[str.size()-1] = '\0';

	int iIndex = CPythonEventManager::Instance().RegisterEventSetFromString(str);

	if (-1 != iIndex)
	{
		CPythonEventManager::Instance().SetVisibleLineCount(iIndex, 30);
		CPythonNetworkStream::Instance().OnScriptEventStart(ScriptPacket.skin,iIndex);
	}

	return true;
}

bool CPythonNetworkStream::SendScriptAnswerPacket(int iAnswer)
{
	TPacketCGScriptAnswer ScriptAnswer;

	ScriptAnswer.header = HEADER_CG_SCRIPT_ANSWER;
	ScriptAnswer.answer = (BYTE) iAnswer;
	if (!Send(sizeof(TPacketCGScriptAnswer), &ScriptAnswer))
	{
		Tracen("Send Script Answer Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendScriptButtonPacket(unsigned int iIndex)
{
	TPacketCGScriptButton ScriptButton;

	ScriptButton.header = HEADER_CG_SCRIPT_BUTTON;
	ScriptButton.idx = iIndex;
	if (!Send(sizeof(TPacketCGScriptButton), &ScriptButton))
	{
		Tracen("Send Script Button Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendAnswerMakeGuildPacket(const char * c_szName)
{
	TPacketCGAnswerMakeGuild Packet;

	Packet.header = HEADER_CG_ANSWER_MAKE_GUILD;
	strncpy(Packet.guild_name, c_szName, GUILD_NAME_MAX_LEN);
	Packet.guild_name[GUILD_NAME_MAX_LEN] = '\0';

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracen("SendAnswerMakeGuild Packet Error");
		return false;
	}

// 	Tracef(" SendAnswerMakeGuildPacket : %s", c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendQuestInputStringPacket(const char * c_szString)
{
	TPacketCGQuestInputString Packet;
	Packet.bHeader = HEADER_CG_QUEST_INPUT_STRING;
	strncpy(Packet.szString, c_szString, QUEST_INPUT_STRING_MAX_NUM);

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracen("SendQuestInputStringPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendQuestConfirmPacket(BYTE byAnswer, DWORD dwPID)
{
	TPacketCGQuestConfirm kPacket;
	kPacket.header = HEADER_CG_QUEST_CONFIRM;
	kPacket.answer = byAnswer;
	kPacket.requestPID = dwPID;

	if (!Send(sizeof(kPacket), &kPacket))
	{
		Tracen("SendQuestConfirmPacket Error");
		return false;
	}

	Tracenf(" SendQuestConfirmPacket : %d, %d", byAnswer, dwPID);
	return SendSequence();
}

bool CPythonNetworkStream::RecvSkillCoolTimeEnd()
{
	TPacketGCSkillCoolTimeEnd kPacketSkillCoolTimeEnd;
	if (!Recv(sizeof(kPacketSkillCoolTimeEnd), &kPacketSkillCoolTimeEnd))
	{
		Tracen("CPythonNetworkStream::RecvSkillCoolTimeEnd - RecvError");
		return false;
	}

	CPythonPlayer::Instance().EndSkillCoolTime(kPacketSkillCoolTimeEnd.bSkill);

	return true;
}

bool CPythonNetworkStream::RecvSkillLevel()
{
	assert(!"CPythonNetworkStream::RecvSkillLevel - Don't use this function");
	TPacketGCSkillLevel packet;
	if (!Recv(sizeof(TPacketGCSkillLevel), &packet))
	{
		Tracen("CPythonNetworkStream::RecvSkillLevel - RecvError");
		return false;
	}

	DWORD dwSlotIndex;

	CPythonPlayer& rkPlayer=CPythonPlayer::Instance();
	for (int i = 0; i < SKILL_MAX_NUM; ++i)
	{
		if (rkPlayer.GetSkillSlotIndex(i, &dwSlotIndex))
			rkPlayer.SetSkillLevel(dwSlotIndex, packet.abSkillLevels[i]);
	}

	__RefreshSkillWindow();
	__RefreshStatus();
	Tracef(" >> RecvSkillLevel\n");
	return true;
}

bool CPythonNetworkStream::RecvSkillLevelNew()
{
	TPacketGCSkillLevelNew packet;

	if (!Recv(sizeof(TPacketGCSkillLevelNew), &packet))
	{
		Tracen("CPythonNetworkStream::RecvSkillLevelNew - RecvError");
		return false;
	}

	CPythonPlayer& rkPlayer=CPythonPlayer::Instance();

	rkPlayer.SetSkill(7, 0);
	rkPlayer.SetSkill(8, 0);

	for (int i = 0; i < SKILL_MAX_NUM; ++i)
	{
		TPlayerSkill & rPlayerSkill = packet.skills[i];

		if (i >= 112 && i <= 115 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(7, i);

		if (i >= 116 && i <= 119 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(8, i);

		rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
	}

	__RefreshSkillWindow();
	__RefreshStatus();
	//Tracef(" >> RecvSkillLevelNew\n");
	return true;
}


bool CPythonNetworkStream::RecvDamageInfoPacket()
{
	TPacketGCDamageInfo DamageInfoPacket;

	if (!Recv(sizeof(TPacketGCDamageInfo), &DamageInfoPacket))
	{
		Tracen("Recv Target Packet Error");
		return false;
	}

	CInstanceBase * pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(DamageInfoPacket.dwVID);
	bool bSelf = (pInstTarget == CPythonCharacterManager::Instance().GetMainInstancePtr());
	// bool bTarget = (pInstTarget == m_pInstTarget);
	bool bTarget = true;
	if (pInstTarget)
	{
		if (DamageInfoPacket.damage >= 0)
			pInstTarget->AddDamageEffect(DamageInfoPacket.damage, DamageInfoPacket.flag, bSelf, bTarget);
		else
			TraceError("Damage is equal or below 0.");
	}

	return true;
}
bool CPythonNetworkStream::RecvTargetPacket()
{
	TPacketGCTarget TargetPacket;

	if (!Recv(sizeof(TPacketGCTarget), &TargetPacket))
	{
		Tracen("Recv Target Packet Error");
		return false;
	}

	CInstanceBase * pInstPlayer = CPythonCharacterManager::Instance().GetMainInstancePtr();
	CInstanceBase * pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(TargetPacket.dwVID);
	if (pInstPlayer && pInstTarget)
	{
		if (!pInstTarget->IsDead())
		{
			if (pInstTarget->IsPC() || pInstTarget->IsBuilding())
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoardIfDifferent", Py_BuildValue("(i)", TargetPacket.dwVID));
			else if (pInstPlayer->CanViewTargetHP(*pInstTarget))
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetHPTargetBoard", Py_BuildValue("(ii)", TargetPacket.dwVID, TargetPacket.bHPPercent));
			else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));

			m_pInstTarget = pInstTarget;
		}
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));
	}

	return true;
}

#ifdef ENABLE_SEND_TARGET_INFO
bool CPythonNetworkStream::RecvTargetInfoPacket()
{
	TPacketGCTargetInfo pInfo;
	if (!Recv(sizeof(TPacketGCTargetInfo), &pInfo))
	{
		Tracen("Recv Info Target Packet Error");
		return false;
	}

	pInfo.wSize -= sizeof(pInfo);

	CInstanceBase * pInstPlayer = CPythonCharacterManager::Instance().GetMainInstancePtr();
	CInstanceBase * pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(pInfo.dwVID);
	if (pInstPlayer && pInstTarget)
	{
		if (!pInstTarget->IsDead())
		{
			if (pInstTarget->IsEnemy() || pInstTarget->IsStone())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ClearTargetMonsterDropInfo", Py_BuildValue("()"));

				while (pInfo.wSize > 0)
				{
					TGCTargetItem pInfoItem;
					if (!Recv(sizeof(pInfoItem), &pInfoItem))
					{
						return false;
					}

					pInfo.wSize -= sizeof(pInfoItem);

					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddTargetMonsterDropInfo",
						Py_BuildValue("(iiii)"
						, pInfo.race
						, pInfoItem.dwVnum
						, pInfoItem.iCount
						, pInfoItem.lSocket)
					);
				}

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RefreshTargetMonsterDropInfo", Py_BuildValue("(i)", pInfo.race));
			}
			else
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));
			}
		}
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));
	}

	return true;
}
#endif


bool CPythonNetworkStream::RecvMountPacket()
{
	TPacketGCMount MountPacket;

	if (!Recv(sizeof(TPacketGCMount), &MountPacket))
	{
		Tracen("Recv Mount Packet Error");
		return false;
	}

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(MountPacket.vid);

	if (pInstance)
	{
		// Mount
		if (0 != MountPacket.mount_vid)
		{
//			pInstance->Ride(MountPacket.pos, MountPacket.mount_vid);
		}
		// Unmount
		else
		{
//			pInstance->Unride(MountPacket.pos, MountPacket.x, MountPacket.y);
		}
	}

	if (CPythonPlayer::Instance().IsMainCharacterIndex(MountPacket.vid))
	{
//		CPythonPlayer::Instance().SetRidingVehicleIndex(MountPacket.mount_vid);
	}

	return true;
}

bool CPythonNetworkStream::RecvChangeSpeedPacket()
{
	TPacketGCChangeSpeed SpeedPacket;

	if (!Recv(sizeof(TPacketGCChangeSpeed), &SpeedPacket))
	{
		Tracen("Recv Speed Packet Error");
		return false;
	}

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(SpeedPacket.vid);

	if (!pInstance)
		return true;

//	pInstance->SetWalkSpeed(SpeedPacket.walking_speed);
//	pInstance->SetRunSpeed(SpeedPacket.running_speed);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Recv


bool CPythonNetworkStream::SendAttackPacket(UINT uMotAttack, DWORD dwVIDVictim)
{
#ifdef ENABLE_RASCAL_ANTICHEAT
	rascal::FunctionProtect(reinterpret_cast<void*>(getcalleraddr()));
#endif

	if (!__CanActMainInstance())
		return true;

#ifdef ATTACK_TIME_LOG
	static DWORD prevTime = timeGetTime();
	DWORD curTime = timeGetTime();
	TraceError("TIME: %.4f(%.4f) ATTACK_PACKET: %d TARGET: %d", curTime/1000.0f, (curTime-prevTime)/1000.0f, uMotAttack, dwVIDVictim);
	prevTime = curTime;
#endif

	TPacketCGAttack kPacketAtk;

	kPacketAtk.header = HEADER_CG_ATTACK;
	kPacketAtk.bType = uMotAttack;
	kPacketAtk.dwVictimVID = dwVIDVictim;

	if (!SendSpecial(sizeof(kPacketAtk), &kPacketAtk))
	{
		Tracen("Send Battle Attack Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendSpecial(int nLen, void * pvBuf)
{
	BYTE bHeader = *(BYTE *) pvBuf;

	switch (bHeader)
	{
		case HEADER_CG_ATTACK:
			{
				TPacketCGAttack * pkPacketAtk = (TPacketCGAttack *) pvBuf;
				pkPacketAtk->bCRCMagicCubeProcPiece = GetProcessCRCMagicCubePiece();
				pkPacketAtk->bCRCMagicCubeFilePiece = GetProcessCRCMagicCubePiece();
				return Send(nLen, pvBuf);
			}
			break;
	}

	return Send(nLen, pvBuf);
}

bool CPythonNetworkStream::RecvAddFlyTargetingPacket()
{
	TPacketGCFlyTargeting kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	__GlobalPositionToLocalPosition(kPacket.lX, kPacket.lY);

	Tracef("VID [%d] Added to target settings\n",kPacket.dwShooterVID);

	CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

	CInstanceBase * pShooter = rpcm.GetInstancePtr(kPacket.dwShooterVID);

	if (!pShooter)
	{
#ifndef _DEBUG
		TraceError("CPythonNetworkStream::RecvFlyTargetingPacket() - dwShooterVID[%d] NOT EXIST", kPacket.dwShooterVID);
#endif
		return true;
	}

	CInstanceBase * pTarget = rpcm.GetInstancePtr(kPacket.dwTargetVID);

	if (kPacket.dwTargetVID && pTarget)
	{
		pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(pTarget->GetGraphicThingInstancePtr());
	}
	else
	{
		float h = CPythonBackground::Instance().GetHeight(kPacket.lX,kPacket.lY) + 60.0f; // TEMPORARY HEIGHT
		pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(D3DXVECTOR3(kPacket.lX,kPacket.lY,h));
		//pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(kPacket.kPPosTarget.x,kPacket.kPPosTarget.y,);
	}

	return true;
}

bool CPythonNetworkStream::RecvFlyTargetingPacket()
{
	TPacketGCFlyTargeting kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	__GlobalPositionToLocalPosition(kPacket.lX, kPacket.lY);

	//Tracef("CPythonNetworkStream::RecvFlyTargetingPacket - VID [%d]\n",kPacket.dwShooterVID);

	CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

	CInstanceBase * pShooter = rpcm.GetInstancePtr(kPacket.dwShooterVID);

	if (!pShooter)
	{
#ifdef _DEBUG
		TraceError("CPythonNetworkStream::RecvFlyTargetingPacket() - dwShooterVID[%d] NOT EXIST", kPacket.dwShooterVID);
#endif
		return true;
	}

	CInstanceBase * pTarget = rpcm.GetInstancePtr(kPacket.dwTargetVID);

	if (kPacket.dwTargetVID && pTarget)
	{
		pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(pTarget->GetGraphicThingInstancePtr());
	}
	else
	{
		float h = CPythonBackground::Instance().GetHeight(kPacket.lX, kPacket.lY) + 60.0f; // TEMPORARY HEIGHT
		pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(D3DXVECTOR3(kPacket.lX,kPacket.lY,h));
		//pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(kPacket.kPPosTarget.x,kPacket.kPPosTarget.y,);
	}

	return true;
}

bool CPythonNetworkStream::SendShootPacket(UINT uSkill)
{
	TPacketCGShoot kPacketShoot;
	kPacketShoot.bHeader=HEADER_CG_SHOOT;
	kPacketShoot.bType=uSkill;

	if (!Send(sizeof(kPacketShoot), &kPacketShoot))
	{
		Tracen("SendShootPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendAddFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition & kPPosTarget)
{
	TPacketCGFlyTargeting packet;

	//CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

	packet.bHeader	= HEADER_CG_ADD_FLY_TARGETING;
	packet.dwTargetVID = dwTargetVID;
	packet.lX = kPPosTarget.x;
	packet.lY = kPPosTarget.y;

	__LocalPositionToGlobalPosition(packet.lX, packet.lY);

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send FlyTargeting Packet Error");
		return false;
	}

	return SendSequence();
}


bool CPythonNetworkStream::SendFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition & kPPosTarget)
{
	TPacketCGFlyTargeting packet;

	//CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

	packet.bHeader	= HEADER_CG_FLY_TARGETING;
	packet.dwTargetVID = dwTargetVID;
	packet.lX = kPPosTarget.x;
	packet.lY = kPPosTarget.y;

	__LocalPositionToGlobalPosition(packet.lX, packet.lY);

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send FlyTargeting Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvCreateFlyPacket()
{
	TPacketGCCreateFly kPacket;
	if (!Recv(sizeof(TPacketGCCreateFly), &kPacket))
		return false;

	CFlyingManager& rkFlyMgr = CFlyingManager::Instance();
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	CInstanceBase * pkStartInst = rkChrMgr.GetInstancePtr(kPacket.dwStartVID);
	CInstanceBase * pkEndInst = rkChrMgr.GetInstancePtr(kPacket.dwEndVID);
	if (!pkStartInst || !pkEndInst)
		return true;

	rkFlyMgr.CreateIndexedFly(kPacket.bType, pkStartInst->GetGraphicThingInstancePtr(), pkEndInst->GetGraphicThingInstancePtr());

	return true;
}

bool CPythonNetworkStream::SendTargetPacket(DWORD dwVID)
{
	TPacketCGTarget packet;
	packet.header = HEADER_CG_TARGET;
	packet.dwVID = dwVID;

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send Target Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendSyncPositionElementPacket(DWORD dwVictimVID, DWORD dwVictimX, DWORD dwVictimY)
{
	TPacketCGSyncPositionElement kSyncPos;
	kSyncPos.dwVID=dwVictimVID;
	kSyncPos.lX=dwVictimX;
	kSyncPos.lY=dwVictimY;

	__LocalPositionToGlobalPosition(kSyncPos.lX, kSyncPos.lY);

	if (!Send(sizeof(kSyncPos), &kSyncPos))
	{
		Tracen("CPythonNetworkStream::SendSyncPositionElementPacket - ERROR");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvMessenger()
{
	TPacketGCMessenger p;
	if (!Recv(sizeof(p), &p))
		return false;

	int iSize = p.size - sizeof(p);
	char char_name[24+1];

	switch (p.subheader)
	{
		case MESSENGER_SUBHEADER_GC_LIST:
		{
			TPacketGCMessengerListOnline on;
			while(iSize)
			{
				if (!Recv(sizeof(TPacketGCMessengerListOffline),&on))
					return false;

				if (!Recv(on.length, char_name))
					return false;

				char_name[on.length] = 0;

				if (on.connected & MESSENGER_CONNECTED_STATE_ONLINE)
					CPythonMessenger::Instance().OnFriendLogin(char_name);
				else
					CPythonMessenger::Instance().OnFriendLogout(char_name);

				if (on.connected & MESSENGER_CONNECTED_STATE_MOBILE)
					CPythonMessenger::Instance().SetMobile(char_name, TRUE);

				iSize -= sizeof(TPacketGCMessengerListOffline);
				iSize -= on.length;
			}
			break;
		}

		case MESSENGER_SUBHEADER_GC_LOGIN:
		{
			TPacketGCMessengerLogin p;
			if (!Recv(sizeof(p),&p))
				return false;
			if (!Recv(p.length, char_name))
				return false;
			char_name[p.length] = 0;
			CPythonMessenger::Instance().OnFriendLogin(char_name);
			__RefreshTargetBoardByName(char_name);
			break;
		}

		case MESSENGER_SUBHEADER_GC_LOGOUT:
		{
			TPacketGCMessengerLogout logout;
			if (!Recv(sizeof(logout),&logout))
				return false;
			if (!Recv(logout.length, char_name))
				return false;
			char_name[logout.length] = 0;
			CPythonMessenger::Instance().OnFriendLogout(char_name);
			break;
		}

		case MESSENGER_SUBHEADER_GC_MOBILE:
		{
			BYTE byState;
			BYTE byLength;
			if (!Recv(sizeof(byState), &byState))
				return false;
			if (!Recv(sizeof(byLength), &byLength))
				return false;
			if (!Recv(byLength, char_name))
				return false;
			char_name[byLength] = 0;
			CPythonMessenger::Instance().SetMobile(char_name, byState);
			break;
		}

		#ifdef ENABLE_MESSENGER_BLOCK
		case MESSENGER_SUBHEADER_GC_BLOCK_LIST:
		{
			TPacketGCMessengerBlockListOnline onn;
			while(iSize)
			{
				if (!Recv(sizeof(TPacketGCMessengerBlockListOffline),&onn))
					return false;

				if (!Recv(onn.length, char_name))
					return false;

				char_name[onn.length] = 0;

				if (onn.connected & MESSENGER_CONNECTED_STATE_ONLINE)
					CPythonMessenger::Instance().OnBlockLogin(char_name);
				else
					CPythonMessenger::Instance().OnBlockLogout(char_name);

				iSize -= sizeof(TPacketGCMessengerBlockListOffline);
				iSize -= onn.length;
			}
			break;
		}

		case MESSENGER_SUBHEADER_GC_BLOCK_LOGIN:
		{
			TPacketGCMessengerLogin pp;
			if (!Recv(sizeof(pp),&pp))
				return false;
			if (!Recv(pp.length, char_name))
				return false;
			char_name[pp.length] = 0;
			CPythonMessenger::Instance().OnBlockLogin(char_name);
			__RefreshTargetBoardByName(char_name);
			break;
		}

		case MESSENGER_SUBHEADER_GC_BLOCK_LOGOUT:
		{
			TPacketGCMessengerLogout logout2;
			if (!Recv(sizeof(logout2),&logout2))
				return false;
			if (!Recv(logout2.length, char_name))
				return false;
			char_name[logout2.length] = 0;
			CPythonMessenger::Instance().OnBlockLogout(char_name);
			break;
		}
		#endif

		#ifdef ENABLE_MESSENGER_TEAM
		case MESSENGER_SUBHEADER_GC_TEAM_LIST:
		{
			TPacketGCMessengerTeamListOnline on;
			while (iSize)
			{
				if (!Recv(sizeof(TPacketGCMessengerTeamListOffline), &on))
					return false;

				if (!Recv(on.length, char_name))
					return false;

				char_name[on.length] = 0;

				if (on.connected & MESSENGER_CONNECTED_STATE_ONLINE)
					CPythonMessenger::Instance().OnTeamLogin(char_name);
				else
					CPythonMessenger::Instance().OnTeamLogout(char_name);

				iSize -= sizeof(TPacketGCMessengerTeamListOffline);
				iSize -= on.length;
			}
			break;
		}

		case MESSENGER_SUBHEADER_GC_TEAM_LOGIN:
		{
			TPacketGCMessengerLogin p;
			if (!Recv(sizeof(p), &p))
				return false;
			if (!Recv(p.length, char_name))
				return false;
			char_name[p.length] = 0;
			CPythonMessenger::Instance().OnTeamLogin(char_name);
			__RefreshTargetBoardByName(char_name);
			break;
		}

		case MESSENGER_SUBHEADER_GC_TEAM_LOGOUT:
		{
			TPacketGCMessengerLogout logout;
			if (!Recv(sizeof(logout), &logout))
				return false;
			if (!Recv(logout.length, char_name))
				return false;
			char_name[logout.length] = 0;
			CPythonMessenger::Instance().OnTeamLogout(char_name);
			break;
		}
		#endif
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Party

bool CPythonNetworkStream::SendPartyInvitePacket(DWORD dwVID)
{
	TPacketCGPartyInvite kPartyInvitePacket;
	kPartyInvitePacket.header = HEADER_CG_PARTY_INVITE;
	kPartyInvitePacket.vid = dwVID;

	if (!Send(sizeof(kPartyInvitePacket), &kPartyInvitePacket))
	{
		Tracenf("CPythonNetworkStream::SendPartyInvitePacket [%ud] - PACKET SEND ERROR", dwVID);
		return false;
	}

	Tracef(" << SendPartyInvitePacket : %d\n", dwVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyInviteAnswerPacket(DWORD dwLeaderVID, BYTE byAnswer)
{
	TPacketCGPartyInviteAnswer kPartyInviteAnswerPacket;
	kPartyInviteAnswerPacket.header = HEADER_CG_PARTY_INVITE_ANSWER;
	kPartyInviteAnswerPacket.leader_pid = dwLeaderVID;
	kPartyInviteAnswerPacket.accept = byAnswer;

	if (!Send(sizeof(kPartyInviteAnswerPacket), &kPartyInviteAnswerPacket))
	{
		Tracenf("CPythonNetworkStream::SendPartyInviteAnswerPacket [%ud %ud] - PACKET SEND ERROR", dwLeaderVID, byAnswer);
		return false;
	}

	Tracef(" << SendPartyInviteAnswerPacket : %d, %d\n", dwLeaderVID, byAnswer);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyRemovePacket(DWORD dwPID)
{
	TPacketCGPartyRemove kPartyInviteRemove;
	kPartyInviteRemove.header = HEADER_CG_PARTY_REMOVE;
	kPartyInviteRemove.pid = dwPID;

	if (!Send(sizeof(kPartyInviteRemove), &kPartyInviteRemove))
	{
		Tracenf("CPythonNetworkStream::SendPartyRemovePacket [%ud] - PACKET SEND ERROR", dwPID);
		return false;
	}

	Tracef(" << SendPartyRemovePacket : %d\n", dwPID);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartySetStatePacket(DWORD dwVID, BYTE byState, BYTE byFlag)
{
	TPacketCGPartySetState kPartySetState;
	kPartySetState.byHeader = HEADER_CG_PARTY_SET_STATE;
	kPartySetState.dwVID = dwVID;
	kPartySetState.byState = byState;
	kPartySetState.byFlag = byFlag;

	if (!Send(sizeof(kPartySetState), &kPartySetState))
	{
		Tracenf("CPythonNetworkStream::SendPartySetStatePacket(%ud, %ud) - PACKET SEND ERROR", dwVID, byState);
		return false;
	}

	Tracef(" << SendPartySetStatePacket : %d, %d, %d\n", dwVID, byState, byFlag);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyUseSkillPacket(BYTE bySkillIndex, DWORD dwVID)
{
	TPacketCGPartyUseSkill kPartyUseSkill;
	kPartyUseSkill.byHeader = HEADER_CG_PARTY_USE_SKILL;
	kPartyUseSkill.bySkillIndex = bySkillIndex;
	kPartyUseSkill.dwTargetVID = dwVID;

	if (!Send(sizeof(kPartyUseSkill), &kPartyUseSkill))
	{
		Tracenf("CPythonNetworkStream::SendPartyUseSkillPacket(%ud, %ud) - PACKET SEND ERROR", bySkillIndex, dwVID);
		return false;
	}

	Tracef(" << SendPartyUseSkillPacket : %d, %d\n", bySkillIndex, dwVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyParameterPacket(BYTE byDistributeMode)
{
	TPacketCGPartyParameter kPartyParameter;
	kPartyParameter.bHeader = HEADER_CG_PARTY_PARAMETER;
	kPartyParameter.bDistributeMode = byDistributeMode;

	if (!Send(sizeof(kPartyParameter), &kPartyParameter))
	{
		Tracenf("CPythonNetworkStream::SendPartyParameterPacket(%d) - PACKET SEND ERROR", byDistributeMode);
		return false;
	}

	Tracef(" << SendPartyParameterPacket : %d\n", byDistributeMode);
	return SendSequence();
}

bool CPythonNetworkStream::RecvPartyInvite()
{
	TPacketGCPartyInvite kPartyInvitePacket;
	if (!Recv(sizeof(kPartyInvitePacket), &kPartyInvitePacket))
		return false;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kPartyInvitePacket.leader_pid);
	if (!pInstance)
	{
		TraceError(" CPythonNetworkStream::RecvPartyInvite - Failed to find leader instance [%d]\n", kPartyInvitePacket.leader_pid);
		return true;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RecvPartyInviteQuestion", Py_BuildValue("(is)", kPartyInvitePacket.leader_pid, pInstance->GetNameString()));
	Tracef(" >> RecvPartyInvite : %d, %s\n", kPartyInvitePacket.leader_pid, pInstance->GetNameString());

	return true;
}

bool CPythonNetworkStream::RecvPartyAdd()
{
	TPacketGCPartyAdd kPartyAddPacket;
	if (!Recv(sizeof(kPartyAddPacket), &kPartyAddPacket))
		return false;

	CPythonPlayer::Instance().AppendPartyMember(kPartyAddPacket.pid, kPartyAddPacket.name);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AddPartyMember", Py_BuildValue("(is)", kPartyAddPacket.pid, kPartyAddPacket.name));
	Tracef(" >> RecvPartyAdd : %d, %s\n", kPartyAddPacket.pid, kPartyAddPacket.name);

	return true;
}

bool CPythonNetworkStream::RecvPartyUpdate()
{
	TPacketGCPartyUpdate kPartyUpdatePacket;
	if (!Recv(sizeof(kPartyUpdatePacket), &kPartyUpdatePacket))
		return false;

	CPythonPlayer::TPartyMemberInfo * pPartyMemberInfo;
	if (!CPythonPlayer::Instance().GetPartyMemberPtr(kPartyUpdatePacket.pid, &pPartyMemberInfo))
		return true;

	BYTE byOldState = pPartyMemberInfo->byState;

	CPythonPlayer::Instance().UpdatePartyMemberInfo(kPartyUpdatePacket.pid, kPartyUpdatePacket.state, kPartyUpdatePacket.percent_hp);
	for (int i = 0; i < PARTY_AFFECT_SLOT_MAX_NUM; ++i)
	{
		CPythonPlayer::Instance().UpdatePartyMemberAffect(kPartyUpdatePacket.pid, i, kPartyUpdatePacket.affects[i]);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UpdatePartyMemberInfo", Py_BuildValue("(i)", kPartyUpdatePacket.pid));

	DWORD dwVID;
	if (CPythonPlayer::Instance().PartyMemberPIDToVID(kPartyUpdatePacket.pid, &dwVID))
	if (byOldState != kPartyUpdatePacket.state)
	{
		__RefreshTargetBoardByVID(dwVID);
	}

// 	Tracef(" >> RecvPartyUpdate : %d, %d, %d\n", kPartyUpdatePacket.pid, kPartyUpdatePacket.state, kPartyUpdatePacket.percent_hp);

	return true;
}

bool CPythonNetworkStream::RecvPartyRemove()
{
	TPacketGCPartyRemove kPartyRemovePacket;
	if (!Recv(sizeof(kPartyRemovePacket), &kPartyRemovePacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RemovePartyMember", Py_BuildValue("(i)", kPartyRemovePacket.pid));
	Tracef(" >> RecvPartyRemove : %d\n", kPartyRemovePacket.pid);

	return true;
}

bool CPythonNetworkStream::RecvPartyLink()
{
	TPacketGCPartyLink kPartyLinkPacket;
	if (!Recv(sizeof(kPartyLinkPacket), &kPartyLinkPacket))
		return false;

	CPythonPlayer::Instance().LinkPartyMember(kPartyLinkPacket.pid, kPartyLinkPacket.vid);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "LinkPartyMember", Py_BuildValue("(ii)", kPartyLinkPacket.pid, kPartyLinkPacket.vid));
	Tracef(" >> RecvPartyLink : %d, %d\n", kPartyLinkPacket.pid, kPartyLinkPacket.vid);

	return true;
}

bool CPythonNetworkStream::RecvPartyUnlink()
{
	TPacketGCPartyUnlink kPartyUnlinkPacket;
	if (!Recv(sizeof(kPartyUnlinkPacket), &kPartyUnlinkPacket))
		return false;

	CPythonPlayer::Instance().UnlinkPartyMember(kPartyUnlinkPacket.pid);

	if (CPythonPlayer::Instance().IsMainCharacterIndex(kPartyUnlinkPacket.vid))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UnlinkAllPartyMember", Py_BuildValue("()"));
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UnlinkPartyMember", Py_BuildValue("(i)", kPartyUnlinkPacket.pid));
	}

	Tracef(" >> RecvPartyUnlink : %d, %d\n", kPartyUnlinkPacket.pid, kPartyUnlinkPacket.vid);

	return true;
}

bool CPythonNetworkStream::RecvPartyParameter()
{
	TPacketGCPartyParameter kPartyParameterPacket;
	if (!Recv(sizeof(kPartyParameterPacket), &kPartyParameterPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ChangePartyParameter", Py_BuildValue("(i)", kPartyParameterPacket.bDistributeMode));
	Tracef(" >> RecvPartyParameter : %d\n", kPartyParameterPacket.bDistributeMode);

	return true;
}

// Party
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Guild

bool CPythonNetworkStream::SendGuildAddMemberPacket(DWORD dwVID)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_ADD_MEMBER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwVID), &dwVID))
		return false;

	Tracef(" SendGuildAddMemberPacket\n", dwVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildRemoveMemberPacket(DWORD dwPID)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_REMOVE_MEMBER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwPID), &dwPID))
		return false;

	Tracef(" SendGuildRemoveMemberPacket %d\n", dwPID);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeGradeNamePacket(BYTE byGradeNumber, const char * c_szName)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(byGradeNumber), &byGradeNumber))
		return false;

	char szName[GUILD_GRADE_NAME_MAX_LEN+1];
	strncpy(szName, c_szName, GUILD_GRADE_NAME_MAX_LEN);
	szName[GUILD_GRADE_NAME_MAX_LEN] = '\0';

	if (!Send(sizeof(szName), &szName))
		return false;

	Tracef(" SendGuildChangeGradeNamePacket %d, %s\n", byGradeNumber, c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeGradeAuthorityPacket(BYTE byGradeNumber, BYTE byAuthority)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(byGradeNumber), &byGradeNumber))
		return false;
	if (!Send(sizeof(byAuthority), &byAuthority))
		return false;

	Tracef(" SendGuildChangeGradeAuthorityPacket %d, %d\n", byGradeNumber, byAuthority);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildOfferPacket(DWORD dwExperience)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_OFFER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwExperience), &dwExperience))
		return false;

	Tracef(" SendGuildOfferPacket %d\n", dwExperience);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildPostCommentPacket(const char * c_szMessage)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_POST_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	BYTE bySize = BYTE(strlen(c_szMessage)) + 1;
	if (!Send(sizeof(bySize), &bySize))
		return false;
	if (!Send(bySize, c_szMessage))
		return false;

	Tracef(" SendGuildPostCommentPacket %d, %s\n", bySize, c_szMessage);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildDeleteCommentPacket(DWORD dwIndex)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_DELETE_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwIndex), &dwIndex))
		return false;

	Tracef(" SendGuildDeleteCommentPacket %d\n", dwIndex);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildRefreshCommentsPacket(DWORD dwHighestIndex)
{
	static DWORD s_LastTime = timeGetTime() - 1001;

	if (timeGetTime() - s_LastTime < 1000)
		return true;
	s_LastTime = timeGetTime();

	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_REFRESH_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	Tracef(" SendGuildRefreshCommentPacket %d\n", dwHighestIndex);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeMemberGradePacket(DWORD dwPID, BYTE byGrade)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwPID), &dwPID))
		return false;
	if (!Send(sizeof(byGrade), &byGrade))
		return false;

	Tracef(" SendGuildChangeMemberGradePacket %d, %d\n", dwPID, byGrade);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildUseSkillPacket(DWORD dwSkillID, DWORD dwTargetVID)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_USE_SKILL;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwSkillID), &dwSkillID))
		return false;
	if (!Send(sizeof(dwTargetVID), &dwTargetVID))
		return false;

	Tracef(" SendGuildUseSkillPacket %d, %d\n", dwSkillID, dwTargetVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeMemberGeneralPacket(DWORD dwPID, BYTE byFlag)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwPID), &dwPID))
		return false;
	if (!Send(sizeof(byFlag), &byFlag))
		return false;

	Tracef(" SendGuildChangeMemberGeneralFlagPacket %d, %d\n", dwPID, byFlag);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildInviteAnswerPacket(DWORD dwGuildID, BYTE byAnswer)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwGuildID), &dwGuildID))
		return false;
	if (!Send(sizeof(byAnswer), &byAnswer))
		return false;

	Tracef(" SendGuildInviteAnswerPacket %d, %d\n", dwGuildID, byAnswer);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChargeGSPPacket(DWORD dwMoney)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHARGE_GSP;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildChargeGSPPacket %d\n", dwMoney);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildDepositMoneyPacket(DWORD dwMoney)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_DEPOSIT_MONEY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildDepositMoneyPacket %d\n", dwMoney);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildWithdrawMoneyPacket(DWORD dwMoney)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_WITHDRAW_MONEY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildWithdrawMoneyPacket %d\n", dwMoney);
	return SendSequence();
}

bool CPythonNetworkStream::RecvGuild()
{
    TPacketGCGuild GuildPacket;
	if (!Recv(sizeof(GuildPacket), &GuildPacket))
		return false;

	switch(GuildPacket.subheader)
	{
		case GUILD_SUBHEADER_GC_LOGIN:
		{
			DWORD dwPID;
			if (!Recv(sizeof(DWORD), &dwPID))
				return false;

			// Messenger
			CPythonGuild::TGuildMemberData * pGuildMemberData;
			if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
				if (0 != pGuildMemberData->strName.compare(CPythonPlayer::Instance().GetName()))
					CPythonMessenger::Instance().LoginGuildMember(pGuildMemberData->strName.c_str());

			//Tracef(" <Login> %d\n", dwPID);
			break;
		}
		case GUILD_SUBHEADER_GC_LOGOUT:
		{
			DWORD dwPID;
			if (!Recv(sizeof(DWORD), &dwPID))
				return false;

			// Messenger
			CPythonGuild::TGuildMemberData * pGuildMemberData;
			if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
				if (0 != pGuildMemberData->strName.compare(CPythonPlayer::Instance().GetName()))
					CPythonMessenger::Instance().LogoutGuildMember(pGuildMemberData->strName.c_str());

			//Tracef(" <Logout> %d\n", dwPID);
			break;
		}
		case GUILD_SUBHEADER_GC_REMOVE:
		{
			DWORD dwPID;
			if (!Recv(sizeof(dwPID), &dwPID))
				return false;

			if (CPythonGuild::Instance().IsMainPlayer(dwPID))
			{
				CPythonGuild::Instance().Destroy();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "DeleteGuild", Py_BuildValue("()"));
				CPythonMessenger::Instance().RemoveAllGuildMember();
				__SetGuildID(0);
				__RefreshMessengerWindow();
				__RefreshTargetBoard();
				__RefreshCharacterWindow();
			}
			else
			{
				// Get Member Name
				std::string strMemberName = "";
				CPythonGuild::TGuildMemberData * pData;
				if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pData))
				{
					strMemberName = pData->strName;
					CPythonMessenger::Instance().RemoveGuildMember(pData->strName.c_str());
				}

				CPythonGuild::Instance().RemoveMember(dwPID);

				// Refresh
				__RefreshTargetBoardByName(strMemberName.c_str());
				__RefreshGuildWindowMemberPage();
			}

			Tracef(" <Remove> %d\n", dwPID);
			break;
		}
		case GUILD_SUBHEADER_GC_LIST:
		{
			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);

			for (; iPacketSize > 0;)
			{
				TPacketGCGuildSubMember memberPacket;
				if (!Recv(sizeof(memberPacket), &memberPacket))
					return false;

				char szName[CHARACTER_NAME_MAX_LEN+1] = "";
				if (memberPacket.byNameFlag)
				{
					if (!Recv(sizeof(szName), &szName))
						return false;

					iPacketSize -= CHARACTER_NAME_MAX_LEN+1;
				}
				else
				{
					CPythonGuild::TGuildMemberData * pMemberData;
					if (CPythonGuild::Instance().GetMemberDataPtrByPID(memberPacket.pid, &pMemberData))
					{
						strncpy(szName, pMemberData->strName.c_str(), CHARACTER_NAME_MAX_LEN);
					}
				}

				//Tracef(" <List> %d : %s, %d (%d, %d, %d)\n", memberPacket.pid, szName, memberPacket.byGrade, memberPacket.byJob, memberPacket.byLevel, memberPacket.dwOffer);

				CPythonGuild::SGuildMemberData GuildMemberData;
				GuildMemberData.dwPID = memberPacket.pid;
				GuildMemberData.byGrade = memberPacket.byGrade;
				GuildMemberData.strName = szName;
				GuildMemberData.byJob = memberPacket.byJob;
				GuildMemberData.byLevel = memberPacket.byLevel;
				GuildMemberData.dwOffer = memberPacket.dwOffer;
				GuildMemberData.byGeneralFlag = memberPacket.byIsGeneral;
				CPythonGuild::Instance().RegisterMember(GuildMemberData);

				// Messenger
				if (strcmp(szName, CPythonPlayer::Instance().GetName()))
					CPythonMessenger::Instance().AppendGuildMember(szName);

				__RefreshTargetBoardByName(szName);

				iPacketSize -= sizeof(memberPacket);
			}

			__RefreshGuildWindowInfoPage();
			__RefreshGuildWindowMemberPage();
			__RefreshMessengerWindow();
			__RefreshCharacterWindow();
			break;
		}
		case GUILD_SUBHEADER_GC_GRADE:
		{
			BYTE byCount;
			if (!Recv(sizeof(byCount), &byCount))
				return false;

			for (BYTE i = 0; i < byCount; ++ i)
			{
				BYTE byIndex;
				if (!Recv(sizeof(byCount), &byIndex))
					return false;
				TPacketGCGuildSubGrade GradePacket;
				if (!Recv(sizeof(GradePacket), &GradePacket))
					return false;

				CPythonGuild::Instance().SetGradeData(byIndex, CPythonGuild::SGuildGradeData(GradePacket.auth_flag, GradePacket.grade_name));
				//Tracef(" <Grade> [%d/%d] : %s, %d\n", byIndex, byCount, GradePacket.grade_name, GradePacket.auth_flag);
			}
			__RefreshGuildWindowGradePage();
			__RefreshGuildWindowMemberPageGradeComboBox();
			break;
		}
		case GUILD_SUBHEADER_GC_GRADE_NAME:
		{
			BYTE byGradeNumber;
			if (!Recv(sizeof(byGradeNumber), &byGradeNumber))
				return false;

			char szGradeName[GUILD_GRADE_NAME_MAX_LEN+1] = "";
			if (!Recv(sizeof(szGradeName), &szGradeName))
				return false;

			CPythonGuild::Instance().SetGradeName(byGradeNumber, szGradeName);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGrade", Py_BuildValue("()"));

			Tracef(" <Change Grade Name> %d, %s\n", byGradeNumber, szGradeName);
			__RefreshGuildWindowGradePage();
			__RefreshGuildWindowMemberPageGradeComboBox();
			break;
		}
		case GUILD_SUBHEADER_GC_GRADE_AUTH:
		{
			BYTE byGradeNumber;
			if (!Recv(sizeof(byGradeNumber), &byGradeNumber))
				return false;
			BYTE byAuthorityFlag;
			if (!Recv(sizeof(byAuthorityFlag), &byAuthorityFlag))
				return false;

			CPythonGuild::Instance().SetGradeAuthority(byGradeNumber, byAuthorityFlag);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGrade", Py_BuildValue("()"));

			Tracef(" <Change Grade Authority> %d, %d\n", byGradeNumber, byAuthorityFlag);
			__RefreshGuildWindowGradePage();
			break;
		}
		case GUILD_SUBHEADER_GC_INFO:
		{
			TPacketGCGuildInfo GuildInfo;
			if (!Recv(sizeof(GuildInfo), &GuildInfo))
				return false;

			CPythonGuild::Instance().EnableGuild();
			CPythonGuild::TGuildInfo & rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
			strncpy(rGuildInfo.szGuildName, GuildInfo.name, GUILD_NAME_MAX_LEN);
			rGuildInfo.szGuildName[GUILD_NAME_MAX_LEN] = '\0';

			rGuildInfo.dwGuildID = GuildInfo.guild_id;
			rGuildInfo.dwMasterPID = GuildInfo.master_pid;
			rGuildInfo.dwGuildLevel = GuildInfo.level;
			rGuildInfo.dwCurrentExperience = GuildInfo.exp;
			rGuildInfo.dwCurrentMemberCount = GuildInfo.member_count;
			rGuildInfo.dwMaxMemberCount = GuildInfo.max_member_count;
			rGuildInfo.dwGuildMoney = GuildInfo.gold;
			rGuildInfo.bHasLand = GuildInfo.hasLand;

			//Tracef(" <Info> %s, %d, %d : %d\n", GuildInfo.name, GuildInfo.master_pid, GuildInfo.level, rGuildInfo.bHasLand);
			__RefreshGuildWindowInfoPage();
			break;
		}
		case GUILD_SUBHEADER_GC_COMMENTS:
		{
			BYTE byCount;
			if (!Recv(sizeof(byCount), &byCount))
				return false;

			CPythonGuild::Instance().ClearComment();
			//Tracef(" >>> Comments Count : %d\n", byCount);

			for (BYTE i = 0; i < byCount; ++i)
			{
				DWORD dwCommentID;
				if (!Recv(sizeof(dwCommentID), &dwCommentID))
					return false;

				char szName[CHARACTER_NAME_MAX_LEN+1] = "";
				if (!Recv(sizeof(szName), &szName))
					return false;

				char szComment[GULID_COMMENT_MAX_LEN+1] = "";
				if (!Recv(sizeof(szComment), &szComment))
					return false;

				//Tracef(" [Comment-%d] : %s, %s\n", dwCommentID, szName, szComment);
				CPythonGuild::Instance().RegisterComment(dwCommentID, szName, szComment);
			}

			__RefreshGuildWindowBoardPage();
			break;
		}
		case GUILD_SUBHEADER_GC_CHANGE_EXP:
		{
			BYTE byLevel;
			if (!Recv(sizeof(byLevel), &byLevel))
				return false;
			DWORD dwEXP;
			if (!Recv(sizeof(dwEXP), &dwEXP))
				return false;
			CPythonGuild::Instance().SetGuildEXP(byLevel, dwEXP);
			Tracef(" <ChangeEXP> %d, %d\n", byLevel, dwEXP);
			__RefreshGuildWindowInfoPage();
			break;
		}
		case GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE:
		{
			DWORD dwPID;
			if (!Recv(sizeof(dwPID), &dwPID))
				return false;
			BYTE byGrade;
			if (!Recv(sizeof(byGrade), &byGrade))
				return false;
			CPythonGuild::Instance().ChangeGuildMemberGrade(dwPID, byGrade);
			Tracef(" <ChangeMemberGrade> %d, %d\n", dwPID, byGrade);
			__RefreshGuildWindowMemberPage();
			break;
		}
		case GUILD_SUBHEADER_GC_SKILL_INFO:
		{
			CPythonGuild::TGuildSkillData & rSkillData = CPythonGuild::Instance().GetGuildSkillDataRef();
			if (!Recv(sizeof(rSkillData.bySkillPoint), &rSkillData.bySkillPoint))
				return false;
			if (!Recv(sizeof(rSkillData.bySkillLevel), rSkillData.bySkillLevel))
				return false;
			if (!Recv(sizeof(rSkillData.wGuildPoint), &rSkillData.wGuildPoint))
				return false;
			if (!Recv(sizeof(rSkillData.wMaxGuildPoint), &rSkillData.wMaxGuildPoint))
				return false;

			Tracef(" <SkillInfo> %d / %d, %d\n", rSkillData.bySkillPoint, rSkillData.wGuildPoint, rSkillData.wMaxGuildPoint);
			__RefreshGuildWindowSkillPage();
			break;
		}
		case GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL:
		{
			DWORD dwPID;
			if (!Recv(sizeof(dwPID), &dwPID))
				return false;
			BYTE byFlag;
			if (!Recv(sizeof(byFlag), &byFlag))
				return false;

			CPythonGuild::Instance().ChangeGuildMemberGeneralFlag(dwPID, byFlag);
			Tracef(" <ChangeMemberGeneralFlag> %d, %d\n", dwPID, byFlag);
			__RefreshGuildWindowMemberPage();
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_INVITE:
		{
			DWORD dwGuildID;
			if (!Recv(sizeof(dwGuildID), &dwGuildID))
				return false;
			char szGuildName[GUILD_NAME_MAX_LEN+1];
			if (!Recv(GUILD_NAME_MAX_LEN, &szGuildName))
				return false;

			szGuildName[GUILD_NAME_MAX_LEN] = 0;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RecvGuildInviteQuestion", Py_BuildValue("(is)", dwGuildID, szGuildName));
			Tracef(" <Guild Invite> %d, %s\n", dwGuildID, szGuildName);
			break;
		}
		case GUILD_SUBHEADER_GC_WAR:
		{
			TPacketGCGuildWar kGuildWar;
			if (!Recv(sizeof(kGuildWar), &kGuildWar))
				return false;

			switch (kGuildWar.bWarState)
			{
				case GUILD_WAR_SEND_DECLARE:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_SEND_DECLARE\n");
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"BINARY_GuildWar_OnSendDeclare",
						Py_BuildValue("(i)", kGuildWar.dwGuildOpp)
					);
					break;

				case GUILD_WAR_RECV_DECLARE:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_RECV_DECLARE\n");
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"BINARY_GuildWar_OnRecvDeclare",
						Py_BuildValue(
							#ifdef ENABLE_GUILD_WAR_OPTION_SYSTEM
							"(iiiiibbbb)"
							#else
							"(ii)"
							#endif
							, kGuildWar.dwGuildOpp
							, kGuildWar.bType
							#ifdef ENABLE_GUILD_WAR_OPTION_SYSTEM
							, kGuildWar.iScore
							, kGuildWar.iMinLevel
							, kGuildWar.iMaxPlayer
							, kGuildWar.bWarrior
							, kGuildWar.bAssassin
							, kGuildWar.bSura
							, kGuildWar.bShaman
							#endif
						)
					);
					break;

				case GUILD_WAR_ON_WAR:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_ON_WAR : %d, %d\n", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp);
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"BINARY_GuildWar_OnStart",
						Py_BuildValue("(ii)", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp)
					);
					CPythonGuild::Instance().StartGuildWar(kGuildWar.dwGuildOpp);
					break;

				case GUILD_WAR_END:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_END\n");
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"BINARY_GuildWar_OnEnd",
						Py_BuildValue("(ii)", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp)
					);
					CPythonGuild::Instance().EndGuildWar(kGuildWar.dwGuildOpp);
					break;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_NAME:
		{
			DWORD dwID;
			char szGuildName[GUILD_NAME_MAX_LEN+1];

			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);

			int nItemSize = sizeof(dwID) + GUILD_NAME_MAX_LEN;

			assert(iPacketSize%nItemSize==0 && "GUILD_SUBHEADER_GC_GUILD_NAME");

			for (; iPacketSize > 0;)
			{
				if (!Recv(sizeof(dwID), &dwID))
					return false;

				if (!Recv(GUILD_NAME_MAX_LEN, &szGuildName))
					return false;

				szGuildName[GUILD_NAME_MAX_LEN] = 0;

				//Tracef(" >> GulidName [%d : %s]\n", dwID, szGuildName);
				CPythonGuild::Instance().RegisterGuildName(dwID, szGuildName);
				iPacketSize -= nItemSize;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_WAR_LIST:
		{
			DWORD dwSrcGuildID;
			DWORD dwDstGuildID;

			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);
			int nItemSize = sizeof(dwSrcGuildID) + sizeof(dwDstGuildID);

			assert(iPacketSize%nItemSize==0 && "GUILD_SUBHEADER_GC_GUILD_WAR_LIST");

			for (; iPacketSize > 0;)
			{
				if (!Recv(sizeof(dwSrcGuildID), &dwSrcGuildID))
					return false;

				if (!Recv(sizeof(dwDstGuildID), &dwDstGuildID))
					return false;

				Tracef(" >> GulidWarList [%d vs %d]\n", dwSrcGuildID, dwDstGuildID);
				CInstanceBase::InsertGVGKey(dwSrcGuildID, dwDstGuildID);
				CPythonCharacterManager::Instance().ChangeGVG(dwSrcGuildID, dwDstGuildID);
				iPacketSize -= nItemSize;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST:
		{
			DWORD dwSrcGuildID;
			DWORD dwDstGuildID;

			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);
			int nItemSize = sizeof(dwSrcGuildID) + sizeof(dwDstGuildID);

			assert(iPacketSize%nItemSize==0 && "GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST");

			for (; iPacketSize > 0;)
			{

				if (!Recv(sizeof(dwSrcGuildID), &dwSrcGuildID))
					return false;

				if (!Recv(sizeof(dwDstGuildID), &dwDstGuildID))
					return false;

				Tracef(" >> GulidWarEndList [%d vs %d]\n", dwSrcGuildID, dwDstGuildID);
				CInstanceBase::RemoveGVGKey(dwSrcGuildID, dwDstGuildID);
				CPythonCharacterManager::Instance().ChangeGVG(dwSrcGuildID, dwDstGuildID);
				iPacketSize -= nItemSize;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_WAR_POINT:
		{
			TPacketGuildWarPoint GuildWarPoint;
			if (!Recv(sizeof(GuildWarPoint), &GuildWarPoint))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
				"BINARY_GuildWar_OnRecvPoint",
				Py_BuildValue("(iii)", GuildWarPoint.dwGainGuildID, GuildWarPoint.dwOpponentGuildID, GuildWarPoint.lPoint)
			);
			break;
		}
		case GUILD_SUBHEADER_GC_MONEY_CHANGE:
		{
			DWORD dwMoney;
			if (!Recv(sizeof(dwMoney), &dwMoney))
				return false;

			CPythonGuild::Instance().SetGuildMoney(dwMoney);

			__RefreshGuildWindowInfoPage();
			Tracef(" >> Guild Money Change : %d\n", dwMoney);
			break;
		}
	}

	return true;
}

// Guild
//////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// Fishing

bool CPythonNetworkStream::SendFishingPacket(int iRotation)
{
	BYTE byHeader = HEADER_CG_FISHING;
	if (!Send(sizeof(byHeader), &byHeader))
		return false;
	BYTE byPacketRotation = iRotation / 5;
	if (!Send(sizeof(BYTE), &byPacketRotation))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendGiveItemPacket(DWORD dwTargetVID, TItemPos ItemPos, int iItemCount)
{
	TPacketCGGiveItem GiveItemPacket;
	GiveItemPacket.byHeader = HEADER_CG_GIVE_ITEM;
	GiveItemPacket.dwTargetVID = dwTargetVID;
	GiveItemPacket.ItemPos = ItemPos;
	GiveItemPacket.byItemCount = iItemCount;

	if (!Send(sizeof(GiveItemPacket), &GiveItemPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvFishing()
{
	TPacketGCFishing FishingPacket;
	if (!Recv(sizeof(FishingPacket), &FishingPacket))
		return false;

	CInstanceBase * pFishingInstance = NULL;
	if (FISHING_SUBHEADER_GC_FISH != FishingPacket.subheader)
	{
		pFishingInstance = CPythonCharacterManager::Instance().GetInstancePtr(FishingPacket.info);
		if (!pFishingInstance)
			return true;
	}

	switch (FishingPacket.subheader)
	{
		case FISHING_SUBHEADER_GC_START:
			pFishingInstance->StartFishing(float(FishingPacket.dir) * 5.0f);
			break;
		case FISHING_SUBHEADER_GC_STOP:
			if (pFishingInstance->IsFishing())
				pFishingInstance->StopFishing();
			break;
		case FISHING_SUBHEADER_GC_REACT:
			if (pFishingInstance->IsFishing())
			{
				pFishingInstance->SetFishEmoticon(); // Fish Emoticon
				pFishingInstance->ReactFishing();
			}
			break;
		case FISHING_SUBHEADER_GC_SUCCESS:
			pFishingInstance->CatchSuccess();
			break;
		case FISHING_SUBHEADER_GC_FAIL:
			pFishingInstance->CatchFail();
			if (pFishingInstance == CPythonCharacterManager::Instance().GetMainInstancePtr())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingFailure", Py_BuildValue("()"));
			}
			break;
		case FISHING_SUBHEADER_GC_FISH:
		{
			DWORD dwFishID = FishingPacket.info;

			if (0 == FishingPacket.info)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingNotifyUnknown", Py_BuildValue("()"));
				return true;
			}

			CItemData * pItemData;
			if (!CItemManager::Instance().GetItemDataPointer(dwFishID, &pItemData))
				return true;

			CInstanceBase * pMainInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
			if (!pMainInstance)
				return true;

			if (pMainInstance->IsFishing())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingNotify", Py_BuildValue("(is)", CItemData::ITEM_TYPE_FISH == pItemData->GetType(), pItemData->GetName()));
			}
			else
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingSuccess", Py_BuildValue("(is)", CItemData::ITEM_TYPE_FISH == pItemData->GetType(), pItemData->GetName()));
			}
			break;
		}
	}

	return true;
}
// Fishing
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// Dungeon
bool CPythonNetworkStream::RecvDungeon()
{
	TPacketGCDungeon DungeonPacket;
	if (!Recv(sizeof(DungeonPacket), &DungeonPacket))
		return false;

	switch (DungeonPacket.subheader)
	{
		case DUNGEON_SUBHEADER_GC_TIME_ATTACK_START:
		{
			break;
		}
		case DUNGEON_SUBHEADER_GC_DESTINATION_POSITION:
		{
			DWORD ulx, uly;
			if (!Recv(sizeof(ulx), &ulx))
				return false;
			if (!Recv(sizeof(uly), &uly))
				return false;

			CPythonPlayer::Instance().SetDungeonDestinationPosition(ulx, uly);
			break;
		}
	}

	return true;
}
// Dungeon
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// MyShop
bool CPythonNetworkStream::SendBuildPrivateShopPacket(const char * c_szName, const std::vector<TShopItemTable> & c_rSellingItemStock)
{
	TPacketCGMyShop packet;
	packet.bHeader = HEADER_CG_MYSHOP;
	strncpy(packet.szSign, c_szName, SHOP_SIGN_MAX_LEN);
	packet.bCount = c_rSellingItemStock.size();
	if (!Send(sizeof(packet), &packet))
		return false;

	for (std::vector<TShopItemTable>::const_iterator itor = c_rSellingItemStock.begin(); itor < c_rSellingItemStock.end(); ++itor)
	{
		const TShopItemTable & c_rItem = *itor;
		if (!Send(sizeof(c_rItem), &c_rItem))
			return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvShopSignPacket()
{
	TPacketGCShopSign p;
	if (!Recv(sizeof(TPacketGCShopSign), &p))
		return false;

	CPythonPlayer& rkPlayer=CPythonPlayer::Instance();

	if (0 == strlen(p.szSign))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
			"BINARY_PrivateShop_Disappear",
			Py_BuildValue("(i)", p.dwVID)
		);

		if (rkPlayer.IsMainCharacterIndex(p.dwVID))
			rkPlayer.ClosePrivateShop();
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
			"BINARY_PrivateShop_Appear",
			Py_BuildValue("(is)", p.dwVID, p.szSign)
		);

		if (rkPlayer.IsMainCharacterIndex(p.dwVID))
			rkPlayer.OpenPrivateShop();
	}

	return true;
}
/////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_FAST_OPEN_CHEST
bool CPythonNetworkStream::SendInstantChestOpenPacket(TItemPos iPos){
	TInstantChestOpen gPacket;
	gPacket.bHeader = HEADER_CG_INSTANT_OPEN;
	gPacket.iPos = iPos;
	if (!Send(sizeof(gPacket), &gPacket)){ return false; }
	return true;
}
#endif

bool CPythonNetworkStream::RecvTimePacket()
{
	TPacketGCTime TimePacket;
	if (!Recv(sizeof(TimePacket), &TimePacket))
		return false;

	IAbstractApplication& rkApp=IAbstractApplication::GetSingleton();
	rkApp.SetServerTime(TimePacket.time);

	return true;
}

bool CPythonNetworkStream::RecvWalkModePacket()
{
	TPacketGCWalkMode WalkModePacket;
	if (!Recv(sizeof(WalkModePacket), &WalkModePacket))
		return false;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(WalkModePacket.vid);
	if (pInstance)
	{
		if (WALKMODE_RUN == WalkModePacket.mode)
		{
			pInstance->SetRunMode();
		}
		else
		{
			pInstance->SetWalkMode();
		}
	}

	return true;
}

bool CPythonNetworkStream::RecvChangeSkillGroupPacket()
{
	TPacketGCChangeSkillGroup ChangeSkillGroup;
	if (!Recv(sizeof(ChangeSkillGroup), &ChangeSkillGroup))
		return false;

	m_dwMainActorSkillGroup = ChangeSkillGroup.skill_group;

	CPythonPlayer::Instance().NEW_ClearSkillData();
	__RefreshCharacterWindow();
	return true;
}

void CPythonNetworkStream::__TEST_SetSkillGroupFake(int iIndex)
{
	m_dwMainActorSkillGroup = DWORD(iIndex);

	CPythonPlayer::Instance().NEW_ClearSkillData();
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharacter", Py_BuildValue("()"));
}

bool CPythonNetworkStream::SendRefinePacket(BYTE byPos, BYTE byType)
{
	TPacketCGRefine kRefinePacket;
	kRefinePacket.header = HEADER_CG_REFINE;
	kRefinePacket.pos = byPos;
	kRefinePacket.type = byType;

	if (!Send(sizeof(kRefinePacket), &kRefinePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendSelectItemPacket(DWORD dwItemPos)
{
	TPacketCGScriptSelectItem kScriptSelectItem;
	kScriptSelectItem.header = HEADER_CG_SCRIPT_SELECT_ITEM;
	kScriptSelectItem.selection = dwItemPos;

	if (!Send(sizeof(kScriptSelectItem), &kScriptSelectItem))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvRefineInformationPacket()
{
	TPacketGCRefineInformation kRefineInfoPacket;
	if (!Recv(sizeof(kRefineInfoPacket), &kRefineInfoPacket))
		return false;

	TRefineTable & rkRefineTable = kRefineInfoPacket.refine_table;
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
		"OpenRefineDialog",
		#ifdef ENABLE_FULL_YANG
		Py_BuildValue("(iiKii)",
		#else
		Py_BuildValue("(iiiii)",
		#endif
			kRefineInfoPacket.pos,
			kRefineInfoPacket.refine_table.result_vnum,
			rkRefineTable.cost,
			rkRefineTable.prob));

	for (int i = 0; i < rkRefineTable.material_count; ++i)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AppendMaterialToRefineDialog", Py_BuildValue("(ii)", rkRefineTable.materials[i].vnum, rkRefineTable.materials[i].count));
	}

#ifdef _DEBUG
	Tracef(" >> RecvRefineInformationPacket(pos=%d, result_vnum=%d, cost=%lld, prob=%d)\n",
														kRefineInfoPacket.pos,
														kRefineInfoPacket.refine_table.result_vnum,
														rkRefineTable.cost,
														rkRefineTable.prob);
#endif

	return true;
}

bool CPythonNetworkStream::RecvRefineInformationPacketNew()
{
	TPacketGCRefineInformationNew kRefineInfoPacket;
	if (!Recv(sizeof(kRefineInfoPacket), &kRefineInfoPacket))
		return false;

	TRefineTable & rkRefineTable = kRefineInfoPacket.refine_table;
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
		"OpenRefineDialog",
		#ifdef ENABLE_FULL_YANG
		Py_BuildValue("(iiKii)",
		#else
		Py_BuildValue("(iiiii)",
		#endif
			kRefineInfoPacket.pos,
			kRefineInfoPacket.refine_table.result_vnum,
			rkRefineTable.cost,
			rkRefineTable.prob,
			kRefineInfoPacket.type)
		);

	for (int i = 0; i < rkRefineTable.material_count; ++i)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AppendMaterialToRefineDialog", Py_BuildValue("(ii)", rkRefineTable.materials[i].vnum, rkRefineTable.materials[i].count));
	}

#ifdef _DEBUG
	Tracef(" >> RecvRefineInformationPacketNew(pos=%d, result_vnum=%d, cost=%lld, prob=%d, type=%d)\n",
														kRefineInfoPacket.pos,
														kRefineInfoPacket.refine_table.result_vnum,
														rkRefineTable.cost,
														rkRefineTable.prob,
														kRefineInfoPacket.type);
#endif

	return true;
}

bool CPythonNetworkStream::RecvNPCList()
{
	TPacketGCNPCPosition kNPCPosition;
	if (!Recv(sizeof(kNPCPosition), &kNPCPosition))
		return false;

	assert(int(kNPCPosition.size)-sizeof(kNPCPosition) == kNPCPosition.count*sizeof(TNPCPosition) && "HEADER_GC_NPC_POSITION");

	CPythonMiniMap::Instance().ClearAtlasMarkInfo();

	for (int i = 0; i < kNPCPosition.count; ++i)
	{
		TNPCPosition NPCPosition;
		if (!Recv(sizeof(TNPCPosition), &NPCPosition))
			return false;
#ifdef ENABLE_DUNGEON_BOSS_ICON_IN_MAP
		const std::string name = CPythonNonPlayer::Instance().GetMonsterName(NPCPosition.vnum);
		CPythonMiniMap::Instance().RegisterAtlasMark(NPCPosition.bType, name.c_str(), NPCPosition.x, NPCPosition.y, NPCPosition.vnum, NPCPosition.time);
#else
		CPythonMiniMap::Instance().RegisterAtlasMark(NPCPosition.bType, NPCPosition.name, NPCPosition.x, NPCPosition.y);
#endif
	}

	return true;
}

bool CPythonNetworkStream::__SendCRCReportPacket()
{
	/*
	DWORD dwProcessCRC = 0;
	DWORD dwFileCRC = 0;
	CFilename exeFileName;
	//LPCVOID c_pvBaseAddress = NULL;

	GetExeCRC(dwProcessCRC, dwFileCRC);

	CFilename strRootPackFileName = CEterPackManager::Instance().GetRootPacketFileName();
	strRootPackFileName.ChangeDosPath();

	TPacketCGCRCReport kReportPacket;

	kReportPacket.header = HEADER_CG_CRC_REPORT;
	kReportPacket.byPackMode = CEterPackManager::Instance().GetSearchMode();
	kReportPacket.dwBinaryCRC32 = dwFileCRC;
	kReportPacket.dwProcessCRC32 = dwProcessCRC;
	kReportPacket.dwRootPackCRC32 = GetFileCRC32(strRootPackFileName.c_str());

	if (!Send(sizeof(kReportPacket), &kReportPacket))
		Tracef("SendClientReportPacket Error");

	return SendSequence();
	*/
	return true;
}

bool CPythonNetworkStream::SendClientVersionPacket()
{
	std::string filename;

	GetExcutedFileName(filename);

	filename = CFileNameHelper::NoPath(filename);
	CFileNameHelper::ChangeDosPath(filename);

	if (LocaleService_IsEUROPE() && false == LocaleService_IsYMIR())
	{
		TPacketCGClientVersion2 kVersionPacket;
		kVersionPacket.header = HEADER_CG_CLIENT_VERSION2;
		strncpy(kVersionPacket.filename, filename.c_str(), sizeof(kVersionPacket.filename)-1);
		strncpy(kVersionPacket.timestamp, "1215955205", sizeof(kVersionPacket.timestamp)-1);
		//strncpy(kVersionPacket.timestamp, __TIMESTAMP__, sizeof(kVersionPacket.timestamp)-1); // old_string_ver
		//strncpy(kVersionPacket.timestamp, "1218055205", sizeof(kVersionPacket.timestamp)-1); // new_future
		//strncpy(kVersionPacket.timestamp, "1214055205", sizeof(kVersionPacket.timestamp)-1); // old_past

		if (!Send(sizeof(kVersionPacket), &kVersionPacket))
			Tracef("SendClientReportPacket Error");
	}
	else
	{
		TPacketCGClientVersion kVersionPacket;
		kVersionPacket.header = HEADER_CG_CLIENT_VERSION;
		strncpy(kVersionPacket.filename, filename.c_str(), sizeof(kVersionPacket.filename)-1);
		strncpy(kVersionPacket.timestamp, __TIMESTAMP__, sizeof(kVersionPacket.timestamp)-1);

		if (!Send(sizeof(kVersionPacket), &kVersionPacket))
			Tracef("SendClientReportPacket Error");
	}
	return SendSequence();
}

bool CPythonNetworkStream::RecvAffectAddPacket()
{
	TPacketGCAffectAdd kAffectAdd;
	if (!Recv(sizeof(kAffectAdd), &kAffectAdd))
		return false;

	TPacketAffectElement & rkElement = kAffectAdd.elem;
	if (rkElement.bPointIdxApplyOn == POINT_ENERGY)
	{
		CPythonPlayer::instance().SetStatus (POINT_ENERGY_END_TIME, CPythonApplication::Instance().GetServerTimeStamp() + rkElement.lDuration);
		__RefreshStatus();
	}
#ifdef ENABLE_AFFECT_CHECK
	CPythonPlayer::instance().AddAffect(rkElement.dwType, kAffectAdd.elem);
#endif
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_NEW_AddAffect", Py_BuildValue("(iiii)", rkElement.dwType, rkElement.bPointIdxApplyOn, rkElement.lApplyValue, rkElement.lDuration));
	return true;
}

bool CPythonNetworkStream::RecvAffectRemovePacket()
{
	TPacketGCAffectRemove kAffectRemove;
	if (!Recv(sizeof(kAffectRemove), &kAffectRemove))
		return false;
#ifdef ENABLE_AFFECT_CHECK
	CPythonPlayer::instance().RemoveAffect(kAffectRemove.dwType, kAffectRemove.bApplyOn);
#endif
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_NEW_RemoveAffect", Py_BuildValue("(ii)", kAffectRemove.dwType, kAffectRemove.bApplyOn));
	return true;
}

bool CPythonNetworkStream::RecvChannelPacket()
{
	TPacketGCChannel kChannelPacket;
	if (!Recv(sizeof(kChannelPacket), &kChannelPacket))
		return false;

	#ifdef ENABLE_CHANNEL_INFO_UPDATE
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ReceiveChannel", Py_BuildValue("(i)", kChannelPacket.channel));
	#endif
	//Tracef(" >> CPythonNetworkStream::RecvChannelPacket(channel=%d)\n", kChannelPacket.channel);
	return true;
}

bool CPythonNetworkStream::RecvViewEquipPacket()
{
	TPacketGCViewEquip kViewEquipPacket;
	if (!Recv(sizeof(kViewEquipPacket), &kViewEquipPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenEquipmentDialog", Py_BuildValue("(i)", kViewEquipPacket.dwVID));

	for (int i = 0; i < WEAR_MAX_NUM; ++i)
	{
		TEquipmentItemSet & rItemSet = kViewEquipPacket.equips[i];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogItem", Py_BuildValue("(iiii)", kViewEquipPacket.dwVID, i, rItemSet.vnum, rItemSet.count));

		for (int j = 0; j < ITEM_SOCKET_SLOT_MAX_NUM; ++j)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogSocket", Py_BuildValue("(iiii)", kViewEquipPacket.dwVID, i, j, rItemSet.alSockets[j]));

		for (int k = 0; k < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++k)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogAttr", Py_BuildValue("(iiiii)", kViewEquipPacket.dwVID, i, k, rItemSet.aAttr[k].bType, rItemSet.aAttr[k].sValue));
	}

	return true;
}

bool CPythonNetworkStream::RecvLandPacket()
{
	TPacketGCLandList kLandList;
	if (!Recv(sizeof(kLandList), &kLandList))
		return false;

	std::vector<DWORD> kVec_dwGuildID;

	CPythonMiniMap & rkMiniMap = CPythonMiniMap::Instance();
	CPythonBackground & rkBG = CPythonBackground::Instance();
	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();

	rkMiniMap.ClearGuildArea();
	rkBG.ClearGuildArea();

	int iPacketSize = (kLandList.size - sizeof(TPacketGCLandList));
	for (; iPacketSize > 0; iPacketSize-=sizeof(TLandPacketElement))
	{
		TLandPacketElement kElement;
		if (!Recv(sizeof(TLandPacketElement), &kElement))
			return false;

		rkMiniMap.RegisterGuildArea(kElement.dwID,
									kElement.dwGuildID,
									kElement.x,
									kElement.y,
									kElement.width,
									kElement.height);

		if (pMainInstance)
		if (kElement.dwGuildID == pMainInstance->GetGuildID())
		{
			rkBG.RegisterGuildArea(kElement.x,
								   kElement.y,
								   kElement.x+kElement.width,
								   kElement.y+kElement.height);
		}

		if (0 != kElement.dwGuildID)
			kVec_dwGuildID.push_back(kElement.dwGuildID);
	}
	// @fixme006
	if (kVec_dwGuildID.size()>0)
		__DownloadSymbol(kVec_dwGuildID);

	return true;
}

bool CPythonNetworkStream::RecvTargetCreatePacket()
{
	TPacketGCTargetCreate kTargetCreate;
	if (!Recv(sizeof(kTargetCreate), &kTargetCreate))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName);

//#ifdef _DEBUG
//	char szBuf[256+1];
//	CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
//	Tracef(" >> RecvTargetCreatePacket %d : %s\n", kTargetCreate.lID, kTargetCreate.szTargetName);
//#endif

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenAtlasWindow", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::RecvTargetCreatePacketNew()
{
	TPacketGCTargetCreateNew kTargetCreate;
	if (!Recv(sizeof(kTargetCreate), &kTargetCreate))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	CPythonBackground & rkpyBG = CPythonBackground::Instance();
	if (CREATE_TARGET_TYPE_LOCATION == kTargetCreate.byType)
	{
		rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName);
	}
	else
	{
		rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName, kTargetCreate.dwVID);
		rkpyBG.CreateTargetEffect(kTargetCreate.lID, kTargetCreate.dwVID);
	}

//#ifdef _DEBUG
//	char szBuf[256+1];
//	CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
//	Tracef(" >> RecvTargetCreatePacketNew %d : %d/%d\n", kTargetCreate.lID, kTargetCreate.byType, kTargetCreate.dwVID);
//#endif

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenAtlasWindow", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::RecvTargetUpdatePacket()
{
	TPacketGCTargetUpdate kTargetUpdate;
	if (!Recv(sizeof(kTargetUpdate), &kTargetUpdate))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	rkpyMiniMap.UpdateTarget(kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);

	CPythonBackground & rkpyBG = CPythonBackground::Instance();
	rkpyBG.CreateTargetEffect(kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);

//#ifdef _DEBUG
//	char szBuf[256+1];
//	CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
//	Tracef(" >> RecvTargetUpdatePacket %d : %d, %d\n", kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);
//#endif

	return true;
}

bool CPythonNetworkStream::RecvTargetDeletePacket()
{
	TPacketGCTargetDelete kTargetDelete;
	if (!Recv(sizeof(kTargetDelete), &kTargetDelete))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	rkpyMiniMap.DeleteTarget(kTargetDelete.lID);

	CPythonBackground & rkpyBG = CPythonBackground::Instance();
	rkpyBG.DeleteTargetEffect(kTargetDelete.lID);

//#ifdef _DEBUG
//	Tracef(" >> RecvTargetDeletePacket %d\n", kTargetDelete.lID);
//#endif

	return true;
}

bool CPythonNetworkStream::RecvLoverInfoPacket()
{
	TPacketGCLoverInfo kLoverInfo;
	if (!Recv(sizeof(kLoverInfo), &kLoverInfo))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_LoverInfo", Py_BuildValue("(si)", kLoverInfo.szName, kLoverInfo.byLovePoint));
#ifdef _DEBUG
	Tracef("RECV LOVER INFO : %s, %d\n", kLoverInfo.szName, kLoverInfo.byLovePoint);
#endif
	return true;
}

bool CPythonNetworkStream::RecvLovePointUpdatePacket()
{
	TPacketGCLovePointUpdate kLovePointUpdate;
	if (!Recv(sizeof(kLovePointUpdate), &kLovePointUpdate))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_UpdateLovePoint", Py_BuildValue("(i)", kLovePointUpdate.byLovePoint));
#ifdef _DEBUG
	Tracef("RECV LOVE POINT UPDATE : %d\n", kLovePointUpdate.byLovePoint);
#endif
	return true;
}

bool CPythonNetworkStream::RecvDigMotionPacket()
{
	TPacketGCDigMotion kDigMotion;
	if (!Recv(sizeof(kDigMotion), &kDigMotion))
		return false;

#ifdef _DEBUG
	Tracef(" Dig Motion [%d/%d]\n", kDigMotion.vid, kDigMotion.count);
#endif

	IAbstractCharacterManager& rkChrMgr=IAbstractCharacterManager::GetSingleton();
	CInstanceBase * pkInstMain = rkChrMgr.GetInstancePtr(kDigMotion.vid);
	CInstanceBase * pkInstTarget = rkChrMgr.GetInstancePtr(kDigMotion.target_vid);
	if (NULL == pkInstMain)
		return true;

	if (pkInstTarget)
		pkInstMain->NEW_LookAtDestInstance(*pkInstTarget);

	for (int i = 0; i < kDigMotion.count; ++i)
		pkInstMain->PushOnceMotion(CRaceMotionData::NAME_DIG);

	return true;
}


bool CPythonNetworkStream::SendDragonSoulRefinePacket(BYTE bRefineType, TItemPos* pos)
{
	TPacketCGDragonSoulRefine pk;
	pk.header = HEADER_CG_DRAGON_SOUL_REFINE;
	pk.bSubType = bRefineType;
	memcpy (pk.ItemGrid, pos, sizeof (TItemPos) * DS_REFINE_WINDOW_MAX_NUM);
	if (!Send(sizeof (pk), &pk))
	{
		return false;
	}
	return true;
}

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
bool CPythonNetworkStream::RecvAccePacket(bool bReturn)
{
	TPacketAcce sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return bReturn;

	bReturn = true;
	switch (sPacket.subheader)
	{
		case ACCE_SUBHEADER_GC_OPEN:
			CPythonAcce::Instance().Clear();
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 1, sPacket.bWindow));
			break;


		case ACCE_SUBHEADER_GC_CLOSE:
			CPythonAcce::Instance().Clear();
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 2, sPacket.bWindow));
			break;


		case ACCE_SUBHEADER_GC_ADDED:
			CPythonAcce::Instance().AddMaterial(sPacket.dwPrice, sPacket.bPos, sPacket.tPos);
			if (sPacket.bPos == 1)
			{
				CPythonAcce::Instance().AddResult(sPacket.dwItemVnum, sPacket.dwMinAbs, sPacket.dwMaxAbs);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AlertAcce", Py_BuildValue("(b)", sPacket.bWindow));
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 3, sPacket.bWindow));
			break;



		case ACCE_SUBHEADER_GC_REMOVED:
			CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, sPacket.bPos);
			if (sPacket.bPos == 0)
				CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, 1);

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 4, sPacket.bWindow));
			break;



		case ACCE_SUBHEADER_CG_REFINED:
			if (sPacket.dwMaxAbs == 0)
				CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, 1);
			else
			{
				CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, 0);
				CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, 1);
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 4, sPacket.bWindow));
			break;



		default:
			TraceError("CPythonNetworkStream::RecvAccePacket: unknown subheader %d\n.", sPacket.subheader);
			break;
	}

	return bReturn;
}

bool CPythonNetworkStream::SendAcceClosePacket()
{
	if (!__CanActMainInstance())
		return true;

	TItemPos tPos;
	tPos.window_type	= INVENTORY;
	tPos.cell			= 0;

	TPacketAcce sPacket;
	sPacket.header		= HEADER_CG_ACCE;
	sPacket.subheader	= ACCE_SUBHEADER_CG_CLOSE;
	sPacket.dwPrice		= 0;
	sPacket.bPos		= 0;
	sPacket.tPos		= tPos;
	sPacket.dwItemVnum	= 0;
	sPacket.dwMinAbs	= 0;
	sPacket.dwMaxAbs	= 0;

	if (!Send(sizeof(sPacket), &sPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendAcceAddPacket(TItemPos tPos, BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketAcce sPacket;
	sPacket.header		= HEADER_CG_ACCE;
	sPacket.subheader	= ACCE_SUBHEADER_CG_ADD;
	sPacket.dwPrice		= 0;
	sPacket.bPos		= bPos;
	sPacket.tPos		= tPos;
	sPacket.dwItemVnum	= 0;
	sPacket.dwMinAbs	= 0;
	sPacket.dwMaxAbs	= 0;

	if (!Send(sizeof(sPacket), &sPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendAcceRemovePacket(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;

	TPacketAcce sPacket;
	sPacket.header		= HEADER_CG_ACCE;
	sPacket.subheader	= ACCE_SUBHEADER_CG_REMOVE;
	sPacket.dwPrice		= 0;
	sPacket.bPos		= bPos;
	sPacket.tPos		= tPos;
	sPacket.dwItemVnum	= 0;
	sPacket.dwMinAbs	= 0;
	sPacket.dwMaxAbs	= 0;

	if (!Send(sizeof(sPacket), &sPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendAcceRefinePacket()
{
	if (!__CanActMainInstance())
		return true;

	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;

	TPacketAcce sPacket;
	sPacket.header		= HEADER_CG_ACCE;
	sPacket.subheader	= ACCE_SUBHEADER_CG_REFINE;
	sPacket.dwPrice		= 0;
	sPacket.bPos		= 0;
	sPacket.tPos		= tPos;
	sPacket.dwItemVnum	= 0;
	sPacket.dwMinAbs	= 0;
	sPacket.dwMaxAbs	= 0;

	if (!Send(sizeof(sPacket), &sPacket))
		return false;

	return SendSequence();
}
#endif





#ifdef ENABLE_SWITCHBOT
bool CPythonNetworkStream::RecvSwitchbotPacket()
{
	TPacketGCSwitchbot pack;
	if (!Recv(sizeof(pack), &pack))
	{
		return false;
	}

	size_t packet_size = int(pack.size) - sizeof(TPacketGCSwitchbot);
	if (pack.subheader == SUBHEADER_GC_SWITCHBOT_UPDATE)
	{
		if (packet_size != sizeof(CPythonSwitchbot::TSwitchbotTable))
		{
			return false;
		}

		CPythonSwitchbot::TSwitchbotTable table;
		if (!Recv(sizeof(table), &table))
		{
			return false;
		}

		CPythonSwitchbot::Instance().Update(table);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
	}
	else if (pack.subheader == SUBHEADER_GC_SWITCHBOT_UPDATE_ITEM)
	{
		if (packet_size != sizeof(TSwitchbotUpdateItem))
		{
			return false;
		}

		TSwitchbotUpdateItem update;
		if (!Recv(sizeof(update), &update))
		{
			return false;
		}

		TItemPos pos(SWITCHBOT, update.slot);

		IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();

		#ifdef ENABLE_ITEM_UPDATE_OPTIMIZATION
		rkPlayer.SetItemCount(pos, update.count, false);
		#else
		rkPlayer.SetItemCount(pos, update.count);
		#endif

		for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		{
			rkPlayer.SetItemMetinSocket(pos, i, update.alSockets[i]);

		}

		for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		{
			rkPlayer.SetItemAttribute(pos, j, update.aAttr[j].bType, update.aAttr[j].sValue);
		}

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotItem", Py_BuildValue("(i)", update.slot));
		return true;
	}
	else if (pack.subheader == SUBHEADER_GC_SWITCHBOT_SEND_ATTRIBUTE_INFORMATION)
	{
		CPythonSwitchbot::Instance().ClearAttributeMap();

		size_t table_size = sizeof(CPythonSwitchbot::TSwitchbottAttributeTable);
		while (packet_size >= table_size)
		{
			const int test = sizeof(CPythonSwitchbot::TSwitchbottAttributeTable);

			CPythonSwitchbot::TSwitchbottAttributeTable table;
			if (!Recv(table_size, &table))
			{
				return false;
			}

			CPythonSwitchbot::Instance().AddAttributeToMap(table);
			packet_size -= table_size;
		}
	}

	return true;
}
bool CPythonNetworkStream::SendSwitchbotStartPacket(BYTE slot, std::vector<CPythonSwitchbot::TSwitchbotAttributeAlternativeTable> alternatives)
{
	TPacketCGSwitchbot pack;
	pack.header = HEADER_CG_SWITCHBOT;
	pack.subheader = SUBHEADER_CG_SWITCHBOT_START;
	pack.size = sizeof(TPacketCGSwitchbot) + sizeof(CPythonSwitchbot::TSwitchbotAttributeAlternativeTable) * SWITCHBOT_ALTERNATIVE_COUNT;
	pack.slot = slot;

	if (!Send(sizeof(pack), &pack))
	{
		return false;
	}

	for (const auto& it : alternatives)
	{
		if (!Send(sizeof(it), &it))
		{
			return false;
		}
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendSwitchbotStopPacket(BYTE slot)
{
	TPacketCGSwitchbot pack;
	pack.header = HEADER_CG_SWITCHBOT;
	pack.subheader = SUBHEADER_CG_SWITCHBOT_STOP;
	pack.size = sizeof(TPacketCGSwitchbot);
	pack.slot = slot;

	if (!Send(sizeof(pack), &pack))
	{
		return false;
	}

	return SendSequence();
}
#endif

#ifdef ENABLE_HIDE_COSTUME_PART
bool CPythonNetworkStream::UpdateCostumeVisibleSettings()
{
	TPacketGCUpdateCostumeVisible pack;
	if (!Recv(sizeof(pack), &pack))
	{
		return false;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
		"BINARY_UpdateCostumeVisible",
		Py_BuildValue("(bbb)",
			pack.type[ECostumePartTypes::HIDE_COSTUME_HAIR],
			pack.type[ECostumePartTypes::HIDE_COSTUME_BODY],
			pack.type[ECostumePartTypes::HIDE_COSTUME_WEAPON])
			//pack.type[ECostumePartTypes::HIDE_COSTUME_SASH]
	);

	return true;
}
#endif

#ifdef ENABLE_FULL_YANG
bool CPythonNetworkStream::RecvGoldChange()
{
	TPacketGCGoldChange goldChange;

	if (!Recv(sizeof(TPacketGCGoldChange), &goldChange))
	{
		Tracen("Recv Gold Change Packet Error");
		return false;
	}

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (pInstance)
	{
		if (goldChange.dwVID == pInstance->GetVirtualID())
		{
			CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
			rkPlayer.SetGold(goldChange.ldValue);

			if (goldChange.ldAmount > 0)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickMoney", Py_BuildValue("(K)", goldChange.ldAmount));
			}
		}
	}
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshStatus", Py_BuildValue("()"));


	return true;
}
#endif

#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
bool CPythonNetworkStream::CubeRenewalMakeItem(int index_item, int count_item, int index_item_improve)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGCubeRenewalSend	packet;

	packet.header = HEADER_CG_CUBE_RENEWAL;
	packet.subheader = CUBE_RENEWAL_SUB_HEADER_MAKE_ITEM;
	packet.index_item = index_item;
	packet.count_item = count_item;
	packet.index_item_improve = index_item_improve;

	if (!Send(sizeof(TPacketCGCubeRenewalSend), &packet))
	{
		Tracef("CPythonNetworkStream::CubeRenewalMakeItem Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::CubeRenewalClose()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGCubeRenewalSend	packet;

	packet.header = HEADER_CG_CUBE_RENEWAL;
	packet.subheader = CUBE_RENEWAL_SUB_HEADER_CLOSE;

	if (!Send(sizeof(TPacketCGCubeRenewalSend), &packet))
	{
		Tracef("CPythonNetworkStream::CubeRenewalClose Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvCubeRenewalPacket()
{

	TPacketGCCubeRenewalReceive CubeRenewalPacket;

	if (!Recv(sizeof(CubeRenewalPacket), &CubeRenewalPacket))
		return false;

	switch (CubeRenewalPacket.subheader)
	{
		case CUBE_RENEWAL_SUB_HEADER_OPEN_RECEIVE:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CUBE_RENEWAL_OPEN", Py_BuildValue("()"));
		}
		break;

		case CUBE_RENEWAL_SUB_HEADER_DATES_RECEIVE:
		{
			CPythonCubeRenewal::Instance().ReceiveList(CubeRenewalPacket.date_cube_renewal);
		}
		break;

		case CUBE_RENEWAL_SUB_HEADER_DATES_LOADING:
		{
			CPythonCubeRenewal::Instance().LoadingList();
		}
		break;

		case CUBE_RENEWAL_SUB_HEADER_CLEAR_DATES_RECEIVE:
		{
			CPythonCubeRenewal::Instance().ClearList();
		}
		break;

		#ifdef CUBE_RENEWAL_RELOAD_FIX
		case CUBE_RENEWAL_SUB_HEADER_DATES_RELOAD:
		{
			CPythonCubeRenewal::Instance().ClearList();
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CUBE_RENEWAL_CLOSE", Py_BuildValue("()"));
		}
		break;
		#endif
	}

	return true;
}
#endif

#ifdef __RANKING_SYSTEM__
bool CPythonNetworkStream::RecvRankUpdate()
{
	TPacketRank p;
	if (!Recv(sizeof(TPacketRank), &p))
		return false;

	BYTE subIndex;
	if (!Recv(sizeof(BYTE), &subIndex))
		return false;

	BYTE categoryIndex;
	if (!Recv(sizeof(BYTE), &categoryIndex))
		return false;

	BYTE raceIndex;
	if (!Recv(sizeof(BYTE), &raceIndex))
		return false;

	BYTE rankIndex;
	if (!Recv(sizeof(BYTE), &rankIndex))
		return false;

	if (subIndex == RANK_LOAD)
	{
		BYTE listSize;
		if (!Recv(sizeof(BYTE), &listSize))
			return false;

		std::vector<TRanking> tmpVec;
		tmpVec.clear();
		tmpVec.resize(listSize);
		if (!Recv(sizeof(TRanking) * listSize, &tmpVec[0]))
			return false;

		CPythonPlayer::Instance().SetRankData(categoryIndex, raceIndex, rankIndex, tmpVec);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "LoadRankData", Py_BuildValue("(iiii)", categoryIndex, raceIndex, rankIndex, 1));
	}
	else
	{
		long long rankValue;
		if (!Recv(sizeof(long long), &rankValue))
			return false;

		CPythonPlayer::Instance().SetRankData(categoryIndex, raceIndex, rankIndex, rankValue);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "LoadRankUpdate", Py_BuildValue("(iii)", categoryIndex, raceIndex, rankIndex));
	}
	return true;
}
#endif

#ifdef ENABLE_EVENT_MANAGER
bool CPythonNetworkStream::RecvEventManager()
{
	TPacketGCEventManager p;
	if (!Recv(sizeof(TPacketGCEventManager), &p))
		return false;
	BYTE subIndex;
	if (!Recv(sizeof(BYTE), &subIndex))
		return false;
	if (subIndex == EVENT_MANAGER_LOAD)
	{
		BYTE dayCount;
		if (!Recv(sizeof(BYTE), &dayCount))
			return false;
		int serverTime;
		if (!Recv(sizeof(int), &serverTime))
			return false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ClearEventManager", Py_BuildValue("()"));
		for (BYTE j = 0; j < dayCount; ++j)
		{
			BYTE dayIndex;
			if (!Recv(sizeof(BYTE), &dayIndex))
				return false;
			BYTE dayEventCount;
			if (!Recv(sizeof(BYTE), &dayEventCount))
				return false;
			if (dayEventCount > 0)
			{
				std::vector<TEventManagerData> m_vec;
				m_vec.resize(dayEventCount);
				if (!Recv(dayEventCount * sizeof(TEventManagerData), &m_vec[0]))
					return false;
				for (const auto& eventPtr : m_vec)
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AppendEvent", Py_BuildValue("(iiissiiiiiiiii)", dayIndex, eventPtr.eventID, eventPtr.eventIndex, eventPtr.startTimeText, eventPtr.endTimeText, eventPtr.empireFlag, eventPtr.channelFlag, eventPtr.value[0], eventPtr.value[1], eventPtr.value[2], eventPtr.value[3], eventPtr.startTime - serverTime, eventPtr.endTime == 0 ? 0 : eventPtr.endTime - serverTime, eventPtr.eventStatus));
			}
		}
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshEventManager", Py_BuildValue("()"));
	}
	else if (subIndex == EVENT_MANAGER_EVENT_STATUS)
	{
		WORD eventID;
		if (!Recv(sizeof(WORD), &eventID))
			return false;
		bool eventStatus;
		if (!Recv(sizeof(bool), &eventStatus))
			return false;
		int endTime;
		if (!Recv(sizeof(int), &endTime))
			return false;
		char endTimeText[25];
		if (!Recv(sizeof(endTimeText), &endTimeText))
			return false;
		int serverTime;
		if (!Recv(sizeof(int), &serverTime))
			return false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshEventStatus", Py_BuildValue("(iiis)", eventID, eventStatus, endTime == 0 ? 0 : endTime - serverTime, endTimeText));
	}
	return true;
}
#endif

#ifdef ENABLE_SKILL_RENEWAL
bool CPythonNetworkStream::RecvSkillBookCountPacket()
{
	TPacketGCSkillBookCount p;
	if (!Recv(sizeof(TPacketGCSkillBookCount), &p))
	{
		Tracenf("Recv Skill Book Packet Error");
		return false;
	}

	CPythonPlayer& rkPlayer=CPythonPlayer::Instance();
	rkPlayer.SetSkillBookReadCount(p.readCount);
	rkPlayer.SetSkillBookLeftCount(p.leftCount);
	return true;
}
#endif


#ifdef ENABLE_DISCORD_RPC
#include "discord_rpc.h"
#if defined( _DEBUG ) || defined( DEBUG )
#define _RUNTIME_DEBUG				"_d"
#else
#define _RUNTIME_DEBUG				""
#endif
#pragma comment( lib, "EterDiscordLib" _RUNTIME_DEBUG ".lib" )
static int64_t StartTime;
static const auto DiscordClientID = DISCORD_APP_CLIENT_ID;
auto szSiteName = DISCORD_SITE_NAME;
auto szIconName = "image1";
void CPythonNetworkStream::Discord_Start()
{
	StartTime = time(0);
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	Discord_Initialize(DiscordClientID, &handlers, 1, nullptr);
	Discord_Update(true, false, false);
}

void CPythonNetworkStream::Discord_Update(const bool login, const bool select, const bool loading)
{
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	char szBufName[256 + 1];
	char szBufWarp[256 + 1];
	char szEmpireId[256 + 1];
	char szEmpireName[256 + 1];
	char szRaceName[256 + 1];
	std::string strName;
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (login)
	{
		discordPresence.details = "Giris Ekrani";
		discordPresence.state = szSiteName;
		discordPresence.largeImageKey = szIconName;
		discordPresence.largeImageText = szSiteName;
	}
	else if (select)
	{
		discordPresence.details = "Karakter secimi";
		discordPresence.state = szSiteName;
		discordPresence.largeImageKey = szIconName;
		discordPresence.largeImageText = szSiteName;
	}
	else if (loading)
	{
		discordPresence.details = "Yukleme Ekrani";
		discordPresence.state = szSiteName;
		discordPresence.largeImageKey = szIconName;
		discordPresence.largeImageText = szSiteName;
	}
	else
	{
		std::string GuildName;
		DWORD GuildID = CPythonPlayer::Instance().GetGuildID();
		CPythonGuild::Instance().GetGuildName(GuildID, &GuildName);

		if (GuildID != 0)
			_snprintf(szBufName, sizeof(szBufName), "%s (Lv. %d) - Lonca: %s", CPythonPlayer::Instance().GetName(), pInstance->GetLevel(), &GuildName);
		else
			_snprintf(szBufName, sizeof(szBufName), "%s (Lv. %d)", CPythonPlayer::Instance().GetName(), pInstance->GetLevel());

		_snprintf(szBufWarp, sizeof(szBufWarp), "Harita:%s", GetMapName());
		_snprintf(szEmpireId, sizeof(szEmpireId), "%s", GetEmpireIcon());
		_snprintf(szEmpireName, sizeof(szEmpireName), "Bayrak: %s", GetEmpireName());
		_snprintf(szRaceName, sizeof(szRaceName), "Karakter : %s", GetRaceName());

		if (pInstance->IsGameMaster())
		{
			discordPresence.details = GetStaffRank();
			discordPresence.state = szBufName;
		}
		else
		{
			discordPresence.details = szBufName;
			discordPresence.state = szBufWarp;
		}

		discordPresence.largeImageKey = GetRaceIcon();
		discordPresence.largeImageText = szRaceName;
		discordPresence.smallImageKey = GetEmpireIcon();
		discordPresence.smallImageText = szEmpireName;
	}

	discordPresence.startTimestamp = StartTime;
	Discord_UpdatePresence(&discordPresence);
}

void CPythonNetworkStream::Discord_Close()
{
	Discord_Shutdown();
}

const char* CPythonNetworkStream::GetStaffRank()
{
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (!pInstance)
		return "";

	if (pInstance->IsGameMaster())
		return "Oyun Yetkilisi";
	else
		return szSiteName;
}

const char* CPythonNetworkStream::GetRaceIcon()
{
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (!pInstance)
		return "";

	int raceId = pInstance->GetRace();

	if (raceId == 0)
		return "warrior_m";
	else if (raceId == 1)
		return "ninja_w";
	else if (raceId == 2)
		return "sura_m";
	else if (raceId == 3)
		return "shaman_w";
	else if (raceId == 4)
		return "warrior_w";
	else if (raceId == 5)
		return "ninja_m";
	else if (raceId == 6)
		return "sura_w";
	else if (raceId == 7)
		return "shaman_m";
	#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (raceId == 8)
		return "wolfman_m";
	#endif
	else
		return szIconName;
}

const char* CPythonNetworkStream::GetRaceName()
{
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (!pInstance)
		return "";

	int raceId = pInstance->GetRace();

	if (raceId == 0 || raceId == 4)
		return "Savaşçı";
	else if (raceId == 1 || raceId == 5)
		return "Ninja";
	else if (raceId == 2 || raceId == 6)
		return "Sura";
	else if (raceId == 3 || raceId == 7)
		return "Şaman";
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (raceId == 8)
		return "Lycan";
#endif
	else
		return szSiteName;
}

const char* CPythonNetworkStream::GetEmpireIcon()
{
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (!pInstance)
		return "";

	int empireId = pInstance->GetEmpireID();

	if (empireId == 1)
		return "empire_a";
	else if (empireId == 2)
		return "empire_b";
	else if (empireId == 3)
		return "empire_c";
	else
		return szIconName;
}

const char* CPythonNetworkStream::GetEmpireName()
{
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (!pInstance)
		return "";

	int empireId = pInstance->GetEmpireID();

	if (empireId == 1)
		return "Kirmizi (Red)";
	else if (empireId == 2)
		return "Sari (Yellow)";
	else if (empireId == 3)
		return "Mavi (Blue)";
	else
		return szSiteName;
}

const char* CPythonNetworkStream::GetMapName()
{
	auto WarpName = std::string(CPythonBackground::Instance().GetWarpMapName());

	static const std::map<std::string, std::string> DCmapname
	{
		{ "metin2_map_c1", "Ortak Koy - Pazar Alanı" },
		{ "kamafon_hiyus_desert_25", "Farm Haritası" },
		{ "kamafon_hiyus_desert_50", "Farm Haritası" },
		{ "kamafon_hiyus_desert", "Farm Haritası" },
		{ "kamafon_hiyus_lava_50", "Farm Haritası" },
		{ "kamafon_hiyus_lava_25", "Farm Haritası" },
		{ "kamafon_hiyus_lava", "Farm Haritası" },
		{ "kamafon_hiyus_natural_25", "Farm Haritası" },
		{ "kamafon_hiyus_natural_50", "Farm Haritası" },
		{ "kamafon_hiyus_natural", "Farm Haritası" },
		{ "kamafon_hiyus_valantinie_25", "Farm Haritası" },
		{ "kamafon_hiyus_valantinie_50", "Farm Haritası" },
		{ "kamafon_hiyus_valantinie", "Farm Haritası" },
		{ "kamafon_hiyus_valley", "Farm Haritası" },
		{ "kamafon_hiyus_valley_25", "Farm Haritası" },
		{ "kamafon_hiyus_valley_50", "Farm Haritası" },
		{ "kamafon_hiyus_cube", "Farm Haritası" },
		{ "kamafon_hiyus_boss", "Farm Haritası" },
	};

	auto MapName = (DCmapname.count(WarpName) ? DCmapname.at(WarpName) : WarpName);

	return MapName.c_str();
}
#endif

#ifdef ENABLE_EXCHANGE_LOG
bool CPythonNetworkStream::RecvExchangeLog()
{
	TPacketGCExchangeLog p;
	if (!Recv(sizeof(TPacketGCExchangeLog), &p))
		return false;

	BYTE subIndex;
	if (!Recv(sizeof(BYTE), &subIndex))
		return false;

	if (subIndex == SUB_EXCHANGELOG_LOAD)
	{
		char playerCode[19];
		if (!Recv(sizeof(playerCode), &playerCode))
			return false;

		bool isNeedClean;
		if (!Recv(sizeof(bool), &isNeedClean))
			return false;

		WORD logCount;
		if (!Recv(sizeof(WORD), &logCount))
			return false;

		if (isNeedClean)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ExchangeLogClear", Py_BuildValue("(s)", playerCode));

		for (DWORD j = 0; j < logCount; ++j)
		{
			DWORD logID;
			if (!Recv(sizeof(DWORD), &logID))
				return false;

			TExchangeLog logData;
			if (!Recv(sizeof(TExchangeLog), &logData))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ExchangeLogAppend", Py_BuildValue("(isLssLss)", logID, logData.owner, logData.ownerGold, logData.ownerIP, logData.target, logData.targetGold, logData.targetIP, logData.date));
		}

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ExchangeLogRefresh", Py_BuildValue("(i)", 0));
	}
	else if (subIndex == SUB_EXCHANGELOG_LOAD_ITEM)
	{
		WORD logItemCount;
		if (!Recv(sizeof(WORD), &logItemCount))
			return false;

		DWORD logID;
		if (!Recv(sizeof(DWORD), &logID))
			return false;

		if (logItemCount)
		{
			std::vector<TExchangeLogItem> m_vecLogItems;
			m_vecLogItems.resize(logItemCount);

			if (!Recv(logItemCount * sizeof(TExchangeLogItem), m_vecLogItems.data()))
				return false;

			for (DWORD j = 0; j < m_vecLogItems.size(); ++j)
			{
				const TExchangeLogItem& logItem = m_vecLogItems[j];
				PyObject* poSockets = PyList_New(0);
				for(DWORD x=0;x<ITEM_SOCKET_SLOT_MAX_NUM;++x)
					PyList_Append(poSockets, PyInt_FromLong(logItem.alSockets[x]));

				PyObject* poAttrType = PyList_New(0);
				PyObject* poAttrValue = PyList_New(0);
				for (DWORD x = 0; x < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++x)
				{
					PyList_Append(poAttrType, PyInt_FromLong(logItem.aAttr[x].bType));
					PyList_Append(poAttrValue, PyInt_FromLong(logItem.aAttr[x].sValue));
				}

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ExchangeLogItemAppend", Py_BuildValue("(iiiiOOOi)", logID, logItem.pos, logItem.vnum, logItem.count, poSockets, poAttrType, poAttrValue, logItem.isOwnerItem));

				Py_DECREF(poSockets);
				Py_DECREF(poAttrType);
				Py_DECREF(poAttrValue);
			}
		}

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ExchangeLogRefresh", Py_BuildValue("(i)", logID));
	}

	return true;
}
#endif

#ifdef ENABLE_DRAGON_COIN_SHOP_SYSTEM
bool CPythonNetworkStream::SendItemShopPacket(BYTE subHeader, BYTE itemCount, DWORD itemID, BYTE itemPriceType, char* szCategoryStr)
{
	TCGItemShopProcess gPacket;
	gPacket.bHeader = HEADER_CG_ITEMSHOP_PROCESS;
	gPacket.bSubHeader = subHeader;
	gPacket.itemCount = itemCount;
	gPacket.itemID = itemID;
	gPacket.itemPriceType = itemPriceType;
	snprintf(gPacket.szCategoryStr, sizeof(gPacket.szCategoryStr), szCategoryStr);

	if (!Send(sizeof(gPacket), &gPacket))
	{
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvItemShopGeneralPacket()
{
	TGCItemShopSize gPacket;
	if (!Recv(sizeof(gPacket), &gPacket))
	{
		return false;
	}

	BYTE bSubHeader = gPacket.bSubHeader;
	gPacket.dataSize -= sizeof(gPacket);

	switch(bSubHeader)
	{
		case DC_SUB_HEADER_CATEGORYS:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopClear", Py_BuildValue("()"));

			while (gPacket.dataSize > 0)
			{
				TGCIShopCategory catData;
				if (!Recv(sizeof(catData), &catData)) { return false; }
				gPacket.dataSize -= sizeof(catData);

				if (catData.mainID == 255)
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopAddCategory", Py_BuildValue("(si)", catData.categoryName, catData.catID));
				else
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopFillCategory", Py_BuildValue("(isi)", catData.mainID, catData.categoryName, catData.catID));
			}
		}
		break;

		case DC_SUB_HEADER_ITEMS:
		{
			while (gPacket.dataSize > 0)
			{
				TGCIShopItem itemData;
				if (!Recv(sizeof(itemData), &itemData)) { return false; }
				gPacket.dataSize -= sizeof(itemData);

				char szMetinStr[48], szAttrStr[48];
				snprintf(szMetinStr, sizeof(szMetinStr), 
					"%d|%d|%d|%d"
					, itemData.alSockets[0]
					, itemData.alSockets[1]
					, itemData.alSockets[2]
					, itemData.alSockets[3]
				);

				snprintf(szAttrStr, sizeof(szAttrStr), 
					"%d|%d`%d|%d`%d|%d`%d|%d`%d|%d`%d|%d`%d|%d"
					, itemData.aAttr[0].bType, itemData.aAttr[0].sValue
					, itemData.aAttr[1].bType, itemData.aAttr[1].sValue
					, itemData.aAttr[2].bType, itemData.aAttr[2].sValue
					, itemData.aAttr[3].bType, itemData.aAttr[3].sValue
					, itemData.aAttr[4].bType, itemData.aAttr[4].sValue
					, itemData.aAttr[5].bType, itemData.aAttr[5].sValue
					, itemData.aAttr[6].bType, itemData.aAttr[6].sValue
				);

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopAppendItem", Py_BuildValue(
					"(iiiiiiiiiiiiss)"
					, itemData.categoryID
					, itemData.iID
					, itemData.iVnum
					, itemData.iCount
					, itemData.iPrice
					, itemData.iDiscount
					, itemData.bTimeType
					, itemData.iStock
					, itemData.wSellCount
					, itemData.bSellType
					, itemData.dwAddTime
					, itemData.dwOfferTime
					, szMetinStr
					, szAttrStr
				));
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopOpen", Py_BuildValue("()"));
		}
		break;

		#ifdef ENABLE_DRAGON_COIN_EM_MARKET
		case DC_SUB_HEADER_EM_CATEGORYS:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopEmClear", Py_BuildValue("()"));

			while (gPacket.dataSize > 0)
			{
				TGCIShopCategory catData;
				if (!Recv(sizeof(catData), &catData)) { return false; }
				gPacket.dataSize -= sizeof(catData);

				if (catData.mainID == 255)
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopEmAddCategory", Py_BuildValue("(si)", catData.categoryName, catData.catID));
				else
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopEmFillCategory", Py_BuildValue("(isi)", catData.mainID, catData.categoryName, catData.catID));
			}
		}
		break;

		case DC_SUB_HEADER_EM_ITEMS:
		{
			while (gPacket.dataSize > 0)
			{
				TGCIShopItem itemData;
				if (!Recv(sizeof(itemData), &itemData)) { return false; }
				gPacket.dataSize -= sizeof(itemData);

				char szMetinStr[48], szAttrStr[48];
				snprintf(szMetinStr, sizeof(szMetinStr), 
					"%d|%d|%d|%d"
					, itemData.alSockets[0]
					, itemData.alSockets[1]
					, itemData.alSockets[2]
					, itemData.alSockets[3]
				);

				snprintf(szAttrStr, sizeof(szAttrStr), 
					"%d|%d`%d|%d`%d|%d`%d|%d`%d|%d`%d|%d`%d|%d"
					, itemData.aAttr[0].bType, itemData.aAttr[0].sValue
					, itemData.aAttr[1].bType, itemData.aAttr[1].sValue
					, itemData.aAttr[2].bType, itemData.aAttr[2].sValue
					, itemData.aAttr[3].bType, itemData.aAttr[3].sValue
					, itemData.aAttr[4].bType, itemData.aAttr[4].sValue
					, itemData.aAttr[5].bType, itemData.aAttr[5].sValue
					, itemData.aAttr[6].bType, itemData.aAttr[6].sValue
				);

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopEmAppendItem", Py_BuildValue(
					"(iiiiiiiiiiiiss)"
					, itemData.categoryID
					, itemData.iID
					, itemData.iVnum
					, itemData.iCount
					, itemData.iPrice
					, itemData.iDiscount
					, itemData.bTimeType
					, itemData.iStock
					, itemData.wSellCount
					, itemData.bSellType
					, itemData.dwAddTime
					, itemData.dwOfferTime
					, szMetinStr
					, szAttrStr
				));
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopEmOpen", Py_BuildValue("()"));
		}
		break;
		#endif

		case DC_SUB_HEADER_LOGS:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopHideLoading", Py_BuildValue("()"));
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopLogClear", Py_BuildValue("()"));

			while (gPacket.dataSize > 0)
			{
				TDcShopLog logData;
				if (!Recv(sizeof(logData), &logData)) { return false; }
				gPacket.dataSize -= sizeof(logData);

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopAppendLog", Py_BuildValue(
					"(sissiiKi)"
					, logData.buyDate
					, logData.buyTime
					, logData.playerName
					, logData.ipAdress
					, logData.itemVnum
					, logData.itemCount
					, logData.itemPrice
					, logData.itemPriceType
				));
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopPurchasesWindow", Py_BuildValue("()"));
		}
		break;

		case DC_SUB_HEADER_POPUP_MESSAGE:
		{
			// receiving string
			TItemShopPopupMessage pack;
			if (!Recv(sizeof(pack), &pack))
				return false;

			// safely terminating string
			pack.localeString[pack.MESSAGE_LEN-1] = 0;
			const std::string& message = pack.localeString;
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopPopupMessage", Py_BuildValue("(si)", message.c_str(), pack.iData));
		}
		break;
	}

	return true;
}
#endif

#ifdef ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM
bool CPythonNetworkStream::RecvItemShopGeneralAdminPacket()
{
	TGCItemShopSize gPacket;
	if (!Recv(sizeof(gPacket), &gPacket))
	{
		return false;
	}

	BYTE bSubHeader = gPacket.bSubHeader;
	gPacket.dataSize -= sizeof(gPacket);

	switch(bSubHeader)
	{
		case DC_SUB_HEADER_ADMIN_CATEGORYS:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopAdminClear", Py_BuildValue("()"));

			while (gPacket.dataSize > 0)
			{
				TGCIShopCategory catData;
				if (!Recv(sizeof(catData), &catData)) { return false; }
				gPacket.dataSize -= sizeof(catData);

				if (catData.mainID == 255)
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopAddCategoryAdmin", Py_BuildValue("(si)", catData.categoryName, catData.catID));
				else
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopFillCategoryAdmin", Py_BuildValue("(isi)", catData.mainID, catData.categoryName, catData.catID));
			}
		}
		break;

		case DC_SUB_HEADER_ADMIN_ITEMS:
		{
			while (gPacket.dataSize > 0)
			{
				TGCIShopItemAdmin itemData;
				if (!Recv(sizeof(itemData), &itemData)) { return false; }
				gPacket.dataSize -= sizeof(itemData);

				char szMetinStr[48], szAttrStr[48];
				snprintf(szMetinStr, sizeof(szMetinStr), 
					"%d|%d|%d|%d"
					, itemData.alSockets[0]
					, itemData.alSockets[1]
					, itemData.alSockets[2]
					, itemData.alSockets[3]
				);

				snprintf(szAttrStr, sizeof(szAttrStr), 
					"%d|%d`%d|%d`%d|%d`%d|%d`%d|%d`%d|%d`%d|%d"
					, itemData.aAttr[0].bType, itemData.aAttr[0].sValue
					, itemData.aAttr[1].bType, itemData.aAttr[1].sValue
					, itemData.aAttr[2].bType, itemData.aAttr[2].sValue
					, itemData.aAttr[3].bType, itemData.aAttr[3].sValue
					, itemData.aAttr[4].bType, itemData.aAttr[4].sValue
					, itemData.aAttr[5].bType, itemData.aAttr[5].sValue
					, itemData.aAttr[6].bType, itemData.aAttr[6].sValue
				);

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopAppendItemAdmin", Py_BuildValue(
					"(iiiiiiiiiiiiissss)"
					, itemData.categoryID
					, itemData.iID
					, itemData.iVnum
					, itemData.iCount
					, itemData.iPrice
					, itemData.iDiscount
					, itemData.bTimeType
					, itemData.bStatus
					, itemData.iStock
					, itemData.wSellCount
					, itemData.bSellType
					, itemData.dwAddTime
					, itemData.dwOfferTime
					, itemData.szAddTime
					, itemData.szOfferTime
					, szMetinStr
					, szAttrStr
				));
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopOpenAdmin", Py_BuildValue("()"));
		}
		break;
	}

	return true;
}
#endif

#ifdef ENABLE_PM_ALL_SEND_SYSTEM
bool CPythonNetworkStream::SendBulkWhisperPacket(const char * c_szContent)
{
	if (strlen(c_szContent) == 0)
		return true;

	if (strlen(c_szContent) >= 512)
		return true;

	int iTextLen = strlen(c_szContent) + 1;
	TPacketCGBulkWhisper WhisperPacket;
	WhisperPacket.header = HEADER_CG_BULK_WHISPER;
	strncpy(WhisperPacket.szText, c_szContent, sizeof(WhisperPacket.szText) - 1);

	if (!Send(sizeof(WhisperPacket), &WhisperPacket))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvBulkWhisperPacket()
{
	TPacketGCBulkWhisper kWhisper;
	char buf[1024 + 1];

	if (!Recv(sizeof(kWhisper), &kWhisper))
		return false;

	UINT uWhisperSize = kWhisper.size - sizeof(kWhisper);

	if (!Recv(uWhisperSize, buf))
		return false;

	buf[uWhisperSize] = '\0';

	TraceError("CPythonNetworkStream::RecvBulkWhisperPacket - %s", buf);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OnRecvBulkWhisper", Py_BuildValue("(s)", buf));
	return true;
}
#endif

#ifdef ENABLE_BAKIM_DUYURU
bool CPythonNetworkStream::RecvMaintenanceInfo()
{
	TPacketGCMaintenanceInfo pack;
	if (!Recv(sizeof(pack), &pack))
	{
		return false;
	}

	std::vector<char> cause(pack.cause_len);
	if (!Recv(pack.cause_len, &cause[0]))
	{
		return false;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Maintenance", Py_BuildValue("(isi)", pack.remaining_time, &cause[0], pack.duration));

	return true;
}
#endif


#ifdef ENABLE_ENTITY_PRELOADING
bool CPythonNetworkStream::RecvPreloadEntitiesPacket()
{
	TPacketGCPreloadEntities pack{};
	if (!Recv(sizeof(pack), &pack))
		return false;

	assert(int32_t(pack.size) - sizeof(pack) == pack.count * sizeof(uint32_t) && "HEADER_GC_PRELOAD_ENTITIES");

	uint32_t dwEntityRace = 0;
	CInstanceBase::SCreateData d{};
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

	for (uint16_t i = 0; i < pack.count; i++)
	{
		if (!Recv(sizeof(uint32_t), &dwEntityRace))
			return false;

#ifdef _DEBUG
		TraceError("Preloading %d", dwEntityRace);
#endif
		d.m_dwRace = dwEntityRace;
		d.m_dwVID = dwEntityRace;

		if (rkChrMgr.CreateInstance(d))//create the entity, force it to load absolutely everything
			rkChrMgr.DeleteInstance(dwEntityRace);// now get rid of it
		else
			TraceError("Failed to preload race %d", dwEntityRace);
	}

	return true;
}
#endif
#ifdef SPIN_WHEEL
bool CPythonNetworkStream::RecvSpinWheelPacket()
{
	TPacketGCSpinWheel kSpinWheelPacket;
	if (!Recv(sizeof(kSpinWheelPacket), &kSpinWheelPacket))
	{
		TraceError("Err packet take");
		return false;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ClearSpinWheel", Py_BuildValue("()"));

	for (int i = 0; i < spinWheelMaxItem; i++)
	{
		if (kSpinWheelPacket.itemVnums[i] == 0)
			break;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddSpinWheelResult", Py_BuildValue("(iii)", kSpinWheelPacket.itemVnums[i], kSpinWheelPacket.itemCounts[i], kSpinWheelPacket.itemRarities[i]));
	}
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ShuffleList", Py_BuildValue("()"));
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_StartSpinWheel", Py_BuildValue("(i)", kSpinWheelPacket.dwRewardItem));

	return true;
}

bool CPythonNetworkStream::RecvSpinWheelRewardsPacket()
{
	TPacketGCSpinWheelRewards kSpinWheelRewardsPacket;

	if (!Recv(sizeof(kSpinWheelRewardsPacket), &kSpinWheelRewardsPacket))
	{
		TraceError("Err packet take");
		return false;
	}

	for (int i = 0; i < spinWheelMaxItem; i++)
	{
		if (kSpinWheelRewardsPacket.itemVnums[i] == 0)
			break;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddSpinWheelReward", Py_BuildValue("(iiii)", kSpinWheelRewardsPacket.bIndex, kSpinWheelRewardsPacket.itemVnums[i], kSpinWheelRewardsPacket.itemCounts[i], kSpinWheelRewardsPacket.itemRarities[i]));
	}

	return true;
}
#endif



#ifdef ENABLE_OFFER_SYSTEM
#define OS_ITEM_SOCKET_MAX_NUM 4
#define OS_ITEM_ATTRIBUTE_MAX_NUM 7
bool CPythonNetworkStream::RecvOfferListPacket()
{
	typedef struct SOfferInfo
	{
		uint32_t	offerID;
		LONG		offerStartRemainSec;
		LONG		offerEndRemainSec;
		DWORD		offerOldPrice;
		DWORD		offerNewPrice;
	} TOfferInfo;

	typedef struct SOfferItemInfo
	{
		DWORD	itemID;
		DWORD	itemOfferID;
		BYTE	itemPos;
		DWORD	itemCount;
		DWORD	itemVnum;
		LONG	alSockets[OS_ITEM_SOCKET_MAX_NUM];
		TPlayerItemAttribute    aAttr[OS_ITEM_ATTRIBUTE_MAX_NUM];
		BYTE	timeType;
		BYTE	timeSocket;
	} TOfferItemInfo;

	TGCOfferListSize gPacket;
	if (!Recv(sizeof(gPacket), &gPacket))
	{
		return false;
	}

	gPacket.dataSize -= sizeof(gPacket);

	if (gPacket.dataType == 0)
	{
		while (gPacket.dataSize > 0)
		{
			TOfferInfo oInfo;
			if (!Recv(sizeof(oInfo), &oInfo))
			{
				return false;
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddToOfferList", Py_BuildValue("(iiiii)",
				oInfo.offerID, oInfo.offerStartRemainSec, oInfo.offerEndRemainSec, oInfo.offerOldPrice, oInfo.offerNewPrice));

			gPacket.dataSize -= sizeof(oInfo);
		}
	}

	if (gPacket.dataType == 1)
	{
		while (gPacket.dataSize > 0)
		{
			TOfferItemInfo oInfo;
			if (!Recv(sizeof(oInfo), &oInfo))
			{
				return false;
			}

			char szMetinStr[48], szAttrStr[48];
			for (BYTE i = 0; i < OS_ITEM_SOCKET_MAX_NUM; i++)
			{
				if (i == 0)
				{
					snprintf(szMetinStr, sizeof(szMetinStr), "%d", oInfo.alSockets[i]);
				}
				else
				{
					snprintf(szMetinStr, sizeof(szMetinStr), "%s|%d", szMetinStr, oInfo.alSockets[i]);
				}
			}

			for (BYTE j = 0; j < OS_ITEM_ATTRIBUTE_MAX_NUM; j++)
			{
				if (j == 0)
				{
					snprintf(szAttrStr, sizeof(szAttrStr), "%d|%d", oInfo.aAttr[j].bType, oInfo.aAttr[j].sValue);
				}
				else
				{
					snprintf(szAttrStr, sizeof(szAttrStr), "%s`%d|%d", szAttrStr, oInfo.aAttr[j].bType, oInfo.aAttr[j].sValue);
				}
			}
	
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddToOfferItemList", Py_BuildValue("(iiiiiss)",
				oInfo.itemID, oInfo.itemOfferID, oInfo.itemPos, oInfo.itemCount, oInfo.itemVnum, szMetinStr, szAttrStr));

			gPacket.dataSize -= sizeof(oInfo);
		}
	}
	return true;
}
#endif

#ifdef ENABLE_WHEEL_OF_FORTUNE
bool CPythonNetworkStream::RecvWheelOfFortunePacket()
{
	TPacketGCWheelOfFortune gPacket;
	if (!Recv(sizeof(gPacket), &gPacket)) { return false; }
	BYTE bSubHeader = gPacket.bSubHeader;
	gPacket.wSize -= sizeof(gPacket);

	switch(bSubHeader)
	{
		case WHEEL_TYPE_EP:
		case WHEEL_TYPE_MP:
		case WHEEL_TYPE_BRONZE_COIN:
		case WHEEL_TYPE_SILVER_COIN:
		case WHEEL_TYPE_GOLD_COIN:
		case WHEEL_TYPE_EMERALD_COIN:
		case WHEEL_TYPE_BRONZE_YANG:
		case WHEEL_TYPE_SILVER_YANG:
		case WHEEL_TYPE_GOLD_YANG:
		case WHEEL_TYPE_EMERALD_YANG:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_WofItemClear", Py_BuildValue("()"));

			while (gPacket.wSize > 0)
			{
				TWheelOfFortuneInfoTable iPacket;
				if (!Recv(sizeof(iPacket), &iPacket)) { return false; }
				gPacket.wSize -= sizeof(iPacket);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_WofItemAdd", Py_BuildValue("(ii)", iPacket.iVnum, iPacket.iCount));
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_WofItemOpen", Py_BuildValue("(iii)", bSubHeader, gPacket.iNeedVnum, gPacket.iNeedCount));
		}
		break;

		default:
			break;
	}

	return true;
}
#endif

#ifdef ENABLE_TANAKA_EVENT
bool CPythonNetworkStream::RecvTanakaEventPacket()
{
	TDynamicPacketInfo p;
	if (!Recv(sizeof(p), &p))
	{
		Tracenf("RecvTanakaEventPacket Packet Error");
		return false;
	}

	p.wSize -= sizeof(p);
	unsigned int iPacketSize = p.wSize;

	switch (p.bSubHeader)
	{
		case TANAKA_EVENT_SUBHEADER_GC_OPEN_UI:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenTanakaEventUI", Py_BuildValue("()"));
		}
		break;

		case TANAKA_EVENT_SUBHEADER_GC_UPDATE:
		{
			p.wSize -= sizeof(TTanakaGCUpdate);
			TTanakaGCUpdate info;
			if (!Recv(sizeof(TTanakaGCUpdate), &info)) { return false; }
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_TanakaEventUpdate", Py_BuildValue("(bbb)", info.eventLevel, info.tanakaCount, info.leftTanakaCount));
		}
		break;

		case TANAKA_EVENT_SUBHEADER_GC_UPDATE_PLAYER_INFO:
		{
			p.wSize -= sizeof(TTanakaGCPlayerInfo);
			TTanakaGCPlayerInfo playerInfo;
			if (!Recv(sizeof(TTanakaGCPlayerInfo), &playerInfo)) { return false; }
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_TanakaEventUpdatePlayerInfo", Py_BuildValue("(k)", playerInfo.totalDamage));
		}
		break;

		case TANAKA_EVENT_SUBHEADER_GC_UPDATE_ITEM_INFO:
		{
			p.wSize -= sizeof(TTanakaGCUpdateItemInfo);
			TTanakaGCUpdateItemInfo info;
			if (!Recv(sizeof(TTanakaGCUpdateItemInfo), &info)) { return false; }
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_TanakaEventUpdateItemInfo", Py_BuildValue("(sii)", info.szName, info.vnum, info.count));
		}
		break;

		default: break;
	}

	return true;
}
#endif

#ifdef ENABLE_BATTLE_PASS
bool CPythonNetworkStream::SendBattlePassAction(BYTE bAction, int32_t iArg)
{
	if (!__CanActMainInstance()) return true;
	TPacketCGBattlePassAction packet;
	packet.bHeader = HEADER_CG_BATTLE_PASS;
	packet.bAction = bAction;
	packet.iArg = iArg;
	if (!Send(sizeof(TPacketCGBattlePassAction), &packet))
	{
		Tracef("SendBattlePassAction Send Packet Error\n");
		return false;
	}
	return true;
}

bool CPythonNetworkStream::RecvBattlePassPacket()
{
	TPacketGCBattlePass packet;
	if (!Recv(sizeof(packet), &packet)) { return false; }

	packet.wSize -= sizeof(packet);

	while (packet.wSize > 0)
	{
		TBattlePassMissionInfo missionInfo;
		if (!Recv(sizeof(missionInfo), &missionInfo)) { return false; }

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
			"BINARY_BattlePassAddMission", Py_BuildValue("(iiiiis)"
			, missionInfo.bMissionID
			, missionInfo.bMissionType
			, missionInfo.dwMissionInfo[0]
			, missionInfo.dwMissionInfo[1]
			, missionInfo.dwMissionInfo[2]
			, missionInfo.szMissionName
		));

		for (int i = 0; i < 3; ++i)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
				"BINARY_BattlePassAddMissionReward", Py_BuildValue("(iiii)"
				, missionInfo.bMissionID
				, missionInfo.aRewardList[i].bRewardType
				, missionInfo.aRewardList[i].dwVnum
				, missionInfo.aRewardList[i].wCount
			));
		}

		packet.wSize -= sizeof(missionInfo);
	}

	while (packet.wRewardSize > 0)
	{
		TBattlePassRewardItem rewardInfo;
		if (!Recv(sizeof(rewardInfo), &rewardInfo)) { return false; }

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
			"BINARY_BattlePassAddReward", Py_BuildValue("(iii)"
			, rewardInfo.bRewardType
			, rewardInfo.dwVnum
			, rewardInfo.wCount
		));

		packet.wRewardSize -= sizeof(rewardInfo);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_BattlePassOpen", Py_BuildValue("(s)", packet.battlePassName));
	return true;
}

bool CPythonNetworkStream::RecvBattlePassUpdatePacket()
{
	TPacketGCBattlePassUpdate packet;
	if (!Recv(sizeof(packet), &packet)) return false;
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_BattlePassUpdate", Py_BuildValue("(ii)", packet.bMissionID, packet.dwNewProgress));
	return true;
}
#endif

#ifdef ENABLE_TELEPORTION_WIZARD_SYSTEM
bool CPythonNetworkStream::RecvTpWizardInfoPacket()
{
	TPacketGCTpWizardInfo gPacket;
	if (!Recv(sizeof(gPacket), &gPacket)) { return false; }
	BYTE bSubHeader = gPacket.bSubHeader;
	gPacket.wSize -= sizeof(gPacket);

	switch(bSubHeader)
	{
		case TP_WIZARD_DATA_INFO:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ClearTeleportWizardInfo", Py_BuildValue("()"));

			while (gPacket.wSize > 0)
			{
				TWizardDataInfoTable catData;
				if (!Recv(sizeof(catData), &catData)) { return false; }
				gPacket.wSize -= sizeof(catData);
				if (catData.show <= 0) { continue; }

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_UpdateTeleportWizardInfo", Py_BuildValue(
					"(iisiiiiiiiiii"
					#ifdef ENABLE_FULL_YANG
					"K"
					#else
					"i"
					#endif
					")"
					, catData.id
					, catData.type
					, catData.comment
					, catData.min_level
					, catData.max_level
					, catData.alignment
					, catData.item_vnum
					, catData.item_count
					, catData.image_num
					, catData.race_num
					, catData.element
					, catData.cooldown
					, catData.status
					, catData.gold
				));
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenTeleportWizardInfo", Py_BuildValue("()"));
		}
		break;

		case TP_WIZARD_DROP_INFO:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ClearTpWizardDropInfo", Py_BuildValue("()"));

			while (gPacket.wSize > 0)
			{
				TWizardDropInfoTable itemData;
				if (!Recv(sizeof(itemData), &itemData)) { return false; }
				gPacket.wSize -= sizeof(itemData);

				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddTpWizardDropInfo", Py_BuildValue(
					"(iiiii)"
					, gPacket.dwRaceNum
					, itemData.bPageIndex
					, itemData.bSlotIndex
					, itemData.dwItemVnum
					, itemData.iItemCount
				));
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RefreshTpWizardDropInfo", Py_BuildValue("(i)", gPacket.dwRaceNum));
		}
		break;
	}

	return true;
}
#endif

#ifdef ENABLE_CHANGELOOK_SYSTEM
bool CPythonNetworkStream::RecvChangeLookPacket()
{
	TPacketChangeLook sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return false;

	switch (sPacket.subheader)
	{
		case CL_SUBHEADER_OPEN:
			{
				CPythonChangeLook::Instance().Clear();
				CPythonChangeLook::Instance().SetCost(sPacket.dwCost);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 1));
			}
			break;

		case CL_SUBHEADER_CLOSE:
			{
				CPythonChangeLook::Instance().Clear();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 2));
			}
			break;

		case CL_SUBHEADER_ADD:
			{
				CPythonChangeLook::Instance().AddMaterial(sPacket.bPos, sPacket.tPos);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 3));
				if (sPacket.bPos == 1)
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AlertChangeLook", Py_BuildValue("()"));
			}
			break;

		case CL_SUBHEADER_REMOVE:
			{
				if (sPacket.bPos == 1 || sPacket.bPos == 2)
					CPythonChangeLook::Instance().RemoveMaterial(sPacket.bPos);
				else
					CPythonChangeLook::Instance().RemoveAllMaterials();
				
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 4));
			}
			break;

		case CL_SUBHEADER_REFINE:
			{
				CPythonChangeLook::Instance().RemoveAllMaterials();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 4));
			}
			break;

		default:
			TraceError("CPythonNetworkStream::RecvChangeLookPacket: unknown subheader %d\n.", sPacket.subheader);
			break;
	}

	return true;
}

bool CPythonNetworkStream::SendClClosePacket()
{
	if (!__CanActMainInstance())
		return true;

	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;

	TPacketChangeLook sPacket;
	sPacket.header = HEADER_CG_CL;
	sPacket.subheader = CL_SUBHEADER_CLOSE;
	sPacket.dwCost = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendClAddPacket(TItemPos tPos, BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketChangeLook sPacket;
	sPacket.header = HEADER_CG_CL;
	sPacket.subheader = CL_SUBHEADER_ADD;
	sPacket.dwCost = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendClRemovePacket(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;

	TPacketChangeLook sPacket;
	sPacket.header = HEADER_CG_CL;
	sPacket.subheader = CL_SUBHEADER_REMOVE;
	sPacket.dwCost = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;

	if (!Send(sizeof(sPacket), &sPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendClRefinePacket()
{
	if (!__CanActMainInstance())
		return true;

	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;

	TPacketChangeLook sPacket;
	sPacket.header = HEADER_CG_CL;
	sPacket.subheader = CL_SUBHEADER_REFINE;
	sPacket.dwCost = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;

	return SendSequence();
}
#endif

#ifdef BOSS_DMG_RANKING
bool CPythonNetworkStream::RecvBossRankingPacket()
{
	TPacketGCDamageRanking rPacket;
	if (!Recv(sizeof(rPacket), &rPacket)) { return false; }

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BR_ClearPlayerInfo", Py_BuildValue("()"));

	bool isRefresh = false;
	if (rPacket.fPercents[0] == (-5.0f))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BR_CloseWindow", Py_BuildValue("()"));
		goto End;
	}

	for (BYTE i = 0; i < MAX_PLAYERS_IN_TABLE; i++)
	{
		if (rPacket.bLevels[i] == 0) { continue; }
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BR_AddPlayer", Py_BuildValue("(siiiKf)", rPacket.szNames[i], rPacket.bLevels[i], rPacket.bJobs[i], rPacket.bEmpire[i], rPacket.lDamages[i], rPacket.fPercents[i]));
		isRefresh = true;
	}

	if (isRefresh)
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BR_Refresh", Py_BuildValue("()"));
End:;
	return true;
}
#endif

#ifdef ENABLE_BLACK_MARKET_SYSTEM
bool CPythonNetworkStream::RecvBlackMarketPacket()
{
	TPacketGCItemBlackMarket gPacket;
	if (!Recv(sizeof(gPacket), &gPacket)) { return false; }
	gPacket.wSize -= sizeof(gPacket);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "blackmarket_clear", Py_BuildValue("()"));

	while (gPacket.wSize > 0)
	{
		TBlackMarketData infoData;
		if (!Recv(sizeof(infoData), &infoData)) { return false; }
		gPacket.wSize -= sizeof(infoData);

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "blackmarket_insert", Py_BuildValue(
			"(i"
			#ifdef ENABLE_FULL_YANG
			"K"
			#else
			"i"
			#endif
			"i)"
			, infoData.dwVnum
			, infoData.llPrice
			, infoData.iStock
			)
		);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "blackmarket_open", Py_BuildValue("()"));
	return true;
}
#endif

#ifdef ENABLE_REWARD_SYSTEM
bool CPythonNetworkStream::RecvRewardInfoPacket()
{
	TPacketGCRewardMain gPacket;
	if (!Recv(sizeof(gPacket), &gPacket)) { return false; }
	gPacket.wSize -= sizeof(gPacket);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "reward_clear", Py_BuildValue("()"));

	while (gPacket.wSize > 0)
	{
		TPacketRewardInfo infoData;
		if (!Recv(sizeof(infoData), &infoData)) { return false; }
		gPacket.wSize -= sizeof(infoData);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "reward_insert", Py_BuildValue("(is)", infoData.deskIndex, infoData.playerName));
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "reward_update", Py_BuildValue("()"));
	return true;
}
#endif

#ifdef ENABLE_GUILD_STATISTICS_SYSTEM
bool CPythonNetworkStream::RecvGuildStatisticsPacket()
{
	TPacketGCGuildStaticsInfo infoData;
	if (!Recv(sizeof(infoData), &infoData)) { return false; }

	switch (infoData.bSubHeader)
	{
		case GUILD_STATISTICS_ADD:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "GuildWarAddListNew", Py_BuildValue(
				"(sKiiiii)"
				, infoData.szName
				, infoData.ullDamage
				, infoData.bRank
				, infoData.wRace
				, infoData.bKill
				, infoData.bDead
				, infoData.bAssist
				)
			);
		}
		break;

		case GUILD_STATISTICS_UPDATE:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "GuildWarDamageUpdate", Py_BuildValue(
				"(iiiK)"
				, infoData.bKill
				, infoData.bDead
				, infoData.bAssist
				, infoData.ullDamage
				)
			);
		}
		break;
	}

	return true;
}
#endif

#ifdef ENABLE_PUBLISHER_SYSTEM
bool CPythonNetworkStream::RecvPublisherInfoPacket()
{
	TPacketGCPublisherInfo gPacket;
	if (!Recv(sizeof(gPacket), &gPacket)) { return false; }
	gPacket.wSize -= sizeof(gPacket);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_PublisherInfoClear", Py_BuildValue("()"));

	while (gPacket.wSize > 0)
	{
		TPublisherInfo iPacket;
		if (!Recv(sizeof(iPacket), &iPacket)) { return false; }
		gPacket.wSize -= sizeof(iPacket);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_PublisherInfoAdd", Py_BuildValue("(iss)", iPacket.iOrder, iPacket.szTitle, iPacket.szLink));
	}

	return true;
}
#endif

#ifdef ENABLE_RONARK_SYSTEM
bool CPythonNetworkStream::RecvRonarkInfoPacket()
{
	TPacketGCRonarkInfo gPacket;
	if (!Recv(sizeof(gPacket), &gPacket)) { return false; }
	gPacket.wSize -= sizeof(gPacket);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RonarkUpdateClear", Py_BuildValue("()"));

	int iList = 0;
	while (gPacket.wSize > 0)
	{
		TRonarkTeamData iPacket;
		if (!Recv(sizeof(iPacket), &iPacket)) { return false; }
		gPacket.wSize -= sizeof(iPacket);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RonarkUpdateInfo", Py_BuildValue("(iiiiii)", iList++, iPacket.dwID, iPacket.dwDamage, iPacket.iJoinerCount, iPacket.iMemberCount, iPacket.iRonarkHP));
	}

	return true;
}
#endif

#ifdef ENABLE_AUTO_ITEM_SELL_SYSTEM
bool CPythonNetworkStream::SendAutoItemSell(BYTE bSubHeader, DWORD dwValue)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGAutoItemSellData packet;
	packet.header = HEADER_CG_AUTO_ITEM_SELL;
	packet.subHeader = bSubHeader;
	packet.value = dwValue;

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("SendAutoItemSell Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvAutoItemSellPacket()
{
	TPacketGCAutoItemSellData gPacket;
	if (!Recv(sizeof(gPacket), &gPacket)) { return false; }
	gPacket.size -= sizeof(TPacketGCAutoItemSellData);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ClearAutoItemSell", Py_BuildValue("()"));

	while (gPacket.size > 0)
	{
		DWORD iPacket;
		if (!Recv(sizeof(iPacket), &iPacket)) { return false; }
		gPacket.size -= sizeof(DWORD);
		if (iPacket > 0)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddAutoItemSell", Py_BuildValue("(i)", iPacket));
		}
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_StatusAutoItemSell", Py_BuildValue("(i)", gPacket.status));
	return true;
}
#endif

//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
