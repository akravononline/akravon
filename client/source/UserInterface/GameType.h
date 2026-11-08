#pragma once
#include "../GameLib/ItemData.h"

struct SAffects
{
	enum
	{
		AFFECT_MAX_NUM = 32,
	};

	SAffects() : dwAffects(0) {}
	SAffects(const DWORD & c_rAffects)
	{
		__SetAffects(c_rAffects);
	}
	int operator = (const DWORD & c_rAffects)
	{
		__SetAffects(c_rAffects);
	}

	BOOL IsAffect(BYTE byIndex)
	{
		return dwAffects & (1 << byIndex);
	}

	void __SetAffects(const DWORD & c_rAffects)
	{
		dwAffects = c_rAffects;
	}

	DWORD dwAffects;
};

extern std::string g_strGuildSymbolPathName;

constexpr DWORD c_Name_Max_Length = 64;
constexpr DWORD c_FileName_Max_Length = 128;
constexpr DWORD c_Short_Name_Max_Length = 32;

constexpr DWORD c_Inventory_Page_Column = 5;
constexpr DWORD c_Inventory_Page_Row = 7;	// AKRAVON: 5x7 goz, 5 sayfa (175 yuva); sunucudaki 180 yuva duzeni bozulmaz
constexpr DWORD c_Inventory_Page_Size = c_Inventory_Page_Column*c_Inventory_Page_Row; // x*y

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
constexpr DWORD c_Inventory_Page_Count = 5;
#else
constexpr DWORD c_Inventory_Page_Count = 2;
#endif

constexpr DWORD c_ItemSlot_Count = 180;	// sunucu yuva sayisi (gorunen 5x7x5 = 175; son 5 yuva gizli)
constexpr DWORD c_Equipment_Count = 12;

constexpr DWORD c_Equipment_Start = 180;	// sunucu ile ayni: ekipman 180'den baslar (envanter 175 yuva gorunur)

constexpr DWORD c_Equipment_Body	= c_Equipment_Start + CItemData::WEAR_BODY;
constexpr DWORD c_Equipment_Head	= c_Equipment_Start + CItemData::WEAR_HEAD;
constexpr DWORD c_Equipment_Shoes	= c_Equipment_Start + CItemData::WEAR_FOOTS;
constexpr DWORD c_Equipment_Wrist	= c_Equipment_Start + CItemData::WEAR_WRIST;
constexpr DWORD c_Equipment_Weapon	= c_Equipment_Start + CItemData::WEAR_WEAPON;
constexpr DWORD c_Equipment_Neck	= c_Equipment_Start + CItemData::WEAR_NECK;
constexpr DWORD c_Equipment_Ear		= c_Equipment_Start + CItemData::WEAR_EAR;
constexpr DWORD c_Equipment_Unique1	= c_Equipment_Start + CItemData::WEAR_UNIQUE1;
constexpr DWORD c_Equipment_Unique2	= c_Equipment_Start + CItemData::WEAR_UNIQUE2;
constexpr DWORD c_Equipment_Arrow	= c_Equipment_Start + CItemData::WEAR_ARROW;
constexpr DWORD c_Equipment_Shield	= c_Equipment_Start + CItemData::WEAR_SHIELD;

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
constexpr DWORD c_New_Equipment_Start = c_Equipment_Start + CItemData::WEAR_RING1;
constexpr DWORD c_Equipment_Ring1 = c_Equipment_Start + CItemData::WEAR_RING1;
constexpr DWORD c_Equipment_Ring2 = c_Equipment_Start + CItemData::WEAR_RING2;
#ifdef ENABLE_NEW_RING
constexpr DWORD c_Equipment_Ring3 = c_Equipment_Start + CItemData::WEAR_RING3;
constexpr DWORD c_Equipment_Ring4 = c_Equipment_Start + CItemData::WEAR_RING4;
constexpr DWORD c_Equipment_Ring5 = c_Equipment_Start + CItemData::WEAR_RING5;
constexpr DWORD c_Equipment_Ring6 = c_Equipment_Start + CItemData::WEAR_RING6;
constexpr DWORD c_New_Equipment_Count = 6;
#else
constexpr DWORD c_New_Equipment_Count = 3;
#endif
constexpr DWORD c_Equipment_Belt  = c_Equipment_Start + CItemData::WEAR_BELT;
#endif

