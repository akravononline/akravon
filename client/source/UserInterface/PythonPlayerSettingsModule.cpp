#include "StdAfx.h"

#ifdef ENABLE_CPP_PSM
#include "PythonPlayerSettingsModule.h"
#include "InstanceBase.h"
#include "../EffectLib/EffectManager.h"
#include "PythonSkill.h"
#include "../gamelib/RaceManager.h"
#include "PythonItem.h"
#include "PythonPlayer.h"

class CRaceMotionData;
class CInstanceBase;

static const DWORD GUILD_SKILL_DRAGONBLOOD = CRaceMotionData::NAME_SKILL + 101;
static const DWORD GUILD_SKILL_DRAGONBLESS = CRaceMotionData::NAME_SKILL + 102;
static const DWORD GUILD_SKILL_BLESSARMOR = CRaceMotionData::NAME_SKILL + 103;
static const DWORD GUILD_SKILL_SPPEDUP = CRaceMotionData::NAME_SKILL + 104;
static const DWORD GUILD_SKILL_DRAGONWRATH = CRaceMotionData::NAME_SKILL + 105;
static const DWORD GUILD_SKILL_MAGICUP = CRaceMotionData::NAME_SKILL + 106;

static const DWORD HORSE_SKILL_WILDATTACK = CRaceMotionData::NAME_SKILL + 121;
static const DWORD HORSE_SKILL_CHARGE = CRaceMotionData::NAME_SKILL + 122;
static const DWORD HORSE_SKILL_SPLASH = CRaceMotionData::NAME_SKILL + 123;

const bool CPlayerSettingsModule::LoadInitData()
{
	static const std::vector<TEffect> m_vecEffectData =
	{
		{ CInstanceBase::EFFECT_DUST, "", "d:/ymir work/effect/etc/dust/dust.mse" },
		{ CInstanceBase::EFFECT_HORSE_DUST, "", "d:/ymir work/effect/etc/dust/running_dust.mse" },
		{ CInstanceBase::EFFECT_HIT, "", "d:/ymir work/effect/hit/blow_1/blow_1_low.mse" },

		{ CInstanceBase::EFFECT_HPUP_RED, "", "d:/ymir work/effect/etc/recuperation/drugup_red.mse" },
		{ CInstanceBase::EFFECT_SPUP_BLUE, "", "d:/ymir work/effect/etc/recuperation/drugup_blue.mse" },
		{ CInstanceBase::EFFECT_SPEEDUP_GREEN, "", "d:/ymir work/effect/etc/recuperation/drugup_green.mse" },
		{ CInstanceBase::EFFECT_DXUP_PURPLE, "", "d:/ymir work/effect/etc/recuperation/drugup_purple.mse" },

		//#HP, SP
		{ CInstanceBase::EFFECT_AUTO_HPUP, "", "d:/ymir work/effect/etc/recuperation/autodrugup_red.mse" },
		{ CInstanceBase::EFFECT_AUTO_SPUP, "", "d:/ymir work/effect/etc/recuperation/autodrugup_blue.mse" },

		//#
		{ CInstanceBase::EFFECT_RAMADAN_RING_EQUIP, "", "d:/ymir work/effect/etc/buff/buff_item1.mse" },

		//#
		{ CInstanceBase::EFFECT_HALLOWEEN_CANDY_EQUIP, "", "d:/ymir work/effect/etc/buff/buff_item2.mse" },

		//#
		{ CInstanceBase::EFFECT_HAPPINESS_RING_EQUIP, "", "d:/ymir work/effect/etc/buff/buff_item3.mse" },

		//#
		{ CInstanceBase::EFFECT_LOVE_PENDANT_EQUIP, "", "d:/ymir work/effect/etc/buff/buff_item4.mse" },

		#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		{ CInstanceBase::EFFECT_ACCE_SUCCEDED, "", "d:/ymir work/effect/etc/buff/buff_item6.mse" },
		{ CInstanceBase::EFFECT_ACCE_EQUIP, "", "d:/ymir work/effect/etc/buff/buff_item7.mse" },
		#endif

		{ CInstanceBase::EFFECT_PENETRATE, "Bip01", "d:/ymir work/effect/hit/gwantong.mse" },
		{ CInstanceBase::EFFECT_FIRECRACKER, "", "d:/ymir work/effect/etc/firecracker/newyear_firecracker.mse" },
		{ CInstanceBase::EFFECT_SPIN_TOP, "", "d:/ymir work/effect/etc/firecracker/paing_i.mse" },
		{ CInstanceBase::EFFECT_SELECT, "", "d:/ymir work/effect/etc/click/click_select.mse" },
		{ CInstanceBase::EFFECT_TARGET, "", "d:/ymir work/effect/etc/click/click_glow_select.mse" },
		{ CInstanceBase::EFFECT_STUN, "Bip01 Head", "d:/ymir work/effect/etc/stun/stun.mse" },
		{ CInstanceBase::EFFECT_CRITICAL, "Bip01 R Hand", "d:/ymir work/effect/hit/critical.mse" },

		{ CInstanceBase::EFFECT_DAMAGE_TARGET, "", "d:/ymir work/effect/affect/damagevalue/target.mse" },
		{ CInstanceBase::EFFECT_DAMAGE_NOT_TARGET, "", "d:/ymir work/effect/affect/damagevalue/nontarget.mse" },
		{ CInstanceBase::EFFECT_DAMAGE_SELFDAMAGE, "", "d:/ymir work/effect/affect/damagevalue/damage.mse" },
		{ CInstanceBase::EFFECT_DAMAGE_SELFDAMAGE2, "", "d:/ymir work/effect/affect/damagevalue/damage_1.mse" },
		{ CInstanceBase::EFFECT_DAMAGE_POISON, "", "d:/ymir work/effect/affect/damagevalue/poison.mse" },
		{ CInstanceBase::EFFECT_DAMAGE_MISS, "", "d:/ymir work/effect/affect/damagevalue/miss.mse" },
		{ CInstanceBase::EFFECT_DAMAGE_TARGETMISS, "", "d:/ymir work/effect/affect/damagevalue/target_miss.mse" },
		{ CInstanceBase::EFFECT_DAMAGE_CRITICAL, "", "d:/ymir work/effect/affect/damagevalue/critical.mse" },

		{ CInstanceBase::EFFECT_LEVELUP_ON_14_FOR_GERMANY, "","season1/effect/paymessage_warning.mse" },
		{ CInstanceBase::EFFECT_LEVELUP_UNDER_15_FOR_GERMANY, "", "season1/effect/paymessage_decide.mse" },

		{ CInstanceBase::EFFECT_PERCENT_DAMAGE1, "", "d:/ymir work/effect/hit/percent_damage1.mse" },
		{ CInstanceBase::EFFECT_PERCENT_DAMAGE2, "", "d:/ymir work/effect/hit/percent_damage2.mse" },
		{ CInstanceBase::EFFECT_PERCENT_DAMAGE3, "", "d:/ymir work/effect/hit/percent_damage3.mse" },

		#ifdef ENABLE_BOSS_EFFECT_SYSTEM
		{ CInstanceBase::EFFECT_BOSS, "", "boss_eff/boss_icon.mse" },
		#endif

		#ifdef ENABLE_SMITH_EFFECT_SYSTEM
		{ CInstanceBase::EFFECT_FR_SUCCESS, "", "smith/up_ok.mse" },
		{ CInstanceBase::EFFECT_FR_FAIL, "", "smith/up_failed.mse" },
		#endif
	};

	static std::vector<TRaceData> m_vecRaceData =
	{
		{ RACE_WARRIOR_M, "locale/common/warrior_m.msm", "d:/ymir work/pc/warrior/intro/" },
		{ RACE_WARRIOR_W, "locale/common/warrior_w.msm", "d:/ymir work/pc2/warrior/intro/" },

		{ RACE_ASSASSIN_W, "locale/common/assassin_w.msm", "d:/ymir work/pc/assassin/intro/" },
		{ RACE_ASSASSIN_M, "locale/common/assassin_m.msm", "d:/ymir work/pc2/assassin/intro/" },

		{ RACE_SURA_M, "locale/common/sura_m.msm", "d:/ymir work/pc/sura/intro/" },
		{ RACE_SURA_W, "locale/common/sura_w.msm", "d:/ymir work/pc2/sura/intro/" },

		{ RACE_SHAMAN_W, "locale/common/shaman_w.msm", "d:/ymir work/pc/shaman/intro/" },
		{ RACE_SHAMAN_M, "locale/common/shaman_m.msm", "d:/ymir work/pc2/shaman/intro/" },

		#ifdef ENABLE_WOLFMAN_CHARACTER
		{ RACE_WOLFMAN_M, "locale/common/wolfman_m.msm", "d:/ymir work/pc3/wolfman/intro/" },
		#endif
	};

	std::vector<SMotion> m_vecMotion =
	{
		{ CRaceMotionData::NAME_INTRO_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_INTRO_SELECTED, "selected.msa", 0 },
		{ CRaceMotionData::NAME_INTRO_NOT_SELECTED, "not_selected.msa", 0 },
	};

	CInstanceBase::SetDustGap(DUST_GAP);
	CInstanceBase::SetHorseDustGap(HORSE_DUST_GAP);

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.RegisterEffect(rkPlayer.EFFECT_PICK, "d:/ymir work/effect/etc/click/click.mse", true);

	for (const auto& it : m_vecEffectData)
	{
		CInstanceBase::RegisterEffect(it.uiType, it.stBone, it.stEffect, true);
	}

	CRaceManager& pkManager = CRaceManager::Instance();
	char szFileName[FILE_MAX_NUM];

	for (auto& it : m_vecRaceData)
	{
		pkManager.CreateRace(it.dwRace);
		pkManager.SelectRace(it.dwRace);

		CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
		if (!pRaceData)
			return false;

		if (!pRaceData->LoadRaceData(it.stMSM))
		{
			return false;
		}

		pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
		for (const auto& it2 : m_vecMotion)
		{
			snprintf(szFileName, sizeof(szFileName), "%s%s", it.stIntroMotion, it2.stName);
			pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it2.wMotionIndex, szFileName, it2.byPercentage);
		}
	}

	// m_vecEffectData.clear();
	m_vecRaceData.clear();
	m_vecMotion.clear();

	return true;
}

