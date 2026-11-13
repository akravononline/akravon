#pragma once

//        Icon, Model (droped on ground), Game Data

#include "../eterLib/GrpSubImage.h"
#include "../eterGrnLib/Thing.h"
#include "GameLibDefines.h"


class CItemData
{
	public:
		enum
		{
			ITEM_NAME_MAX_LEN = 64,
			ITEM_LIMIT_MAX_NUM = 2,
			ITEM_VALUES_MAX_NUM = 6,
			ITEM_SMALL_DESCR_MAX_LEN = 256,
			ITEM_APPLY_MAX_NUM = 3,
			ITEM_SOCKET_MAX_NUM = 3,
			#ifdef __UNIMPLEMENTED__
			ITEM_PICK_MAX_LEVEL = 9,
			ITEM_ROD_MAX_LEVEL = 20,
			#endif
		};

		#ifdef ENABLE_SOULBIND_SYSTEM
		enum ESealBind
		{
			SEAL_BIND_FLAG_DROP,
			SEAL_BIND_FLAG_UPGRADE,
			SEAL_BIND_FLAG_SELL,
			SEAL_BIND_FLAG_ENCHANT,
			SEAL_BIND_FLAG_TRADE,
			SEAL_BIND_FLAG_UNSEAL,
			SEAL_BIND_FLAG_MAX,
		};

		enum ESealDate
		{
			SEAL_DATE_DEFAULT_TIMESTAMP = 0, // 2015/11/12 it's -1
			SEAL_DATE_UNLIMITED_TIMESTAMP = -1, // 2015/11/12 it doesn't exist
		};

		enum ESealItem
		{
			SEAL_ITEM_BINDING_VNUM = 50263,
			SEAL_ITEM_UNBINDING_VNUM = 50264,
		};
		#endif

		#ifdef __UNIMPLEMENTED__
		enum EPetData
		{
			EGG_USE_SUCCESS = 0,
			EGG_USE_FAILED_TIMEOVER = 2,
			EGG_USE_FAILED_BECAUSE_NAME = 1,
			GROWTH_PET_ITEM_USE_COOL_TIME = 1,
			PET_EGG_USE_TRUE = 0,
			PET_EGG_USE_FAILED_BECAUSE_TRADING = 1,
			PET_EGG_USE_FAILED_BECAUSE_SHOP_OPEN = 2,
			PET_EGG_USE_FAILED_BECAUSE_MALL_OPEN = 3,
			PET_EGG_USE_FAILED_BECAUSE_SAFEBOX_OPEN = 4,
			PET_HATCHING_MONEY = 100000,
			PET_NAME_MAX_SIZE = 20,
			PET_NAME_MIN_SIZE = 4,
		};
		#endif

		enum EItemType
		{
			ITEM_TYPE_NONE,					//0
			ITEM_TYPE_WEAPON,				//1
			ITEM_TYPE_ARMOR,				//2
			ITEM_TYPE_USE,					//3
			ITEM_TYPE_AUTOUSE,				//4
			ITEM_TYPE_MATERIAL,				//5
			ITEM_TYPE_SPECIAL,				//6
			ITEM_TYPE_TOOL,					//7
			ITEM_TYPE_LOTTERY,				//8
			ITEM_TYPE_ELK,					//9
			ITEM_TYPE_METIN,				//10
			ITEM_TYPE_CONTAINER,			//11
			ITEM_TYPE_FISH,					//12
			ITEM_TYPE_ROD,					//13
			ITEM_TYPE_RESOURCE,				//14
			ITEM_TYPE_CAMPFIRE,				//15
			ITEM_TYPE_UNIQUE,				//16
			ITEM_TYPE_SKILLBOOK,			//17
			ITEM_TYPE_QUEST,				//18
			ITEM_TYPE_POLYMORPH,			//19
			ITEM_TYPE_TREASURE_BOX,			//20
			ITEM_TYPE_TREASURE_KEY,			//21
			ITEM_TYPE_SKILLFORGET,			//22
			ITEM_TYPE_GIFTBOX,				//23
			ITEM_TYPE_PICK,					//24
			ITEM_TYPE_HAIR,					//25
			ITEM_TYPE_TOTEM,				//26
			ITEM_TYPE_BLEND,				//27
			ITEM_TYPE_COSTUME,				//28
			ITEM_TYPE_DS,					//29
			ITEM_TYPE_SPECIAL_DS,			//30
			ITEM_TYPE_EXTRACT,				//31
			ITEM_TYPE_SECONDARY_COIN,		//32
			ITEM_TYPE_RING,					//33
			ITEM_TYPE_BELT,					//34
			ITEM_TYPE_PET,					//35