#ifdef ENABLE_TRADABLE_ICON
enum ETopWindowTypes
{
	ON_TOP_WND_NONE,
	ON_TOP_WND_SHOP,
	ON_TOP_WND_EXCHANGE,
	ON_TOP_WND_SAFEBOX,
	ON_TOP_WND_PRIVATE_SHOP,
	ON_TOP_WND_ITEM_COMB,
	ON_TOP_WND_PET_FEED,

	ON_TOP_WND_MAX,
};
#endif

enum EDragonSoulDeckType
{
	DS_DECK_1,
	DS_DECK_2,
	DS_DECK_MAX_NUM = 2,
};

enum EDragonSoulGradeTypes
{
	DRAGON_SOUL_GRADE_NORMAL,
	DRAGON_SOUL_GRADE_BRILLIANT,
	DRAGON_SOUL_GRADE_RARE,
	DRAGON_SOUL_GRADE_ANCIENT,
	DRAGON_SOUL_GRADE_LEGENDARY,
	DRAGON_SOUL_GRADE_MAX,

};

enum EDragonSoulStepTypes
{
	DRAGON_SOUL_STEP_LOWEST,
	DRAGON_SOUL_STEP_LOW,
	DRAGON_SOUL_STEP_MID,
	DRAGON_SOUL_STEP_HIGH,
	DRAGON_SOUL_STEP_HIGHEST,
	DRAGON_SOUL_STEP_MAX,
};

#ifdef ENABLE_COSTUME_SYSTEM
	constexpr DWORD c_Costume_Slot_Start	= c_Equipment_Start + CItemData::WEAR_COSTUME_BODY;
	constexpr DWORD	c_Costume_Slot_Body		= c_Costume_Slot_Start + CItemData::WEAR_COSTUME_BODY;
	constexpr DWORD	c_Costume_Slot_Hair		= c_Costume_Slot_Start + CItemData::WEAR_COSTUME_HAIR;
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	constexpr DWORD	c_Costume_Slot_Mount = c_Costume_Slot_Start + CItemData::WEAR_COSTUME_MOUNT;
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	constexpr DWORD	c_Costume_Slot_Acce = c_Costume_Slot_Start + CItemData::WEAR_COSTUME_ACCE;
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	constexpr DWORD	c_Costume_Slot_Weapon	= c_Equipment_Start + CItemData::WEAR_COSTUME_WEAPON;
#endif
#ifdef ENABLE_COSTUME_PET_SYSTEM
	constexpr DWORD	c_Costume_Slot_Pet		= c_Equipment_Start + CItemData::WEAR_COSTUME_PET;
#endif
#if defined(ENABLE_WEAPON_COSTUME_SYSTEM) || defined(ENABLE_ACCE_COSTUME_SYSTEM) || defined(ENABLE_MOUNT_COSTUME_SYSTEM) || defined(ENABLE_COSTUME_PET_SYSTEM)
	constexpr DWORD c_Costume_Slot_Count	= 6;
	constexpr DWORD c_Costume_Slot_End		= c_Costume_Slot_Start + c_Costume_Slot_Count;
#endif
#endif

const DWORD c_Wear_Max = CItemData::WEAR_MAX_NUM;
const DWORD c_DragonSoul_Equip_Start = c_ItemSlot_Count + c_Wear_Max;
const DWORD c_DragonSoul_Equip_Slot_Max = 6;
const DWORD c_DragonSoul_Equip_End = c_DragonSoul_Equip_Start + c_DragonSoul_Equip_Slot_Max * DS_DECK_MAX_NUM;

