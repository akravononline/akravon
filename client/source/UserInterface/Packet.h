#pragma once

#include "Locale.h"
#include "../gamelib/RaceData.h"
#include "../gamelib/ItemData.h"

typedef BYTE TPacketHeader;

enum
{
	/////////////////////////////////////////////////
	// To Server
	// HEADER_BLANK is the not use(for future use)
	HEADER_CG_LOGIN									= 1,
	HEADER_CG_ATTACK								= 2,
	HEADER_CG_CHAT									= 3,
	HEADER_CG_PLAYER_CREATE							= 4,
	HEADER_CG_PLAYER_DESTROY						= 5,
	HEADER_CG_PLAYER_SELECT							= 6,
	HEADER_CG_CHARACTER_MOVE						= 7,
	HEADER_CG_SYNC_POSITION  						= 8,
	HEADER_CG_DIRECT_ENTER							= 9,
	HEADER_CG_ENTERGAME								= 10,
	HEADER_CG_ITEM_USE								= 11,
	#ifndef ENABLE_DROP_DIALOG_EXTENDED_SYSTEM
	HEADER_CG_ITEM_DROP								= 12,
	#endif
	HEADER_CG_ITEM_MOVE								= 13,
	HEADER_CG_ITEM_PICKUP							= 15,
	HEADER_CG_QUICKSLOT_ADD							= 16,
	HEADER_CG_QUICKSLOT_DEL							= 17,
	HEADER_CG_QUICKSLOT_SWAP						= 18,
	HEADER_CG_WHISPER								= 19,

	#ifdef ENABLE_DROP_DIALOG_EXTENDED_SYSTEM
	HEADER_CG_ITEM_DELETE							= 20,
	HEADER_CG_ITEM_SELL								= 21,
	#else
	HEADER_CG_ITEM_DROP2							= 20,
	#endif

	//HEADER_BLANK21								= 21,
	//HEADER_BLANK22								= 22,
	//HEADER_BLANK22								= 23,
	//HEADER_BLANK24								= 24,
	//HEADER_BLANK25								= 25,
	HEADER_CG_ON_CLICK								= 26,
	HEADER_CG_EXCHANGE								= 27,
	HEADER_CG_CHARACTER_POSITION					= 28,
	HEADER_CG_SCRIPT_ANSWER							= 29,
	HEADER_CG_QUEST_INPUT_STRING					= 30,
	HEADER_CG_QUEST_CONFIRM							= 31,
	//HEADER_BLANK32								= 32,
	//HEADER_BLANK33								= 33,
	//HEADER_BLANK34								= 34,
	//HEADER_BLANK35								= 35,
	//HEADER_BLANK36								= 36,
	//HEADER_BLANK37								= 37,
	//HEADER_BLANK38								= 38,
	//HEADER_BLANK39								= 39,
	//HEADER_BLANK40								= 40,
	HEADER_CG_PVP									= 41,
	//HEADER_BLANK42								= 42,
	//HEADER_BLANK43								= 43,
	//HEADER_BLANK44								= 44,
	//HEADER_BLANK45								= 45,
	//HEADER_BLANK46								= 46,
	//HEADER_BLANK47								= 47,
	//HEADER_BLANK48								= 48,
	//HEADER_BLANK49								= 49,
	HEADER_CG_SHOP									= 50,
	HEADER_CG_FLY_TARGETING							= 51,
	HEADER_CG_USE_SKILL								= 52,
	HEADER_CG_ADD_FLY_TARGETING						= 53,
	HEADER_CG_SHOOT									= 54,
	HEADER_CG_MYSHOP								= 55,
	//HEADER_BLANK56								= 56,
	//HEADER_BLANK57								= 57,
	//HEADER_BLANK58								= 58,

	#ifdef ENABLE_SEND_TARGET_INFO
	HEADER_CG_TARGET_INFO_LOAD						= 59,
	#endif

	HEADER_CG_ITEM_USE_TO_ITEM						= 60,
	HEADER_CG_TARGET								= 61,
	//HEADER_BLANK62								= 62,
	//HEADER_BLANK63								= 63,
	//HEADER_BLANK64								= 64,
	HEADER_CG_WARP									= 65,
	HEADER_CG_SCRIPT_BUTTON							= 66,
	HEADER_CG_MESSENGER								= 67,
	//HEADER_BLANK68								= 68,
	HEADER_CG_MALL_CHECKOUT							= 69,
	HEADER_CG_SAFEBOX_CHECKIN						= 70,
	HEADER_CG_SAFEBOX_CHECKOUT						= 71,
	HEADER_CG_PARTY_INVITE							= 72,
	HEADER_CG_PARTY_INVITE_ANSWER					= 73,
	HEADER_CG_PARTY_REMOVE							= 74,
	HEADER_CG_PARTY_SET_STATE						= 75,
	HEADER_CG_PARTY_USE_SKILL						= 76,
	HEADER_CG_SAFEBOX_ITEM_MOVE						= 77,
	HEADER_CG_PARTY_PARAMETER						= 78,
	//HEADER_BLANK68								= 79,
	HEADER_CG_GUILD									= 80,
	HEADER_CG_ANSWER_MAKE_GUILD						= 81,
	HEADER_CG_FISHING								= 82,
	HEADER_CG_GIVE_ITEM								= 83,
	//HEADER_BLANK84								= 84,
	//HEADER_BLANK85								= 85,
	//HEADER_BLANK86								= 86,
	//HEADER_BLANK87								= 87,
	//HEADER_BLANK88								= 88,
	//HEADER_BLANK89								= 89,
	HEADER_CG_EMPIRE								= 90,
	//HEADER_BLANK91								= 91,
	//HEADER_BLANK92								= 92,
	//HEADER_BLANK93								= 93,
	//HEADER_BLANK94								= 94,
	//HEADER_BLANK95								= 95,
	HEADER_CG_REFINE								= 96,
	//HEADER_BLANK97								= 97,
	//HEADER_BLANK98								= 98,
	//HEADER_BLANK99								= 99,

	HEADER_CG_MARK_LOGIN							= 100,
	HEADER_CG_MARK_CRCLIST							= 101,
	HEADER_CG_MARK_UPLOAD							= 102,
	HEADER_CG_MARK_IDXLIST							= 104,

	HEADER_CG_CRC_REPORT							= 103,

	HEADER_CG_HACK									= 105,
	HEADER_CG_CHANGE_NAME							= 106,
	HEADER_CG_SMS									= 107,
	HEADER_CG_LOGIN2								= 109,
	HEADER_CG_DUNGEON								= 110,
	HEADER_CG_LOGIN3								= 111,
	HEADER_CG_GUILD_SYMBOL_UPLOAD					= 112,
	HEADER_CG_GUILD_SYMBOL_CRC						= 113,
	HEADER_CG_SCRIPT_SELECT_ITEM					= 114,
	HEADER_CG_LOGIN4								= 115,

	#ifdef ENABLE_IKASHOP_RENEWAL
	HEADER_CG_NEW_OFFLINESHOP						= 119,
	#endif

	#ifdef ENABLE_DRAGON_COIN_SHOP_SYSTEM
	HEADER_CG_ITEMSHOP_PROCESS						= 160,
	#endif

	#ifdef ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM
	HEADER_CG_ITEMSHOP_ADMIN_PROCESS				= 161,
	#endif

	#ifdef ENABLE_BATTLE_PASS
	HEADER_CG_BATTLE_PASS							= 162,
	#endif

	#ifdef ENABLE_CHANGELOOK_SYSTEM
	HEADER_CG_CL									= 163,
	#endif

	HEADER_CG_DRAGON_SOUL_REFINE					= 205,
	HEADER_CG_STATE_CHECKER							= 206,

	#ifdef ENABLE_SHOW_CHEST_DROP
	HEADER_CG_CHEST_DROP_INFO						= 207,
	#endif

	#ifdef ENABLE_SWITCHBOT
	HEADER_CG_SWITCHBOT								= 208,
	#endif

	#ifdef ENABLE_FAST_OPEN_CHEST
	HEADER_CG_INSTANT_OPEN							= 209,
	#endif

	#ifdef ENABLE_STYLE_ATTRIBUTE_SYSTEM
	HEADER_CG_ITEM_USE_NEW_ATTRIBUTE				= 210,
	#endif

	#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	HEADER_CG_ACCE									= 211,
	#endif

	#ifdef ENABLE_HIDE_COSTUME_PART
	HEADER_CG_CHANGE_COSTUME_VISIBILITY_STATUS		= 212,
	#endif

	#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
	HEADER_CG_CUBE_RENEWAL							= 213,
	#endif

	#ifdef ENABLE_PM_ALL_SEND_SYSTEM
	HEADER_CG_BULK_WHISPER							= 216,
	#endif

	#ifdef ENABLE_OFFER_SYSTEM
	HEADER_CG_OFFER_REQUEST							= 217,
	#endif

	#ifdef ENABLE_BLACK_MARKET_SYSTEM
	HEADER_CG_BLACKMARKET							= 218,
	#endif

	#if defined(__BL_LUCKY_BOX__)
	HEADER_CG_LUCKY_BOX								= 220,
	#endif

	#ifdef ENABLE_AUTO_ITEM_SELL_SYSTEM
	HEADER_CG_AUTO_ITEM_SELL						= 221,
	#endif

	HEADER_CG_KEY_AGREEMENT							= 0xfb, // _IMPROVED_PACKET_ENCRYPTION_
	HEADER_CG_TIME_SYNC								= 0xfc,
	HEADER_CG_CLIENT_VERSION						= 0xfd,
	HEADER_CG_CLIENT_VERSION2						= 0xf1,
	HEADER_CG_PONG									= 0xfe,
	HEADER_CG_HANDSHAKE								= 0xff,

	/////////////////////////////////////////////////
	// From Server
	HEADER_GC_CHARACTER_ADD							= 1,
	HEADER_GC_CHARACTER_DEL							= 2,
	HEADER_GC_CHARACTER_MOVE						= 3,
	HEADER_GC_CHAT									= 4,
	HEADER_GC_SYNC_POSITION 						= 5,
	HEADER_GC_LOGIN_SUCCESS3						= 6,
	HEADER_GC_LOGIN_FAILURE							= 7,
	HEADER_GC_PLAYER_CREATE_SUCCESS					= 8,
	HEADER_GC_PLAYER_CREATE_FAILURE					= 9,
	HEADER_GC_PLAYER_DELETE_SUCCESS					= 10,
	HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID			= 11,
	// 12
	HEADER_GC_STUN									= 13,
	HEADER_GC_DEAD									= 14,

	HEADER_GC_MAIN_CHARACTER						= 15,
	HEADER_GC_PLAYER_POINTS							= 16,
	HEADER_GC_PLAYER_POINT_CHANGE					= 17,
	HEADER_GC_CHANGE_SPEED							= 18,
	HEADER_GC_CHARACTER_UPDATE						= 19,

	HEADER_GC_ITEM_DEL								= 20,
	HEADER_GC_ITEM_SET								= 21,
	HEADER_GC_ITEM_USE								= 22,
	HEADER_GC_ITEM_DROP								= 23,
	HEADER_GC_ITEM_UPDATE							= 25,
	HEADER_GC_ITEM_GROUND_ADD						= 26,
	HEADER_GC_ITEM_GROUND_DEL						= 27,
	HEADER_GC_QUICKSLOT_ADD							= 28,
	HEADER_GC_QUICKSLOT_DEL							= 29,
	HEADER_GC_QUICKSLOT_SWAP						= 30,
	HEADER_GC_ITEM_OWNERSHIP						= 31,
	HEADER_GC_LOGIN_SUCCESS4						= 32,
	HEADER_GC_ITEM_UNBIND_TIME						= 33,
	HEADER_GC_WHISPER								= 34,
	HEADER_GC_ALERT									= 35,

	HEADER_GC_MOTION								= 36,

	HEADER_GC_SHOP									= 38,
	HEADER_GC_SHOP_SIGN								= 39,

	HEADER_GC_DUEL_START							= 40,
	HEADER_GC_PVP									= 41,
	HEADER_GC_EXCHANGE								= 42,
	HEADER_GC_CHARACTER_POSITION					= 43,

	HEADER_GC_PING									= 44,

	HEADER_GC_SCRIPT								= 45,
	HEADER_GC_QUEST_CONFIRM							= 46,

	HEADER_GC_AFFECT_RESET							= 57,

	#ifdef ENABLE_SEND_TARGET_INFO
	HEADER_GC_TARGET_INFO							= 58,
	#endif

	HEADER_GC_MOUNT									= 61,
	HEADER_GC_OWNERSHIP								= 62,
	HEADER_GC_TARGET								= 63,
	HEADER_GC_WARP									= 65,
	HEADER_GC_ADD_FLY_TARGETING						= 69,

	HEADER_GC_CREATE_FLY							= 70,
	HEADER_GC_FLY_TARGETING							= 71,
	HEADER_GC_SKILL_LEVEL							= 72,
	HEADER_GC_SKILL_COOLTIME_END					= 73,
	HEADER_GC_MESSENGER								= 74,
	HEADER_GC_GUILD									= 75,
	HEADER_GC_SKILL_LEVEL_NEW						= 76,

	HEADER_GC_PARTY_INVITE							= 77,
	HEADER_GC_PARTY_ADD								= 78,
	HEADER_GC_PARTY_UPDATE							= 79,
	HEADER_GC_PARTY_REMOVE							= 80,

	HEADER_GC_QUEST_INFO							= 81,
	HEADER_GC_REQUEST_MAKE_GUILD					= 82,
	HEADER_GC_PARTY_PARAMETER						= 83,

	HEADER_GC_SAFEBOX_MONEY_CHANGE					= 84,
	HEADER_GC_SAFEBOX_SET							= 85,
	HEADER_GC_SAFEBOX_DEL							= 86,
	HEADER_GC_SAFEBOX_WRONG_PASSWORD				= 87,
	HEADER_GC_SAFEBOX_SIZE							= 88,

	HEADER_GC_FISHING								= 89,

	HEADER_GC_EMPIRE								= 90,

	HEADER_GC_PARTY_LINK							= 91,
	HEADER_GC_PARTY_UNLINK							= 92,

	HEADER_GC_REFINE_INFORMATION					= 95,

	HEADER_GC_OBSERVER_ADD							= 96,
	HEADER_GC_OBSERVER_REMOVE						= 97,
	HEADER_GC_OBSERVER_MOVE							= 98,
	HEADER_GC_VIEW_EQUIP							= 99,

	HEADER_GC_MARK_BLOCK							= 100,
	HEADER_GC_MARK_DIFF_DATA						= 101,
	HEADER_GC_MARK_IDXLIST							= 102,

	HEADER_GC_TIME									= 106,
	HEADER_GC_CHANGE_NAME							= 107,

	HEADER_GC_DUNGEON								= 110,
	HEADER_GC_WALK_MODE								= 111,
	HEADER_GC_CHANGE_SKILL_GROUP					= 112,

	// SUPPORT_BGM
	HEADER_GC_MAIN_CHARACTER2_EMPIRE				= 113,
	// END_OF_SUPPORT_BGM

	HEADER_GC_SEPCIAL_EFFECT						= 114,
	HEADER_GC_NPC_POSITION							= 115,

	HEADER_GC_LOGIN_KEY								= 118,
	HEADER_GC_REFINE_INFORMATION_NEW				= 119,
	HEADER_GC_CHARACTER_ADD2						= 120,
	HEADER_GC_CHANNEL								= 121,

	HEADER_GC_MALL_OPEN								= 122,
	HEADER_GC_TARGET_UPDATE							= 123,
	HEADER_GC_TARGET_DELETE							= 124,
	HEADER_GC_TARGET_CREATE_NEW						= 125,

	HEADER_GC_AFFECT_ADD							= 126,
	HEADER_GC_AFFECT_REMOVE							= 127,

	HEADER_GC_MALL_SET								= 128,
	HEADER_GC_MALL_DEL								= 129,
	HEADER_GC_LAND_LIST								= 130,
	HEADER_GC_LOVER_INFO							= 131,
	HEADER_GC_LOVE_POINT_UPDATE						= 132,
	HEADER_GC_GUILD_SYMBOL_DATA						= 133,
	HEADER_GC_DIG_MOTION							= 134,

	HEADER_GC_DAMAGE_INFO							= 135,
	HEADER_GC_CHAR_ADDITIONAL_INFO					= 136,

	// SUPPORT_BGM
	HEADER_GC_MAIN_CHARACTER3_BGM					= 137,
	HEADER_GC_MAIN_CHARACTER4_BGM_VOL				= 138,
	// END_OF_SUPPORT_BGM

	#ifdef ENABLE_BATTLE_PASS
	HEADER_GC_BATTLE_PASS_OPEN						= 148,
	HEADER_GC_BATTLE_PASS_UPDATE					= 149,
	#endif

	HEADER_GC_AUTH_SUCCESS							= 150,
	HEADER_GC_PANAMA_PACK							= 151,