			ITEM_TYPE_MAX_NUM,
		};

		enum EPetSubTypes
		{
			PET_EGG,
			PET_UPBRINGING,
			PET_BAG,
			PET_FEEDSTUFF,
			PET_SKILL,
			PET_SKILL_DEL_BOOK,
			PET_NAME_CHANGE,
			PET_EXPFOOD,
			PET_SKILL_ALL_DEL_BOOK,
			PET_EXPFOOD_PER,
			PET_ATTR_DETERMINE,
			PET_ATTR_CHANGE,
			PET_PAY,
			PET_PRIMIUM_FEEDSTUFF,
		};

		enum EWeaponSubTypes
		{
			WEAPON_SWORD, //0
			WEAPON_DAGGER,
			WEAPON_BOW, //2
			WEAPON_TWO_HANDED, //3
			WEAPON_BELL, //4
			WEAPON_FAN, //5
			WEAPON_ARROW, //6
			WEAPON_MOUNT_SPEAR, //7
			#ifdef ENABLE_WOLFMAN_CHARACTER
			WEAPON_CLAW = 8, //8
			#endif
			#ifdef ENABLE_QUIVER_SYSTEM
			WEAPON_QUIVER = 9, //9
			#endif
			#ifdef __UNIMPLEMENTED__
			WEAPON_BOUQUET = 10, //10
			#endif
			WEAPON_NUM_TYPES, //11 2015/11/12

			WEAPON_NONE = WEAPON_NUM_TYPES+1,
		};

		enum EMaterialSubTypes
		{
			MATERIAL_LEATHER,
			MATERIAL_BLOOD,
			MATERIAL_ROOT,
			MATERIAL_NEEDLE,
			MATERIAL_JEWEL,
			MATERIAL_DS_REFINE_NORMAL,
			MATERIAL_DS_REFINE_BLESSED,
			MATERIAL_DS_REFINE_HOLLY,
		};

		enum EArmorSubTypes
		{
			ARMOR_BODY,
			ARMOR_HEAD,
			ARMOR_SHIELD,
			ARMOR_WRIST,
			ARMOR_FOOTS,
		    ARMOR_NECK,
			ARMOR_EAR,
			ARMOR_NUM_TYPES
		};

		enum ECostumeSubTypes
		{
			COSTUME_BODY,
			COSTUME_HAIR,
			#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
			COSTUME_MOUNT	= 2,		//2
			#endif
			#ifdef ENABLE_ACCE_COSTUME_SYSTEM
			COSTUME_ACCE	= 3,		//3
			#endif
			#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
			COSTUME_WEAPON	= 4,		//4
			#endif
			#ifdef ENABLE_COSTUME_PET_SYSTEM
			COSTUME_PET		= 5,		//5
			#endif
			COSTUME_NUM_TYPES,
		};

		enum EUseSubTypes
		{
			USE_POTION,						// 0
			USE_TALISMAN,					// 1
			USE_TUNING,						// 2
			USE_MOVE,						// 3
			USE_TREASURE_BOX,				// 4
			USE_MONEYBAG,					// 5
			USE_BAIT,						// 6
			USE_ABILITY_UP,					// 7
			USE_AFFECT,						// 8
			USE_CREATE_STONE,				// 9
			USE_SPECIAL,					// 10
			USE_POTION_NODELAY,				// 11
			USE_CLEAR,						// 12
			USE_INVISIBILITY,				// 13
			USE_DETACHMENT,					// 14
			USE_BUCKET,						// 15
			USE_POTION_CONTINUE,			// 16
			USE_CLEAN_SOCKET,				// 17
			USE_CHANGE_ATTRIBUTE,			// 18
			USE_ADD_ATTRIBUTE,				// 19
			USE_ADD_ACCESSORY_SOCKET,		// 20
			USE_PUT_INTO_ACCESSORY_SOCKET,	// 21
			USE_ADD_ATTRIBUTE2,				// 22
			USE_RECIPE,						// 23
			USE_CHANGE_ATTRIBUTE2,			// 24
			USE_BIND,						// 25
			USE_UNBIND,						// 26
			USE_TIME_CHARGE_PER,			// 27
			USE_TIME_CHARGE_FIX,			// 28
			USE_PUT_INTO_BELT_SOCKET,		// 29
			USE_PUT_INTO_RING_SOCKET,		// 30
			#ifdef ENABLE_USE_COSTUME_ATTR
			USE_CHANGE_COSTUME_ATTR,		// 31
			USE_RESET_COSTUME_ATTR,			// 32
			#endif
			#ifdef ENABLE_KOSTUM_SAC_BINEK_PET_EFSUNLAMA
			USE_ATTRIBUTE_MOUNT,			// 33
			USE_ATTRIBUTE_PET,				// 34
			#endif
			#ifdef ENABLE_COSTUME_WEAPON_ATTR
			USE_ATTRIBUTE_COSTUME_WEAPON,	// 35
			#endif
		};

