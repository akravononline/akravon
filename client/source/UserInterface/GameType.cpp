#include "StdAfx.h"
#include "GameType.h"
#include "Packet.h"
std::string g_strResourcePath = "d:/ymir work/";
std::string g_strImagePath = "d:/ymir work/ui/";

std::string g_strGuildSymbolPathName = "data/symbol/10/";

// DEFAULT_FONT
static std::string gs_strDefaultFontName = "±¼¸²Ã¼:12.fnt";
static std::string gs_strDefaultItalicFontName = "±¼¸²Ã¼:12i.fnt";
CResource* gs_pkDefaultFont = NULL;
static CResource* gs_pkDefaultItalicFont = NULL;

static bool gs_isReloadDefaultFont = false;

void DefaultFont_Startup()
{
	gs_pkDefaultFont = NULL;
}

void DefaultFont_Cleanup()
{
	if (gs_pkDefaultFont)
		gs_pkDefaultFont->Release();
}

void DefaultFont_SetName(const char * c_szFontName)
{
	gs_strDefaultFontName = c_szFontName;
	gs_strDefaultFontName += ".fnt";

	gs_strDefaultItalicFontName = c_szFontName;
	if(strchr(c_szFontName, ':'))
		gs_strDefaultItalicFontName += "i";
	gs_strDefaultItalicFontName += ".fnt";

	gs_isReloadDefaultFont = true;
}

bool ReloadDefaultFonts()
{
	CResourceManager& rkResMgr = CResourceManager::Instance();

	gs_isReloadDefaultFont = false;

	CResource* pkNewFont = rkResMgr.GetResourcePointer(gs_strDefaultFontName.c_str());
	pkNewFont->AddReference();
	if (gs_pkDefaultFont)
		gs_pkDefaultFont->Release();
	gs_pkDefaultFont = pkNewFont;

	CResource* pkNewItalicFont = rkResMgr.GetResourcePointer(gs_strDefaultItalicFontName.c_str());
	pkNewItalicFont->AddReference();
	if (gs_pkDefaultItalicFont)
		gs_pkDefaultItalicFont->Release();
	gs_pkDefaultItalicFont = pkNewItalicFont;

	return true;
}

CResource* DefaultFont_GetResource()
{
	if (!gs_pkDefaultFont || gs_isReloadDefaultFont)
		ReloadDefaultFonts();
	return gs_pkDefaultFont;
}

CResource* DefaultItalicFont_GetResource()
{
	if (!gs_pkDefaultItalicFont || gs_isReloadDefaultFont)
		ReloadDefaultFonts();
	return gs_pkDefaultItalicFont;
}

// END_OF_DEFAULT_FONT

void SetGuildSymbolPath(const char * c_szPathName)
{
	g_strGuildSymbolPathName = "data/symbol/";
	g_strGuildSymbolPathName += c_szPathName;
	g_strGuildSymbolPathName += "/";
}

const char * GetGuildSymbolFileName(DWORD dwGuildID)
{
	return _getf("%s%03d.jpg", g_strGuildSymbolPathName.c_str(), dwGuildID);
}

BYTE c_aSlotTypeToInvenType[SLOT_TYPE_MAX] =
{
	RESERVED_WINDOW,		// SLOT_TYPE_NONE
	INVENTORY,				// SLOT_TYPE_INVENTORY
	RESERVED_WINDOW,		// SLOT_TYPE_SKILL
	RESERVED_WINDOW,		// SLOT_TYPE_EMOTION
	RESERVED_WINDOW,		// SLOT_TYPE_SHOP
	RESERVED_WINDOW,		// SLOT_TYPE_EXCHANGE_OWNER
	RESERVED_WINDOW,		// SLOT_TYPE_EXCHANGE_TARGET
	RESERVED_WINDOW,		// SLOT_TYPE_QUICK_SLOT
	RESERVED_WINDOW,
	RESERVED_WINDOW,		// SLOT_TYPE_PRIVATE_SHOP
	RESERVED_WINDOW,
	DRAGON_SOUL_INVENTORY,	// SLOT_TYPE_DRAGON_SOUL_INVENTORY
	#ifdef ENABLE_SPECIAL_STORAGE
	UPGRADE_INVENTORY,
	BOOK_INVENTORY,
	STONE_INVENTORY,
	CHEST_INVENTORY,
	#endif
	#ifdef ENABLE_SWITCHBOT
	SWITCHBOT,
	#endif
};