	//HYBRID CRYPT
	HEADER_GC_HYBRIDCRYPT_KEYS						= 152,
	HEADER_GC_HYBRIDCRYPT_SDB						= 153, // SDB means Supplmentary Data Blocks
	//HYBRID CRYPT

	#ifdef ENABLE_DRAGON_COIN_SHOP_SYSTEM
	HEADER_GC_ITEMSHOP_GENERAL						= 160,
	#endif

	#ifdef ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM
	HEADER_GC_ITEMSHOP_ADMIN_GENERAL				= 161,
	#endif

	#ifdef ENABLE_TANAKA_EVENT
	HEADER_GC_TANAKA_EVENT							= 162,
	#endif

	#ifdef ENABLE_CHANGELOOK_SYSTEM
	HEADER_GC_CL									= 163,
	#endif

	#ifdef ENABLE_REWARD_SYSTEM
	HEADER_GC_REWARD_INFO							= 164,
	#endif

	#ifdef ENABLE_GUILD_STATISTICS_SYSTEM
	HEADER_GC_GUILD_STATISTICS						= 165,
	#endif

	#ifdef ENABLE_AUTO_ITEM_SELL_SYSTEM
	HEADER_GC_AUTO_ITEM_SELL						= 166,
	#endif

	HEADER_GC_SPECIFIC_EFFECT						= 208,
	HEADER_GC_DRAGON_SOUL_REFINE					= 209,
	HEADER_GC_RESPOND_CHANNELSTATUS					= 210,

	#ifdef ENABLE_SHOW_CHEST_DROP
	HEADER_GC_CHEST_DROP_INFO						= 211,
	#endif

	#ifdef ENABLE_SWITCHBOT
	HEADER_GC_SWITCHBOT								= 212,
	#endif

	#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	HEADER_GC_ACCE									= 213,
	#endif

	#ifdef ENABLE_HIDE_COSTUME_PART
	HEADER_GC_UPDATE_COSTUME_VISIBILITY_STATUS		= 214,
	#endif

	#ifdef ENABLE_FULL_YANG
	HEADER_GC_GOLD									= 215,
	HEADER_GC_GOLD_CHANGE							= 216,
	#endif

	#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
	HEADER_GC_CUBE_RENEWAL							= 217,
	#endif

	#ifdef ENABLE_WHEEL_OF_FORTUNE
	HEADER_GC_WHEEL_OF_FORTUNE						= 218,
	#endif

	#ifdef __RANKING_SYSTEM__
	HEADER_GC_RANKING_UPDATE						= 219,
	#endif

	#ifdef ENABLE_TELEPORTION_WIZARD_SYSTEM
	HEADER_GC_TELEPORTION_WIZARD_INFO				= 220,
	#endif

	#ifdef ENABLE_EVENT_MANAGER
	HEADER_GC_EVENT_MANAGER							= 221,
	#endif

	#ifdef ENABLE_SKILL_RENEWAL
	HEADER_GC_SKILL_BOOK_COUNT						= 222,
	#endif

	#ifdef ENABLE_PM_ALL_SEND_SYSTEM
	HEADER_GC_BULK_WHISPER							= 225,
	#endif

	#ifdef ENABLE_BAKIM_DUYURU
	HEADER_GC_ENABLE_BAKIM_DUYURU					= 226,
	#endif

	#ifdef BOSS_DMG_RANKING
	HEADER_GC_BOSS_RANKING							= 227,
	#endif

	#ifdef ENABLE_OFFER_SYSTEM
	HEADER_GC_OFFER									= 228,
	#endif

	#ifdef ENABLE_EXCHANGE_LOG
	HEADER_GC_EXCHANGE_LOG							= 229,
	#endif

	#ifdef ENABLE_ENTITY_PRELOADING
	HEADER_GC_PRELOAD_ENTITIES						= 230,
	#endif

	#ifdef ENABLE_IKASHOP_RENEWAL
	HEADER_GC_NEW_OFFLINESHOP						= 231,
	#endif

	#ifdef ENABLE_BLACK_MARKET_SYSTEM
	HEADER_GC_BLACKMARKET							= 232,
	#endif

	#if defined(__BL_LUCKY_BOX__)
	HEADER_GC_LUCKY_BOX								= 234,
	#endif

	#ifdef ENABLE_PUBLISHER_SYSTEM
	HEADER_GC_PUBLISHER_INFO						= 235,
	#endif

	#ifdef ENABLE_RONARK_SYSTEM
	HEADER_GC_RONARK_INFO							= 236,
	#endif
#ifdef SPIN_WHEEL
	HEADER_GC_SPIN_WHEEL = 237,
	HEADER_GC_SPINWHEEL_REWARD = 238,
#endif

	HEADER_GC_KEY_AGREEMENT_COMPLETED				= 0xfa, // _IMPROVED_PACKET_ENCRYPTION_
	HEADER_GC_KEY_AGREEMENT							= 0xfb, // _IMPROVED_PACKET_ENCRYPTION_
	HEADER_GC_HANDSHAKE_OK							= 0xfc, // 252
	HEADER_GC_PHASE									= 0xfd,	// 253
	HEADER_GC_BINDUDP								= 0xfe, // 254
	HEADER_GC_HANDSHAKE								= 0xff, // 255
};

enum
{
	ID_MAX_NUM = 30,
	PASS_MAX_NUM = 16,
	CHAT_MAX_NUM = 128,
	PATH_NODE_MAX_NUM = 64,
	SHOP_SIGN_MAX_LEN = 32,

	PLAYER_PER_ACCOUNT3 = 3,
#ifndef ENABLE_PLAYER_PER_ACCOUNT5
	PLAYER_PER_ACCOUNT4 = 4,
#else
	PLAYER_PER_ACCOUNT4 = 5,
	PLAYER_PER_ACCOUNT5 = 5,
#endif

	PLAYER_ITEM_SLOT_MAX_NUM = 20,

	QUICKSLOT_MAX_LINE = 4,
	QUICKSLOT_MAX_COUNT_PER_LINE = 8,
	QUICKSLOT_MAX_COUNT = QUICKSLOT_MAX_LINE * QUICKSLOT_MAX_COUNT_PER_LINE,

	QUICKSLOT_MAX_NUM = 36,

	SHOP_HOST_ITEM_MAX_NUM = 40,

	METIN_SOCKET_COUNT = 6,

	PARTY_AFFECT_SLOT_MAX_NUM = 7,

	GUILD_GRADE_NAME_MAX_LEN = 8,
	GUILD_NAME_MAX_LEN = 16,//12
	GUILD_GRADE_COUNT = 15,
	GULID_COMMENT_MAX_LEN = 50,

	MARK_CRC_NUM = 8*8,
	MARK_DATA_SIZE = 16*12,
	SYMBOL_DATA_SIZE = 128*256,
	QUEST_INPUT_STRING_MAX_NUM = 64,

	PRIVATE_CODE_LENGTH = 8,
	SOCIAL_ID_MAX_LEN = 18,

	REFINE_MATERIAL_MAX_NUM = 10,//5

	WEAR_MAX_NUM = CItemData::WEAR_MAX_NUM,

	SHOP_TAB_NAME_MAX = 32,
	SHOP_TAB_COUNT_MAX = 3,

	#ifdef ENABLE_SERVER_PIN
	PIN_MAX_NUM = 6,
	#endif

	#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
	CUBE_MATERIAL_MAX_NUM = 10,
	#endif

	#ifdef ENABLE_AUTO_ITEM_SELL_SYSTEM
	AUTO_ITEM_SELL_MAX = 225,
	#endif
};

#pragma pack(push)
#pragma pack(1)
#ifdef SPIN_WHEEL
static constexpr int spinWheelMaxItem = 26;
typedef struct SPacketGCSpinWheel
{
	BYTE bHeader;
	DWORD itemVnums[spinWheelMaxItem];
	DWORD itemCounts[spinWheelMaxItem];
	DWORD itemRarities[spinWheelMaxItem];
	DWORD dwRewardItem;
}TPacketGCSpinWheel;
typedef struct SPacketGCSpinWheelRewards
{
	BYTE bHeader;
	BYTE bIndex;
	DWORD itemVnums[spinWheelMaxItem];
	DWORD itemCounts[spinWheelMaxItem];
	DWORD itemRarities[spinWheelMaxItem];
}TPacketGCSpinWheelRewards;
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mark
typedef struct command_mark_login
{
	BYTE	header;
	DWORD	handle;
	DWORD	random_key;
} TPacketCGMarkLogin;

typedef struct command_mark_upload
{
	BYTE	header;
	DWORD	gid;
	BYTE	image[16*12*4];
} TPacketCGMarkUpload;

typedef struct command_mark_idxlist
{
	BYTE	header;
} TPacketCGMarkIDXList;

typedef struct command_mark_crclist
{
	BYTE	header;
	BYTE	imgIdx;
	DWORD	crclist[80];
} TPacketCGMarkCRCList;

typedef struct packet_mark_idxlist
{
	BYTE	header;
	DWORD	bufSize;
	WORD	count;
} TPacketGCMarkIDXList;

typedef struct packet_mark_block
{
	BYTE	header;
	DWORD	bufSize;
	BYTE	imgIdx;
	DWORD	count;
} TPacketGCMarkBlock;

typedef struct command_symbol_upload
{
	BYTE	header;
	WORD	size;
	DWORD	handle;
} TPacketCGSymbolUpload;

typedef struct command_symbol_crc
{
	BYTE	header;
	DWORD	dwGuildID;
	DWORD	dwCRC;
	DWORD	dwSize;
} TPacketCGSymbolCRC;

typedef struct packet_symbol_data
{
	BYTE header;
	WORD size;
	DWORD guild_id;
} TPacketGCGuildSymbolData;

//
//
//
typedef struct packet_observer_add
{
	BYTE	header;
	DWORD	vid;
	WORD	x;
	WORD	y;
} TPacketGCObserverAdd;

typedef struct packet_observer_move
{
	BYTE	header;
	DWORD	vid;
	WORD	x;
	WORD	y;
} TPacketGCObserverMove;


typedef struct packet_observer_remove
{
	BYTE	header;
	DWORD	vid;
} TPacketGCObserverRemove;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// To Server

typedef struct command_checkin
{
	BYTE header;
	char name[ID_MAX_NUM+1];
	char pwd[PASS_MAX_NUM+1];
} TPacketCGCheckin;

typedef struct command_login
{
	BYTE header;
	char name[ID_MAX_NUM + 1];
	char pwd[PASS_MAX_NUM + 1];
} TPacketCGLogin;

typedef struct command_login2
{
	BYTE	header;
	char	name[ID_MAX_NUM + 1];
	DWORD	login_key;
	DWORD	adwClientKey[4];
} TPacketCGLogin2;

typedef struct command_login3
{
	BYTE	header;
	char	name[ID_MAX_NUM + 1];
	char	pwd[PASS_MAX_NUM + 1];
	#ifdef ENABLE_SERVER_PIN
	char	pin[PIN_MAX_NUM + 1];
	#endif
	#ifdef ENABLE_PCBAN_SYSTEM
	int		pcban;
	#endif
	#ifdef ENABLE_VERSION_SYSTEM
	int		version;
	#endif
	#ifdef RENEWAL_HWID_BLOCK
	char	hwid_router[18+1];
	#endif
	#ifdef RENEWAL_DISCORD_BLOCK
	char	discord_nick[128 + 1];
	char	discord_id[128 + 1];
	#endif
	DWORD	adwClientKey[4];
} TPacketCGLogin3;

typedef struct command_direct_enter
{
	BYTE		bHeader;
	char		login[ID_MAX_NUM + 1];
	char		passwd[PASS_MAX_NUM + 1];
	BYTE		index;
} TPacketCGDirectEnter;

typedef struct command_player_select
{
	BYTE	header;
	BYTE	player_index;
} TPacketCGSelectCharacter;

typedef struct command_attack
{
	BYTE	header;
	BYTE	bType;
	DWORD	dwVictimVID;
	BYTE	bCRCMagicCubeProcPiece;
	BYTE	bCRCMagicCubeFilePiece;
} TPacketCGAttack;

typedef struct command_chat
{
	BYTE	header;
	WORD	length;
	BYTE	type;
} TPacketCGChat;