		enum EDragonSoulSubType
		{
			DS_SLOT1,
			DS_SLOT2,
			DS_SLOT3,
			DS_SLOT4,
			DS_SLOT5,
			DS_SLOT6,
			DS_SLOT_NUM_TYPES = 6,
		};

		enum EMetinSubTypes
		{
			METIN_NORMAL,
			METIN_GOLD,
		};

		enum ELimitTypes
		{
			LIMIT_NONE,

			LIMIT_LEVEL,
			LIMIT_STR,
			LIMIT_DEX,
			LIMIT_INT,
			LIMIT_CON,
			LIMIT_PCBANG,

			LIMIT_REAL_TIME,

			LIMIT_REAL_TIME_START_FIRST_USE,

			LIMIT_TIMER_BASED_ON_WEAR,

			LIMIT_MAX_NUM
		};

		enum EItemAntiFlag
		{
			ITEM_ANTIFLAG_FEMALE		= (1 << 0),
			ITEM_ANTIFLAG_MALE			= (1 << 1),
			ITEM_ANTIFLAG_WARRIOR		= (1 << 2),
			ITEM_ANTIFLAG_ASSASSIN		= (1 << 3),
			ITEM_ANTIFLAG_SURA			= (1 << 4),
			ITEM_ANTIFLAG_SHAMAN		= (1 << 5),
			ITEM_ANTIFLAG_GET			= (1 << 6),
			ITEM_ANTIFLAG_DROP			= (1 << 7),
			ITEM_ANTIFLAG_SELL			= (1 << 8),
			ITEM_ANTIFLAG_EMPIRE_A		= (1 << 9),
			ITEM_ANTIFLAG_EMPIRE_B		= (1 << 10),
			ITEM_ANTIFLAG_EMPIRE_R		= (1 << 11),
			ITEM_ANTIFLAG_SAVE			= (1 << 12),
			ITEM_ANTIFLAG_GIVE			= (1 << 13),
			ITEM_ANTIFLAG_PKDROP		= (1 << 14),
			ITEM_ANTIFLAG_STACK			= (1 << 15),
			ITEM_ANTIFLAG_MYSHOP		= (1 << 16),
			ITEM_ANTIFLAG_SAFEBOX		= (1 << 17),
			#ifdef ENABLE_WOLFMAN_CHARACTER
			ITEM_ANTIFLAG_WOLFMAN		= (1 << 18),
			#endif
		};

		enum EItemFlag
		{
			ITEM_FLAG_REFINEABLE			= (1 << 0),
			ITEM_FLAG_SAVE					= (1 << 1),
			ITEM_FLAG_STACKABLE				= (1 << 2),
			ITEM_FLAG_COUNT_PER_1GOLD		= (1 << 3),
			ITEM_FLAG_SLOW_QUERY			= (1 << 4),
			ITEM_FLAG_RARE					= (1 << 5),
			ITEM_FLAG_UNIQUE				= (1 << 6),
			ITEM_FLAG_MAKECOUNT				= (1 << 7),
			ITEM_FLAG_IRREMOVABLE			= (1 << 8),
			ITEM_FLAG_CONFIRM_WHEN_USE		= (1 << 9),
			ITEM_FLAG_QUEST_USE				= (1 << 10),
			ITEM_FLAG_QUEST_USE_MULTIPLE	= (1 << 11),
			ITEM_FLAG_QUEST_GIVE			= (1 << 12),
			ITEM_FLAG_LOG					= (1 << 13),
			ITEM_FLAG_APPLICABLE			= (1 << 14),
			#ifdef ENABLE_EVOLUTION_SYSTEM
			ITEM_FLAG_ITEM_RARE_ABILITY		= (1 << 15),
			#endif
		};