const bool CPlayerSettingsModule::LoadGameEffect()
{
	CFlyingManager& pkFly = CFlyingManager::Instance();

	char GM_MARK[FILE_MAX_NUM];
	snprintf(GM_MARK, sizeof(GM_MARK), "locale/common/effect/gm.mse");

	std::vector<TEffect> m_vecEffectData =
	{
		///-------------------------------------------------------------------------------------------///
		{ CInstanceBase::EFFECT_SPAWN_APPEAR, "Bip01", "d:/ymir work/effect/etc/appear_die/monster_appear.mse" },
		{ CInstanceBase::EFFECT_SPAWN_DISAPPEAR, "Bip01", "d:/ymir work/effect/etc/appear_die/monster_die.mse" },
		{ CInstanceBase::EFFECT_FLAME_ATTACK, "equip_right_hand", "d:/ymir work/effect/hit/blow_flame/flame_3_weapon.mse" },
		{ CInstanceBase::EFFECT_FLAME_HIT, "", "d:/ymir work/effect/hit/blow_flame/flame_3_blow.mse" },
		{ CInstanceBase::EFFECT_FLAME_ATTACH, "", "d:/ymir work/effect/hit/blow_flame/flame_3_body.mse" },
		{ CInstanceBase::EFFECT_ELECTRIC_ATTACK, "equip_right", "d:/ymir work/effect/hit/blow_electric/light_1_weapon.mse" },
		{ CInstanceBase::EFFECT_ELECTRIC_HIT, "", "d:/ymir work/effect/hit/blow_electric/light_1_blow.mse" },
		{ CInstanceBase::EFFECT_ELECTRIC_ATTACH, "", "d:/ymir work/effect/hit/blow_electric/light_1_body.mse" },

		{ CInstanceBase::EFFECT_LEVELUP, "", "d:/ymir work/effect/etc/levelup_1/level_up.mse" },
		{ CInstanceBase::EFFECT_SKILLUP, "", "d:/ymir work/effect/etc/skillup/skillup_1.mse" },
		///-------------------------------------------------------------------------------------------///

		///-------------------------------------------------------------------------------------------///
		{ CInstanceBase::EFFECT_EMPIRE + 1, "Bip01", "d:/ymir work/effect/etc/empire/empire_A.mse" },
		{ CInstanceBase::EFFECT_EMPIRE + 2, "Bip01", "d:/ymir work/effect/etc/empire/empire_B.mse" },
		{ CInstanceBase::EFFECT_EMPIRE + 3, "Bip01", "d:/ymir work/effect/etc/empire/empire_C.mse" },

		{ CInstanceBase::EFFECT_WEAPON + 1, "equip_right_hand", "d:/ymir work/pc/warrior/effect/geom_sword_loop.mse" },
		{ CInstanceBase::EFFECT_WEAPON + 2, "equip_right_hand", "d:/ymir work/pc/warrior/effect/geom_spear_loop.mse" },
		///-------------------------------------------------------------------------------------------///

		///-------------------------------------------------------------------------------------------///
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_POISON, "Bip01", "d:/ymir work/effect/hit/blow_poison/poison_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_SLOW, "", "d:/ymir work/effect/affect/slow.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_STUN, "Bip01 Head", "d:/ymir work/effect/etc/stun/stun_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_DUNGEON_READY, "", "d:/ymir work/effect/etc/ready/ready.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_CHEONGEUN, "", "d:/ymir work/pc/warrior/effect/gyeokgongjang_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_GYEONGGONG, "", "d:/ymir work/pc/assassin/effect/gyeonggong_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_GWIGEOM, "Bip01 R Finger2", "d:/ymir work/pc/sura/effect/gwigeom_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_GONGPO, "", "d:/ymir work/pc/sura/effect/fear_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_JUMAGAP, "", "d:/ymir work/pc/sura/effect/jumagap_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_HOSIN, "", "d:/ymir work/pc/shaman/effect/3hosin_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_BOHO, "", "d:/ymir work/pc/shaman/effect/boho_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_KWAESOK, "", "d:/ymir work/pc/shaman/effect/10kwaesok_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_HEUKSIN, "", "d:/ymir work/pc/sura/effect/heuksin_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_MUYEONG, "", "d:/ymir work/pc/sura/effect/muyeong_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_FIRE, "Bip01", "d:/ymir work/effect/hit/blow_flame/flame_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_GICHEON, "Bip01 R Hand", "d:/ymir work/pc/shaman/effect/6gicheon_hand.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_JEUNGRYEOK, "Bip01 L Hand", "d:/ymir work/pc/shaman/effect/jeungryeok_hand.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_PABEOP, "Bip01 Head", "d:/ymir work/pc/sura/effect/pabeop_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_FALLEN_CHEONGEUN, "", "d:/ymir work/pc/warrior/effect/gyeokgongjang_loop.mse" },
		// 34 Polymoph
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_WAR_FLAG1, "", "d:/ymir work/effect/etc/guild_war_flag/flag_red.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_WAR_FLAG2, "", "d:/ymir work/effect/etc/guild_war_flag/flag_blue.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_WAR_FLAG3, "", "d:/ymir work/effect/etc/guild_war_flag/flag_yellow.mse" },
		#ifdef ENABLE_WOLFMAN_CHARACTER
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_BLEEDING, "Bip01", "d:/ymir work/effect/hit/blow_poison/bleeding_loop.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_RED_POSSESSION, "Bip01", "d:/ymir work/effect/hit/blow_flame/flame_loop_w.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_BLUE_POSSESSION, "", "d:/ymir work/pc3/common/effect/gyeokgongjang_loop_w.mse" },
		#endif
		#ifdef __AUTO_HUNT__
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_AUTO_HUNT, "Bip01", "locale/common/effect/auto_hunt.mse" },
		#endif
	#ifdef ENABLE_PLAYER_TAG_SYSTEM
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_TAG_YOUTUBE, "Bip01", "locale/common/effect/youtube.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_TAG_TWITCH, "Bip01", "locale/common/effect/twitch.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_TAG_TIKTOK, "Bip01", "locale/common/effect/tiktok_oyuncu.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_TAG_KICK, "Bip01", "locale/common/effect/tiktok_oyuncu.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_TAG_KING, "Bip01", "locale/common/effect/kral_oyuncu.mse" },
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_TAG_KING_GUILD, "Bip01", "locale/common/effect/kral_lonca.mse" },
		#ifdef ENABLE_RONARK_SYSTEM
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_TAG_KING_RONARK, "Bip01", "d:/ymir work/effect/ronark_hukumdari_zency.mse" },
		#endif
	#endif
		#ifdef ENABLE_DEFENCE_SHIELD
		{ CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_DEFENCE_SHIELD, "Bip01", "d:/ymir work/effect/pvm_def.mse" },
		#endif
		///-------------------------------------------------------------------------------------------///

		///-------------------------------------------------------------------------------------------///
		//## SWORD
		{ CInstanceBase::EFFECT_REFINED + 1, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_7.mse" },
		{ CInstanceBase::EFFECT_REFINED + 2, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_8.mse" },
		{ CInstanceBase::EFFECT_REFINED + 3, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_9.mse" },
		//## BOW
		{ CInstanceBase::EFFECT_REFINED + 4, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_7_b.mse" },
		{ CInstanceBase::EFFECT_REFINED + 5, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_8_b.mse" },
		{ CInstanceBase::EFFECT_REFINED + 6, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_9_b.mse" },
		//## FAN
		{ CInstanceBase::EFFECT_REFINED + 7, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_7_f.mse" },
		{ CInstanceBase::EFFECT_REFINED + 8, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_8_f.mse" },
		{ CInstanceBase::EFFECT_REFINED + 9, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_9_f.mse" },
		//## DEGER RIGHT
		{ CInstanceBase::EFFECT_REFINED + 10, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_7_s.mse" },
		{ CInstanceBase::EFFECT_REFINED + 11, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_8_s.mse" },
		{ CInstanceBase::EFFECT_REFINED + 12, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_9_s.mse" },
		//## DEGER LEFT
		{ CInstanceBase::EFFECT_REFINED + 13, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_7_s.mse" },
		{ CInstanceBase::EFFECT_REFINED + 14, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_8_s.mse" },
		{ CInstanceBase::EFFECT_REFINED + 15, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/sword_9_s.mse" },

		{ CInstanceBase::EFFECT_REFINED + 16, "Bip01", "D:/ymir work/pc/common/effect/armor/armor_7.mse" },
		{ CInstanceBase::EFFECT_REFINED + 17, "Bip01", "D:/ymir work/pc/common/effect/armor/armor_8.mse" },
		{ CInstanceBase::EFFECT_REFINED + 18, "Bip01", "D:/ymir work/pc/common/effect/armor/armor_9.mse" },

		{ CInstanceBase::EFFECT_REFINED + 19, "Bip01", "D:/ymir work/pc/common/effect/armor/armor-4-2-1.mse" },
		{ CInstanceBase::EFFECT_REFINED + 20, "Bip01", "D:/ymir work/pc/common/effect/armor/armor-4-2-2.mse" },

		#ifdef ENABLE_LVL115_ARMOR_EFFECT
		{ CInstanceBase::EFFECT_REFINED + 21, "Bip01", "D:/ymir work/pc/common/effect/armor/armor-5-1.mse" },
		#endif

		#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		{ CInstanceBase::EFFECT_REFINED + 22, "Bip01", "d:/ymir work/pc/common/effect/armor/acc_01.mse" },
		#endif

		#ifdef ENABLE_EVOLUTION_SYSTEM
		{ CInstanceBase::EFFECT_REFINED + 23, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_1_yesil.mse" },
		{ CInstanceBase::EFFECT_REFINED + 24, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_2_turuncu.mse" },
		{ CInstanceBase::EFFECT_REFINED + 25, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_3_kirmizi.mse" },
		{ CInstanceBase::EFFECT_REFINED + 26, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/sword_4_mavi.mse" },

		{ CInstanceBase::EFFECT_REFINED + 27, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/bow_1_yesil.mse" },
		{ CInstanceBase::EFFECT_REFINED + 28, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/bow_2_turuncu.mse" },
		{ CInstanceBase::EFFECT_REFINED + 29, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/bow_3_kirmizi.mse" },
		{ CInstanceBase::EFFECT_REFINED + 30, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/bow_4_mavi.mse" },

		{ CInstanceBase::EFFECT_REFINED + 31, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/fan_1_yesil.mse" },
		{ CInstanceBase::EFFECT_REFINED + 32, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/fan_2_turuncu.mse" },
		{ CInstanceBase::EFFECT_REFINED + 33, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/fan_3_kirmizi.mse" },
		{ CInstanceBase::EFFECT_REFINED + 34, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/fan_4_mavi.mse" },

		{ CInstanceBase::EFFECT_REFINED + 35, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/dagger_1_yesil.mse" },
		{ CInstanceBase::EFFECT_REFINED + 36, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/dagger_2_turuncu.mse" },
		{ CInstanceBase::EFFECT_REFINED + 37, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/dagger_3_kirmizi.mse" },
		{ CInstanceBase::EFFECT_REFINED + 38, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/dagger_4_mavi.mse" },

		{ CInstanceBase::EFFECT_REFINED + 39, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/dagger_1_yesil.mse" },
		{ CInstanceBase::EFFECT_REFINED + 40, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/dagger_2_turuncu.mse" },
		{ CInstanceBase::EFFECT_REFINED + 41, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/dagger_3_kirmizi.mse" },
		{ CInstanceBase::EFFECT_REFINED + 42, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/dagger_4_mavi.mse" },

		{ CInstanceBase::EFFECT_REFINED + 43, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/claw_1_yesil.mse" },
		{ CInstanceBase::EFFECT_REFINED + 44, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/claw_2_turuncu.mse" },
		{ CInstanceBase::EFFECT_REFINED + 45, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/claw_3_kirmizi.mse" },
		{ CInstanceBase::EFFECT_REFINED + 46, "PART_WEAPON", "D:/ymir work/pc/common/effect/sword/claw_4_mavi.mse" },

		{ CInstanceBase::EFFECT_REFINED + 47, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/claw_1_yesil.mse" },
		{ CInstanceBase::EFFECT_REFINED + 48, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/claw_2_turuncu.mse" },
		{ CInstanceBase::EFFECT_REFINED + 49, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/claw_3_kirmizi.mse" },
		{ CInstanceBase::EFFECT_REFINED + 50, "PART_WEAPON_LEFT", "D:/ymir work/pc/common/effect/sword/claw_4_mavi.mse" },

		{ CInstanceBase::EFFECT_REFINED + 51, "Bip01", "D:/ymir work/pc/common/effect/armor/zency_sari.mse" },
		{ CInstanceBase::EFFECT_REFINED + 52, "Bip01", "D:/ymir work/pc/common/effect/armor/zency_beyaz.mse" },
		{ CInstanceBase::EFFECT_REFINED + 53, "Bip01", "D:/ymir work/pc/common/effect/armor/zency_mor.mse" },
		{ CInstanceBase::EFFECT_REFINED + 54, "Bip01", "D:/ymir work/pc/common/effect/armor/zency_kirmizi.mse" },
		{ CInstanceBase::EFFECT_REFINED + 55, "Bip01", "D:/ymir work/pc/common/effect/armor/zency_turuncu.mse" },
		{ CInstanceBase::EFFECT_REFINED + 56, "Bip01", "D:/ymir work/pc/common/effect/armor/zency_yesil.mse" },
		{ CInstanceBase::EFFECT_REFINED + 57, "Bip01", "D:/ymir work/pc/common/effect/armor/zency_mavi.mse" },
		#endif

		#ifdef ENABLE_KADIM_SILAH
		{ CInstanceBase::EFFECT_REFINED + 58, "PART_WEAPON", "D:/ymir work/item/weapon/kozik.mse" },
		{ CInstanceBase::EFFECT_REFINED + 59, "PART_WEAPON_LEFT", "D:/ymir work/item/weapon/kozik.mse" },
		{ CInstanceBase::EFFECT_REFINED + 60, "PART_WEAPON", "D:/ymir work/item/weapon/kadim_sonbahar_yelpazesi.mse" },
		{ CInstanceBase::EFFECT_REFINED + 61, "PART_WEAPON", "D:/ymir work/item/weapon/kadim_antik_can.mse" },
		{ CInstanceBase::EFFECT_REFINED + 62, "PART_WEAPON_LEFT", "D:/ymir work/item/weapon/jelonek.mse" },
		// 63 Lycan boş.
		// 64 Lycan boş.
		{ CInstanceBase::EFFECT_REFINED + 65, "PART_WEAPON", "D:/ymir work/item/weapon/kadim_dolunay.mse" },
		{ CInstanceBase::EFFECT_REFINED + 66, "PART_WEAPON", "D:/ymir work/item/weapon/kadim_kdp.mse" },
		{ CInstanceBase::EFFECT_REFINED + 67, "PART_WEAPON", "D:/ymir work/item/weapon/kadim_seytan_kanadi.mse" },
		{ CInstanceBase::EFFECT_REFINED + 68, "PART_WEAPON_LEFT", "D:/ymir work/item/weapon/kadim_seytan_kanadi.mse" },
		{ CInstanceBase::EFFECT_REFINED + 69, "PART_WEAPON", "D:/ymir work/item/weapon/kadim_orkide_can.mse" },
		{ CInstanceBase::EFFECT_REFINED + 70, "PART_WEAPON_LEFT", "D:/ymir work/item/weapon/kadim_havali_celik_yay.mse" },
		// 71 Lycan boş.
		// 72 Lycan boş.
		{ CInstanceBase::EFFECT_REFINED + 73, "PART_WEAPON", "D:/ymir work/item/weapon/kadim_zehir.mse" },
		{ CInstanceBase::EFFECT_REFINED + 74, "PART_WEAPON", "D:/ymir work/item/weapon/kadim_kin.mse" },
		{ CInstanceBase::EFFECT_REFINED + 75, "PART_WEAPON", "D:/ymir work/item/weapon/kadim_aslan_kilic.mse" },
		#endif


		///-------------------------------------------------------------------------------------------///

		{ CInstanceBase::EFFECT_EMOTICON + 0, "", "d:/ymir work/effect/etc/emoticon/sweat.mse" },
		{ CInstanceBase::EFFECT_EMOTICON + 1, "", "d:/ymir work/effect/etc/emoticon/money.mse" },
		{ CInstanceBase::EFFECT_EMOTICON + 2, "", "d:/ymir work/effect/etc/emoticon/happy.mse" },
		{ CInstanceBase::EFFECT_EMOTICON + 3, "", "d:/ymir work/effect/etc/emoticon/love_s.mse" },
		{ CInstanceBase::EFFECT_EMOTICON + 4, "", "d:/ymir work/effect/etc/emoticon/love_l.mse" },
		{ CInstanceBase::EFFECT_EMOTICON + 5, "", "d:/ymir work/effect/etc/emoticon/angry.mse" },
		{ CInstanceBase::EFFECT_EMOTICON + 6, "", "d:/ymir work/effect/etc/emoticon/aha.mse" },
		{ CInstanceBase::EFFECT_EMOTICON + 7, "", "d:/ymir work/effect/etc/emoticon/gloom.mse" },
		{ CInstanceBase::EFFECT_EMOTICON + 8, "", "d:/ymir work/effect/etc/emoticon/sorry.mse" },
		{ CInstanceBase::EFFECT_EMOTICON + 9, "", "d:/ymir work/effect/etc/emoticon/!_mix_back.mse" },
		{ CInstanceBase::EFFECT_EMOTICON + 10, "", "d:/ymir work/effect/etc/emoticon/question.mse" },
		{ CInstanceBase::EFFECT_EMOTICON + 11, "", "d:/ymir work/effect/etc/emoticon/fish.mse" },
	};

	enum
	{
		FLY_NONE,
		FLY_EXP,
		FLY_HP_MEDIUM,
		FLY_HP_BIG,
		FLY_SP_SMALL,
		FLY_SP_MEDIUM,
		FLY_SP_BIG,
		FLY_FIREWORK1,
		FLY_FIREWORK2,
		FLY_FIREWORK3,
		FLY_FIREWORK4,
		FLY_FIREWORK5,
		FLY_FIREWORK6,
		FLY_FIREWORK_XMAS,
		FLY_CHAIN_LIGHTNING,
		FLY_HP_SMALL,
		FLY_SKILL_MUYEONG,
	};

	std::vector<SFly> m_vecFlyData = {
		{ FLY_EXP, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_yellow_small2.msf" },
		{ FLY_HP_MEDIUM, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_red_small.msf" },
		{ FLY_HP_BIG, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_red_big.msf" },
		{ FLY_SP_SMALL, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_blue_warrior_small.msf" },
		{ FLY_SP_MEDIUM, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_blue_small.msf" },
		{ FLY_SP_BIG, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_blue_big.msf" },
		{ FLY_FIREWORK1, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_1.msf" },
		{ FLY_FIREWORK2, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_2.msf" },
		{ FLY_FIREWORK3, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_3.msf" },
		{ FLY_FIREWORK4, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_4.msf" },
		{ FLY_FIREWORK5, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_5.msf" },
		{ FLY_FIREWORK6, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_6.msf" },
		{ FLY_FIREWORK_XMAS, pkFly.INDEX_FLY_TYPE_FIRE_CRACKER, "d:/ymir work/effect/etc/firecracker/firecracker_xmas.msf" },
		{ FLY_CHAIN_LIGHTNING, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/pc/shaman/effect/pokroe.msf" },
		{ FLY_HP_SMALL, pkFly.INDEX_FLY_TYPE_NORMAL, "d:/ymir work/effect/etc/gathering/ga_piece_red_smallest.msf" },
		{ FLY_SKILL_MUYEONG, pkFly.INDEX_FLY_TYPE_AUTO_FIRE, "d:/ymir work/pc/sura/effect/muyeong_fly.msf" },
	};

	// GM_EFFECT
	TEffect effect;
	effect.uiType = CInstanceBase::EFFECT_AFFECT + 0;
	memcpy(effect.stBone, "Bip01", sizeof(effect.stBone));
	memcpy(effect.stEffect, GM_MARK, sizeof(effect.stEffect));

	m_vecEffectData.push_back(effect);
	// GM_EFFECT

	for (const auto& it : m_vecEffectData)
	{
		CInstanceBase::RegisterEffect(it.uiType, it.stBone, it.stEffect, false);
	}

	for (const auto& it : m_vecFlyData)
	{
		pkFly.RegisterIndexedFlyData(it.dwIndex, it.byType, it.stName);
	}

	m_vecEffectData.clear();
	m_vecFlyData.clear();

	return true;
}

const bool CPlayerSettingsModule::RegisterEmotionAnis(char stFolder[FILE_MAX_NUM])
{
	CRaceManager& pkManager = CRaceManager::Instance();

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	std::vector<SMotion> m_vecEmotion =
	{
		{ CRaceMotionData::NAME_CLAP, "clap.msa", 0 },
		{ CRaceMotionData::NAME_CHEERS_1, "cheers_1.msa", 0 },
		{ CRaceMotionData::NAME_CHEERS_2, "cheers_2.msa", 0 },
		{ CRaceMotionData::NAME_DANCE_1, "dance_1.msa", 0 },
		{ CRaceMotionData::NAME_DANCE_2, "dance_2.msa", 0 },
		{ CRaceMotionData::NAME_DANCE_3, "dance_3.msa", 0 },
		{ CRaceMotionData::NAME_DANCE_4, "dance_4.msa", 0 },
		{ CRaceMotionData::NAME_DANCE_5, "dance_5.msa", 0 },
		{ CRaceMotionData::NAME_DANCE_6, "dance_6.msa", 0 },
		{ CRaceMotionData::NAME_CONGRATULATION, "congratulation.msa", 0 },
		{ CRaceMotionData::NAME_FORGIVE, "forgive.msa", 0 },
		{ CRaceMotionData::NAME_ANGRY, "angry.msa", 0 },
		{ CRaceMotionData::NAME_ATTRACTIVE, "attractive.msa", 0 },
		{ CRaceMotionData::NAME_SAD, "sad.msa", 0 },
		{ CRaceMotionData::NAME_SHY, "shy.msa", 0 },
		{ CRaceMotionData::NAME_CHEERUP, "cheerup.msa", 0 },
		{ CRaceMotionData::NAME_BANTER, "banter.msa", 0 },
		{ CRaceMotionData::NAME_JOY, "joy.msa", 0 },
		{ CRaceMotionData::NAME_FRENCH_KISS_WITH_WARRIOR, "french_kiss_with_warrior.msa", 0 },
		{ CRaceMotionData::NAME_FRENCH_KISS_WITH_ASSASSIN, "french_kiss_with_assassin.msa", 0 },
		{ CRaceMotionData::NAME_FRENCH_KISS_WITH_SURA, "french_kiss_with_sura.msa", 0 },
		{ CRaceMotionData::NAME_FRENCH_KISS_WITH_SHAMAN, "french_kiss_with_shaman.msa", 0 },
		{ CRaceMotionData::NAME_KISS_WITH_WARRIOR, "kiss_with_warrior.msa", 0 },
		{ CRaceMotionData::NAME_KISS_WITH_ASSASSIN, "kiss_with_assassin.msa", 0 },
		{ CRaceMotionData::NAME_KISS_WITH_SURA, "kiss_with_sura.msa", 0 },
		{ CRaceMotionData::NAME_KISS_WITH_SHAMAN, "kiss_with_shaman.msa", 0 },
		{ CRaceMotionData::NAME_SLAP_HIT_WITH_WARRIOR, "slap_hit.msa", 0 },
		{ CRaceMotionData::NAME_SLAP_HIT_WITH_ASSASSIN, "slap_hit.msa", 0 },
		{ CRaceMotionData::NAME_SLAP_HIT_WITH_SURA, "slap_hit.msa", 0 },
		{ CRaceMotionData::NAME_SLAP_HIT_WITH_SHAMAN, "slap_hit.msa", 0 },
		{ CRaceMotionData::NAME_SLAP_HURT_WITH_WARRIOR, "slap_hurt.msa", 0 },
		{ CRaceMotionData::NAME_SLAP_HURT_WITH_ASSASSIN, "slap_hurt.msa", 0 },
		{ CRaceMotionData::NAME_SLAP_HURT_WITH_SURA, "slap_hurt.msa", 0 },
		{ CRaceMotionData::NAME_SLAP_HURT_WITH_SHAMAN, "slap_hurt.msa", 0 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
	for (const auto& it : m_vecEmotion)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "action/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_WEDDING_DRESS);
	for (const auto& it : m_vecEmotion)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "action/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_WEDDING_DRESS, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecEmotionDress =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "walk.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_WEDDING_DRESS);
	for (const auto& it : m_vecEmotionDress)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "wedding/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_WEDDING_DRESS, it.wMotionIndex, szFileName, it.byPercentage);
	}

	m_vecEmotion.clear();
	m_vecEmotionDress.clear();

	return true;
}

const bool CPlayerSettingsModule::LoadGeneralMotion(char stFolder[FILE_MAX_NUM])
{
	CRaceManager& pkManager = CRaceManager::Instance();

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	std::vector<SMotion> m_vecBasicMotion =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_FLYING, "damage_flying.msa", 0 },
		{ CRaceMotionData::NAME_STAND_UP, "falling_stand.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE_FLYING_BACK, "back_damage_flying.msa", 0 },
		{ CRaceMotionData::NAME_STAND_UP_BACK, "back_falling_stand.msa", 0 },
		{ CRaceMotionData::NAME_DEAD, "dead.msa", 0 },
		{ CRaceMotionData::NAME_DIG, "dig.msa", 0 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
	for (const auto& it : m_vecBasicMotion)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
	}

	#ifdef ENABLE_STANDING_MOUNT
	std::vector<SMotion> m_vecBasicBaoardMotion =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "wait.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "wait.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_FLYING, "damage_flying.msa", 0 },
		{ CRaceMotionData::NAME_STAND_UP, "falling_stand.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE_FLYING_BACK, "back_damage_flying.msa", 0 },
		{ CRaceMotionData::NAME_STAND_UP_BACK, "back_falling_stand.msa", 0 },
		{ CRaceMotionData::NAME_DEAD, "dead.msa", 0 },
		{ CRaceMotionData::NAME_DIG, "dig.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND);
	for (const auto& it : m_vecBasicBaoardMotion)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND, it.wMotionIndex, szFileName, it.byPercentage);
	}
	#endif

	m_vecBasicMotion.clear();
	#ifdef ENABLE_STANDING_MOUNT
	m_vecBasicBaoardMotion.clear();
	#endif
	return true;
}

#ifdef ENABLE_WOLFMAN_CHARACTER
const bool CPlayerSettingsModule::LoadGeneralMotionForWolfman(char stFolder[FILE_MAX_NUM])
{
	CRaceManager& pkManager = CRaceManager::Instance();

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	std::vector<SMotion> m_vecBasicMotion =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "back_damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "back_damage1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "back_damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "back_damage1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_FLYING, "back_damage_flying.msa", 0 },
		{ CRaceMotionData::NAME_STAND_UP, "front_falling_standup.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE_FLYING_BACK, "back_damage_flying.msa", 0 },
		{ CRaceMotionData::NAME_STAND_UP_BACK, "back_falling_standup.msa", 0 },
		{ CRaceMotionData::NAME_DEAD, "dead.msa", 0 },
		{ CRaceMotionData::NAME_DIG, "dig.msa", 0 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
	for (const auto& it : m_vecBasicMotion)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
	}

	m_vecBasicMotion.clear();

	return true;
}
#endif

const bool CPlayerSettingsModule::LoadGameWarrior(DWORD dwRace, char stFolder[FILE_MAX_NUM])
{
	CRaceManager& pkManager = CRaceManager::Instance();
	pkManager.SelectRace(dwRace);

	if (!LoadGeneralMotion(stFolder))
		return false;

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	if (!pRaceData->SetMotionRandomWeight(CRaceMotionData::MODE_GENERAL, CRaceMotionData::NAME_WAIT, 0, 70))
		return false;

	std::vector<SMotion> m_vecMotionGeneral =
	{
		{ CRaceMotionData::NAME_WAIT, "wait_1.msa", 30 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "attack.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "attack_1.msa", 50 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
	for (const auto& it : m_vecMotionGeneral)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionSkill =
	{
		{ GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa", 0 },
		{ GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa", 0 },
		{ GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa", 0 },
		{ GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa", 0 },
		{ GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa", 0 },
		{ GUILD_SKILL_MAGICUP, "guild_jumunsul.msa", 0 },
	};

	std::vector<std::pair<BYTE, const char*>> m_vecSkillName = {
		{ 1, "samyeon" },
		{ 2, "palbang" },
		{ 3, "jeongwi" },
		{ 4, "geomgyeong" },
		{ 5, "tanhwan" },
		{ 6, "gihyeol" },
		{ 16, "gigongcham" },
		{ 17, "gyeoksan" },
		{ 18, "daejin" },
		{ 19, "cheongeun" },
		{ 20, "geompung" },
		{ 21, "noegeom" },
	};

	char szSkillAdd[4] = "";
	for (WORD i = 0; i < CPythonSkill::SKILL_EFFECT_COUNT; i++)
	{
		for (auto& it : m_vecSkillName)
		{
			SMotion motion;

			motion.wMotionIndex = CRaceMotionData::NAME_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + it.first;
			motion.byPercentage = 0;

			if (i > 0)
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_%d", i + 1);
			else
				snprintf(szSkillAdd, sizeof(szSkillAdd), "");

			snprintf(motion.stName, sizeof(motion.stName), "%s%s.msa", it.second, szSkillAdd);
			m_vecMotionSkill.push_back(motion);
		}
	}

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
	#ifdef ENABLE_STANDING_MOUNT
	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND);
	#endif
	for (const auto& it : m_vecMotionSkill)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "skill/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
		#ifdef ENABLE_STANDING_MOUNT
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND, it.wMotionIndex, szFileName, it.byPercentage);
		#endif
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_GENERAL, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_GENERAL, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	#ifdef ENABLE_STANDING_MOUNT
	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	#endif

	RegisterEmotionAnis(stFolder);

	std::vector<SMotion> m_vecMotionSword =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 50 },
		{ CRaceMotionData::NAME_WAIT, "wait_1.msa", 50 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa" , 0},
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_ONEHAND_SWORD);
	for (const auto& it : m_vecMotionSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "onehand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_ONEHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);	

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);

	std::vector<SMotion> m_vecMotionTwoHandSword =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 70 },
		{ CRaceMotionData::NAME_WAIT, "wait_1.msa", 30 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_TWOHAND_SWORD);
	for (const auto& it : m_vecMotionSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "twohand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_TWOHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);

	#ifdef ENABLE_STANDING_MOUNT
	std::vector<SMotion> m_vecMotionHorseBoardSword =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 50 },
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 50 },
		{ CRaceMotionData::NAME_WALK, "wait.msa", 50 },
		{ CRaceMotionData::NAME_RUN, "wait.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa" , 0},
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND_SWORD);
	for (const auto& it : m_vecMotionHorseBoardSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "onehand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);

	std::vector<SMotion> m_vecMotionHorseBoardTwoHandSword =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 70 },
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 30 },
		{ CRaceMotionData::NAME_WALK, "wait.msa", 70 },
		{ CRaceMotionData::NAME_RUN, "wait.msa", 70 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD);
	for (const auto& it : m_vecMotionHorseBoardTwoHandSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "twohand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);
	#endif

	std::vector<SMotion> m_vecMotionFishing =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_THROW, "throw.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_WAIT, "fishing_wait.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_STOP, "fishing_cancel.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_REACT, "fishing_react.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_CATCH, "fishing_catch.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_FAIL, "fishing_fail.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_FISHING);
	for (const auto& it : m_vecMotionFishing)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "fishing/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_FISHING, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionHorse =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 90 },
		{ CRaceMotionData::NAME_WAIT, "wait_1.msa", 9 },
		{ CRaceMotionData::NAME_WAIT, "wait_2.msa", 1 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage.msa", 0 },
		{ CRaceMotionData::NAME_DEAD, "dead.msa", 0 },
		{ HORSE_SKILL_CHARGE, "skill_charge.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE);
	for (const auto& it : m_vecMotionHorse)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionHorseSword =
	{
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
		{ HORSE_SKILL_SPLASH, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
	for (const auto& it : m_vecMotionHorseSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_onehand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);

	std::vector<SMotion> m_vecMotionHorseTwoHandSword =
	{
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
		{ HORSE_SKILL_SPLASH, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD);
	for (const auto& it : m_vecMotionHorseTwoHandSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_twohand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);

	//##Bone
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON, "equip_right_hand");

	#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_ACCE, "Bip01 Spine2");
	#endif

	m_vecMotionGeneral.clear();
	m_vecMotionSkill.clear();
	m_vecSkillName.clear();
	m_vecMotionSword.clear();
	m_vecMotionTwoHandSword.clear();
	m_vecMotionFishing.clear();
	m_vecMotionHorse.clear();
	m_vecMotionHorseSword.clear();
	m_vecMotionHorseTwoHandSword.clear();
	#ifdef ENABLE_STANDING_MOUNT
	m_vecMotionHorseBoardSword.clear();
	m_vecMotionHorseBoardTwoHandSword.clear();
	#endif
	return true;
}

const bool CPlayerSettingsModule::LoadGameAssassin(DWORD dwRace, char stFolder[FILE_MAX_NUM])
{
	CRaceManager& pkManager = CRaceManager::Instance();
	pkManager.SelectRace(dwRace);

	if (!LoadGeneralMotion(stFolder))
		return false;

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	if (!pRaceData->SetMotionRandomWeight(CRaceMotionData::MODE_GENERAL, CRaceMotionData::NAME_WAIT, 0, 70))
		return false;

	std::vector<SMotion> m_vecMotionGeneral =
	{
		{ CRaceMotionData::NAME_WAIT, "wait_1.msa", 30 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "attack.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "attack_1.msa", 50 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
	for (const auto& it : m_vecMotionGeneral)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionSkill =
	{
		{ GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa", 0 },
		{ GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa", 0 },
		{ GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa", 0 },
		{ GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa", 0 },
		{ GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa", 0 },
		{ GUILD_SKILL_MAGICUP, "guild_jumunsul.msa", 0 },
	};

	std::vector<std::pair<BYTE, const char*>> m_vecSkillName = {
		{ 1, "amseup" },
		{ 2, "gungsin" },
		{ 3, "charyun" },
		{ 4, "eunhyeong" },
		{ 5, "sangong" },
		{ 6, "seomjeon" },
		{ 16, "yeonsa" },
		{ 17, "gwangyeok" },
		{ 18, "hwajo" },
		{ 19, "gyeonggong" },
		{ 20, "dokgigung" },
		{ 21, "seomgwang" },
	};

	char szSkillAdd[4] = "";
	for (WORD i = 0; i < CPythonSkill::SKILL_EFFECT_COUNT; i++)
	{
		for (auto& it : m_vecSkillName)
		{
			SMotion motion;

			motion.wMotionIndex = CRaceMotionData::NAME_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + it.first;
			motion.byPercentage = 0;

			if (i > 0)
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_%d", i + 1);
			else
				snprintf(szSkillAdd, sizeof(szSkillAdd), "");

			snprintf(motion.stName, sizeof(motion.stName), "%s%s.msa", it.second, szSkillAdd);
			m_vecMotionSkill.push_back(motion);
		}
	}

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
	#ifdef ENABLE_STANDING_MOUNT
	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND);
	#endif
	for (const auto& it : m_vecMotionSkill)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "skill/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
		#ifdef ENABLE_STANDING_MOUNT
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND, it.wMotionIndex, szFileName, it.byPercentage);
		#endif
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_GENERAL, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_GENERAL, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	#ifdef ENABLE_STANDING_MOUNT
	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	#endif

	RegisterEmotionAnis(stFolder);

	std::vector<SMotion> m_vecMotionSword =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 70 },
		{ CRaceMotionData::NAME_WAIT, "wait_1.msa", 30 },
		{ CRaceMotionData::NAME_WALK, "walk.msa" , 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa" , 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa" , 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa" , 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa" , 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa" , 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa" , 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa" , 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa" , 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_ONEHAND_SWORD);
	for (const auto& it : m_vecMotionSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "onehand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_ONEHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);

	std::vector<SMotion> m_vecMotionDagger =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 70 },
		{ CRaceMotionData::NAME_WAIT, "wait_1.msa", 30 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_8, "combo_08.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_DUALHAND_SWORD);
	for (const auto& it : m_vecMotionDagger)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "dualhand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_DUALHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_8);	

	std::vector<SMotion> m_vecMotionBow =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 70 },
		{ CRaceMotionData::NAME_WAIT, "wait_1.msa", 30},
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50},
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50},
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50},
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50},
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "attack.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_BOW);
	for (const auto& it : m_vecMotionBow)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "bow/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_BOW, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_BOW, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BOW, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);

	#ifdef ENABLE_STANDING_MOUNT
	std::vector<SMotion> m_vecMotionBoardSword =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 50 },
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 50 },
		{ CRaceMotionData::NAME_WALK, "wait.msa", 50 },
		{ CRaceMotionData::NAME_RUN, "wait.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa" , 0},
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND_SWORD);
	for (const auto& it : m_vecMotionBoardSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "onehand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);

	std::vector<SMotion> m_vecMotionBoardDagger =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 50 },
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 50 },
		{ CRaceMotionData::NAME_WALK, "wait.msa", 50 },
		{ CRaceMotionData::NAME_RUN, "wait.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD);
	for (const auto& it : m_vecMotionBoardDagger)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "dualhand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_8);	

	std::vector<SMotion> m_vecMotionBoardBow =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 70 },
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 30},
		{ CRaceMotionData::NAME_WALK, "wait.msa", 50 },
		{ CRaceMotionData::NAME_RUN, "wait.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50},
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50},
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50},
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50},
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "attack.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND_BOW);
	for (const auto& it : m_vecMotionBoardBow)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "bow/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND_BOW, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_BOW, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BOW, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	#endif

	std::vector<SMotion> m_vecMotionFishing =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_THROW, "throw.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_WAIT, "fishing_wait.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_STOP, "fishing_cancel.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_REACT, "fishing_react.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_CATCH, "fishing_catch.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_FAIL, "fishing_fail.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_FISHING);
	for (const auto& it : m_vecMotionFishing)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "fishing/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_FISHING, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionHorse =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 90 },
		{ CRaceMotionData::NAME_WAIT, "wait_1.msa", 9 },
		{ CRaceMotionData::NAME_WAIT, "wait_2.msa", 1 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage.msa", 0 },
		{ CRaceMotionData::NAME_DEAD, "dead.msa", 0 },
		{ HORSE_SKILL_CHARGE, "skill_charge.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE);
	for (const auto& it : m_vecMotionHorse)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionHorseSword =
	{
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
		{ HORSE_SKILL_SPLASH, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
	for (const auto& it : m_vecMotionHorseSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_onehand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);

	std::vector<SMotion> m_vecMotionHorseDagger =
	{
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
		{ HORSE_SKILL_SPLASH, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_DUALHAND_SWORD);
	for (const auto& it : m_vecMotionHorseDagger)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_dualhand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_DUALHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_DUALHAND_SWORD, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);

	std::vector<SMotion> m_vecMotionHorseBow =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 90 },
		{ CRaceMotionData::NAME_WAIT, "wait_1.msa", 9 },
		{ CRaceMotionData::NAME_WAIT, "wait_2.msa", 1 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 0 },
		{ CRaceMotionData::NAME_DEAD, "dead.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "attack.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
		{ HORSE_SKILL_SPLASH, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_BOW);
	for (const auto& it : m_vecMotionHorseBow)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_bow/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_BOW, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_BOW, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_BOW, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);

	//##Bone
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON, "equip_right");
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON_LEFT, "equip_left");

	#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_ACCE, "Bip01 Spine2");
	#endif

	m_vecMotionGeneral.clear();
	m_vecMotionSkill.clear();
	m_vecSkillName.clear();
	m_vecMotionSword.clear();
	m_vecMotionDagger.clear();
	m_vecMotionBow.clear();
	m_vecMotionFishing.clear();
	m_vecMotionHorse.clear();
	m_vecMotionHorseSword.clear();
	m_vecMotionHorseDagger.clear();
	m_vecMotionHorseBow.clear();
	#ifdef ENABLE_STANDING_MOUNT
	m_vecMotionBoardSword.clear();
	m_vecMotionBoardDagger.clear();
	m_vecMotionBoardBow.clear();
	#endif
	return true;
}

const bool CPlayerSettingsModule::LoadGameSura(DWORD dwRace, char stFolder[FILE_MAX_NUM])
{
	CRaceManager& pkManager = CRaceManager::Instance();
	pkManager.SelectRace(dwRace);

	if (!LoadGeneralMotion(stFolder))
		return false;

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	std::vector<SMotion> m_vecMotionGeneral =
	{
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "attack.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "attack_1.msa", 50 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
	for (const auto& it : m_vecMotionGeneral)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionSkill =
	{
		{ GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa", 0 },
		{ GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa", 0 },
		{ GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa", 0 },
		{ GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa", 0 },
		{ GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa", 0 },
		{ GUILD_SKILL_MAGICUP, "guild_jumunsul.msa", 0 },
	};

	std::vector<std::pair<BYTE, const char*>> m_vecSkillName = {
		{ 1, "swaeryeong" },
		{ 2, "yonggwon" },
		{ 3, "gwigeom" },
		{ 4, "gongpo" },
		{ 5, "jumagap" },
		{ 6, "pabeop" },
		{ 16, "maryeong" },
		{ 17, "hwayeom" },
		{ 18, "muyeong" },
		{ 19, "heuksin" },
		{ 20, "tusok" },
		{ 21, "mahwan" },
	};

	char szSkillAdd[4] = "";
	for (WORD i = 0; i < CPythonSkill::SKILL_EFFECT_COUNT; i++)
	{
		for (auto& it : m_vecSkillName)
		{
			SMotion motion;

			motion.wMotionIndex = CRaceMotionData::NAME_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + it.first;
			motion.byPercentage = 0;

			if (i > 0)
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_%d", i + 1);
			else
				snprintf(szSkillAdd, sizeof(szSkillAdd), "");

			snprintf(motion.stName, sizeof(motion.stName), "%s%s.msa", it.second, szSkillAdd);
			m_vecMotionSkill.push_back(motion);
		}
	}

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
	#ifdef ENABLE_STANDING_MOUNT
	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND);
	#endif
	for (const auto& it : m_vecMotionSkill)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "skill/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
		#ifdef ENABLE_STANDING_MOUNT
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND, it.wMotionIndex, szFileName, it.byPercentage);
		#endif
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_GENERAL, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_GENERAL, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	#ifdef ENABLE_STANDING_MOUNT
	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	#endif

	RegisterEmotionAnis(stFolder);

	std::vector<SMotion> m_vecMotionSword =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "walk.msa" , 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa" , 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa" , 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa" , 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa" , 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa" , 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa" , 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa" , 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa" , 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_ONEHAND_SWORD);
	for (const auto& it : m_vecMotionSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "onehand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_ONEHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);

	#ifdef ENABLE_STANDING_MOUNT
	std::vector<SMotion> m_vecMotionHorseBoardSword =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 50 },
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 50 },
		{ CRaceMotionData::NAME_WALK, "wait.msa", 50 },
		{ CRaceMotionData::NAME_RUN, "wait.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa" , 0},
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND_SWORD);
	for (const auto& it : m_vecMotionHorseBoardSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "onehand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);	

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);
	#endif

	std::vector<SMotion> m_vecMotionFishing =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_THROW, "throw.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_WAIT, "fishing_wait.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_STOP, "fishing_cancel.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_REACT, "fishing_react.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_CATCH, "fishing_catch.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_FAIL, "fishing_fail.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_FISHING);
	for (const auto& it : m_vecMotionFishing)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "fishing/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_FISHING, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionHorse =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 90 },
		{ CRaceMotionData::NAME_WAIT, "wait_1.msa", 9 },
		{ CRaceMotionData::NAME_WAIT, "wait_2.msa", 1 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage.msa", 0 },
		{ CRaceMotionData::NAME_DEAD, "dead.msa", 0 },
		{ HORSE_SKILL_CHARGE, "skill_charge.msa", 0 },
		{ HORSE_SKILL_SPLASH, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE);
	for (const auto& it : m_vecMotionHorse)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionHorseSword =
	{
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
	for (const auto& it : m_vecMotionHorseSword)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_onehand_sword/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);

	//##Bone
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON, "equip_right");

	#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_ACCE, "Bip01 Spine2");
	#endif

	m_vecMotionGeneral.clear();
	m_vecMotionSkill.clear();
	m_vecSkillName.clear();
	m_vecMotionSword.clear();
	m_vecMotionFishing.clear();
	m_vecMotionHorse.clear();
	m_vecMotionHorseSword.clear();
	#ifdef ENABLE_STANDING_MOUNT
	m_vecMotionHorseBoardSword.clear();
	#endif
	return true;
}

const bool CPlayerSettingsModule::LoadGameShaman(DWORD dwRace, char stFolder[FILE_MAX_NUM])
{
	CRaceManager& pkManager = CRaceManager::Instance();
	pkManager.SelectRace(dwRace);

	if (!LoadGeneralMotion(stFolder))
		return false;

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	std::vector<SMotion> m_vecMotionGeneral =
	{
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "attack.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "attack_1.msa", 50 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
	for (const auto& it : m_vecMotionGeneral)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionSkill =
	{
		{ CRaceMotionData::NAME_SKILL + 1, "bipabu.msa" },
		{ CRaceMotionData::NAME_SKILL + 2, "yongpa.msa" },
		{ CRaceMotionData::NAME_SKILL + 3, "paeryong.msa" },
		{ CRaceMotionData::NAME_SKILL + 4, "hosin_target.msa" },
		{ CRaceMotionData::NAME_SKILL + 5, "boho_target.msa" },
		{ CRaceMotionData::NAME_SKILL + 6, "gicheon_target.msa" },
		{ CRaceMotionData::NAME_SKILL + 16, "noejeon.msa" },
		{ CRaceMotionData::NAME_SKILL + 17, "byeorak.msa" },
		{ CRaceMotionData::NAME_SKILL + 18, "pokroe.msa" },
		{ CRaceMotionData::NAME_SKILL + 19, "jeongeop_target.msa" },
		{ CRaceMotionData::NAME_SKILL + 20, "kwaesok_target.msa" },
		{ CRaceMotionData::NAME_SKILL + 21, "jeungryeok_target.msa" },
		{ GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa", 0 },
		{ GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa", 0 },
		{ GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa", 0 },
		{ GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa", 0 },
		{ GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa", 0 },
		{ GUILD_SKILL_MAGICUP, "guild_jumunsul.msa", 0 },
	};

	std::vector<std::pair<BYTE, const char*>> m_vecSkillName = {
		{ 1, "bipabu" },
		{ 2, "yongpa" },
		{ 3, "paeryong" },
		{ 4, "hosin" },
		{ 5, "boho" },
		{ 6, "gicheon" },
		{ 16, "noejeon" },
		{ 17, "byeorak" },
		{ 18, "pokroe" },
		{ 19, "jeongeop" },
		{ 20, "kwaesok" },
		{ 21, "jeungryeok" },
	};

	char szSkillAdd[4] = "";
	for (WORD i = 1; i < CPythonSkill::SKILL_EFFECT_COUNT; i++)
	{
		for (auto& it : m_vecSkillName)
		{
			SMotion motion;

			motion.wMotionIndex = CRaceMotionData::NAME_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + it.first;
			motion.byPercentage = 0;

			snprintf(szSkillAdd, sizeof(szSkillAdd), "_%d", i + 1);
			snprintf(motion.stName, sizeof(motion.stName), "%s%s.msa", it.second, szSkillAdd);
			m_vecMotionSkill.push_back(motion);
		}
	}

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
	#ifdef ENABLE_STANDING_MOUNT
	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND);
	#endif
	for (const auto& it : m_vecMotionSkill)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "skill/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
		#ifdef ENABLE_STANDING_MOUNT
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND, it.wMotionIndex, szFileName, it.byPercentage);
		#endif
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_GENERAL, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_GENERAL, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	#ifdef ENABLE_STANDING_MOUNT
	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	#endif

	RegisterEmotionAnis(stFolder);

	std::vector<SMotion> m_vecMotionFan =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50,},
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50, },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50,},
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50, },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_FAN);
	for (const auto& it : m_vecMotionFan)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "fan/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_FAN, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);	

	std::vector<SMotion> m_vecMotionBell =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50,},
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50, },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50,},
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50, },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_BELL);
	for (const auto& it : m_vecMotionBell)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "bell/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_BELL, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);

	#ifdef ENABLE_STANDING_MOUNT
	std::vector<SMotion> m_vecMotionBoardFan =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "wait.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "wait.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50,},
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50, },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50,},
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50, },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND_FAN);
	for (const auto& it : m_vecMotionBoardFan)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "fan/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND_FAN, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_FAN, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);	

	std::vector<SMotion> m_vecMotionBoardBell =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "wait.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "wait.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 50,},
		{ CRaceMotionData::NAME_DAMAGE, "damage_1.msa", 50, },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_2.msa", 50,},
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage_3.msa", 50, },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND_BELL);
	for (const auto& it : m_vecMotionBoardBell)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "bell/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND_BELL, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_BELL, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);
	#endif

	std::vector<SMotion> m_vecMotionFishing =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_THROW, "throw.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_WAIT, "fishing_wait.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_STOP, "fishing_cancel.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_REACT, "fishing_react.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_CATCH, "fishing_catch.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_FAIL, "fishing_fail.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_FISHING);
	for (const auto& it : m_vecMotionFishing)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "fishing/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_FISHING, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionHorse =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 90 },
		{ CRaceMotionData::NAME_WAIT, "wait_1.msa", 9 },
		{ CRaceMotionData::NAME_WAIT, "wait_2.msa", 1 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "damage.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "damage.msa", 0 },
		{ CRaceMotionData::NAME_DEAD, "dead.msa", 0 },
		{ HORSE_SKILL_CHARGE, "skill_charge.msa", 0 },
		{ HORSE_SKILL_SPLASH, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE);
	for (const auto& it : m_vecMotionHorse)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionHorseFan =
	{
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_FAN);
	for (const auto& it : m_vecMotionHorseFan)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_fan/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_FAN, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_FAN, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_FAN, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_FAN, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_FAN, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);	

	std::vector<SMotion> m_vecMotionHorseBell =
	{
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_BELL);
	for (const auto& it : m_vecMotionHorseBell)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_bell/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_BELL, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_BELL, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_BELL, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_BELL, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_BELL, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);

	//##Bone
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON, "equip_right");
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON_LEFT, "equip_left");

	#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_ACCE, "Bip01 Spine2");
	#endif

	m_vecMotionGeneral.clear();
	m_vecMotionSkill.clear();
	m_vecSkillName.clear();
	m_vecMotionFan.clear();
	m_vecMotionBell.clear();
	m_vecMotionFishing.clear();
	m_vecMotionHorse.clear();
	m_vecMotionHorseFan.clear();
	m_vecMotionHorseBell.clear();
	#ifdef ENABLE_STANDING_MOUNT
	m_vecMotionBoardFan.clear();
	m_vecMotionBoardBell.clear();
	#endif
	return true;
}

#ifdef ENABLE_WOLFMAN_CHARACTER
const bool CPlayerSettingsModule::LoadGameWolfman(DWORD dwRace, char stFolder[FILE_MAX_NUM])
{
	CRaceManager& pkManager = CRaceManager::Instance();
	pkManager.SelectRace(dwRace);

	if (!LoadGeneralMotionForWolfman(stFolder))
		return false;

	CRaceData* pRaceData = pkManager.GetSelectedRaceDataPointer();
	if (!pRaceData)
		return false;

	if (!pRaceData->SetMotionRandomWeight(CRaceMotionData::MODE_GENERAL, CRaceMotionData::NAME_WAIT, 0, 70))
		return false;

	std::vector<SMotion> m_vecMotionGeneral =
	{
		{ CRaceMotionData::NAME_WAIT, "wait1.msa", 30 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "attack1.msa", 50 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "attack2.msa", 50 },
	};

	char szFileName[FILE_MAX_NUM];
	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
	for (const auto& it : m_vecMotionGeneral)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "general/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionSkill =
	{
		{ CRaceMotionData::NAME_SKILL + 1, "split_slash.msa" },
		{ CRaceMotionData::NAME_SKILL + 2, "wind_death.msa" },
		{ CRaceMotionData::NAME_SKILL + 3, "reef_attack.msa" },
		{ CRaceMotionData::NAME_SKILL + 4, "wreckage.msa" },
		{ CRaceMotionData::NAME_SKILL + 5, "red_possession.msa" },
		{ CRaceMotionData::NAME_SKILL + 6, "blue_possession.msa" },
		{ GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa", 0 },
		{ GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa", 0 },
		{ GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa", 0 },
		{ GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa", 0 },
		{ GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa", 0 },
		{ GUILD_SKILL_MAGICUP, "guild_jumunsul.msa", 0 },
	};

	std::vector<std::pair<BYTE, const char*>> m_vecSkillName = {
		{ 1, "split_slash" },
		{ 2, "wind_death" },
		{ 3, "reef_attack" },
		{ 4, "wreckage" },
		{ 5, "red_possession" },
		{ 6, "blue_possession" },
	};

	char szSkillAdd[4] = "";
	for (WORD i = 0; i < CPythonSkill::SKILL_EFFECT_COUNT; i++)
	{
		for (auto& it : m_vecSkillName)
		{
			SMotion motion;

			motion.wMotionIndex = CRaceMotionData::NAME_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + it.first;
			motion.byPercentage = 0;

			if (i > 0)
				snprintf(szSkillAdd, sizeof(szSkillAdd), "_%d", i);
			else
				snprintf(szSkillAdd, sizeof(szSkillAdd), "");

			snprintf(motion.stName, sizeof(motion.stName), "%s%s.msa", it.second, szSkillAdd);
			m_vecMotionSkill.push_back(motion);
		}
	}

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_GENERAL);
	#ifdef ENABLE_STANDING_MOUNT
	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND);
	#endif
	for (const auto& it : m_vecMotionSkill)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "skill/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_GENERAL, it.wMotionIndex, szFileName, it.byPercentage);
		#ifdef ENABLE_STANDING_MOUNT
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND, it.wMotionIndex, szFileName, it.byPercentage);
		#endif
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_GENERAL, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_GENERAL, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	#ifdef ENABLE_STANDING_MOUNT
	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND, COMBO_TYPE_1, 1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	#endif

	RegisterEmotionAnis(stFolder);

	std::vector<SMotion> m_vecMotionClaw =
	{
		{ CRaceMotionData::NAME_WAIT, "wait1.msa", 50 },
		{ CRaceMotionData::NAME_WAIT, "wait2.msa", 50 },
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 50 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "front_damage.msa", 50,},
		{ CRaceMotionData::NAME_DAMAGE, "back_damage.msa", 50, },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "back_damage.msa", 50,},
		{ CRaceMotionData::NAME_DAMAGE_BACK, "back_damage1.msa", 50, },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_CLAW);
	for (const auto& it : m_vecMotionClaw)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "claw/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_CLAW, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_CLAW, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);

	#ifdef ENABLE_STANDING_MOUNT
	std::vector<SMotion> m_vecMotionBoardClaw =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 50 },
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 50 },
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 50 },
		{ CRaceMotionData::NAME_WALK, "wait.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "wait.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "front_damage.msa", 50,},
		{ CRaceMotionData::NAME_DAMAGE, "back_damage.msa", 50, },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "back_damage.msa", 50,},
		{ CRaceMotionData::NAME_DAMAGE_BACK, "back_damage1.msa", 50, },
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_4, "combo_04.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_5, "combo_05.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_6, "combo_06.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_7, "combo_07.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_STAND_CLAW);
	for (const auto& it : m_vecMotionBoardClaw)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "claw/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_STAND_CLAW, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_1, 4);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_1, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_4);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_2, 5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_2, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_2, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_2, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_2, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_2, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_7);

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_3, 6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_3, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_3, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_3, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_3, COMBO_INDEX_4, CRaceMotionData::NAME_COMBO_ATTACK_5);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_3, COMBO_INDEX_5, CRaceMotionData::NAME_COMBO_ATTACK_6);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_STAND_CLAW, COMBO_TYPE_3, COMBO_INDEX_6, CRaceMotionData::NAME_COMBO_ATTACK_4);
	#endif

	std::vector<SMotion> m_vecMotionFishing =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 0 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_THROW, "throw.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_WAIT, "fishing_wait.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_STOP, "fishing_cancel.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_REACT, "fishing_react.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_CATCH, "fishing_catch.msa", 0 },
		{ CRaceMotionData::NAME_FISHING_FAIL, "fishing_fail.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_FISHING);
	for (const auto& it : m_vecMotionFishing)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "fishing/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_FISHING, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionHorse =
	{
		{ CRaceMotionData::NAME_WAIT, "wait.msa", 90 },
		{ CRaceMotionData::NAME_WAIT, "wait1.msa", 9 },
		{ CRaceMotionData::NAME_WAIT, "wait2.msa", 1 },
		{ CRaceMotionData::NAME_WALK, "walk.msa", 0 },
		{ CRaceMotionData::NAME_RUN, "run.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE, "front_damage.msa", 0 },
		{ CRaceMotionData::NAME_DAMAGE_BACK, "front_damage.msa", 0 },
		{ CRaceMotionData::NAME_DEAD, "dead.msa", 0 },
		{ HORSE_SKILL_CHARGE, "skill_charge.msa", 0 },
		{ HORSE_SKILL_SPLASH, "skill_splash.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE);
	for (const auto& it : m_vecMotionHorse)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE, it.wMotionIndex, szFileName, it.byPercentage);
	}

	std::vector<SMotion> m_vecMotionHorseClaw =
	{
		{ CRaceMotionData::NAME_COMBO_ATTACK_1, "combo_01.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_2, "combo_02.msa", 0 },
		{ CRaceMotionData::NAME_COMBO_ATTACK_3, "combo_03.msa", 0 },
		{ HORSE_SKILL_WILDATTACK, "skill_wildattack.msa", 0 },
	};

	pRaceData->RegisterMotionMode(CRaceMotionData::MODE_HORSE_CLAW);
	for (const auto& it : m_vecMotionHorseClaw)
	{
		snprintf(szFileName, sizeof(szFileName), "%s%s%s", stFolder, "horse_claw/", it.stName);
		pRaceData->RegisterMotionData(CRaceMotionData::MODE_HORSE_CLAW, it.wMotionIndex, szFileName, it.byPercentage);
	}

	pRaceData->ReserveComboAttack(CRaceMotionData::MODE_HORSE_CLAW, COMBO_TYPE_1, 3);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_CLAW, COMBO_TYPE_1, COMBO_INDEX_1, CRaceMotionData::NAME_COMBO_ATTACK_1);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_CLAW, COMBO_TYPE_1, COMBO_INDEX_2, CRaceMotionData::NAME_COMBO_ATTACK_2);
	pRaceData->RegisterComboAttack(CRaceMotionData::MODE_HORSE_CLAW, COMBO_TYPE_1, COMBO_INDEX_3, CRaceMotionData::NAME_COMBO_ATTACK_3);	

	//##Bone
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON, "equip_right_weapon");
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_WEAPON_LEFT, "equip_left_weapon");

	#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	pRaceData->RegisterAttachingBoneName(CRaceData::PART_ACCE, "Bip01 Spine2");
	#endif

	m_vecMotionGeneral.clear();
	m_vecMotionSkill.clear();
	m_vecSkillName.clear();
	m_vecMotionClaw.clear();
	m_vecMotionFishing.clear();
	m_vecMotionHorse.clear();
	m_vecMotionHorseClaw.clear();
	#ifdef ENABLE_STANDING_MOUNT
	m_vecMotionBoardClaw.clear();
	#endif
	return true;
}
#endif

const bool CPlayerSettingsModule::LoadGameSound()
{
	std::vector<TSound> m_vecSound = {
		{ CPythonItem::USESOUND_DEFAULT, "sound/ui/drop.wav" },
		{ CPythonItem::USESOUND_ACCESSORY, "sound/ui/equip_ring_amulet.wav" },
		{ CPythonItem::USESOUND_ARMOR, "sound/ui/equip_metal_armor.wav" },
		{ CPythonItem::USESOUND_BOW, "sound/ui/equip_bow.wav" },
		{ CPythonItem::USESOUND_WEAPON, "sound/ui/equip_metal_weapon.wav" },
		{ CPythonItem::USESOUND_POTION, "sound/ui/eat_potion.wav" },
		{ CPythonItem::USESOUND_PORTAL, "sound/ui/potal_scroll.wav" },
	};

	CPythonItem& rkItem = CPythonItem::Instance();
	for (const auto &it : m_vecSound)
		rkItem.SetUseSoundFileName(it.dwType, it.stName);

	std::vector<TSound> m_vecDropSound = {
		{ CPythonItem::DROPSOUND_DEFAULT, "sound/ui/drop.wav" },
		{ CPythonItem::DROPSOUND_ACCESSORY, "sound/ui/equip_ring_amulet.wav" },
		{ CPythonItem::DROPSOUND_ARMOR, "sound/ui/equip_metal_armor.wav" },
		{ CPythonItem::DROPSOUND_BOW, "sound/ui/equip_bow.wav" },
		{ CPythonItem::DROPSOUND_WEAPON, "sound/ui/equip_metal_weapon.wav" },
	};

	for (const auto &it : m_vecDropSound)
		rkItem.SetDropSoundFileName(it.dwType, it.stName);

	m_vecSound.clear();
	m_vecDropSound.clear();

	return true;
}

const bool CPlayerSettingsModule::LoadGameSkill()
{
	return true;
}

PyObject* cpsmInitData(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadInitData();
	return Py_BuildNone();
}

PyObject* cpsmLoadGameSound(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameSound();
	return Py_BuildNone();
}

PyObject* cpsmLoadGameEffect(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameEffect();
	return Py_BuildNone();
}

PyObject* cpsmLoadGameWarrior(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameWarrior(CPlayerSettingsModule::RACE_WARRIOR_M, "d:/ymir work/pc/warrior/");
	CPlayerSettingsModule::LoadGameWarrior(CPlayerSettingsModule::RACE_WARRIOR_W, "d:/ymir work/pc2/warrior/");
	return Py_BuildNone();
}

PyObject* cpsmLoadGameAssassin(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameAssassin(CPlayerSettingsModule::RACE_ASSASSIN_W, "d:/ymir work/pc/assassin/");
	CPlayerSettingsModule::LoadGameAssassin(CPlayerSettingsModule::RACE_ASSASSIN_M, "d:/ymir work/pc2/assassin/");
	return Py_BuildNone();
}

PyObject* cpsmLoadGameSura(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameSura(CPlayerSettingsModule::RACE_SURA_M, "d:/ymir work/pc/sura/");
	CPlayerSettingsModule::LoadGameSura(CPlayerSettingsModule::RACE_SURA_W, "d:/ymir work/pc2/sura/");
	return Py_BuildNone();
}

PyObject* cpsmLoadGameShaman(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameShaman(CPlayerSettingsModule::RACE_SHAMAN_W, "d:/ymir work/pc/shaman/");
	CPlayerSettingsModule::LoadGameShaman(CPlayerSettingsModule::RACE_SHAMAN_M, "d:/ymir work/pc2/shaman/");
	return Py_BuildNone();
}

#ifdef ENABLE_WOLFMAN_CHARACTER
PyObject* cpsmLoadGameWolfman(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameWolfman(CPlayerSettingsModule::RACE_WOLFMAN_M, "d:/ymir work/pc3/wolfman/");
	return Py_BuildNone();
}
#endif

PyObject* cpsmLoadGameSkill(PyObject* poSelf, PyObject* poArgs)
{
	CPlayerSettingsModule::LoadGameSkill();
	return Py_BuildNone();
}

void initplayersettingsmodule()
{
	static PyMethodDef s_methods[] =
	{
		{ "InitData",					cpsmInitData,						METH_VARARGS },
		{ "LoadGameSound",				cpsmLoadGameSound,					METH_VARARGS },
		{ "LoadGameEffect",				cpsmLoadGameEffect,					METH_VARARGS },

		{ "LoadGameWarrior",			cpsmLoadGameWarrior,				METH_VARARGS },
		{ "LoadGameAssassin",			cpsmLoadGameAssassin,				METH_VARARGS },
		{ "LoadGameSura",				cpsmLoadGameSura,					METH_VARARGS },
		{ "LoadGameShaman",				cpsmLoadGameShaman,					METH_VARARGS },
		#ifdef ENABLE_WOLFMAN_CHARACTER
		{ "LoadGameWolfman",			cpsmLoadGameWolfman,				METH_VARARGS },
		#endif

		{ "LoadGameSkill",				cpsmLoadGameSkill,					METH_VARARGS },

		{ NULL,							NULL,								NULL		 },
	};

	PyObject* poModule = Py_InitModule("cpsm", s_methods);
}
#endif