const DWORD c_DragonSoul_Equip_Reserved_Count = c_DragonSoul_Equip_Slot_Max * 3;

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	const DWORD c_Belt_Inventory_Slot_Start = c_DragonSoul_Equip_End + c_DragonSoul_Equip_Reserved_Count;
	const DWORD c_Belt_Inventory_Width = 4;
	const DWORD c_Belt_Inventory_Height= 4;
	const DWORD c_Belt_Inventory_Slot_Count = c_Belt_Inventory_Width * c_Belt_Inventory_Height;
	const DWORD c_Belt_Inventory_Slot_End = c_Belt_Inventory_Slot_Start + c_Belt_Inventory_Slot_Count;

	const DWORD c_Inventory_Count	= c_Belt_Inventory_Slot_End;
#else
	const DWORD c_Inventory_Count	= c_DragonSoul_Equip_End;
#endif

const DWORD c_DragonSoul_Inventory_Start = 0;
const DWORD c_DragonSoul_Inventory_Box_Size = 32;
const DWORD c_DragonSoul_Inventory_Count = CItemData::DS_SLOT_NUM_TYPES * DRAGON_SOUL_GRADE_MAX * c_DragonSoul_Inventory_Box_Size;
const DWORD c_DragonSoul_Inventory_End = c_DragonSoul_Inventory_Start + c_DragonSoul_Inventory_Count;

#ifdef ENABLE_SPECIAL_STORAGE
const DWORD c_Special_Inventory_Page_Size = 5 * 9;
const DWORD c_Special_Inventory_Page_Count = 4;
const DWORD c_Special_ItemSlot_Count = c_Special_Inventory_Page_Size * c_Special_Inventory_Page_Count;
#endif

enum ESlotType
{
	SLOT_TYPE_NONE,
	SLOT_TYPE_INVENTORY,
	SLOT_TYPE_SKILL,
	SLOT_TYPE_EMOTION,
	SLOT_TYPE_SHOP,
	SLOT_TYPE_EXCHANGE_OWNER,
	SLOT_TYPE_EXCHANGE_TARGET,
	SLOT_TYPE_QUICK_SLOT,
	SLOT_TYPE_SAFEBOX,
	SLOT_TYPE_PRIVATE_SHOP,
	SLOT_TYPE_MALL,
	SLOT_TYPE_DRAGON_SOUL_INVENTORY,
	#ifdef ENABLE_SPECIAL_STORAGE
	SLOT_TYPE_UPGRADE_INVENTORY,
	SLOT_TYPE_BOOK_INVENTORY,
	SLOT_TYPE_STONE_INVENTORY,
	SLOT_TYPE_CHEST_INVENTORY,
	#endif
	#ifdef ENABLE_SWITCHBOT
	SLOT_TYPE_SWITCHBOT,
	#endif
	SLOT_TYPE_MAX,
};

enum EWindows
{
	RESERVED_WINDOW,
	INVENTORY,
	EQUIPMENT,
	SAFEBOX,
	MALL,
	DRAGON_SOUL_INVENTORY,
	#ifdef ENABLE_SPECIAL_STORAGE
	UPGRADE_INVENTORY,
	BOOK_INVENTORY,
	STONE_INVENTORY,
	CHEST_INVENTORY,
	#endif
	#ifdef ENABLE_SWITCHBOT
	SWITCHBOT,
	#endif
	BELT_INVENTORY,
	GROUND,
	#ifdef ENABLE_IKASHOP_RENEWAL
	IKASHOP_OFFLINESHOP,
	IKASHOP_SAFEBOX,
	IKASHOP_AUCTION,
	#endif
	WINDOW_TYPE_MAX,
};

#ifdef ENABLE_SWITCHBOT
enum ESwitchbotValues
{
	SWITCHBOT_SLOT_COUNT = 5,
	SWITCHBOT_ALTERNATIVE_COUNT = 2,
	MAX_NORM_ATTR_NUM = 5,
};

enum EAttributeSet
{
	ATTRIBUTE_SET_WEAPON,
	ATTRIBUTE_SET_BODY,
	ATTRIBUTE_SET_WRIST,
	ATTRIBUTE_SET_FOOTS,
	ATTRIBUTE_SET_NECK,
	ATTRIBUTE_SET_HEAD,
	ATTRIBUTE_SET_SHIELD,
	ATTRIBUTE_SET_EAR,
	ATTRIBUTE_SET_MAX_NUM,
};
#endif