		enum EWearPositions
		{
			WEAR_BODY,				// 0
			WEAR_HEAD,				// 1
			WEAR_FOOTS,				// 2
			WEAR_WRIST,				// 3
			WEAR_WEAPON,			// 4
			WEAR_NECK,				// 5
			WEAR_EAR,				// 6
			WEAR_UNIQUE1,			// 7
			WEAR_UNIQUE2,			// 8
			WEAR_ARROW,				// 9
			WEAR_SHIELD,			// 10
			WEAR_ABILITY1,			// 11
			WEAR_ABILITY2,			// 12
			WEAR_ABILITY3,			// 13
			WEAR_ABILITY4,			// 14
			WEAR_ABILITY5,			// 15
			WEAR_ABILITY6,			// 16
			WEAR_ABILITY7,			// 17
			WEAR_ABILITY8,			// 18
			WEAR_COSTUME_BODY,		// 19
			WEAR_COSTUME_HAIR,		// 20
			#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
			WEAR_COSTUME_MOUNT,		// 21
			#endif
			#ifdef ENABLE_ACCE_COSTUME_SYSTEM
			WEAR_COSTUME_ACCE,		// 22
			#endif
			#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
			WEAR_COSTUME_WEAPON,	// 23
			#endif
			#ifdef ENABLE_COSTUME_PET_SYSTEM
			WEAR_COSTUME_PET,		// 24
			#endif
			WEAR_RING1,				// 25
			WEAR_RING2,				// 26
			#ifdef ENABLE_NEW_RING
			WEAR_RING3,				// 27
			WEAR_RING4,				// 28
			WEAR_RING5,				// 29
			WEAR_RING6,				// 30
			#endif

			WEAR_BELT,				// 31

			WEAR_MAX_NUM			= 64,
		};

		enum EItemWearableFlag
		{
			WEARABLE_BODY		= (1 << 0),
			WEARABLE_HEAD		= (1 << 1),
			WEARABLE_FOOTS		= (1 << 2),
			WEARABLE_WRIST		= (1 << 3),
			WEARABLE_WEAPON		= (1 << 4),
			WEARABLE_NECK		= (1 << 5),
			WEARABLE_EAR		= (1 << 6),
			WEARABLE_UNIQUE		= (1 << 7),
			WEARABLE_SHIELD		= (1 << 8),
			WEARABLE_ARROW		= (1 << 9),
		};