typedef struct command_whisper
{
	BYTE        bHeader;
	WORD        wSize;
	char        szNameTo[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGWhisper;

typedef struct command_sms
{
	BYTE        bHeader;
	WORD        wSize;
	char        szNameTo[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGSMS;

enum EBattleMode
{
	BATTLEMODE_ATTACK = 0,
	BATTLEMODE_DEFENSE = 1,
};

typedef struct command_EnterFrontGame
{
	BYTE header;
} TPacketCGEnterFrontGame;

typedef struct command_item_use
{
	BYTE header;
	TItemPos pos;
} TPacketCGItemUse;

typedef struct command_item_use_to_item
{
	BYTE header;
	TItemPos source_pos;
	TItemPos target_pos;
} TPacketCGItemUseToItem;

#ifdef ENABLE_DROP_DIALOG_EXTENDED_SYSTEM
typedef struct command_item_delete
{
	BYTE		header;
	TItemPos	item_pos;
} TPacketCGItemDelete;

typedef struct command_item_sell
{
	BYTE		header;
	TItemPos	item_pos;
	bool		is_auto;
} TPacketCGItemSell;
#else
typedef struct command_item_drop
{
	BYTE  header;
	TItemPos pos;
	DWORD elk;
} TPacketCGItemDrop;

typedef struct command_item_drop2
{
	BYTE        header;
	TItemPos	pos;
	DWORD       gold;
#ifdef ENABLE_NEW_MAX_COUNT
	short		count;
#else
	BYTE        count;
#endif
} TPacketCGItemDrop2;
#endif

typedef struct command_item_move
{
	BYTE header;
	TItemPos pos;
	TItemPos change_pos;
#ifdef ENABLE_NEW_MAX_COUNT
	short num;
#else
	BYTE num;
#endif
} TPacketCGItemMove;

typedef struct command_item_pickup
{
	BYTE header;
	DWORD vid;
} TPacketCGItemPickUp;

typedef struct command_quickslot_add
{
	BYTE        header;
	BYTE        pos;
	TQuickSlot	slot;
}TPacketCGQuickSlotAdd;

typedef struct command_quickslot_del
{
	BYTE        header;
	BYTE        pos;
}TPacketCGQuickSlotDel;

typedef struct command_quickslot_swap
{
	BYTE        header;
	BYTE        pos;
	BYTE        change_pos;
}TPacketCGQuickSlotSwap;

typedef struct command_on_click
{
	BYTE		header;
	DWORD		vid;
} TPacketCGOnClick;

enum
{
	SHOP_SUBHEADER_CG_END,
	SHOP_SUBHEADER_CG_BUY,
	SHOP_SUBHEADER_CG_SELL,
	SHOP_SUBHEADER_CG_SELL2,
};

typedef struct shop_item_detail
{
	TItemPos	itemPos;
#ifdef ENABLE_NEW_MAX_COUNT
	short		byCount;
#else
	BYTE		byCount;
#endif
} TPacketGCShopDetail;

typedef struct command_shop
{
	BYTE        header;
#ifdef ENABLE_NEW_MAX_COUNT
	short		subheader;
#else
	BYTE		subheader;
#endif
} TPacketCGShop;

enum
{
	EXCHANGE_SUBHEADER_CG_START,			// arg1 == vid of target character
	EXCHANGE_SUBHEADER_CG_ITEM_ADD,			// arg1 == position of item
	EXCHANGE_SUBHEADER_CG_ITEM_DEL,			// arg1 == position of item
	EXCHANGE_SUBHEADER_CG_ELK_ADD,			// arg1 == amount of elk
	EXCHANGE_SUBHEADER_CG_ACCEPT,			// arg1 == not used
	EXCHANGE_SUBHEADER_CG_CANCEL,			// arg1 == not used
};

typedef struct command_exchange
{
	BYTE		header;
	BYTE		subheader;
	#ifdef ENABLE_FULL_YANG
	uint64_t	arg1;
	#else
	DWORD		arg1;
	#endif
	BYTE		arg2;
	TItemPos	Pos;
} TPacketCGExchange;

typedef struct command_position
{
	BYTE		header;
	BYTE		position;
} TPacketCGPosition;

typedef struct command_script_answer
{
	BYTE		header;
	BYTE		answer;
} TPacketCGScriptAnswer;

typedef struct command_script_button
{
	BYTE			header;
	unsigned int	idx;
} TPacketCGScriptButton;

typedef struct command_target
{
	BYTE		header;
	DWORD		dwVID;
} TPacketCGTarget;

typedef struct command_move
{
	BYTE		bHeader;
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	LONG		lX;
	LONG		lY;
	DWORD		dwTime;
} TPacketCGMove;

typedef struct command_sync_position_element
{
	DWORD		dwVID;
	LONG		lX;
	LONG		lY;
} TPacketCGSyncPositionElement;

typedef struct command_sync_position
{
	BYTE		bHeader;
	WORD		wSize;
} TPacketCGSyncPosition;

typedef struct command_fly_targeting
{
	BYTE		bHeader;
	DWORD		dwTargetVID;
	LONG		lX;
	LONG		lY;
} TPacketCGFlyTargeting;

typedef struct packet_fly_targeting
{
	BYTE		bHeader;
	DWORD		dwShooterVID;
	DWORD		dwTargetVID;
	LONG		lX;
	LONG		lY;
} TPacketGCFlyTargeting;

typedef struct packet_shoot
{
	BYTE		bHeader;
	BYTE		bType;
} TPacketCGShoot;

typedef struct command_warp
{
	BYTE		bHeader;
} TPacketCGWarp;

enum
{
	#ifdef ENABLE_MESSENGER_BLOCK
	MESSENGER_SUBHEADER_GC_BLOCK_LIST,
	MESSENGER_SUBHEADER_GC_BLOCK_LOGIN,
	MESSENGER_SUBHEADER_GC_BLOCK_LOGOUT,
	MESSENGER_SUBHEADER_GC_BLOCK_INVITE,//not used
	#endif
	MESSENGER_SUBHEADER_GC_LIST,
	MESSENGER_SUBHEADER_GC_LOGIN,
	MESSENGER_SUBHEADER_GC_LOGOUT,
	#ifdef ENABLE_MESSENGER_TEAM
	MESSENGER_SUBHEADER_GC_TEAM_LIST,
	MESSENGER_SUBHEADER_GC_TEAM_LOGIN,
	MESSENGER_SUBHEADER_GC_TEAM_LOGOUT,
	#endif
	MESSENGER_SUBHEADER_GC_INVITE,
	MESSENGER_SUBHEADER_GC_MOBILE,
};

typedef struct packet_messenger
{
	BYTE header;
	WORD size;
	BYTE subheader;
} TPacketGCMessenger;

typedef struct packet_messenger_list_offline
{
	BYTE connected; // always 0
	BYTE length;
} TPacketGCMessengerListOffline;

#ifdef ENABLE_MESSENGER_BLOCK
typedef struct packet_messenger_block_list_offline
{
	BYTE connected; // always 0
	BYTE length;
} TPacketGCMessengerBlockListOffline;

typedef struct packet_messenger_block_list_online
{
	BYTE connected; // always 1
	BYTE length;
} TPacketGCMessengerBlockListOnline;
#endif

#ifdef ENABLE_MESSENGER_TEAM
typedef struct packet_messenger_team_list_offline
{
	BYTE connected; // always 0
	BYTE length;
} TPacketGCMessengerTeamListOffline;

typedef struct packet_messenger_team_list_online
{
	BYTE connected; // always 1
	BYTE length;
} TPacketGCMessengerTeamListOnline;
#endif

enum
{
	MESSENGER_CONNECTED_STATE_OFFLINE,
	MESSENGER_CONNECTED_STATE_ONLINE,
	MESSENGER_CONNECTED_STATE_MOBILE,
};

typedef struct packet_messenger_list_online
{
	BYTE connected;
	BYTE length;
	//BYTE length_char_name;
} TPacketGCMessengerListOnline;

typedef struct packet_messenger_login
{
	//BYTE length_login;
	//BYTE length_char_name;
	BYTE length;
} TPacketGCMessengerLogin;

typedef struct packet_messenger_logout
{
	BYTE length;
} TPacketGCMessengerLogout;

enum
{
	#ifdef ENABLE_MESSENGER_BLOCK
	MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_VID,
	MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_NAME,
	MESSENGER_SUBHEADER_CG_REMOVE_BLOCK,
	#endif
	MESSENGER_SUBHEADER_CG_ADD_BY_VID,
	MESSENGER_SUBHEADER_CG_ADD_BY_NAME,
	MESSENGER_SUBHEADER_CG_REMOVE,
};

typedef struct command_messenger
{
	BYTE header;
	BYTE subheader;
} TPacketCGMessenger;

typedef struct command_messenger_remove
{
	BYTE length;
} TPacketCGMessengerRemove;

enum
{
	SAFEBOX_MONEY_STATE_SAVE,
	SAFEBOX_MONEY_STATE_WITHDRAW,
};

typedef struct command_safebox_money
{
	BYTE		bHeader;
	BYTE		bState;
	DWORD		dwMoney;
} TPacketCGSafeboxMoney;

typedef struct command_safebox_checkout
{
	BYTE		bHeader;
	BYTE		bSafePos;
	TItemPos	ItemPos;
} TPacketCGSafeboxCheckout;

typedef struct command_safebox_checkin
{
	BYTE		bHeader;
	BYTE		bSafePos;
	TItemPos	ItemPos;
} TPacketCGSafeboxCheckin;

typedef struct command_mall_checkout
{
	BYTE		bHeader;
	BYTE		bMallPos;
	TItemPos	ItemPos;
} TPacketCGMallCheckout;

///////////////////////////////////////////////////////////////////////////////////
// Party

typedef struct command_use_skill
{
	BYTE bHeader;
	DWORD dwVnum;
	DWORD dwTargetVID;
} TPacketCGUseSkill;

typedef struct command_party_invite
{
	BYTE header;
	DWORD vid;
} TPacketCGPartyInvite;

typedef struct command_party_invite_answer
{
	BYTE header;
	DWORD leader_pid;
	BYTE accept;
} TPacketCGPartyInviteAnswer;

typedef struct command_party_remove
{
	BYTE header;
	DWORD pid;
} TPacketCGPartyRemove;

typedef struct command_party_set_state
{
	BYTE byHeader;
	DWORD dwVID;
	BYTE byState;
	BYTE byFlag;
} TPacketCGPartySetState;

typedef struct packet_party_link
{
	BYTE header;
	DWORD pid;
	DWORD vid;
} TPacketGCPartyLink;

typedef struct packet_party_unlink
{
	BYTE header;
	DWORD pid;
	DWORD vid;
} TPacketGCPartyUnlink;

typedef struct command_party_use_skill
{
	BYTE byHeader;
	BYTE bySkillIndex;
	DWORD dwTargetVID;
} TPacketCGPartyUseSkill;

enum
{
	GUILD_SUBHEADER_CG_ADD_MEMBER,
	GUILD_SUBHEADER_CG_REMOVE_MEMBER,
	GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME,
	GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY,
	GUILD_SUBHEADER_CG_OFFER,
	GUILD_SUBHEADER_CG_POST_COMMENT,
	GUILD_SUBHEADER_CG_DELETE_COMMENT,
	GUILD_SUBHEADER_CG_REFRESH_COMMENT,
	GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE,
	GUILD_SUBHEADER_CG_USE_SKILL,
	GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL,
	GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER,
	GUILD_SUBHEADER_CG_CHARGE_GSP,
	GUILD_SUBHEADER_CG_DEPOSIT_MONEY,
	GUILD_SUBHEADER_CG_WITHDRAW_MONEY,
};

typedef struct command_guild
{
	BYTE byHeader;
	BYTE bySubHeader;
} TPacketCGGuild;

typedef struct command_guild_answer_make_guild
{
	BYTE header;
	char guild_name[GUILD_NAME_MAX_LEN+1];
} TPacketCGAnswerMakeGuild;

typedef struct command_give_item
{
	BYTE byHeader;
	DWORD dwTargetVID;
	TItemPos ItemPos;
	#ifdef ENABLE_NEW_MAX_COUNT
	short byItemCount;
	#else
	BYTE byItemCount;
	#endif
} TPacketCGGiveItem;

typedef struct SPacketCGHack
{
	BYTE		bHeader;
	char		szBuf[255 + 1];
} TPacketCGHack;

typedef struct command_dungeon
{
	BYTE		bHeader;
	WORD		size;
} TPacketCGDungeon;

// Private Shop
typedef struct SShopItemTable
{
	DWORD		vnum;
	#ifdef ENABLE_NEW_MAX_COUNT
	short		count;
	#else
	BYTE		count;
	#endif
	TItemPos	pos;
	#ifdef ENABLE_FULL_YANG
	int64_t		price;
	#else
	DWORD		price;
	#endif
	BYTE		display_pos;
} TShopItemTable;

typedef struct SPacketCGMyShop
{
	BYTE		bHeader;
	char		szSign[SHOP_SIGN_MAX_LEN + 1];
	#ifdef ENABLE_NEW_MAX_COUNT
	short		bCount;	// count of TShopItemTable, max 39
	#else
	BYTE		bCount;	// count of TShopItemTable, max 39
	#endif
} TPacketCGMyShop;

typedef struct SPacketCGRefine
{
	BYTE		header;
	BYTE		pos;
	BYTE		type;
} TPacketCGRefine;

typedef struct SPacketCGChangeName
{
	BYTE header;
	BYTE index;
	char name[CHARACTER_NAME_MAX_LEN+1];
} TPacketCGChangeName;

typedef struct command_client_version
{
	BYTE header;
	char filename[32+1];
	char timestamp[32+1];
} TPacketCGClientVersion;

typedef struct command_client_version2
{
	BYTE header;
	char filename[32+1];
	char timestamp[32+1];
} TPacketCGClientVersion2;

typedef struct command_crc_report
{
	BYTE header;
	BYTE byPackMode;
	DWORD dwBinaryCRC32;
	DWORD dwProcessCRC32;
	DWORD dwRootPackCRC32;
} TPacketCGCRCReport;

enum EPartyExpDistributionType
{
	PARTY_EXP_DISTRIBUTION_NON_PARITY,
	PARTY_EXP_DISTRIBUTION_PARITY,
};

typedef struct command_party_parameter
{
	BYTE		bHeader;
	BYTE		bDistributeMode;
} TPacketCGPartyParameter;

typedef struct command_quest_input_string
{
	BYTE		bHeader;
	char		szString[QUEST_INPUT_STRING_MAX_NUM+1];
} TPacketCGQuestInputString;

typedef struct command_quest_confirm
{
	BYTE header;
	BYTE answer;
	DWORD requestPID;
} TPacketCGQuestConfirm;

typedef struct command_script_select_item
{
	BYTE header;
	DWORD selection;
} TPacketCGScriptSelectItem;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// From Server
enum EPhase
{
	PHASE_CLOSE,
	PHASE_HANDSHAKE,
	PHASE_LOGIN,
	PHASE_SELECT,
	PHASE_LOADING,
	PHASE_GAME,
	PHASE_DEAD,

	PHASE_DBCLIENT_CONNECTING,
	PHASE_DBCLIENT,
	PHASE_P2P,
	PHASE_AUTH,
};

typedef struct packet_phase
{
	BYTE		header;
	BYTE		phase;
} TPacketGCPhase;

typedef struct packet_blank
{
	BYTE		header;
} TPacketGCBlank;

typedef struct packet_blank_dynamic
{
	BYTE		header;
	WORD		size;
} TPacketGCBlankDynamic;

typedef struct packet_header_handshake
{
	BYTE		header;
	DWORD		dwHandshake;
	DWORD		dwTime;
	LONG		lDelta;
} TPacketGCHandshake;

typedef struct packet_header_bindudp
{
	BYTE		header;
	DWORD		addr;
	WORD		port;
} TPacketGCBindUDP;

typedef struct packet_header_dynamic_size
{
	BYTE		header;
	WORD		size;
} TDynamicSizePacketHeader;

#ifdef ENABLE_LARGE_DYNAMIC_PACKETS
typedef struct packet_header_large_dynamic_size
{
	BYTE		header;
	int			size;
} TLargeDynamicSizePacketHeader;
#endif

typedef struct SSimplePlayerInformation
{
	DWORD				dwID;
	char				szName[CHARACTER_NAME_MAX_LEN + 1];
	BYTE				byJob;
	BYTE				byLevel;
	DWORD				dwPlayMinutes;
	BYTE				byST, byHT, byDX, byIQ;
	DWORD				wMainPart;
	BYTE				bChangeName;
	DWORD				wHairPart;
	#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	DWORD				wAccePart;
	#endif
	BYTE				bDummy[4];
	LONG				x, y;
	LONG				lAddr;
	WORD				wPort;
	BYTE				bySkillGroup;
} TSimplePlayerInformation;

typedef struct packet_login_success3
{
	BYTE						header;
	TSimplePlayerInformation	akSimplePlayerInformation[PLAYER_PER_ACCOUNT3];
	DWORD						guild_id[PLAYER_PER_ACCOUNT3];
	char						guild_name[PLAYER_PER_ACCOUNT3][GUILD_NAME_MAX_LEN+1];
	DWORD handle;
	DWORD random_key;
} TPacketGCLoginSuccess3;

typedef struct packet_login_success4
{
	BYTE						header;
	TSimplePlayerInformation	akSimplePlayerInformation[PLAYER_PER_ACCOUNT4];
	DWORD						guild_id[PLAYER_PER_ACCOUNT4];
	char						guild_name[PLAYER_PER_ACCOUNT4][GUILD_NAME_MAX_LEN+1];
	DWORD handle;
	DWORD random_key;
} TPacketGCLoginSuccess4;

#ifdef ENABLE_PLAYER_PER_ACCOUNT5
typedef struct packet_login_success5
{
	BYTE						header;
	TSimplePlayerInformation	akSimplePlayerInformation[PLAYER_PER_ACCOUNT5];
	DWORD						guild_id[PLAYER_PER_ACCOUNT5];
	char						guild_name[PLAYER_PER_ACCOUNT5][GUILD_NAME_MAX_LEN+1];
	DWORD handle;
	DWORD random_key;
} TPacketGCLoginSuccess5;
#endif

enum { LOGIN_STATUS_MAX_LEN = 8 };
typedef struct packet_login_failure
{
	BYTE	header;
	char	szStatus[LOGIN_STATUS_MAX_LEN + 1];
} TPacketGCLoginFailure;

typedef struct command_player_create
{
	BYTE        header;
	BYTE        index;
	char        name[CHARACTER_NAME_MAX_LEN + 1];
	WORD        job;
	BYTE		shape;
	BYTE		CON;
	BYTE		INT;
	BYTE		STR;
	BYTE		DEX;
} TPacketCGCreateCharacter;

typedef struct command_player_create_success
{
	BYTE						header;
	BYTE						bAccountCharacterSlot;
	TSimplePlayerInformation	kSimplePlayerInfomation;
} TPacketGCPlayerCreateSuccess;

typedef struct command_create_failure
{
	BYTE	header;
	BYTE	bType;
} TPacketGCCreateFailure;

typedef struct command_player_delete
{
	BYTE		header;
	BYTE		index;
	// char		szPrivateCode[PRIVATE_CODE_LENGTH];
	char		szPrivateCode[SOCIAL_ID_MAX_LEN + 1];
} TPacketCGDestroyCharacter;

typedef struct packet_player_delete_success
{
	BYTE		header;
	BYTE		account_index;
} TPacketGCDestroyCharacterSuccess;

enum
{
	ADD_CHARACTER_STATE_DEAD   = (1 << 0),
	ADD_CHARACTER_STATE_SPAWN  = (1 << 1),
	ADD_CHARACTER_STATE_GUNGON = (1 << 2),
	ADD_CHARACTER_STATE_KILLER = (1 << 3),
	ADD_CHARACTER_STATE_PARTY  = (1 << 4),
};

enum EPKModes
{
	PK_MODE_PEACE,
	PK_MODE_REVENGE,
	PK_MODE_FREE,
	PK_MODE_PROTECT,
	PK_MODE_GUILD,
	PK_MODE_MAX_NUM,
};

enum ECharacterEquipmentPart
{
	CHR_EQUIPPART_ARMOR,
	CHR_EQUIPPART_WEAPON,
	CHR_EQUIPPART_HEAD,
	CHR_EQUIPPART_HAIR,
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	CHR_EQUIPPART_ACCE,
#endif
	CHR_EQUIPPART_NUM,
};

typedef struct packet_char_additional_info
{
	BYTE	header;
	DWORD	dwVID;
	char	name[CHARACTER_NAME_MAX_LEN + 1];
	DWORD	awPart[CHR_EQUIPPART_NUM];
	BYTE	bEmpire;
	DWORD	dwGuildID;
	DWORD	dwLevel;
	int		iAlignment;
	BYTE	bPKMode;
	DWORD	dwMountVnum;
	#ifdef ENABLE_EVOLUTION_SYSTEM
	int		iEvolutionWeapon;
	int		iEvolutionBody;
	#endif
} TPacketGCCharacterAdditionalInfo;

typedef struct packet_add_char
{
	BYTE		header;

	DWORD		dwVID;

	float		angle;
	LONG		x;
	LONG		y;
	LONG		z;

	BYTE		bType;
	DWORD		wRaceNum; // @fixme501
	BYTE		bMovingSpeed;
	BYTE		bAttackSpeed;

	BYTE		bStateFlag;
	DWORD		dwAffectFlag[2];
} TPacketGCCharacterAdd;

typedef struct packet_add_char2
{
	BYTE		header;

	DWORD		dwVID;

	char		name[CHARACTER_NAME_MAX_LEN + 1];

	float		angle;
	LONG		x;
	LONG		y;
	LONG		z;

	BYTE		bType;
	DWORD		wRaceNum; // @fixme501
	DWORD		awPart[CHR_EQUIPPART_NUM];
	BYTE		bMovingSpeed;
	BYTE		bAttackSpeed;

	BYTE		bStateFlag;
	DWORD		dwAffectFlag[2];
	BYTE		bEmpire;

	DWORD		dwGuild;
	int			iAlignment;
	BYTE		bPKMode;
	DWORD		dwMountVnum;

	#ifdef ENABLE_EVOLUTION_SYSTEM
	int			iEvolutionWeapon;
	int			iEvolutionBody;
	#endif
} TPacketGCCharacterAdd2;

typedef struct packet_update_char
{
	BYTE		header;
	DWORD		dwVID;

	DWORD		awPart[CHR_EQUIPPART_NUM];
	BYTE		bMovingSpeed;
	BYTE		bAttackSpeed;

	BYTE		bStateFlag;
	DWORD		dwAffectFlag[2];

	DWORD		dwGuildID;
	int			iAlignment;
	BYTE		bPKMode;
	DWORD		dwMountVnum;
	DWORD		dwLevel;
	#ifdef ENABLE_EVOLUTION_SYSTEM
	int			iEvolutionWeapon;
	int			iEvolutionBody;
	#endif
} TPacketGCCharacterUpdate;

typedef struct packet_del_char
{
	BYTE	header;
	DWORD	dwVID;
} TPacketGCCharacterDelete;

typedef struct packet_GlobalTime
{
	BYTE	header;
	float	GlobalTime;
} TPacketGCGlobalTime;

enum EChatType
{
	CHAT_TYPE_TALKING,
	CHAT_TYPE_INFO,
	CHAT_TYPE_NOTICE,
	CHAT_TYPE_PARTY,
	CHAT_TYPE_GUILD,
	CHAT_TYPE_COMMAND,
	CHAT_TYPE_SHOUT,
	CHAT_TYPE_WHISPER,
	CHAT_TYPE_BIG_NOTICE,
	CHAT_TYPE_MONARCH_NOTICE,
#ifdef ENABLE_DICE_SYSTEM
	CHAT_TYPE_DICE_INFO, //11
#endif
	CHAT_TYPE_MAX_NUM,
};

typedef struct packet_chatting
{
	BYTE	header;
	WORD	size;
	BYTE	type;
	DWORD	dwVID;
	BYTE	bEmpire;
} TPacketGCChat;

typedef struct packet_whisper
{
	BYTE        bHeader;
	WORD        wSize;
	BYTE        bType;
	char        szNameFrom[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCWhisper;

typedef struct packet_stun
{
	BYTE		header;
	DWORD		vid;
} TPacketGCStun;

typedef struct packet_dead
{
	BYTE		header;
	DWORD		vid;
} TPacketGCDead;

typedef struct packet_main_character
{
	BYTE        header;
	DWORD       dwVID;
	DWORD		wRaceNum; // @fixme501
	char        szName[CHARACTER_NAME_MAX_LEN + 1];
	LONG        lX, lY, lZ;
	BYTE		bySkillGroup;
} TPacketGCMainCharacter;

// SUPPORT_BGM
typedef struct packet_main_character2_empire
{
	BYTE        header;
	DWORD       dwVID;
	DWORD		wRaceNum; // @fixme501
	char        szName[CHARACTER_NAME_MAX_LEN + 1];
	LONG        lX, lY, lZ;
	BYTE		byEmpire;
	BYTE		bySkillGroup;
} TPacketGCMainCharacter2_EMPIRE;

typedef struct packet_main_character3_bgm
{
	enum
	{
		MUSIC_NAME_MAX_LEN = 24,
	};
	BYTE		header;
	DWORD		dwVID;
	DWORD		wRaceNum; // @fixme501
	char		szUserName[CHARACTER_NAME_MAX_LEN + 1];
	char		szBGMName[MUSIC_NAME_MAX_LEN + 1];
	LONG		lX, lY, lZ;
	BYTE		byEmpire;
	BYTE		bySkillGroup;
} TPacketGCMainCharacter3_BGM;

typedef struct packet_main_character4_bgm_vol
{
	enum
	{
		MUSIC_NAME_MAX_LEN = 24,
	};
	BYTE		header;
	DWORD		dwVID;
	DWORD		wRaceNum; // @fixme501
	char		szUserName[CHARACTER_NAME_MAX_LEN + 1];
	char		szBGMName[MUSIC_NAME_MAX_LEN + 1];
	float		fBGMVol;
	LONG		lX, lY, lZ;
	BYTE		byEmpire;
	BYTE		bySkillGroup;
} TPacketGCMainCharacter4_BGM_VOL;
// END_OF_SUPPORT_BGM

enum EPointTypes
{
	POINT_NONE,						// 0
	POINT_LEVEL,					// 1
	POINT_VOICE,					// 2
	POINT_EXP,						// 3
	POINT_NEXT_EXP,					// 4
	POINT_HP,						// 5
	POINT_MAX_HP,					// 6
	POINT_SP,						// 7
	POINT_MAX_SP,					// 8
	POINT_STAMINA,					// 9
	POINT_MAX_STAMINA,				// 10

	POINT_GOLD,						// 11
	POINT_ST,						// 12
	POINT_HT,						// 13
	POINT_DX,						// 14
	POINT_IQ,						// 15
	POINT_ATT_POWER,				// 16
	POINT_ATT_SPEED,				// 17
	POINT_EVADE_RATE,				// 18
	POINT_MOV_SPEED,				// 19
	POINT_DEF_GRADE,				// 20
	POINT_CASTING_SPEED,			// 21
	POINT_MAGIC_ATT_GRADE,			// 22
	POINT_MAGIC_DEF_GRADE,			// 23
	POINT_EMPIRE_POINT,				// 24
	POINT_LEVEL_STEP,				// 25
	POINT_STAT,						// 26
	POINT_SUB_SKILL,				// 27
	POINT_SKILL,					// 28
	POINT_MIN_ATK,					// 29
	POINT_MAX_ATK,					// 30
	POINT_PLAYTIME,					// 31
	POINT_HP_REGEN,					// 32
	POINT_SP_REGEN,					// 33

	POINT_BOW_DISTANCE,				// 34

	POINT_HP_RECOVERY,				// 35
	POINT_SP_RECOVERY,				// 36

	POINT_POISON_PCT,				// 37
	POINT_STUN_PCT,					// 38
	POINT_SLOW_PCT,					// 39
	POINT_CRITICAL_PCT,				// 40
	POINT_PENETRATE_PCT,			// 41
	POINT_CURSE_PCT,				// 42

	POINT_ATTBONUS_HUMAN,			// 43
	POINT_ATTBONUS_ANIMAL,			// 44
	POINT_ATTBONUS_ORC,				// 45
	POINT_ATTBONUS_MILGYO,			// 46
	POINT_ATTBONUS_UNDEAD,			// 47
	POINT_ATTBONUS_DEVIL,			// 48
	POINT_ATTBONUS_INSECT,			// 49
	POINT_ATTBONUS_FIRE,			// 50
	POINT_ATTBONUS_ICE,				// 51
	POINT_ATTBONUS_DESERT,			// 52
	POINT_ATTBONUS_MONSTER,			// 53
	POINT_ATTBONUS_WARRIOR,			// 54
	POINT_ATTBONUS_ASSASSIN,		// 55
	POINT_ATTBONUS_SURA,			// 56
	POINT_ATTBONUS_SHAMAN,			// 57
	POINT_ATTBONUS_TREE,			// 58

	POINT_RESIST_WARRIOR,			// 59
	POINT_RESIST_ASSASSIN,			// 60
	POINT_RESIST_SURA,				// 61
	POINT_RESIST_SHAMAN,			// 62

	POINT_STEAL_HP,					// 63
	POINT_STEAL_SP,					// 64

	POINT_MANA_BURN_PCT,			// 65

	POINT_DAMAGE_SP_RECOVER,		// 66

	POINT_BLOCK,					// 67
	POINT_DODGE,					// 68

	POINT_RESIST_SWORD,				// 69
	POINT_RESIST_TWOHAND,			// 70
	POINT_RESIST_DAGGER,			// 71
	POINT_RESIST_BELL,				// 72
	POINT_RESIST_FAN,				// 73
	POINT_RESIST_BOW,				// 74
	POINT_RESIST_FIRE,				// 75
	POINT_RESIST_ELEC,				// 76
	POINT_RESIST_MAGIC,				// 77
	POINT_RESIST_WIND,				// 78

	POINT_REFLECT_MELEE,			// 79

	POINT_REFLECT_CURSE,			// 80
	POINT_POISON_REDUCE,			// 81

	POINT_KILL_SP_RECOVER,			// 82
	POINT_EXP_DOUBLE_BONUS,			// 83
	POINT_GOLD_DOUBLE_BONUS,		// 84
	POINT_ITEM_DROP_BONUS,			// 85

	POINT_POTION_BONUS,				// 86
	POINT_KILL_HP_RECOVER,			// 87

	POINT_IMMUNE_STUN,				// 88
	POINT_IMMUNE_SLOW,				// 89
	POINT_IMMUNE_FALL,				// 90
	//////////////////

	POINT_PARTY_ATT_GRADE,			// 91
	POINT_PARTY_DEF_GRADE,			// 92

	POINT_ATT_BONUS,				// 93
	POINT_DEF_BONUS,				// 94

	POINT_ATT_GRADE_BONUS,			// 95
	POINT_DEF_GRADE_BONUS,			// 96
	POINT_MAGIC_ATT_GRADE_BONUS,	// 97
	POINT_MAGIC_DEF_GRADE_BONUS,	// 98

	POINT_RESIST_NORMAL_DAMAGE,		// 99

	POINT_STAT_RESET_COUNT			= 112,
	POINT_HORSE_SKILL				= 113,

	POINT_MALL_ATTBONUS,			// 114
	POINT_MALL_DEFBONUS,			// 115
	POINT_MALL_EXPBONUS,			// 116
	POINT_MALL_ITEMBONUS,			// 117
	POINT_MALL_GOLDBONUS,			// 118
	POINT_MAX_HP_PCT,				// 119
	POINT_MAX_SP_PCT,				// 120

	POINT_SKILL_DAMAGE_BONUS,		// 121
	POINT_NORMAL_HIT_DAMAGE_BONUS,	// 122

	POINT_SKILL_DEFEND_BONUS,		// 123
	POINT_NORMAL_HIT_DEFEND_BONUS,	// 124
	POINT_PC_BANG_EXP_BONUS,		// 125
	POINT_PC_BANG_DROP_BONUS,		// 126

	POINT_ENERGY					= 128,

	POINT_ENERGY_END_TIME			= 129,

	POINT_COSTUME_ATTR_BONUS		= 130,
	POINT_MAGIC_ATT_BONUS_PER		= 131,
	POINT_MELEE_MAGIC_ATT_BONUS_PER	= 132,

	POINT_RESIST_ICE				= 133,
	POINT_RESIST_EARTH				= 134,
	POINT_RESIST_DARK				= 135,

	POINT_RESIST_CRITICAL			= 136,
	POINT_RESIST_PENETRATE			= 137,

	#ifdef ENABLE_WOLFMAN_CHARACTER
	POINT_BLEEDING_REDUCE			= 138,
	POINT_BLEEDING_PCT				= 139,

	POINT_ATTBONUS_WOLFMAN			= 140,
	POINT_RESIST_WOLFMAN			= 141,
	POINT_RESIST_CLAW				= 142,
	#endif

	#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	POINT_ACCEDRAIN_RATE			= 143,
	#endif

	#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
	POINT_RESIST_MAGIC_REDUCTION	= 144,
	#endif

	POINT_ENCHANT_ELECT				= 145,
	POINT_ENCHANT_FIRE				= 146,
	POINT_ENCHANT_ICE				= 147,
	POINT_ENCHANT_WIND				= 148,
	POINT_ENCHANT_EARTH				= 149,
	POINT_ENCHANT_DARK				= 150,

	#ifdef ENABLE_AUTO_PICKUP_SYSTEM
	POINT_OTO_METIN					= 151,
	POINT_OTO_CANAVAR				= 152,
	POINT_OTO_PATRON				= 153,
	#endif

	#ifdef ENABLE_ANTI_EXP
	POINT_ANTI_EXP					= 154,
	#endif

	#ifdef ENABLE_NEW_ATTRIBUTE_TYPES
	POINT_ATTBONUS_STONE			= 155,
	POINT_ATTBONUS_BOSS				= 156,
	POINT_RESIST_HUMAN				= 157,
	POINT_RESIST_MONSTER			= 158,
	#endif

	// POINT_NONE					= 159,
	// POINT_NONE					= 160,

	#ifdef ENABLE_BATTLE_PASS
	POINT_BATTLE_PASS_ID			= 161,
	#endif

	#ifdef ENABLE_PVM_BONUS
	POINT_SKILL_DAMAGE_BONUS_PVM	= 162,
	POINT_HIT_DAMAGE_BONUS_PVM		= 163,
	POINT_ATT_GRADE_BONUS_PVM		= 164,
	POINT_DEF_GRADE_BONUS_PVM		= 165,
	POINT_BERSERKER_BONUS_PVM		= 166,
	#endif

	POINT_MIN_WEP = 200,
	POINT_MAX_WEP,
	POINT_MIN_MAGIC_WEP,
	POINT_MAX_MAGIC_WEP,
	POINT_HIT_RATE,

	POINT_MAX,
};

typedef struct packet_points
{
	BYTE		header;
	int64_t		points[POINT_MAX_NUM];
} TPacketGCPoints;

typedef struct packet_point_change
{
	int			header;

	DWORD		dwVID;
	BYTE		Type;

	int64_t		amount;
	int64_t		value;
} TPacketGCPointChange;

typedef struct packet_motion
{
	BYTE		header;
	DWORD		vid;
	DWORD		victim_vid;
	WORD		motion;
} TPacketGCMotion;

typedef struct packet_item_del_deprecated
{
	BYTE	header;
	TItemPos Cell;
} TPacketGCItemDelDeprecated;

typedef struct packet_set_item
{
	BYTE		header;
	TItemPos	Cell;
	DWORD		vnum;
	#ifdef ENABLE_NEW_MAX_COUNT
	short		count;
	#else
	BYTE		count;
	#endif
	DWORD		flags;
	DWORD		anti_flags;
	bool		highlight;
	LONG		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
	#ifdef ENABLE_EVOLUTION_SYSTEM
	int			iEvolution;
	#endif
	#ifdef ENABLE_CHANGELOOK_SYSTEM
	DWORD		dwTransmutation;
	#endif
	#ifdef ENABLE_LOCKED_SYSTEM
	BYTE		bLocked;
	#endif
} TPacketGCItemSet;

typedef struct packet_item_del
{
	BYTE	header;
	BYTE	pos;
} TPacketGCItemDel;

typedef struct packet_use_item
{
	BYTE		header;
	TItemPos	Cell;
	DWORD		ch_vid;
	DWORD		victim_vid;
	DWORD		vnum;
} TPacketGCItemUse;

typedef struct packet_update_item
{
	BYTE		header;
	TItemPos	Cell;
	#ifdef ENABLE_NEW_MAX_COUNT
	short		count;
	#else
	BYTE		count;
	#endif
	LONG		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
	#ifdef ENABLE_EVOLUTION_SYSTEM
	int			iEvolution;
	#endif
	#ifdef ENABLE_CHANGELOOK_SYSTEM
	DWORD		dwTransmutation;
	#endif
	#ifdef ENABLE_LOCKED_SYSTEM
	BYTE		bLocked;
	#endif
} TPacketGCItemUpdate;

typedef struct packet_ground_add_item
{
	BYTE		bHeader;
	LONG		lX;
	LONG		lY;
	LONG		lZ;

	DWORD		dwVID;
	DWORD		dwVnum;
} TPacketGCItemGroundAdd;

typedef struct packet_ground_del_item
{
	BYTE		header;
	DWORD		vid;
} TPacketGCItemGroundDel;

typedef struct packet_item_ownership
{
	BYTE		bHeader;
	DWORD		dwVID;
	char		szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCItemOwnership;

typedef struct packet_quickslot_add
{
	BYTE		header;
	BYTE		pos;
	TQuickSlot	slot;
} TPacketGCQuickSlotAdd;

typedef struct packet_quickslot_del
{
	BYTE		header;
	BYTE		pos;
} TPacketGCQuickSlotDel;

typedef struct packet_quickslot_swap
{
	BYTE		header;
	BYTE		pos;
	BYTE		change_pos;
} TPacketGCQuickSlotSwap;

typedef struct packet_shop_start
{
	struct packet_shop_item		items[SHOP_HOST_ITEM_MAX_NUM];
} TPacketGCShopStart;

typedef struct packet_shop_start_ex
{
	typedef struct sub_packet_shop_tab
	{
		char name[SHOP_TAB_NAME_MAX];
		BYTE coin_type;
		packet_shop_item items[SHOP_HOST_ITEM_MAX_NUM];
	} TSubPacketShopTab;
	DWORD owner_vid;
	BYTE shop_tab_count;
} TPacketGCShopStartEx;


typedef struct packet_shop_update_item
{
	BYTE						pos;
	struct packet_shop_item		item;
} TPacketGCShopUpdateItem;

typedef struct packet_shop_update_price
{
	int iElkAmount;
} TPacketGCShopUpdatePrice;

enum EPacketShopSubHeaders
{
	SHOP_SUBHEADER_GC_START,
	SHOP_SUBHEADER_GC_END,
	SHOP_SUBHEADER_GC_UPDATE_ITEM,
	SHOP_SUBHEADER_GC_UPDATE_PRICE,
	SHOP_SUBHEADER_GC_OK,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY,
	SHOP_SUBHEADER_GC_SOLDOUT,
	SHOP_SUBHEADER_GC_INVENTORY_FULL,
	SHOP_SUBHEADER_GC_INVALID_POS,
	SHOP_SUBHEADER_GC_SOLD_OUT,
	SHOP_SUBHEADER_GC_START_EX,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX,
};

typedef struct packet_shop
{
	BYTE		header;
	WORD		size;
	BYTE		subheader;
} TPacketGCShop;

typedef struct packet_exchange
{
	BYTE		header;
	BYTE		subheader;
	BYTE		is_me;
	#ifdef ENABLE_FULL_YANG
	uint64_t	arg1;
	#else
	DWORD		arg1;
	#endif
	TItemPos	arg2;
	DWORD		arg3;
	#ifdef ENABLE_TRADABLE_ICON
	TItemPos	arg4;
	#endif
	LONG		alValues[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
	#ifdef ENABLE_EVOLUTION_SYSTEM
	int			iEvolution;
	#endif
	#ifdef ENABLE_CHANGELOOK_SYSTEM
	DWORD		dwTransmutation;
	#endif
} TPacketGCExchange;

enum
{
	EXCHANGE_SUBHEADER_GC_START,			// arg1 == vid
	EXCHANGE_SUBHEADER_GC_ITEM_ADD,			// arg1 == vnum  arg2 == pos  arg3 == count
	EXCHANGE_SUBHEADER_GC_ITEM_DEL,			// arg1 == pos
	EXCHANGE_SUBHEADER_GC_ELK_ADD,			// arg1 == elk
	EXCHANGE_SUBHEADER_GC_ACCEPT,			// arg1 == accept
	EXCHANGE_SUBHEADER_GC_END,				// arg1 == not used
	EXCHANGE_SUBHEADER_GC_ALREADY,			// arg1 == not used
	EXCHANGE_SUBHEADER_GC_LESS_ELK,			// arg1 == not used
};

typedef struct packet_position
{
	BYTE		header;
	DWORD		vid;
	BYTE		position;
} TPacketGCPosition;

typedef struct packet_ping
{
	BYTE		header;
} TPacketGCPing;

typedef struct packet_pong
{
	BYTE		bHeader;
} TPacketCGPong;

typedef struct packet_script
{
	BYTE		header;
	WORD		size;
	BYTE		skin;
	WORD		src_size;
} TPacketGCScript;

typedef struct packet_target
{
	BYTE		header;
	DWORD		dwVID;
	BYTE		bHPPercent;
} TPacketGCTarget;

#ifdef ENABLE_SEND_TARGET_INFO
typedef struct SGCTargetItem
{
	DWORD	dwVnum;
	#ifdef ENABLE_NEW_MAX_COUNT
	short	iCount;
	#else
	BYTE	iCount;
	#endif
	LONG	lSocket;
} TGCTargetItem;

typedef struct packet_target_info
{
	BYTE	header;
	WORD	wSize;
	DWORD	dwVID;
	DWORD	race;
} TPacketGCTargetInfo;

typedef struct packet_target_info_load
{
	BYTE header;
	DWORD dwVID;
} TPacketCGTargetInfoLoad;
#endif

typedef struct packet_damage_info
{
	BYTE header;
	DWORD dwVID;
	BYTE flag;
	int  damage;
} TPacketGCDamageInfo;

typedef struct packet_mount
{
	BYTE		header;
	DWORD		vid;
	DWORD		mount_vid;
	BYTE		pos;
	DWORD		_x, _y;
} TPacketGCMount;

typedef struct packet_change_speed
{
	BYTE		header;
	DWORD		vid;
	WORD		moving_speed;
} TPacketGCChangeSpeed;

typedef struct packet_move
{
	BYTE		bHeader;
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	DWORD		dwVID;
	LONG		lX;
	LONG		lY;
	DWORD		dwTime;
	DWORD		dwDuration;
} TPacketGCMove;

enum
{
	QUEST_SEND_IS_BEGIN			= 1 << 0,
	QUEST_SEND_TITLE			= 1 << 1,
	QUEST_SEND_CLOCK_NAME		= 1 << 2,
	QUEST_SEND_CLOCK_VALUE		= 1 << 3,
	QUEST_SEND_COUNTER_NAME		= 1 << 4,
	QUEST_SEND_COUNTER_VALUE	= 1 << 5,
	QUEST_SEND_ICON_FILE		= 1 << 6,
};

typedef struct packet_quest_info
{
	BYTE header;
	WORD size;
	WORD index;
	BYTE flag;
} TPacketGCQuestInfo;

typedef struct packet_quest_confirm
{
	BYTE header;
	char msg[64+1];
	LONG timeout;
	DWORD requestPID;
} TPacketGCQuestConfirm;

typedef struct packet_attack
{
	BYTE		header;
	DWORD		dwVID;
	DWORD		dwVictimVID;
	BYTE		bType;
} TPacketGCAttack;

typedef struct packet_c2c
{
	BYTE		header;
	WORD		wSize;
} TPacketGCC2C;

typedef struct packetd_sync_position_element
{
	DWORD		dwVID;
	LONG		lX;
	LONG		lY;
} TPacketGCSyncPositionElement;

typedef struct packetd_sync_position
{
	BYTE		bHeader;
	WORD		wSize;
} TPacketGCSyncPosition;

typedef struct packet_ownership
{
	BYTE		bHeader;
	DWORD		dwOwnerVID;
	DWORD		dwVictimVID;
} TPacketGCOwnership;

#define	SKILL_MAX_NUM 255

typedef struct packet_skill_level
{
	BYTE		bHeader;
	BYTE		abSkillLevels[SKILL_MAX_NUM];
} TPacketGCSkillLevel;

typedef struct SPlayerSkill
{
	BYTE bMasterType;
	BYTE bLevel;
	LONG tNextRead;
} TPlayerSkill;

typedef struct packet_skill_level_new
{
	BYTE bHeader;
	TPlayerSkill skills[SKILL_MAX_NUM];
} TPacketGCSkillLevelNew;

// fly
typedef struct packet_fly
{
	BYTE		bHeader;
	BYTE		bType;
	DWORD		dwStartVID;
	DWORD		dwEndVID;
} TPacketGCCreateFly;

enum EPVPModes
{
	PVP_MODE_NONE,
	PVP_MODE_AGREE,
	PVP_MODE_FIGHT,
	PVP_MODE_REVENGE,
};

typedef struct packet_duel_start
{
	BYTE		header ;
	WORD		wSize ;
} TPacketGCDuelStart ;

typedef struct packet_pvp
{
	BYTE		header;
	DWORD		dwVIDSrc;
	DWORD		dwVIDDst;
	BYTE		bMode;
} TPacketGCPVP;

typedef struct packet_skill_cooltime_end
{
	BYTE		header;
	BYTE		bSkill;
} TPacketGCSkillCoolTimeEnd;

typedef struct packet_warp
{
	BYTE			bHeader;
	LONG			lX;
	LONG			lY;
	LONG			lAddr;
	WORD			wPort;
} TPacketGCWarp;

typedef struct packet_party_invite
{
	BYTE header;
	DWORD leader_pid;
} TPacketGCPartyInvite;

typedef struct packet_party_add
{
	BYTE header;
	DWORD pid;
	char name[CHARACTER_NAME_MAX_LEN+1];
} TPacketGCPartyAdd;

typedef struct packet_party_update
{
	BYTE header;
	DWORD pid;
	BYTE state;
	BYTE percent_hp;
	short affects[PARTY_AFFECT_SLOT_MAX_NUM];
} TPacketGCPartyUpdate;

typedef struct packet_party_remove
{
	BYTE header;
	DWORD pid;
} TPacketGCPartyRemove;

typedef TPacketCGSafeboxCheckout TPacketGCSafeboxCheckout;
typedef TPacketCGSafeboxCheckin TPacketGCSafeboxCheckin;

typedef struct packet_safebox_wrong_password
{
	BYTE        bHeader;
} TPacketGCSafeboxWrongPassword;

typedef struct packet_safebox_size
{
	BYTE bHeader;
	BYTE bSize;
} TPacketGCSafeboxSize;

typedef struct packet_safebox_money_change
{
	BYTE bHeader;
	DWORD dwMoney;
} TPacketGCSafeboxMoneyChange;

typedef struct command_empire
{
	BYTE        bHeader;
	BYTE        bEmpire;
} TPacketCGEmpire;

typedef struct packet_empire
{
	BYTE        bHeader;
	BYTE        bEmpire;
} TPacketGCEmpire;

enum
{
	FISHING_SUBHEADER_GC_START,
	FISHING_SUBHEADER_GC_STOP,
	FISHING_SUBHEADER_GC_REACT,
	FISHING_SUBHEADER_GC_SUCCESS,
	FISHING_SUBHEADER_GC_FAIL,
	FISHING_SUBHEADER_GC_FISH,
};

typedef struct packet_fishing
{
	BYTE header;
	BYTE subheader;
	DWORD info;
	BYTE dir;
} TPacketGCFishing;

typedef struct paryt_parameter
{
	BYTE        bHeader;
	BYTE        bDistributeMode;
} TPacketGCPartyParameter;

//////////////////////////////////////////////////////////////////////////
// Guild

enum
{
	GUILD_SUBHEADER_GC_LOGIN,
	GUILD_SUBHEADER_GC_LOGOUT,
	GUILD_SUBHEADER_GC_LIST,
	GUILD_SUBHEADER_GC_GRADE,
	GUILD_SUBHEADER_GC_ADD,
	GUILD_SUBHEADER_GC_REMOVE,
	GUILD_SUBHEADER_GC_GRADE_NAME,
	GUILD_SUBHEADER_GC_GRADE_AUTH,
	GUILD_SUBHEADER_GC_INFO,
	GUILD_SUBHEADER_GC_COMMENTS,
	GUILD_SUBHEADER_GC_CHANGE_EXP,
	GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE,
	GUILD_SUBHEADER_GC_SKILL_INFO,
	GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL,
	GUILD_SUBHEADER_GC_GUILD_INVITE,
	GUILD_SUBHEADER_GC_WAR,
	GUILD_SUBHEADER_GC_GUILD_NAME,
	GUILD_SUBHEADER_GC_GUILD_WAR_LIST,
	GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST,
	GUILD_SUBHEADER_GC_WAR_POINT,
	GUILD_SUBHEADER_GC_MONEY_CHANGE,
};

typedef struct packet_guild
{
	BYTE header;
	WORD size;
	BYTE subheader;
} TPacketGCGuild;

// SubHeader - Grade
enum
{
	GUILD_AUTH_ADD_MEMBER       = (1 << 0),
	GUILD_AUTH_REMOVE_MEMBER    = (1 << 1),
	GUILD_AUTH_NOTICE           = (1 << 2),
	GUILD_AUTH_SKILL            = (1 << 3),
};

typedef struct packet_guild_sub_grade
{
	char grade_name[GUILD_GRADE_NAME_MAX_LEN+1];
	BYTE auth_flag;
} TPacketGCGuildSubGrade;

typedef struct packet_guild_sub_member
{
	DWORD pid;
	BYTE byGrade;
	BYTE byIsGeneral;
	BYTE byJob;
	BYTE byLevel;
	DWORD dwOffer;
	BYTE byNameFlag;
// if NameFlag is TRUE, name is sent from server.
//	char szName[CHARACTER_ME_MAX_LEN+1];
} TPacketGCGuildSubMember;

typedef struct packet_guild_sub_info
{
	WORD member_count;
	WORD max_member_count;
	DWORD guild_id;
	DWORD master_pid;
	DWORD exp;
	BYTE level;
	char name[GUILD_NAME_MAX_LEN+1];
	DWORD gold;
	BYTE hasLand;
} TPacketGCGuildInfo;

enum EGuildWarState
{
	GUILD_WAR_NONE,
	GUILD_WAR_SEND_DECLARE,
	GUILD_WAR_REFUSE,
	GUILD_WAR_RECV_DECLARE,
	GUILD_WAR_WAIT_START,
	GUILD_WAR_CANCEL,
	GUILD_WAR_ON_WAR,
	GUILD_WAR_END,

	GUILD_WAR_DURATION = 2*60*60,
};

typedef struct packet_guild_war
{
	DWORD	dwGuildSelf;
	DWORD	dwGuildOpp;
	BYTE	bType;
	BYTE	bWarState;
	#ifdef ENABLE_GUILD_WAR_OPTION_SYSTEM
	int		iScore;
	int		iMinLevel;
	int		iMaxPlayer;
	bool	bWarrior;
	bool	bAssassin;
	bool	bSura;
	bool	bShaman;
	#endif
} TPacketGCGuildWar;

typedef struct SPacketGuildWarPoint
{
	DWORD dwGainGuildID;
	DWORD dwOpponentGuildID;
	LONG lPoint;
} TPacketGuildWarPoint;

// SubHeader - Dungeon
enum
{
	DUNGEON_SUBHEADER_GC_TIME_ATTACK_START = 0,
	DUNGEON_SUBHEADER_GC_DESTINATION_POSITION = 1,
};

typedef struct packet_dungeon
{
	BYTE		bHeader;
	WORD		size;
	BYTE		subheader;
} TPacketGCDungeon;

// Private Shop
typedef struct SPacketGCShopSign
{
	BYTE        bHeader;
	DWORD       dwVID;
	char        szSign[SHOP_SIGN_MAX_LEN + 1];
} TPacketGCShopSign;

typedef struct SPacketGCTime
{
	BYTE        bHeader;
	LONG      time;
} TPacketGCTime;

enum
{
	WALKMODE_RUN,
	WALKMODE_WALK,
};

typedef struct SPacketGCWalkMode
{
	BYTE        header;
	DWORD       vid;
	BYTE        mode;
} TPacketGCWalkMode;

typedef struct SPacketGCChangeSkillGroup
{
	BYTE        header;
	BYTE        skill_group;
} TPacketGCChangeSkillGroup;

struct TMaterial
{
	DWORD vnum;
	DWORD count;
};

typedef struct SRefineTable
{
	DWORD src_vnum;
	DWORD result_vnum;
	#ifdef ENABLE_NEW_MAX_COUNT
	short material_count;
	#else
	BYTE material_count;
	#endif
	#ifdef ENABLE_FULL_YANG
	int64_t cost;
	#else
	int cost;
	#endif
	int prob;
	TMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TRefineTable;

typedef struct SPacketGCRefineInformation
{
	BYTE			header;
	BYTE			pos;
	TRefineTable	refine_table;
} TPacketGCRefineInformation;

typedef struct SPacketGCRefineInformationNew
{
	BYTE			header;
	BYTE			type;
	BYTE			pos;
	TRefineTable	refine_table;
} TPacketGCRefineInformationNew;

enum SPECIAL_EFFECT
{
	SE_NONE,
	SE_HPUP_RED,
	SE_SPUP_BLUE,
	SE_SPEEDUP_GREEN,
	SE_DXUP_PURPLE,
	SE_CRITICAL,
	SE_PENETRATE,
	SE_BLOCK,
	SE_DODGE,
	SE_CHINA_FIREWORK,
	SE_SPIN_TOP,
	SE_SUCCESS,
	SE_FAIL,
	SE_FR_SUCCESS,
	SE_LEVELUP_ON_14_FOR_GERMANY,
	SE_LEVELUP_UNDER_15_FOR_GERMANY,
	SE_PERCENT_DAMAGE1,
	SE_PERCENT_DAMAGE2,
	SE_PERCENT_DAMAGE3,
	SE_AUTO_HPUP,
	SE_AUTO_SPUP,
	SE_EQUIP_RAMADAN_RING,
	SE_EQUIP_HALLOWEEN_CANDY,
	SE_EQUIP_HAPPINESS_RING,
	SE_EQUIP_LOVE_PENDANT,
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	SE_EFFECT_ACCE_SUCCEDED,
	SE_EFFECT_ACCE_EQUIP,
#endif
#ifdef ENABLE_SMITH_EFFECT_SYSTEM
	SE_FR_FAIL,
#endif
};

typedef struct SPacketGCSpecialEffect
{
	BYTE header;
	BYTE type;
	DWORD vid;
} TPacketGCSpecialEffect;

typedef struct SPacketGCNPCPosition
{
	BYTE header;
	WORD size;
	WORD count;
} TPacketGCNPCPosition;

#ifdef ENABLE_DUNGEON_BOSS_ICON_IN_MAP
struct TNPCPosition
{
	BYTE bType;
	DWORD vnum;
	LONG x, y;
	DWORD time;
};
#else
struct TNPCPosition
{
	BYTE bType;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	LONG x;
	LONG y;
};
#endif
typedef struct SPacketGCChangeName
{
	BYTE header;
	DWORD pid;
	char name[CHARACTER_NAME_MAX_LEN+1];
} TPacketGCChangeName;

enum EBlockAction
{
	BLOCK_EXCHANGE              = (1 << 0),
	BLOCK_PARTY_INVITE          = (1 << 1),
	BLOCK_GUILD_INVITE          = (1 << 2),
	BLOCK_WHISPER               = (1 << 3),
	BLOCK_MESSENGER_INVITE      = (1 << 4),
	BLOCK_PARTY_REQUEST         = (1 << 5),
};

typedef struct packet_login_key
{
	BYTE	bHeader;
	DWORD	dwLoginKey;
} TPacketGCLoginKey;

typedef struct packet_auth_success
{
	BYTE        bHeader;
	DWORD       dwLoginKey;
	BYTE        bResult;
} TPacketGCAuthSuccess;

typedef struct packet_channel
{
	BYTE header;
	BYTE channel;
} TPacketGCChannel;

typedef struct SEquipmentItemSet
{
	DWORD	vnum;
	#ifdef ENABLE_NEW_MAX_COUNT
	short	count;
	#else
	BYTE	count;
	#endif
	LONG	alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TEquipmentItemSet;

typedef struct pakcet_view_equip
{
	BYTE header;
	DWORD dwVID;
	TEquipmentItemSet equips[WEAR_MAX_NUM];
} TPacketGCViewEquip;

typedef struct SLandPacketElement
{
	DWORD		dwID;
	LONG		x, y;
	LONG		width, height;
	DWORD		dwGuildID;
} TLandPacketElement;

typedef struct packet_land_list
{
	BYTE		header;
	WORD		size;
} TPacketGCLandList;

typedef struct SPacketGCTargetCreate
{
	BYTE		bHeader;
	LONG		lID;
	char		szTargetName[32+1];
} TPacketGCTargetCreate;

enum
{
	CREATE_TARGET_TYPE_NONE,
	CREATE_TARGET_TYPE_LOCATION,
	CREATE_TARGET_TYPE_CHARACTER,
};

typedef struct SPacketGCTargetCreateNew
{
	BYTE		bHeader;
	LONG		lID;
	char		szTargetName[32+1];
	DWORD		dwVID;
	BYTE		byType;
} TPacketGCTargetCreateNew;

typedef struct SPacketGCTargetUpdate
{
	BYTE        bHeader;
	LONG        lID;
	LONG        lX, lY;
} TPacketGCTargetUpdate;

typedef struct SPacketGCTargetDelete
{
	BYTE        bHeader;
	LONG        lID;
} TPacketGCTargetDelete;

typedef struct SPacketAffectElement
{
	DWORD       dwType;
	BYTE        bPointIdxApplyOn;
	LONG        lApplyValue;
	DWORD       dwFlag;
	LONG        lDuration;
	LONG        lSPCost;
} TPacketAffectElement;

typedef struct SPacketGCAffectAdd
{
	BYTE bHeader;
	TPacketAffectElement elem;
} TPacketGCAffectAdd;

typedef struct SPacketGCAffectRemove
{
	BYTE bHeader;
	DWORD dwType;
	BYTE bApplyOn;
} TPacketGCAffectRemove;

typedef struct packet_mall_open
{
	BYTE bHeader;
	BYTE bSize;
} TPacketGCMallOpen;

typedef struct packet_lover_info
{
	BYTE bHeader;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	BYTE byLovePoint;
} TPacketGCLoverInfo;

typedef struct packet_love_point_update
{
	BYTE bHeader;
	BYTE byLovePoint;
} TPacketGCLovePointUpdate;

typedef struct packet_dig_motion
{
	BYTE header;
	DWORD vid;
	DWORD target_vid;
	BYTE count;
} TPacketGCDigMotion;

typedef struct SPacketGCOnTime
{
	BYTE header;
	int ontime;     // sec
} TPacketGCOnTime;

typedef struct SPacketGCResetOnTime
{
	BYTE header;
} TPacketGCResetOnTime;

typedef struct SPacketGCPanamaPack
{
	BYTE    bHeader;
	char    szPackName[256];
	BYTE    abIV[32];
} TPacketGCPanamaPack;

typedef struct SPacketGCHybridCryptKeys
{
private:
	SPacketGCHybridCryptKeys() : m_pStream(NULL) {}

public:
	SPacketGCHybridCryptKeys(int iStreamSize) : iKeyStreamLen(iStreamSize)
	{
		m_pStream = new BYTE[iStreamSize];
	}
	~SPacketGCHybridCryptKeys()
	{
		if( m_pStream )
		{
			delete[] m_pStream;
			m_pStream = NULL;
		}
	}
	static int GetFixedHeaderSize()
	{
		return sizeof(BYTE)+sizeof(WORD)+sizeof(int);
	}

	BYTE	bHeader;
	WORD    wDynamicPacketSize;
	int		iKeyStreamLen;
	BYTE*	m_pStream;

} TPacketGCHybridCryptKeys;


typedef struct SPacketGCHybridSDB
{
private:
	SPacketGCHybridSDB() : m_pStream(NULL) {}

public:
	SPacketGCHybridSDB(int iStreamSize) : iSDBStreamLen(iStreamSize)
	{
		m_pStream = new BYTE[iStreamSize];
	}
	~SPacketGCHybridSDB()
	{
		delete[] m_pStream;
		m_pStream = NULL;
	}
	static int GetFixedHeaderSize()
	{
		return sizeof(BYTE)+sizeof(WORD)+sizeof(int);
	}

	BYTE	bHeader;
	WORD    wDynamicPacketSize;
	int		iSDBStreamLen;
	BYTE*	m_pStream;

} TPacketGCHybridSDB;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Client To Client

typedef struct packet_state
{
	BYTE			bHeader;
	BYTE			bFunc;
	BYTE			bArg;
	BYTE			bRot;
	DWORD			dwVID;
	DWORD			dwTime;
	TPixelPosition	kPPos;
} TPacketCCState;

// AUTOBAN
typedef struct packet_autoban_quiz
{
	BYTE bHeader;
	BYTE bDuration;
	BYTE bCaptcha[64*32];
	char szQuiz[256];
} TPacketGCAutoBanQuiz;
// END_OF_AUTOBAN

#ifdef _IMPROVED_PACKET_ENCRYPTION_
struct TPacketKeyAgreement
{
	static const int MAX_DATA_LEN = 256;
	BYTE bHeader;
	WORD wAgreedLength;
	WORD wDataLength;
	BYTE data[MAX_DATA_LEN];
};

struct TPacketKeyAgreementCompleted
{
	BYTE bHeader;
	BYTE data[3]; // dummy (not used)
};
#endif // _IMPROVED_PACKET_ENCRYPTION_

typedef struct SPacketGCSpecificEffect
{
	BYTE header;
	DWORD vid;
	char effect_file[128];
} TPacketGCSpecificEffect;

enum EDragonSoulRefineWindowRefineType
{
	DragonSoulRefineWindow_UPGRADE,
	DragonSoulRefineWindow_IMPROVEMENT,
	DragonSoulRefineWindow_REFINE,
};

enum EPacketCGDragonSoulSubHeaderType
{
	DS_SUB_HEADER_OPEN,
	DS_SUB_HEADER_CLOSE,
	DS_SUB_HEADER_DO_UPGRADE,
	DS_SUB_HEADER_DO_IMPROVEMENT,
	DS_SUB_HEADER_DO_REFINE,
	DS_SUB_HEADER_REFINE_FAIL,
	DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE,
	DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL,
	DS_SUB_HEADER_REFINE_SUCCEED,
};

typedef struct SPacketCGDragonSoulRefine
{
	SPacketCGDragonSoulRefine() : header (HEADER_CG_DRAGON_SOUL_REFINE)
	{}
	BYTE header;
	BYTE bSubType;
	TItemPos ItemGrid[DS_REFINE_WINDOW_MAX_NUM];
} TPacketCGDragonSoulRefine;

typedef struct SPacketGCDragonSoulRefine
{
	SPacketGCDragonSoulRefine() : header(HEADER_GC_DRAGON_SOUL_REFINE)
	{}
	BYTE header;
	BYTE bSubType;
	TItemPos Pos;
} TPacketGCDragonSoulRefine;

typedef struct SChannelStatus
{
	WORD nPort; // @fixme024 (max port limit 32767 to 65535)
	BYTE bStatus;
} TChannelStatus;

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
enum EAcceInfo
{
	ACCE_ABSORPTION_SOCKET = 0,
	ACCE_ABSORBED_SOCKET = 1,
	ACCE_CLEAN_ATTR_VALUE0 = 7,
	ACCE_WINDOW_MAX_MATERIALS = 2,
	ACCE_REVERSAL_VNUM_1 = 39046,
	ACCE_REVERSAL_VNUM_2 = 90000,
};

enum
{
	ACCE_SUBHEADER_GC_OPEN = 0,
	ACCE_SUBHEADER_GC_CLOSE,
	ACCE_SUBHEADER_GC_ADDED,
	ACCE_SUBHEADER_GC_REMOVED,
	ACCE_SUBHEADER_CG_REFINED,
	ACCE_SUBHEADER_CG_CLOSE = 0,
	ACCE_SUBHEADER_CG_ADD,
	ACCE_SUBHEADER_CG_REMOVE,
	ACCE_SUBHEADER_CG_REFINE,
};

typedef struct SPacketAcce
{
	BYTE	header;
	BYTE	subheader;
	bool	bWindow;
	DWORD	dwPrice;
	BYTE	bPos;
	TItemPos	tPos;
	DWORD	dwItemVnum;
	DWORD	dwMinAbs;
	DWORD	dwMaxAbs;
} TPacketAcce;

typedef struct SAcceMaterial
{
	BYTE	bHere;
	WORD	wCell;
} TAcceMaterial;

typedef struct SAcceResult
{
	DWORD	dwItemVnum;
	DWORD	dwMinAbs;
	DWORD	dwMaxAbs;
} TAcceResult;
#endif

#ifdef ENABLE_IKASHOP_RENEWAL
//ACTIONS PACKETS
typedef struct
{
	BYTE header;
	#ifdef ENABLE_LARGE_DYNAMIC_PACKETS
	int size;
	#else
	WORD size;
	#endif
	BYTE subheader;
} TPacketGCNewOfflineshop;

typedef struct
{
	BYTE header;
	WORD size;
	BYTE subheader;
} TPacketCGNewOfflineShop;

namespace ikashop
{
	typedef struct SPlayerItem
	{
		DWORD id;
		BYTE window;
		WORD pos;
		DWORD count;
		DWORD vnum;
		LONG alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
		TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
		DWORD owner;
		#ifdef ENABLE_EVOLUTION_SYSTEM
		int evolution;
		#endif
		#ifdef ENABLE_CHANGELOOK_SYSTEM
		DWORD transmutation;
		#endif
		#ifdef ENABLE_LOCKED_SYSTEM
		BYTE locked;
		#endif
	} TPlayerItem;

	//patch 08-03-2020
	enum class ExpirationType
	{
		EXPIRE_NONE,
		EXPIRE_REAL_TIME,
		EXPIRE_REAL_TIME_FIRST_USE,
	};

	struct TPriceInfo
	{
		long long yang = 0;
		#ifdef ENABLE_CHEQUE_SYSTEM
		int cheque = 0;
		#endif
		long long GetTotalYangAmount() const {
			return yang
				#ifdef ENABLE_CHEQUE_SYSTEM
				+ static_cast<long long>(cheque) * YANG_PER_CHEQUE
				#endif
				;
		}
	};

	struct TShopPlayerItem : public TPlayerItem
	{
		ExpirationType expiration;
	};

	struct TShopItem : public TShopPlayerItem
	{
		TPriceInfo price;
	};

	struct TOfferInfo
	{
		DWORD offerid;
		DWORD ownerid;
		DWORD itemid;
		DWORD buyerid;
		TPriceInfo price;
		bool notified;
		bool accepted;
		char buyername[CHARACTER_NAME_MAX_LEN + 1];
		int64_t datetime;
	};

	#ifdef ENABLE_IKASHOP_ENTITIES
	struct TShopSpawn
	{
		int x;
		int y;
		int map;
		BYTE bChannel;
	};
	#endif

	//AUCTION
	struct TAuctionInfo : public TShopItem
	{
		char  ownername[CHARACTER_NAME_MAX_LEN + 1];
		DWORD duration;
	};

	struct TAuctionOfferInfo
	{
		TPriceInfo	price;
		DWORD ownerid;
		DWORD buyerid;
		char buyername[CHARACTER_NAME_MAX_LEN + 1];
		bool seenflag;
		int64_t datetime;
	};

	struct TValutesInfo
	{
		long long yang = 0;

		void operator +=(const TValutesInfo& r)
		{
			yang += r.yang;
		}

		void operator -=(const TValutesInfo& r)
		{
			yang -= r.yang;
		}
	};

	struct TShopInfo
	{
		DWORD ownerid;
		DWORD duration;
		char name[OFFLINE_SHOP_NAME_MAX_LEN];
		DWORD count;
		#ifdef ENABLE_IKASHOP_ENTITIES
		TShopSpawn spawn;
		#endif
	};

	struct TFilterInfo
	{
		BYTE type;
		BYTE subtype;
		char name[CItemData::ITEM_NAME_MAX_LEN];
		TPriceInfo pricestart;
		TPriceInfo pricend;
		int levelstart;
		int levelend;
		#ifdef ENABLE_NEW_MAX_COUNT
		short countstart;
		short countend;
		#else
		BYTE countstart;
		BYTE countend;
		#endif
		TPlayerItemAttribute attrs[ITEM_ATTRIBUTE_SLOT_NORM_NUM];
		int sashGrade;
		int alchemyGrade;
	};

	struct TShopItemInfo
	{
		TItemPos pos;
		int destpos;
		TPriceInfo price;
	};

	struct TSearchResultInfo : public ikashop::TShopItem
	{
		bool auction;
		char seller_name[OFFLINE_SHOP_NAME_MAX_LEN];
		int duration;
	};

	#ifdef ENABLE_SHOP_SEARCH_SYSTEM
	struct TFilterName
	{
		char szName[CItemData::ITEM_NAME_MAX_LEN];
		DWORD dwVnum;
		LONG lSocket;
	};

	struct TSearchNameResultInfo
	{
		DWORD dwOwnerID;
		DWORD dwShopVID;
	};
	#endif

	//AUCTION
	struct TAuctionListElement
	{
		TAuctionInfo auction;
		TPriceInfo actual_best;
		DWORD offercount;
	};

	struct TOfferListElement : public TOfferInfo
	{
		TShopItem item;
		char shopname[OFFLINE_SHOP_NAME_MAX_LEN];
		bool incoming;
	};

	//GAME TO CLIENT
	enum eSubHeaderGC
	{
		SUBHEADER_GC_SHOP_OPEN,
		SUBHEADER_GC_SHOP_OPEN_OWNER,
		SUBHEADER_GC_SHOP_OPEN_OWNER_NO_SHOP,
		SUBHEADER_GC_SHOP_EXPIRED_GUESTING,
		SUBHEADER_GC_SHOP_REMOVE_ITEM_OWNER,
		SUBHEADER_GC_SHOP_REMOVE_ITEM_GUEST,
		SUBHEADER_GC_SHOP_EDIT_ITEM_OWNER,
		SUBHEADER_GC_SHOP_EDIT_ITEM_GUEST,

		SUBHEADER_GC_OFFER_LIST,

		SUBHEADER_GC_SHOP_FILTER_RESULT,

		#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		SUBHEADER_GC_SHOP_FILTER_NAME_RESULT,
		#endif

		SUBHEADER_GC_SHOP_SAFEBOX_REFRESH,
		SUBHEADER_GC_SHOP_SAFEBOX_REMOVE_ITEM,
		SUBHEADER_GC_SHOP_SAFEBOX_ADD_ITEM,

		//AUCTION
		SUBHEADER_GC_AUCTION_LIST,
		SUBHEADER_GC_OPEN_MY_AUCTION,
		SUBHEADER_GC_OPEN_MY_AUCTION_NO_AUCTION,
		SUBHEADER_GC_OPEN_AUCTION,

		SUBHEDAER_GC_POPUP_MESSAGE,

		#ifdef ENABLE_IKASHOP_ENTITIES
		SUBHEADER_GC_INSERT_SHOP_ENTITY,
		SUBHEADER_GC_REMOVE_SHOP_ENTITY,
		#endif

		SUBHEADER_GC_BOARD_COUNTERS,
		SUBHEADER_GC_SEARCH_RESULT_DELETE,

		#ifdef ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE
		SUBHEADER_GC_SHOP_AVARAGE_AND_MINMAX_PRICE,
		#endif
	};

	struct TSubPacketGCShopOpen
	{
		TShopInfo shop;
	};

	struct TSubPacketGCShopOpenOwner
	{
		TShopInfo shop;
	};

	struct TSubPacketGCShopExpiredGuesting
	{
		DWORD ownerid;
	};

	struct TSubPacketGCShopRemoveItem
	{
		DWORD itemid;
	};

	struct TSubPacketGCShopEditItem
	{
		DWORD itemid;
		TPriceInfo price;
	};

	struct TSubPacketGCShopFilterResult
	{
		DWORD count;
	};

	struct TSubPacketGCShopOfferList
	{
		DWORD offercount;
	};

	struct TSubPacketGCShopSafeboxRefresh
	{
		TValutesInfo valute;
		DWORD itemcount;
	};

	struct TSubPacketGCShopSafeboxRemoveItem
	{
		DWORD itemid;
	};

	struct TSubPacketGCShoppingSafeboxAddItem
	{
		TShopPlayerItem item;
	};

	//AUCTION
	struct TSubPacketGCAuctionList
	{
		DWORD count;
	};

	struct TSubPacketGCAuctionOpen
	{
		TAuctionInfo auction;
		DWORD offercount;
	};

	struct TSubPacketGCPopupMessage
	{
		static constexpr inline auto MESSAGE_LEN = 80;
		char localeString[MESSAGE_LEN];
	};

	#ifdef ENABLE_IKASHOP_ENTITIES
	struct TSubPacketGCInsertShopEntity
	{
		DWORD vid;
		char name[OFFLINE_SHOP_NAME_MAX_LEN];
		int type;
		int x, y, z;
	};

	struct TSubPacketGCRemoveShopEntity
	{
		DWORD vid;
	};
	#endif

	struct TSubPacketGCBoardCounters
	{
		int safebox;
		int offers;
		int auction;
	};

	// CLIENT TO GAME
	enum eSubHeaderCG
	{
		SUBHEADER_CG_SHOP_CREATE_NEW,
		SUBHEADER_CG_SHOP_FORCE_CLOSE,
		SUBHEADER_CG_SHOP_REQUEST_SHOPLIST,
		SUBHEADER_CG_SHOP_OPEN,
		SUBHEADER_CG_SHOP_OPEN_OWNER,
		SUBHEADER_CG_SHOP_BUY_ITEM,

		SUBHEADER_CG_SHOP_ADD_ITEM,
		SUBHEADER_CG_SHOP_REMOVE_ITEM,
		SUBHEADER_CG_SHOP_EDIT_ITEM,

		SUBHEADER_CG_SHOP_FILTER_REQUEST,
		SUBHEADER_CG_SHOP_SEARCH_FILL_REQUEST,

		#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		SUBHEADER_CG_SHOP_FILTER_NAME_REQUEST,
		#endif

		SUBHEADER_CG_SHOP_OFFER_CREATE,
		SUBHEADER_CG_SHOP_OFFER_ACCEPT,
		SUBHEADER_CG_SHOP_OFFER_CANCEL,
		SUBHEADER_CG_SHOP_REQUEST_OFFER_LIST,

		SUBHEADER_CG_SHOP_SAFEBOX_OPEN,
		SUBHEADER_CG_SHOP_SAFEBOX_GET_ITEM,
		SUBHEADER_CG_SHOP_SAFEBOX_GET_VALUTES,
		SUBHEADER_CG_SHOP_SAFEBOX_CLOSE,

		//AUCTION
		SUBHEADER_CG_AUCTION_LIST_REQUEST,
		SUBHEADER_CG_AUCTION_OPEN_REQUEST,
		SUBHEADER_CG_MY_AUCTION_OPEN_REQUEST,
		SUBHEADER_CG_MY_AUCTION_CLOSE,
		SUBHEADER_CG_MY_AUCTION_CANCEL,
		SUBHEADER_CG_CREATE_AUCTION,
		SUBHEADER_CG_AUCTION_ADD_OFFER,
		SUBHEADER_CG_EXIT_FROM_AUCTION,

		SUBHEADER_CG_CLOSE_MY_SHOP_BOARD,
		SUBHEADER_CG_CLOSE_OFFER_LIST,
		SUBHEADER_CG_CLOSE_SHOP_GUEST,

		#ifdef ENABLE_IKASHOP_ENTITIES
		SUBHEADER_CG_CLICK_ENTITY,
		SUBHEADER_CG_MOVE_SHOP_ENTITY,
		#endif

		#ifdef ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE
		SUBHEADER_CG_SHOP_AVARAGE_AND_MINMAX_PRICE,
		#endif
	};

	struct TSubPacketCGShopCreate
	{
		TShopInfo shop;
	};

	struct TSubPacketCGShopOpen
	{
		DWORD ownerid;
	};

	struct TSubPacketCGAddItem
	{
		TItemPos pos;
		TPriceInfo price;
		int destpos;
	};

	struct TSubPacketCGRemoveItem
	{
		DWORD itemid;
	};

	struct TSubPacketCGEditItem
	{
		DWORD itemid;
		TPriceInfo price;
	};

	struct TSubPacketCGFilterRequest
	{
		TFilterInfo filter;
	};

	#ifdef ENABLE_SHOP_SEARCH_SYSTEM
	struct TSubPacketCGFilterNameRequest
	{
		TFilterName filter;
	};
	#endif

	struct TSubPacketCGOfferCreate
	{
		TOfferInfo offer;
	};

	struct TSubPacketCGOfferAccept
	{
		DWORD offerid;
	};

	struct TSubPacketCGOfferCancel
	{
		DWORD offerid;
		DWORD ownerid;
	};

	struct TSubPacketCGShopSafeboxGetItem
	{
		DWORD itemid;
	};

	struct TSubPacketCGShopBuyItem
	{
		DWORD ownerid;
		DWORD itemid;
		bool searching;
		long long seenprice;
	};

	//AUCTION
	struct TSubPacketCGAuctionOpenRequest
	{
		DWORD ownerid;
	};

	struct TSubPacketCGAuctionCreate
	{
		TItemPos pos;
		TPriceInfo startprice;
	};

	struct TSubPacketCGAuctionAddOffer
	{
		DWORD ownerid;
		TPriceInfo price;
	};

	struct TSubPacketCGAuctionExitFrom
	{
		DWORD ownerid;
	};

	#ifdef ENABLE_IKASHOP_ENTITIES
	struct TSubPacketCGShopClickEntity
	{
		DWORD vid;
	};
	#endif

	#ifdef ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE
	struct TPacketCGShopAvgAndMinMaxPrice
	{
		uint32_t vnum;
	};

	struct TSubPacketGCShopAvarageAndMinMaxPrice
	{
		long long avgPrice;
		long long minPrice;
		long long maxPrice;
	};
	#endif
}
#endif

#ifdef ENABLE_SHOW_CHEST_DROP
enum EPacketCGChestDropInfo
{
	CDI_SUB_HEADER_INV,
	CDI_SUB_HEADER_VNUM,
};

typedef struct SPacketCGChestDropInfo
{
	BYTE	header;
	BYTE	subheader;
	int		arg;
} TPacketCGChestDropInfo;

typedef struct SChestDropInfoTable
{
	BYTE	bPageIndex;
	BYTE	bSlotIndex;
	DWORD	dwItemVnum;
	#ifdef ENABLE_NEW_MAX_COUNT
	short	bItemCount;
	#else
	BYTE	bItemCount;
	#endif
} TChestDropInfoTable;

typedef struct SPacketGCChestDropInfo
{
	BYTE	bHeader;
	WORD	wSize;
	DWORD	dwChestVnum;
} TPacketGCChestDropInfo;
#endif

#ifdef ENABLE_SWITCHBOT
enum ECGSwitchbotSubheader
{
	SUBHEADER_CG_SWITCHBOT_START,
	SUBHEADER_CG_SWITCHBOT_STOP,
};

struct TPacketCGSwitchbot
{
	BYTE header;
	int size;
	BYTE subheader;
	BYTE slot;
};

enum EGCSwitchbotSubheader
{
	SUBHEADER_GC_SWITCHBOT_UPDATE,
	SUBHEADER_GC_SWITCHBOT_UPDATE_ITEM,
	SUBHEADER_GC_SWITCHBOT_SEND_ATTRIBUTE_INFORMATION,
};

struct TPacketGCSwitchbot
{
	BYTE header;
	int size;
	BYTE subheader;
	BYTE slot;
};

struct TSwitchbotUpdateItem
{
	BYTE	slot;
	BYTE	vnum;
	#ifdef ENABLE_NEW_MAX_COUNT
	short	count;
	#else
	BYTE	count;
	#endif
	LONG	alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
};
#endif

#ifdef ENABLE_FAST_OPEN_CHEST
typedef struct SInstantChestOpen
{
	BYTE	bHeader;
	TItemPos iPos;
}TInstantChestOpen;
#endif

#ifdef ENABLE_STYLE_ATTRIBUTE_SYSTEM
typedef struct command_item_new_attribute
{
	BYTE  header;
	TItemPos source_pos;
	TItemPos target_pos;
	BYTE bValues[3 + 1];
} TPacketCGItemNewAttribute;
#endif

#ifdef ENABLE_HIDE_COSTUME_PART
typedef struct SPacketCGChangeCostumeVisibilityStatus
{
	BYTE bHeader;
	BYTE type;
	bool status;
} TPacketCGChangeCostumeVisibilityStatus;

typedef struct SPacketGCUpdateCostumeVisible
{
	BYTE bHeader;
	bool type[ECostumePartTypes::HIDE_COSTUME_COUNT];
} TPacketGCUpdateCostumeVisible;
#endif

#ifdef ENABLE_FULL_YANG
typedef struct packet_gold
{
	BYTE	bHeader;
	uint64_t	lGold;
} TPacketGCGold;

typedef struct packet_gold_change
{
	BYTE	header;
	DWORD	dwVID;
	int64_t	ldAmount;
	uint64_t	ldValue;
} TPacketGCGoldChange;
#endif

#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
enum
{
	CUBE_RENEWAL_SUB_HEADER_OPEN_RECEIVE,
	CUBE_RENEWAL_SUB_HEADER_CLEAR_DATES_RECEIVE,
	CUBE_RENEWAL_SUB_HEADER_DATES_RECEIVE,
	CUBE_RENEWAL_SUB_HEADER_DATES_LOADING,
	#if defined(CUBE_RENEWAL_RELOAD_FIX)
	CUBE_RENEWAL_SUB_HEADER_DATES_RELOAD,
	#endif

	CUBE_RENEWAL_SUB_HEADER_MAKE_ITEM,
	CUBE_RENEWAL_SUB_HEADER_CLOSE,
};

typedef struct  packet_send_cube_renewal
{
	BYTE header;
	BYTE subheader;
	DWORD index_item;
	DWORD count_item;
	DWORD index_item_improve;
}TPacketCGCubeRenewalSend;

typedef struct dates_cube_renewal
{
	DWORD	npc_vnum;
	DWORD	index;

	DWORD	vnum_reward;
	int		count_reward;

	bool	item_reward_stackable;

	DWORD	vnum_material[CUBE_MATERIAL_MAX_NUM];
	int		count_material[CUBE_MATERIAL_MAX_NUM];

	#ifdef ENABLE_FULL_YANG
	int64_t	gold;
	#else
	int		gold;
	#endif

	int		percent;

	char	category[100];
}TInfoDateCubeRenewal;

typedef struct packet_receive_cube_renewal
{
	packet_receive_cube_renewal() : header(HEADER_GC_CUBE_RENEWAL)
	{}

	BYTE header;
	BYTE subheader;
	TInfoDateCubeRenewal	date_cube_renewal;
}TPacketGCCubeRenewalReceive;
#endif

#ifdef __RANKING_SYSTEM__
enum
{
	// Rank Types
	RANK_TYPE_PLAYER = 0,
	RANK_TYPE_DUNGEON,
	RANK_TYPE_BOSS,
	RANK_TYPE_MAX,

	//Rank General
	RANK_LEVEL = 0,
	RANK_K_STONE,
	RANK_K_BOSS,
	RANK_D_STONE,
	RANK_D_BOSS,
	RANK_DUNGEON,
	RANK_PLAYTIME,
	RANK_CHEST,
	RANK_D_SKILL,
	RANK_D_NORMAL,
	RANK_MAX,

	// Rank Dungeons
	RANK_DUNGEON_LUCIFER = 0,
	RANK_DUNGEON_CATACOMB,
	RANK_DUNGEON_BERAN_SETAOU,
	RANK_DUNGEON_BARONES,
	RANK_DUNGEON_RAZADOR,
	RANK_DUNGEON_NEMERE,
	RANK_DUNGEON_MAX,

	// Rank Boss
	RANK_BOSS_ORK = 0,
	RANK_BOSS_QUEEN_SPIDER,
	RANK_BOSS_DARK_GHOST_LEADER,
	RANK_BOSS_GIANT_DESERT_TORTOISE,
	RANK_BOSS_NINE_TAILS,
	RANK_BOSS_FLAME_KING,
	RANK_BOSS_YELLOW_TIGER_GHOST,
	RANK_BOSS_ICE_WITCH,
	RANK_BOSS_GENERAL,
	RANK_BOSS_KING_VOBA,
	RANK_BOSS_MAX,

	// Rank Race Types
	RANK_RACE_ALL = 0,
	RANK_RACE_WARRIOR,
	RANK_RACE_ASSASSIN,
	RANK_RACE_SURA,
	RANK_RACE_SHAMAN,
	RANK_RACE_MAX,

	//Rank Load
	RANK_LOAD = 0,
	RANK_SELF = 1,
};

typedef struct SRanking
{
	BYTE	index;
	char	name[CHARACTER_NAME_MAX_LEN+1];
	BYTE	empire;
	long long	value;
} TRanking;

typedef struct SPacketRank
{
	BYTE	header;
	DWORD	size;
} TPacketRank;
#endif

#ifdef ENABLE_EVENT_MANAGER
typedef struct SPacketGCEventManager
{
	BYTE	header;
	DWORD	size;
} TPacketGCEventManager;

typedef struct event_struct_
{
	WORD	eventID;
	BYTE	eventIndex;
	int		startTime;
	int		endTime;
	BYTE	empireFlag;
	BYTE	channelFlag;
	DWORD	value[4];
	bool	eventStatus;
	bool	eventTypeOnlyStart;
	char	startTimeText[25];
	char	endTimeText[25];
}TEventManagerData;

enum
{
	EVENT_MANAGER_LOAD,
	EVENT_MANAGER_EVENT_STATUS,
	EVENT_MANAGER_REMOVE_EVENT,
	EVENT_MANAGER_UPDATE,

	BONUS_EVENT					= 1,
	DOUBLE_BOSS_LOOT_EVENT		= 2,
	DOUBLE_METIN_LOOT_EVENT		= 3,
	DOUBLE_MISSION_BOOK_EVENT	= 4,
	DUNGEON_COOLDOWN_EVENT		= 5,
	DUNGEON_TICKET_LOOT_EVENT	= 6,
	EMPIRE_WAR_EVENT			= 7,
	TOURNAMENT_EVENT			= 8,
	WHELL_OF_FORTUNE_EVENT		= 9,
	HALLOWEEN_EVENT				= 10,
	NPC_SEARCH_EVENT			= 11,
	EXP_EVENT					= 12,
	ITEM_DROP_EVENT				= 13,
	YANG_DROP_EVENT				= 14,
	CARD_EVENT					= 15,
	CATCH_KING_EVENT			= 16,
	TREASURE_KEY				= 17,
	BEGGINER_EVENT				= 18,

	MOONLIGHT_EVENT				= 19,
	FOOTBALL_EVENT				= 20,
	HEXAGON_EVENT				= 21,
	DROP_ITEM_EVENT				= 22,
	SOCK_EVENT					= 23,
	BAGEL_EVENT					= 24,

	EVENT_MANAGER_MAX_NUM		= 25,
};
#endif

#ifdef ENABLE_SKILL_RENEWAL
typedef struct SPacketGCSkillBookCount
{
	BYTE header;
	BYTE readCount[6];
	BYTE leftCount[6];
} TPacketGCSkillBookCount;
#endif

#ifdef ENABLE_DRAGON_COIN_SHOP_SYSTEM
enum
{
	ITEMSHOP_PROCESS_NONE,
	ITEMSHOP_PROCESS_REFRESH,
	ITEMSHOP_PROCESS_BUY,
	#ifdef ENABLE_DRAGON_COIN_EM_MARKET
	ITEMSHOP_PROCESS_EM_REFRESH,
	#endif
};

enum EPacketGCDragonCoinSubHeaderType
{
	DC_SUB_HEADER_CATEGORYS,
	DC_SUB_HEADER_ITEMS,
	DC_SUB_HEADER_LOGS,
	DC_SUB_HEADER_POPUP_MESSAGE,
	#ifdef ENABLE_DRAGON_COIN_EM_MARKET
	DC_SUB_HEADER_EM_CATEGORYS,
	DC_SUB_HEADER_EM_ITEMS,
	#endif
};

enum
{
	ITEMSHOP_SELL_TYPE_ALL,
	ITEMSHOP_SELL_TYPE_COINS,
	ITEMSHOP_SELL_TYPE_CASH,
	#ifdef ENABLE_DRAGON_COIN_EM_MARKET
	ITEMSHOP_SELL_TYPE_EM,
	#endif
};

typedef struct packet_ishop_size
{
	BYTE	bHeader;
	#ifdef ENABLE_LARGE_DYNAMIC_PACKETS
	int		dataSize;
	#else
	WORD	dataSize;
	#endif
	BYTE	bSubHeader;
}TGCItemShopSize;

typedef struct SGCIShopCategory
{
	WORD	catID;
	char	categoryName[32+1];
	BYTE	mainID;
}TGCIShopCategory;

typedef struct SGCIShopItem
{
	DWORD	iID;
	DWORD	iVnum;
	INT		iCount;
	DWORD	iPrice;
	INT		iDiscount;
	WORD	categoryID;
	DWORD	dwAddTime;
	DWORD	dwOfferTime;
	BYTE	bTimeType;
	int		iStock;
	WORD	wSellCount;
	BYTE	bSellType;
	LONG	alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
}TGCIShopItem;

typedef struct SCGProcess
{
	BYTE	bHeader;
	BYTE	bSubHeader;
	DWORD	itemID;
	BYTE	itemCount;
	BYTE	itemPriceType;
	char	szCategoryStr[32+1];
}TCGItemShopProcess;

typedef struct SDcShopLog
{
	DWORD		accountID;
	char		playerName[CHARACTER_NAME_MAX_LEN+1];
	char		buyDate[21];
	int			buyTime;
	char		ipAdress[16];
	DWORD		itemID;
	DWORD		itemVnum;
	int			itemCount;
	long long	itemPrice;
	BYTE		itemPriceType;
}TDcShopLog;

struct TItemShopPopupMessage
{
	static constexpr inline auto MESSAGE_LEN = 80;
	char localeString[MESSAGE_LEN];
	int iData;
};
#endif

#ifdef ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM
enum
{
	ITEMSHOP_PROCESS_ADMIN_PAGE_OPEN,
	ITEMSHOP_PROCESS_CATE_ADD,
	ITEMSHOP_PROCESS_SUBCATE_ADD,
	ITEMSHOP_PROCESS_CATE_DEL,
	ITEMSHOP_PROCESS_SUBCATE_DEL,
	ITEMSHOP_PROCESS_RENAME,
	ITEMSHOP_PROCESS_ITEM_DEL,
	ITEMSHOP_PROCESS_ITEM_ADD,
	ITEMSHOP_PROCESS_ITEM_UPDATE,
};

enum EPacketGCDragonCoinAdminSubHeaderType
{
	DC_SUB_HEADER_ADMIN_CATEGORYS,
	DC_SUB_HEADER_ADMIN_ITEMS,
};

typedef struct SCGAdminProcess
{
	BYTE	header;
	BYTE	subheader;
} TCGItemShopAdminProcess;

typedef struct SGCIShopCateAdmin
{
	int		iCateID;
	char	szCategory[32+1];
}TGCIShopCateAdmin;

typedef struct SGCIShopItemAdmin
{
	DWORD	iID;
	DWORD	iVnum;
	INT		iCount;
	DWORD	iPrice;
	INT		iDiscount;
	WORD	categoryID;
	DWORD	dwAddTime;
	DWORD	dwOfferTime;
	char	szAddTime[32+1];
	char	szOfferTime[32+1];
	BYTE	bTimeType;
	BYTE	bStatus;
	int		iStock;
	WORD	wSellCount;
	BYTE	bSellType;
	LONG	alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
}TGCIShopItemAdmin;
#endif

#ifdef ENABLE_PM_ALL_SEND_SYSTEM
typedef struct SPacketCGBulkWhisper
{
	BYTE	header;
	char	szText[512 + 1];
} TPacketCGBulkWhisper;

typedef struct packet_bulk_whisper
{
	BYTE	header;
	WORD	size;
} TPacketGCBulkWhisper;
#endif

#ifdef ENABLE_BAKIM_DUYURU
typedef struct SPacketGCMaintenanceInfo
{
	BYTE header;
	WORD size;
	DWORD remaining_time;
	BYTE cause_len;
	DWORD duration;
} TPacketGCMaintenanceInfo;
#endif

#ifdef ENABLE_OFFER_SYSTEM
typedef struct packet_cg_offer
{
	BYTE	bHeader;
	BYTE	reqID;
	DWORD	offerID;
	DWORD	offerOldPrice;
	DWORD	offerNewPrice;
}TCGOfferRequest;

typedef struct packet_offer_size
{
	BYTE	bHeader;
	WORD	dataSize;
	BYTE	dataType;
}TGCOfferListSize;
#endif

#ifdef ENABLE_EXCHANGE_LOG
typedef struct SPacketGCExchangeLog
{
	BYTE	header;
	#ifdef ENABLE_LARGE_DYNAMIC_PACKETS
	int		size;
	#else
	WORD	size;
	#endif
} TPacketGCExchangeLog;

enum
{
	SUB_EXCHANGELOG_LOAD,
	SUB_EXCHANGELOG_LOAD_ITEM,
};

typedef struct SExchangeLog
{
	char		owner[CHARACTER_NAME_MAX_LEN + 1];
	DWORD		ownerPID;
	long long	ownerGold;
	char		ownerIP[15 + 1];
	char		target[CHARACTER_NAME_MAX_LEN + 1];
	DWORD		targetPID;
	long long	targetGold;
	char		targetIP[15 + 1];
	char		date[25];
	bool		itemsLoaded;
} TExchangeLog;

typedef struct SExchangeLogItem
{
	BYTE	pos;
	DWORD	vnum;
	WORD	count;
	LONG	alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
	bool	isOwnerItem;
}TExchangeLogItem;
#endif

#ifdef ENABLE_ENTITY_PRELOADING
typedef struct packet_preload_entities
{
	uint8_t header;
	uint16_t size;
	uint16_t count;
} TPacketGCPreloadEntities;
#endif

#ifdef ENABLE_WHEEL_OF_FORTUNE
enum EWheelOfFortuneTypes
{
	WHEEL_TYPE_NONE,
	//-------------//
	WHEEL_TYPE_EP,
	WHEEL_TYPE_MP,
	//-------------//
	WHEEL_TYPE_BRONZE_COIN,
	WHEEL_TYPE_SILVER_COIN,
	WHEEL_TYPE_GOLD_COIN,
	WHEEL_TYPE_EMERALD_COIN,
	//-------------//
	WHEEL_TYPE_BRONZE_YANG,
	WHEEL_TYPE_SILVER_YANG,
	WHEEL_TYPE_GOLD_YANG,
	WHEEL_TYPE_EMERALD_YANG,
	//-------------//
	WHEEL_TYPE_MAX_NUM,
};

typedef struct SWheelOfFortuneInfoTable
{
	int		iVnum;
	int		iCount;
} TWheelOfFortuneInfoTable;

typedef struct SPacketGCWheelOfFortune
{
	BYTE	bHeader;
	WORD	wSize;
	BYTE	bSubHeader;
	int		iNeedVnum;
	int		iNeedCount;
} TPacketGCWheelOfFortune;
#endif

//Global Dynamic Function
typedef struct SDynamicPacketInfo
{
	BYTE	bHeader;
	WORD	wSize;
	BYTE	bSubHeader;
} TDynamicPacketInfo;
//Global Dynamic Function

#ifdef ENABLE_TANAKA_EVENT
enum ETanakaGCPacketSubheaders
{
	TANAKA_EVENT_SUBHEADER_GC_OPEN_UI,
	TANAKA_EVENT_SUBHEADER_GC_UPDATE,
	TANAKA_EVENT_SUBHEADER_GC_UPDATE_PLAYER_INFO,
	TANAKA_EVENT_SUBHEADER_GC_UPDATE_ITEM_INFO,
};

typedef struct STanakaGCUpdate
{
	uint8_t eventLevel;
	uint8_t tanakaCount;
	uint8_t leftTanakaCount;
} TTanakaGCUpdate;

typedef struct STanakaGCPlayerInfo
{
	DWORD totalDamage;
} TTanakaGCPlayerInfo;

typedef struct STanakaGCUpdateItemInfo
{
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	uint32_t vnum;
	uint8_t count;
} TTanakaGCUpdateItemInfo;
#endif

#ifdef ENABLE_BATTLE_PASS
typedef struct SPacketGCBattlePass
{
	BYTE bHeader;
	#ifdef ENABLE_LARGE_DYNAMIC_PACKETS
	int wSize;
	#else
	WORD wSize;
	#endif
	WORD wRewardSize;
	char battlePassName[64];
} TPacketGCBattlePass;

typedef struct SPacketGCBattlePassUpdate
{
	BYTE bHeader;
	BYTE bMissionID;
	DWORD dwNewProgress;
} TPacketGCBattlePassUpdate;

typedef struct SPacketCGBattlePassAction
{
	BYTE bHeader;
	BYTE bAction;
	int iArg;
} TPacketCGBattlePassAction;
#endif

#ifdef ENABLE_TELEPORTION_WIZARD_SYSTEM
enum
{
	TP_WIZARD_DATA_INFO,
	TP_WIZARD_DROP_INFO,
};

typedef struct SWizardDataInfoTable
{
	WORD	id;
	BYTE	type;
	char	comment[128+1];
	BYTE	min_level;
	BYTE	max_level;
	int		alignment;
	DWORD	item_vnum;
	DWORD	item_count;
	int		image_num;
	DWORD	race_num;
	BYTE	element;
	LONG	mapIndex;
	LONG	coor_x;
	LONG	coor_y;
	int		cooldown;
	BYTE	status;
	BYTE	show;
	#ifdef ENABLE_FULL_YANG
	int64_t	gold;
	#else
	LONG	gold;
	#endif
} TWizardDataInfoTable;

typedef struct SWizardDropInfoTable
{
	BYTE		bPageIndex;
	BYTE		bSlotIndex;
	DWORD		dwItemVnum;
	int			iItemCount;
} TWizardDropInfoTable;

typedef struct SPacketGCTpWizardInfo
{
	BYTE	bHeader;
	WORD	wSize;
	BYTE	bSubHeader;
	DWORD	dwRaceNum;
} TPacketGCTpWizardInfo;
#endif

#ifdef ENABLE_CHANGELOOK_SYSTEM
enum EChangeLookInfo
{
	CL_WINDOW_MAX_MATERIALS = 3,
	CL_CLEAN_ATTR_VALUE0 = 8,
	CL_SCROLL_VNUM = 72326,
};

enum
{
	CL_SUBHEADER_OPEN = 0,
	CL_SUBHEADER_CLOSE,
	CL_SUBHEADER_ADD,
	CL_SUBHEADER_REMOVE,
	CL_SUBHEADER_REFINE,
};

typedef struct SPacketChangeLook
{
	BYTE		header;
	BYTE		subheader;
	#ifdef ENABLE_FULL_YANG
	int64_t		dwCost;
	#else
	DWORD		dwCost;
	#endif
	BYTE		bPos;
	TItemPos	tPos;
} TPacketChangeLook;

typedef struct SChangeLookMaterial
{
	BYTE	bHere;
	WORD	wCell;
} TChangeLookMaterial;
#endif

#ifdef BOSS_DMG_RANKING
typedef struct SPacketGCDamageRanking
{
	BYTE		bHeader;
	char		szNames[CHARACTER_NAME_MAX_LEN + 1][MAX_PLAYERS_IN_TABLE];
	BYTE		bJobs[MAX_PLAYERS_IN_TABLE];
	BYTE		bEmpire[MAX_PLAYERS_IN_TABLE];
	BYTE		bLevels[MAX_PLAYERS_IN_TABLE];
	long long	lDamages[MAX_PLAYERS_IN_TABLE];
	float		fPercents[MAX_PLAYERS_IN_TABLE];
}TPacketGCDamageRanking;
#endif

typedef struct packet_affect_reset
{
	BYTE	header;
	DWORD	dwVID;
	DWORD	dwAffectFlag[2];
} TPacketGCAffectReset;

#ifdef ENABLE_BLACK_MARKET_SYSTEM
enum
{
	BM_SUBHEADER_OPEN,
	BM_SUBHEADER_SELL,
};

typedef struct command_item_blackmarket
{
	BYTE		header;
	BYTE		subHeader;
	TItemPos	Cell;
} TPacketCGItemBlackMarket;

typedef struct SPacketGCItemBlackMarket
{
	BYTE		bHeader;
	WORD		wSize;
} TPacketGCItemBlackMarket;

typedef struct SBlackMarketData
{
	DWORD		dwVnum;
	#ifdef ENABLE_FULL_YANG
	int64_t		llPrice;
	#else
	LONG		llPrice;
	#endif
	int			iStock;
} TBlackMarketData;
#endif

#ifdef ENABLE_REWARD_SYSTEM
enum
{
	REWARD_NONE						= 0,
	REWARD_LEVEL					= 1,
	REWARD_REFINE					= 2,
	REWARD_CRAFT					= 3,
	REWARD_AVERAGE					= 4,
	REWARD_BIOLOG					= 5,
	REWARD_ALIGNMENT				= 6,
	REWARD_PASSIVE_SKILL			= 7,

	REWARD_STONE_DEAD				= 8,
	REWARD_BOSS_DEAD				= 9,

	REWARD_STONE_ALL_DEAD			= 10,
	REWARD_BOSS_ALL_DEAD			= 11,

	REWARD_MAX,
};

typedef struct SPacketGCRewardMain
{
	BYTE bHeader;
	WORD wSize;
} TPacketGCRewardMain;

typedef struct SPacketRewardInfo
{
	BYTE deskIndex;
	char playerName[CHARACTER_NAME_MAX_LEN+1];
} TPacketRewardInfo;
#endif

#ifdef ENABLE_GUILD_STATISTICS_SYSTEM
enum
{
	GUILD_STATISTICS_ADD,
	GUILD_STATISTICS_UPDATE,
};

typedef struct SPacketGCGuildStaticsInfo
{
	SPacketGCGuildStaticsInfo() : bHeader(HEADER_GC_GUILD_STATISTICS), bSubHeader(GUILD_STATISTICS_ADD) {}
	BYTE	bHeader;
	BYTE	bSubHeader;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	unsigned long long ullDamage;
	BYTE	bRank;
	WORD	wRace;
	BYTE	bKill;
	BYTE	bDead;
	BYTE	bAssist;
} TPacketGCGuildStaticsInfo;
#endif

#if defined(__BL_LUCKY_BOX__)
enum ELUCKY_BOX_ACTION
{
	LUCKY_BOX_ACTION_RETRY,
	LUCKY_BOX_ACTION_RECEIVE,
};

typedef struct SPacketCGLuckyBox
{
	BYTE		header;
	BYTE		action;
} TPacketCGLuckyBox;

typedef struct SPacketGCLuckyBox
{
	BYTE		header;
	DWORD		vnum;
	#ifdef ENABLE_NEW_MAX_COUNT
	short		count;
	#else
	BYTE		count;
	#endif
	#ifdef ENABLE_FULL_YANG
	int64_t		need_money;
	#else
	int			need_money;
	#endif
} TPacketGCLuckyBox;
#endif

#ifdef ENABLE_PUBLISHER_SYSTEM
typedef struct SPublisherInfo
{
	int		iOrder;
	char	szTitle[128];
	char	szLink[256];
} TPublisherInfo;

typedef struct SPacketGCPublisherInfo
{
	BYTE	bHeader;
	WORD	wSize;
} TPacketGCPublisherInfo;
#endif

#ifdef ENABLE_RONARK_SYSTEM
typedef struct SRonarkTeamData
{
	DWORD	dwID;
	DWORD	dwDamage;
	int		iJoinerCount;
	int		iMemberCount;
	int64_t	iRonarkHP;
} TRonarkTeamData;

typedef struct SPacketGCRonarkInfo
{
	BYTE	bHeader;
	WORD	wSize;
} TPacketGCRonarkInfo;
#endif

#ifdef ENABLE_AUTO_ITEM_SELL_SYSTEM
enum EAUTO_ITEM_SELL_SYSTEM
{
	SUBHEADER_CG_AUTO_SELL_OPEN,
	SUBHEADER_CG_AUTO_SELL_ADD,
	SUBHEADER_CG_AUTO_SELL_REMOVE,
	SUBHEADER_CG_AUTO_SELL_RESET,
	SUBHEADER_CG_AUTO_SELL_ACTIVE,
	SUBHEADER_CG_AUTO_SELL_PASSIVE,
};

typedef struct SPacketCGAutoItemSellData
{
	BYTE	header;
	BYTE	subHeader;
	DWORD	value;
} TPacketCGAutoItemSellData;

typedef struct SPacketGCAutoItemSellData
{
	BYTE	header;
	WORD	size;
	WORD	itemCount;
	bool	status;
} TPacketGCAutoItemSellData;
#endif

#pragma pack(pop)
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