#ifdef ENABLE_HIDE_COSTUME_PART
enum ECostumePartTypes : std::uint8_t
{
	HIDE_COSTUME_HAIR,
	HIDE_COSTUME_BODY,
	HIDE_COSTUME_WEAPON,
/*#ifdef ENABLE_SHOULDER_SASH_SYSTEM
	HIDE_COSTUME_SASH,
#endif*/
	HIDE_COSTUME_COUNT,
};
#endif

enum EDSInventoryMaxNum
{
	DS_INVENTORY_MAX_NUM = c_DragonSoul_Inventory_Count,
	DS_REFINE_WINDOW_MAX_NUM = 15,
};

#pragma pack (push, 1)
#define WORD_MAX 0xffff

typedef struct SItemPos
{
	BYTE window_type;
	WORD cell;

	SItemPos()
	{
		window_type = INVENTORY;
		cell = WORD_MAX;
	}

	SItemPos(BYTE _window_type, WORD _cell)
	{
		window_type = _window_type;
		cell = _cell;
	}

	bool IsValidCell()
	{
		switch (window_type)
		{
		case INVENTORY:
			return cell < c_Inventory_Count;
			break;
		case EQUIPMENT:
			return cell < c_DragonSoul_Equip_End;
			break;
		case DRAGON_SOUL_INVENTORY:
			return cell < (DS_INVENTORY_MAX_NUM);
			break;
		#ifdef ENABLE_SPECIAL_STORAGE
		case UPGRADE_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
		case BOOK_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
		case STONE_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
		case CHEST_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
		#endif

		#ifdef ENABLE_SWITCHBOT
		case SWITCHBOT:
			return cell < SWITCHBOT_SLOT_COUNT;
			break;
		#endif

		default:
			return false;
		}
	}
	bool IsEquipCell()
	{
		switch (window_type)
		{
		case INVENTORY:
		case EQUIPMENT:
			return (c_Equipment_Start + c_Wear_Max > cell) && (c_Equipment_Start <= cell);
			break;

		case BELT_INVENTORY:
		case DRAGON_SOUL_INVENTORY:
		#ifdef ENABLE_SPECIAL_STORAGE
		case UPGRADE_INVENTORY:
		case BOOK_INVENTORY:
		case STONE_INVENTORY:
		case CHEST_INVENTORY:
		#endif
			return false;
			break;

		default:
			return false;
		}
	}

	#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	bool IsBeltInventoryCell()
	{
		bool bResult = c_Belt_Inventory_Slot_Start <= cell && c_Belt_Inventory_Slot_End > cell;
		return bResult;
	}
	#endif

	bool operator==(const struct SItemPos& rhs) const
	{
		return (window_type == rhs.window_type) && (cell == rhs.cell);
	}

	bool operator<(const struct SItemPos& rhs) const
	{
		return (window_type < rhs.window_type) || ((window_type == rhs.window_type) && (cell < rhs.cell));
	}
} TItemPos;
#pragma pack(pop)

const DWORD c_QuickBar_Line_Count = 3;
const DWORD c_QuickBar_Slot_Count = 12;

const float c_Idle_WaitTime = 5.0f;

const int c_Monster_Race_Start_Number = 6;
const int c_Monster_Model_Start_Number = 20001;

const float c_fAttack_Delay_Time = 0.2f;
const float c_fHit_Delay_Time = 0.1f;
const float c_fCrash_Wave_Time = 0.2f;
const float c_fCrash_Wave_Distance = 3.0f;

const float c_fHeight_Step_Distance = 50.0f;

enum
{
	DISTANCE_TYPE_FOUR_WAY,
	DISTANCE_TYPE_EIGHT_WAY,
	DISTANCE_TYPE_ONE_WAY,
	DISTANCE_TYPE_MAX_NUM,
};

const float c_fMagic_Script_Version = 1.0f;
const float c_fSkill_Script_Version = 1.0f;
const float c_fMagicSoundInformation_Version = 1.0f;
const float c_fBattleCommand_Script_Version = 1.0f;
const float c_fEmotionCommand_Script_Version = 1.0f;
const float c_fActive_Script_Version = 1.0f;
const float c_fPassive_Script_Version = 1.0f;