		enum EApplyTypes
		{
			APPLY_NONE,						// 0
			APPLY_MAX_HP,					// 1
			APPLY_MAX_SP,					// 2
			APPLY_CON,						// 3
			APPLY_INT,						// 4
			APPLY_STR,						// 5
			APPLY_DEX,						// 6
			APPLY_ATT_SPEED,				// 7
			APPLY_MOV_SPEED,				// 8
			APPLY_CAST_SPEED,				// 9
			APPLY_HP_REGEN,					// 10
			APPLY_SP_REGEN,					// 11
			APPLY_POISON_PCT,				// 12
			APPLY_STUN_PCT,					// 13
			APPLY_SLOW_PCT,					// 14
			APPLY_CRITICAL_PCT,				// 15
			APPLY_PENETRATE_PCT,			// 16
			APPLY_ATTBONUS_HUMAN,			// 17
			APPLY_ATTBONUS_ANIMAL,			// 18
			APPLY_ATTBONUS_ORC,				// 19
			APPLY_ATTBONUS_MILGYO,			// 20
			APPLY_ATTBONUS_UNDEAD,			// 21
			APPLY_ATTBONUS_DEVIL,			// 22
			APPLY_STEAL_HP,					// 23
			APPLY_STEAL_SP,					// 24
			APPLY_MANA_BURN_PCT,			// 25
			APPLY_DAMAGE_SP_RECOVER,		// 26
			APPLY_BLOCK,					// 27
			APPLY_DODGE,					// 28
			APPLY_RESIST_SWORD,				// 29
			APPLY_RESIST_TWOHAND,			// 30
			APPLY_RESIST_DAGGER,			// 31
			APPLY_RESIST_BELL,				// 32
			APPLY_RESIST_FAN,				// 33
			APPLY_RESIST_BOW,				// 34
			APPLY_RESIST_FIRE,				// 35
			APPLY_RESIST_ELEC,				// 36
			APPLY_RESIST_MAGIC,				// 37
			APPLY_RESIST_WIND,				// 38
			APPLY_REFLECT_MELEE,			// 39
			APPLY_REFLECT_CURSE,			// 40
			APPLY_POISON_REDUCE,			// 41
			APPLY_KILL_SP_RECOVER,			// 42
			APPLY_EXP_DOUBLE_BONUS,			// 43
			APPLY_GOLD_DOUBLE_BONUS,		// 44
			APPLY_ITEM_DROP_BONUS,			// 45
			APPLY_POTION_BONUS,				// 46
			APPLY_KILL_HP_RECOVER,			// 47
			APPLY_IMMUNE_STUN,				// 48
			APPLY_IMMUNE_SLOW,				// 49
			APPLY_IMMUNE_FALL,				// 50
			APPLY_SKILL,					// 51
			APPLY_BOW_DISTANCE,				// 52
			APPLY_ATT_GRADE_BONUS,			// 53
			APPLY_DEF_GRADE_BONUS,			// 54
			APPLY_MAGIC_ATT_GRADE,			// 55
			APPLY_MAGIC_DEF_GRADE,			// 56
			APPLY_CURSE_PCT,				// 57
			APPLY_MAX_STAMINA,				// 58
			APPLY_ATT_BONUS_TO_WARRIOR,		// 59
			APPLY_ATT_BONUS_TO_ASSASSIN,	// 60
			APPLY_ATT_BONUS_TO_SURA,		// 61
			APPLY_ATT_BONUS_TO_SHAMAN,		// 62
			APPLY_ATT_BONUS_TO_MONSTER,		// 63
			APPLY_MALL_ATTBONUS,			// 64
			APPLY_MALL_DEFBONUS,			// 65
			APPLY_MALL_EXPBONUS,			// 66
			APPLY_MALL_ITEMBONUS,			// 67
			APPLY_MALL_GOLDBONUS,			// 68
			APPLY_MAX_HP_PCT,				// 69
			APPLY_MAX_SP_PCT,				// 70
			APPLY_SKILL_DAMAGE_BONUS,		// 71
			APPLY_NORMAL_HIT_DAMAGE_BONUS,	// 72
			APPLY_SKILL_DEFEND_BONUS,		// 73
			APPLY_NORMAL_HIT_DEFEND_BONUS,	// 74
			APPLY_EXTRACT_HP_PCT,			// 75
			APPLY_PC_BANG_EXP_BONUS,		// 76
			APPLY_PC_BANG_DROP_BONUS,		// 77
			APPLY_RESIST_WARRIOR,			// 78
			APPLY_RESIST_ASSASSIN ,			// 79
			APPLY_RESIST_SURA,				// 80
			APPLY_RESIST_SHAMAN,			// 81
			APPLY_ENERGY,					// 82
			APPLY_DEF_GRADE,				// 83
			APPLY_COSTUME_ATTR_BONUS,		// 84
			APPLY_MAGIC_ATTBONUS_PER,		// 85
			APPLY_MELEE_MAGIC_ATTBONUS_PER,	// 86

			APPLY_RESIST_ICE,				// 87
			APPLY_RESIST_EARTH,				// 88
			APPLY_RESIST_DARK,				// 89

			APPLY_ANTI_CRITICAL_PCT,		// 90
			APPLY_ANTI_PENETRATE_PCT,		// 91

			#ifdef ENABLE_WOLFMAN_CHARACTER
			APPLY_BLEEDING_REDUCE			= 92,
			APPLY_BLEEDING_PCT				= 93,
			APPLY_ATT_BONUS_TO_WOLFMAN		= 94,
			APPLY_RESIST_WOLFMAN			= 95,
			APPLY_RESIST_CLAW				= 96,
			#endif

			#ifdef ENABLE_ACCE_COSTUME_SYSTEM
			APPLY_ACCEDRAIN_RATE			= 97,
			#endif

			#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
			APPLY_RESIST_MAGIC_REDUCTION	= 98,
			#endif

			APPLY_ENCHANT_ELECT				= 99,
			APPLY_ENCHANT_FIRE				= 100,
			APPLY_ENCHANT_ICE				= 101,
			APPLY_ENCHANT_WIND				= 102,
			APPLY_ENCHANT_EARTH				= 103,
			APPLY_ENCHANT_DARK				= 104,

			#ifdef ENABLE_NEW_ATTRIBUTE_TYPES
			APPLY_ATTBONUS_STONE			= 105,
			APPLY_ATTBONUS_BOSS				= 106,
			APPLY_RESIST_HUMAN				= 107,
			APPLY_RESIST_MONSTER			= 108,
			#endif