BYTE SlotTypeToInvenType(BYTE bSlotType)
{
	if (bSlotType >= SLOT_TYPE_MAX)
		return RESERVED_WINDOW;
	else
		return c_aSlotTypeToInvenType[bSlotType];
}

typedef struct SApplyInfo
{
	BYTE	bPointType;
} TApplyInfo;

const TApplyInfo aApplyInfo[CItemData::MAX_APPLY_NUM] =
{
	{ POINT_NONE,						}, // 0
	{ POINT_MAX_HP,						}, // 1
	{ POINT_MAX_SP,						}, // 2
	{ POINT_HT,							}, // 3
	{ POINT_IQ,							}, // 4
	{ POINT_ST,							}, // 5
	{ POINT_DX,							}, // 6
	{ POINT_ATT_SPEED,					}, // 7
	{ POINT_MOV_SPEED,					}, // 8
	{ POINT_CASTING_SPEED,				}, // 9
	{ POINT_HP_REGEN,					}, // 10
	{ POINT_SP_REGEN,					}, // 11
	{ POINT_POISON_PCT,					}, // 12
	{ POINT_STUN_PCT,					}, // 13
	{ POINT_SLOW_PCT,					}, // 14
	{ POINT_CRITICAL_PCT,				}, // 15
	{ POINT_PENETRATE_PCT,				}, // 16
	{ POINT_ATTBONUS_HUMAN,				}, // 17
	{ POINT_ATTBONUS_ANIMAL,			}, // 18
	{ POINT_ATTBONUS_ORC,				}, // 19
	{ POINT_ATTBONUS_MILGYO,			}, // 20
	{ POINT_ATTBONUS_UNDEAD,			}, // 21
	{ POINT_ATTBONUS_DEVIL,				}, // 22
	{ POINT_STEAL_HP,					}, // 23
	{ POINT_STEAL_SP,					}, // 24
	{ POINT_MANA_BURN_PCT,				}, // 25
	{ POINT_DAMAGE_SP_RECOVER,			}, // 26
	{ POINT_BLOCK,						}, // 27
	{ POINT_DODGE,						}, // 28
	{ POINT_RESIST_SWORD,				}, // 29
	{ POINT_RESIST_TWOHAND,				}, // 30
	{ POINT_RESIST_DAGGER,				}, // 31
	{ POINT_RESIST_BELL,				}, // 32
	{ POINT_RESIST_FAN,					}, // 33
	{ POINT_RESIST_BOW,					}, // 34
	{ POINT_RESIST_FIRE,				}, // 35
	{ POINT_RESIST_ELEC,				}, // 36
	{ POINT_RESIST_MAGIC,				}, // 37
	{ POINT_RESIST_WIND,				}, // 38
	{ POINT_REFLECT_MELEE,				}, // 39
	{ POINT_REFLECT_CURSE,				}, // 40
	{ POINT_POISON_REDUCE,				}, // 41
	{ POINT_KILL_SP_RECOVER,			}, // 42
	{ POINT_EXP_DOUBLE_BONUS,			}, // 43
	{ POINT_GOLD_DOUBLE_BONUS,			}, // 44
	{ POINT_ITEM_DROP_BONUS,			}, // 45
	{ POINT_POTION_BONUS,				}, // 46
	{ POINT_KILL_HP_RECOVER,			}, // 47
	{ POINT_IMMUNE_STUN,				}, // 48
	{ POINT_IMMUNE_SLOW,				}, // 49
	{ POINT_IMMUNE_FALL,				}, // 50
	{ POINT_NONE,						}, // 51
	{ POINT_BOW_DISTANCE,				}, // 52
	{ POINT_ATT_GRADE_BONUS,			}, // 53
	{ POINT_DEF_GRADE_BONUS,			}, // 54
	{ POINT_MAGIC_ATT_GRADE_BONUS,		}, // 55
	{ POINT_MAGIC_DEF_GRADE_BONUS,		}, // 56
	{ POINT_CURSE_PCT,					}, // 57
	{ POINT_MAX_STAMINA					}, // 58
	{ POINT_ATTBONUS_WARRIOR			}, // 59
	{ POINT_ATTBONUS_ASSASSIN			}, // 60
	{ POINT_ATTBONUS_SURA				}, // 61
	{ POINT_ATTBONUS_SHAMAN				}, // 62
	{ POINT_ATTBONUS_MONSTER			}, // 63
	{ POINT_ATT_BONUS					}, // 64
	{ POINT_MALL_DEFBONUS				}, // 65
	{ POINT_MALL_EXPBONUS				}, // 66
	{ POINT_MALL_ITEMBONUS				}, // 67
	{ POINT_MALL_GOLDBONUS				}, // 68
	{ POINT_MAX_HP_PCT					}, // 69
	{ POINT_MAX_SP_PCT					}, // 70
	{ POINT_SKILL_DAMAGE_BONUS			}, // 71
	{ POINT_NORMAL_HIT_DAMAGE_BONUS		}, // 72
	{ POINT_SKILL_DEFEND_BONUS			}, // 73
	{ POINT_NORMAL_HIT_DEFEND_BONUS		}, // 74
	{ POINT_PC_BANG_EXP_BONUS			}, // 75
	{ POINT_PC_BANG_DROP_BONUS			}, // 76
	{ POINT_NONE,						}, // 77
	{ POINT_RESIST_WARRIOR,				}, // 78
	{ POINT_RESIST_ASSASSIN,			}, // 79
	{ POINT_RESIST_SURA,				}, // 80
	{ POINT_RESIST_SHAMAN,				}, // 81
	{ POINT_ENERGY						}, // 82
	{ POINT_DEF_GRADE					}, // 83
	{ POINT_COSTUME_ATTR_BONUS			}, // 84
	{ POINT_MAGIC_ATT_BONUS_PER 		}, // 85
	{ POINT_MELEE_MAGIC_ATT_BONUS_PER	}, // 86
	{ POINT_RESIST_ICE,					}, // 87
	{ POINT_RESIST_EARTH,				}, // 88
	{ POINT_RESIST_DARK,				}, // 89
	{ POINT_RESIST_CRITICAL,			}, // 90
	{ POINT_RESIST_PENETRATE,			}, // 91

	#ifdef ENABLE_WOLFMAN_CHARACTER
	{ POINT_BLEEDING_REDUCE,			}, // 92
	{ POINT_BLEEDING_PCT,				}, // 93
	{ POINT_ATTBONUS_WOLFMAN,			}, // 94
	{ POINT_RESIST_WOLFMAN,				}, // 95
	{ POINT_RESIST_CLAW,				}, // 96
	#endif

	#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	{ POINT_ACCEDRAIN_RATE				}, // 97
	#endif

	#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
	{ POINT_RESIST_MAGIC_REDUCTION		}, // 98
	#endif

	{ POINT_ENCHANT_ELECT				}, // 99
	{ POINT_ENCHANT_FIRE				}, // 100
	{ POINT_ENCHANT_ICE					}, // 101
	{ POINT_ENCHANT_WIND				}, // 102
	{ POINT_ENCHANT_EARTH				}, // 103
	{ POINT_ENCHANT_DARK				}, // 104

	// #ifdef ENABLE_AUTO_PICKUP_SYSTEM
	// { POINT_OTO_METIN					}, // 105
	// { POINT_OTO_CANAVAR					}, // 106
	// { POINT_OTO_PATRON					}, // 107
	// #endif

	#ifdef ENABLE_ANTI_EXP
	{ POINT_ANTI_EXP					}, // 108
	#endif

	#ifdef ENABLE_NEW_ATTRIBUTE_TYPES
	{ POINT_ATTBONUS_STONE				}, // 109
	{ POINT_ATTBONUS_BOSS				}, // 110
	{ POINT_RESIST_HUMAN				}, // 111
	{ POINT_RESIST_MONSTER				}, // 112
	#endif

	#ifdef ENABLE_BATTLE_PASS
	{ POINT_BATTLE_PASS_ID				}, // 113
	#endif

	#ifdef ENABLE_PVM_BONUS
	{ POINT_SKILL_DAMAGE_BONUS_PVM		}, // 114
	{ POINT_HIT_DAMAGE_BONUS_PVM		}, // 115
	{ POINT_ATT_GRADE_BONUS_PVM			}, // 116
	{ POINT_DEF_GRADE_BONUS_PVM			}, // 117
	{ POINT_BERSERKER_BONUS_PVM			}, // 118
	#endif

	{ POINT_MIN_WEP						}, // 200
	{ POINT_MAX_WEP						}, // 201
	{ POINT_MIN_MAGIC_WEP				}, // 202
	{ POINT_MAX_MAGIC_WEP				}, // 203
	{ POINT_HIT_RATE					}, // 204
};

BYTE ApplyTypeToPointType(BYTE bApplyType)
{
	if (bApplyType >= CItemData::MAX_APPLY_NUM)
		return POINT_NONE;
	else
		return aApplyInfo[bApplyType].bPointType;
}