// Used by PushMove
const float c_fWalkDistance = 175.0f;
const float c_fRunDistance = 310.0f;

#define FILE_MAX_LEN 128

enum
{
	#ifdef ENABLE_EXTRA_SOCKET_SYSTEM
	ITEM_ACCESSORY_SOCKET_MAX_NUM = 4,
	ITEM_SOCKET_SLOT_MAX_NUM = 4,
	#else
	ITEM_ACCESSORY_SOCKET_MAX_NUM = 3,
	ITEM_SOCKET_SLOT_MAX_NUM = 3,
	#endif

	// refactored attribute slot begin
	ITEM_ATTRIBUTE_SLOT_NORM_NUM	= 5,
	ITEM_ATTRIBUTE_SLOT_RARE_NUM	= 2,

	ITEM_ATTRIBUTE_SLOT_NORM_START	= 0,
	ITEM_ATTRIBUTE_SLOT_NORM_END	= ITEM_ATTRIBUTE_SLOT_NORM_START + ITEM_ATTRIBUTE_SLOT_NORM_NUM,

	ITEM_ATTRIBUTE_SLOT_RARE_START	= ITEM_ATTRIBUTE_SLOT_NORM_END,
	ITEM_ATTRIBUTE_SLOT_RARE_END	= ITEM_ATTRIBUTE_SLOT_RARE_START + ITEM_ATTRIBUTE_SLOT_RARE_NUM,

	ITEM_ATTRIBUTE_SLOT_MAX_NUM		= ITEM_ATTRIBUTE_SLOT_RARE_END, // 7
	// refactored attribute slot end
};

#pragma pack(push)
#pragma pack(1)

typedef struct SQuickSlot
{
	BYTE Type;
	BYTE Position;
} TQuickSlot;

typedef struct TPlayerItemAttribute
{
	BYTE		bType;
	short		sValue;
} TPlayerItemAttribute;

typedef struct packet_item
{
	DWORD		vnum;
	#ifdef ENABLE_NEW_MAX_COUNT
	short		count;
	#else
	BYTE		count;
	#endif
	DWORD		flags;
	DWORD		anti_flags;
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
} TItemData;

typedef struct packet_shop_item
{
	DWORD		vnum;
	#ifdef ENABLE_FULL_YANG
	uint64_t	price;
	#else
	DWORD		price;
	#endif
	#ifdef ENABLE_NEW_MAX_COUNT
	short		count;
	#else
	BYTE		count;
	#endif
	BYTE		display_pos;
	LONG		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TShopItemData;

#ifdef ENABLE_BATTLE_PASS
typedef struct SBattlePassRewardItem
{
	BYTE		bRewardType;
	DWORD dwVnum;
	WORD wCount;
} TBattlePassRewardItem;

typedef struct SBattlePassMissionInfo
{
	BYTE bMissionID;
	char	szMissionName[64];
	BYTE bMissionType;
	DWORD dwMissionInfo[3];
	TBattlePassRewardItem aRewardList[3];
} TBattlePassMissionInfo;
#endif

#pragma pack(pop)

inline float GetSqrtDistance(int ix1, int iy1, int ix2, int iy2) // By sqrt
{
	float dx, dy;

	dx = float(ix1 - ix2);
	dy = float(iy1 - iy2);

	return sqrtf(dx*dx + dy*dy);
}

// DEFAULT_FONT
void DefaultFont_Startup();
void DefaultFont_Cleanup();
void DefaultFont_SetName(const char * c_szFontName);
CResource* DefaultFont_GetResource();
CResource* DefaultItalicFont_GetResource();
// END_OF_DEFAULT_FONT

void SetGuildSymbolPath(const char * c_szPathName);
const char * GetGuildSymbolFileName(DWORD dwGuildID);
BYTE SlotTypeToInvenType(BYTE bSlotType);
BYTE ApplyTypeToPointType(BYTE bApplyType);
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