			#ifdef ENABLE_PVM_BONUS
			APPLY_SKILL_DAMAGE_BONUS_PVM	= 109,
			APPLY_HIT_DAMAGE_BONUS_PVM		= 110,
			APPLY_ATT_GRADE_BONUS_PVM		= 111,
			APPLY_DEF_GRADE_BONUS_PVM		= 112,
			APPLY_BERSERKER_BONUS_PVM		= 113,
			#endif

			MAX_APPLY_NUM					= 127,
		};

		enum EImmuneFlags
		{
			IMMUNE_PARA			= (1 << 0),
			IMMUNE_CURSE		= (1 << 1),
			IMMUNE_STUN			= (1 << 2),
			IMMUNE_SLEEP		= (1 << 3),
			IMMUNE_SLOW			= (1 << 4),
			IMMUNE_POISON		= (1 << 5),
			IMMUNE_TERROR		= (1 << 6),
		};

#pragma pack(push)
#pragma pack(1)
		typedef struct SItemLimit
		{
			BYTE        bType;
			LONG        lValue;
		} TItemLimit;

		typedef struct SItemApply
		{
			BYTE        bType;
			LONG        lValue;
		} TItemApply;

		typedef struct SItemTable_r152
		{
			DWORD       dwVnum;
			char        szName[ITEM_NAME_MAX_LEN + 1];
			char        szLocaleName[ITEM_NAME_MAX_LEN + 1];
			BYTE        bType;
			BYTE        bSubType;

			BYTE        bWeight;
			BYTE        bSize;

			DWORD       dwAntiFlags;
			DWORD       dwFlags;
			DWORD       dwWearFlags;
			DWORD       dwImmuneFlag;

			DWORD       dwIBuyItemPrice;
			DWORD		dwISellItemPrice;

			TItemLimit  aLimits[ITEM_LIMIT_MAX_NUM];
			TItemApply  aApplies[ITEM_APPLY_MAX_NUM];
			LONG        alValues[ITEM_VALUES_MAX_NUM];
			LONG        alSockets[ITEM_SOCKET_MAX_NUM];
			DWORD       dwRefinedVnum;
			WORD		wRefineSet;
			BYTE        bAlterToMagicItemPct;
			BYTE		bSpecular;
			BYTE        bGainSocketPct;
		} TItemTable_r152;

		typedef struct SItemTable_r156
		{
			DWORD       dwVnum;
			DWORD       dwVnumRange;
			char        szName[ITEM_NAME_MAX_LEN + 1];
			char        szLocaleName[ITEM_NAME_MAX_LEN + 1];
			BYTE        bType;
			BYTE        bSubType;

			BYTE        bWeight;
			BYTE        bSize;

			DWORD       dwAntiFlags;
			DWORD       dwFlags;
			DWORD       dwWearFlags;
			DWORD       dwImmuneFlag;

			DWORD       dwIBuyItemPrice;
			DWORD		dwISellItemPrice;

			TItemLimit  aLimits[ITEM_LIMIT_MAX_NUM];
			TItemApply  aApplies[ITEM_APPLY_MAX_NUM];
			LONG        alValues[ITEM_VALUES_MAX_NUM];
			LONG        alSockets[ITEM_SOCKET_MAX_NUM];
			DWORD       dwRefinedVnum;
			WORD		wRefineSet;
			BYTE        bAlterToMagicItemPct;
			BYTE		bSpecular;
			BYTE        bGainSocketPct;
		} TItemTable_r156;

		typedef struct SItemTable_r158
		{
			DWORD       dwVnum;
			DWORD       dwVnumRange;
			char        szName[ITEM_NAME_MAX_LEN + 1];
			char        szLocaleName[ITEM_NAME_MAX_LEN + 1];
			BYTE        bType;
			BYTE        bSubType;

			BYTE        bWeight;
			BYTE        bSize;

			DWORD       dwAntiFlags;
			DWORD       dwFlags;
			DWORD       dwWearFlags;
			DWORD       dwImmuneFlag;

			DWORD       dwIBuyItemPrice;
			DWORD		dwISellItemPrice;

			TItemLimit  aLimits[ITEM_LIMIT_MAX_NUM];
			TItemApply  aApplies[ITEM_APPLY_MAX_NUM];
			LONG        alValues[ITEM_VALUES_MAX_NUM];
			LONG        alSockets[ITEM_SOCKET_MAX_NUM];
			DWORD       dwRefinedVnum;
			WORD		wRefineSet;
			BYTE        bAlterToMagicItemPct;
			BYTE		bSpecular;
			BYTE        bGainSocketPct;
			WORD		wWearableFlag;
		} TItemTable_r158;

		typedef SItemTable_r156 SItemTable, TItemTable;

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		struct SScaleInfo
		{
			float	fScaleX, fScaleY, fScaleZ;
			float	fPositionX, fPositionY, fPositionZ;
		};

		typedef struct SScaleTable
		{
			SScaleInfo	tInfo[10];
		} TScaleTable;
#endif

#ifdef ENABLE_PROTOSTRUCT_AUTODETECT
		typedef struct SItemTableAll
		{
			static bool IsValidStruct(DWORD structSize)
			{
				switch (structSize)
				{
					case sizeof(TItemTable_r152):
					case sizeof(TItemTable_r156):
					case sizeof(TItemTable_r158):
						return true;
						break;
				}
				return false;
			}

			static void Process(void* obj, DWORD structSize, DWORD i, CItemData::TItemTable& t)
			{
				#define ITABLE_COPY_STR(x) strncpy_s(t.##x##, sizeof(t.##x##), r.##x##, _TRUNCATE)
				#define ITABLE_COPY_INT(x) t.##x## = r.##x
				#define ITABLE_ZERO_STR(x) memset(t.##x##, 0, sizeof(t.##x##));
				#define ITABLE_ZERO_INT(x) t.##x## = 0
				#define ITABLE_COUNT(x) _countof(t.##x##)
				#define ITABLE_PROCESS(len)\
					CItemData::TItemTable_r##len## & r = *((CItemData::TItemTable_r##len## *) obj + i);\
					ITABLE_COPY_INT(dwVnum);\
					ITABLE_COPY_STR(szName);\
					ITABLE_COPY_STR(szLocaleName);\
					ITABLE_COPY_INT(bType);\
					ITABLE_COPY_INT(bSubType);\
					ITABLE_COPY_INT(bWeight);\
					ITABLE_COPY_INT(bSize);\
					ITABLE_COPY_INT(dwAntiFlags);\
					ITABLE_COPY_INT(dwFlags);\
					ITABLE_COPY_INT(dwWearFlags);\
					ITABLE_COPY_INT(dwImmuneFlag);\
					ITABLE_COPY_INT(dwIBuyItemPrice);\
					ITABLE_COPY_INT(dwISellItemPrice);\
					for (size_t i=0; i<ITABLE_COUNT(aLimits); ++i)\
					{\
						ITABLE_COPY_INT(aLimits[i].bType);\
						ITABLE_COPY_INT(aLimits[i].lValue);\
					}\
					for (size_t i=0; i<CItemData::ITEM_APPLY_MAX_NUM; ++i)\
					{\
						ITABLE_COPY_INT(aApplies[i].bType);\
						ITABLE_COPY_INT(aApplies[i].lValue);\
					}\
					for (size_t i=0; i<CItemData::ITEM_VALUES_MAX_NUM; ++i)\
					{\
						ITABLE_COPY_INT(alValues[i]);\
					}\
					for (size_t i=0; i<CItemData::ITEM_SOCKET_MAX_NUM; ++i)\
					{\
						ITABLE_COPY_INT(alSockets[i]);\
					}\
					ITABLE_COPY_INT(dwRefinedVnum);\
					ITABLE_COPY_INT(wRefineSet);\
					ITABLE_COPY_INT(bAlterToMagicItemPct);\
					ITABLE_COPY_INT(bSpecular);\
					ITABLE_COPY_INT(bGainSocketPct);

				switch (structSize)
				{
					case sizeof(TItemTable_r152):
						{
							ITABLE_PROCESS(152);
							ITABLE_ZERO_INT(dwVnumRange);
						}
						break;
					case sizeof(TItemTable_r156):
						{
							ITABLE_PROCESS(156);
							ITABLE_COPY_INT(dwVnumRange);
						}
						break;
					case sizeof(TItemTable_r158):
						{
							ITABLE_PROCESS(158);
							ITABLE_COPY_INT(dwVnumRange);
						}
						break;
				}
			}
		} TItemTableAll;
#endif //ENABLE_PROTOSTRUCT_AUTODETECT

#pragma pack(pop)

	public:
		CItemData();
		virtual ~CItemData();

		void Clear();
		void SetSummary(const std::string& c_rstSumm);
		void SetDescription(const std::string& c_rstDesc);

		CGraphicThing * GetModelThing();
		CGraphicThing * GetSubModelThing();
		CGraphicThing * GetDropModelThing();
		CGraphicSubImage * GetIconImage();

		DWORD GetLODModelThingCount();
		BOOL GetLODModelThingPointer(DWORD dwIndex, CGraphicThing ** ppModelThing);

		DWORD GetAttachingDataCount();
		BOOL GetCollisionDataPointer(DWORD dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData);
		BOOL GetAttachingDataPointer(DWORD dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData);

		/////
		const TItemTable*	GetTable() const;
		DWORD GetIndex() const;
		const char * GetName() const;
		const char * GetDescription() const;
		const char * GetSummary() const;
		BYTE GetType() const;
		BYTE GetSubType() const;
		UINT GetRefine() const;
		const char* GetUseTypeString() const;
		DWORD GetWeaponType() const;
		BYTE GetSize() const;
		BOOL IsAntiFlag(DWORD dwFlag) const;
		BOOL IsFlag(DWORD dwFlag) const;
		BOOL IsWearableFlag(DWORD dwFlag) const;
		BOOL HasNextGrade() const;
		DWORD GetWearFlags() const;
		DWORD GetIBuyItemPrice() const;
		DWORD GetISellItemPrice() const;
		BOOL GetLimit(BYTE byIndex, TItemLimit * pItemLimit) const;
		BOOL GetApply(BYTE byIndex, TItemApply * pItemApply) const;
		LONG GetValue(BYTE byIndex) const;
		LONG GetSocket(BYTE byIndex) const;
		LONG SetSocket(BYTE byIndex,DWORD value);
		int GetSocketCount() const;
		DWORD GetIconNumber() const;

		UINT	GetSpecularPoweru() const;
		float	GetSpecularPowerf() const;

		/////

		BOOL IsEquipment() const;

		/////

		//BOOL LoadItemData(const char * c_szFileName);
		void SetDefaultItemData(const char * c_szIconFileName, const char * c_szModelFileName  = NULL);
		void SetItemTableData(TItemTable * pItemTable);
		// AKRAVON: bagli kopya esya - kaynak esyanin gorunumu, tablosu ve aciklamasi; yeni numara, ad ve ek antiflag
		void AkravonCloneFrom(const CItemData & rBase, DWORD dwVnum, const char * c_szName, DWORD dwAddAntiFlags, LONG lValue0 = -1, const char * c_szDesc = NULL);
		void AkravonPatch(const char * c_szName, DWORD dwAddAntiFlags, LONG lValue0, const char * c_szDesc);

	protected:
		void __LoadFiles();
		void __SetIconImage(const char * c_szFileName);

	protected:
		std::string m_strModelFileName;
		std::string m_strSubModelFileName;
		std::string m_strDropModelFileName;
		std::string m_strIconFileName;
		std::string m_strDescription;
		std::string m_strSummary;
		std::vector<std::string> m_strLODModelFileNameVector;

		CGraphicThing * m_pModelThing;
		CGraphicThing * m_pSubModelThing;
		CGraphicThing * m_pDropModelThing;
		CGraphicSubImage * m_pIconImage;
		std::vector<CGraphicThing *> m_pLODModelThingVector;

		NRaceData::TAttachingDataVector m_AttachingDataVector;
		DWORD		m_dwVnum;
		TItemTable m_ItemTable;

	public:
		static void DestroySystem();

		static CItemData* New();
		static void Delete(CItemData* pkItemData);

		static CDynamicPool<CItemData>		ms_kPool;
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	protected:
		typedef struct SItemScaleTable
		{
			D3DXVECTOR3 v3Scale[NRaceData::SEX_MAX_NUM][NRaceData::JOB_MAX_NUM]{};
			float fScaleParticle[NRaceData::SEX_MAX_NUM][NRaceData::JOB_MAX_NUM]{};
		} TItemScaleTable;
		TItemScaleTable m_ItemScaleTable;
	public:
		float GetItemParticleScale(BYTE bJob, BYTE bSex);
		void SetItemTableScaleData(BYTE bJob, BYTE bSex, float fScaleX, float fScaleY, float fScaleZ, float fScaleParticle);
		D3DXVECTOR3& GetItemScaleVector(BYTE bJob, BYTE bSex);
#endif
};
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
