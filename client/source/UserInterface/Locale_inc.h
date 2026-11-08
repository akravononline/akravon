#pragma once

//////////////////////////////////////////////////////////////////////////
// ### ServiceDefs Macros ###
#define _IMPROVED_PACKET_ENCRYPTION_

// #define __PERFORMANCE_CHECK__
//#define ENABLE_NO_COLLISION_MOB			// Mobların içinden geçme

#define ENABLE_TURKLOKASYON_FIREWALL
#ifdef ENABLE_TURKLOKASYON_FIREWALL
#define SERVER_IP	"127.0.0.1"
#endif

//////////////////////////////////////////////////////////////////////////
// ### Default Ymir Macros ###
#define LOCALE_SERVICE_EUROPE
#define ENABLE_COSTUME_SYSTEM
#define ENABLE_ENERGY_SYSTEM
#define ENABLE_DRAGON_SOUL_SYSTEM
#define ENABLE_NEW_EQUIPMENT_SYSTEM
// ### Default Ymir Macros ###
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ### New From LocaleInc ###
#define ENABLE_PACK_GET_CHECK
#define ENABLE_CANSEEHIDDENTHING_FOR_GM
#define ENABLE_PROTOSTRUCT_AUTODETECT
#define ENABLE_PLAYER_PER_ACCOUNT5
#define ENABLE_LEVEL_IN_TRADE
#define ENABLE_DICE_SYSTEM
#define ENABLE_EXTEND_INVEN_SYSTEM
#define ENABLE_LVL115_ARMOR_EFFECT
#define ENABLE_SLOT_WINDOW_EX
#define ENABLE_TEXT_LEVEL_REFRESH
#define ENABLE_USE_COSTUME_ATTR
#define ENABLE_DISCORD_RPC
#define ENABLE_PET_SYSTEM_EX

#define ENABLE_NEW_EVENT_STRUCT
#ifdef ENABLE_NEW_EVENT_STRUCT
#define USE_NEW_EVENT_TEXT_AUTO_Y
#endif
#define WJ_SHOW_MOB_INFO
#ifdef WJ_SHOW_MOB_INFO
#define ENABLE_SHOW_MOBAIFLAG
#define ENABLE_SHOW_MOBLEVEL
#endif
// ### New From LocaleInc ###
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ### From GameLib ###
#define ENABLE_WOLFMAN_CHARACTER
// #define ENABLE_MAGIC_REDUCTION_SYSTEM
#define ENABLE_MOUNT_COSTUME_SYSTEM
#define ENABLE_WEAPON_COSTUME_SYSTEM
// ### From GameLib ###
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ### New System Defines - Extended Version ###

// if is define ENABLE_ACCE_COSTUME_SYSTEM the players can use shoulder sash
// if you want to use object scaling function you must defined ENABLE_OBJ_SCALLING
#define ENABLE_ACCE_COSTUME_SYSTEM
#define ENABLE_OBJ_SCALLING
// #define USE_ACCE_ABSORB_WITH_NO_NEGATIVE_BONUS

//if you want to see highlighted a new item when dropped or when exchanged
#define ENABLE_HIGHLIGHT_NEW_ITEM

// it shows emojis in the textlines
#define ENABLE_EMOJI_SYSTEM

// effects while hidden won't show up
#define __ENABLE_STEALTH_FIX__

// circle dots in minimap instead of squares
#define ENABLE_MINIMAP_WHITEMARK_CIRCLE

// details tooltip remove skill affect
#define ENABLE_RONARK_SYSTEM						// Ronark Sistemi
#define MINIMAP_TIKLA_ISINLAN						// Minimap Tıkladığın yere ışınlanma
#define ENABLE_DUNGEON_BOSS_ICON_IN_MAP				// Minimap boss gösterme
#define __ENABLE_SHAMAN_AND_WOLFMAN_ATTACK_FIX__	//Şaman ve lycan atak kaçırma fixi

#define ENABLE_DISCORD_RPC						// Discord oynuyor eklentisi
#ifdef ENABLE_DISCORD_RPC
#define DISCORD_APP_CLIENT_ID	"1382733377303482522"
#define DISCORD_SITE_NAME		"www.Barbaros2.com"
#endif

#define ENABLE_DRAGON_COIN_SHOP_SYSTEM			// Nesne Market
#ifdef ENABLE_DRAGON_COIN_SHOP_SYSTEM
	#define ENABLE_DRAGON_COIN_SHOP_ADMIN_SYSTEM	// Nesne Market Admin Panel
	#define ENABLE_DRAGON_COIN_EM_MARKET			// Nesne Market Ejderha Markası
#endif

#define __BL_LUCKY_BOX__						// Official Şans Kutusu Sistemi
#define ENABLE_HIYUS_FIX						// Sanal PC Fix
#define ENABLE_BOSS_FALL_FIX					// boss sektirme
#define ENABLE_SERVER_PIN						// Pin
#define ENABLE_VERSION_SYSTEM					// Version
#define ENABLE_SKILL_RENEWAL					// Kitap okuma sayma
#define ENABLE_BIYOLOG							// Biyolog
#define ENABLE_RUZGARIN_AYAKKABI				// Sınırsız Rüzgar Ayakkabısı
#define ENABLE_RENEWAL_AFFECT_SHOWER			// Skill vs kapatma
#define ENABLE_SEND_TARGET_INFO					// Slottan & Metinden düşen sistemi.
#define ENABLE_DETAILS_INTERFACE				// Official efsun ayrıntı paneli
#define ENABLE_SHOW_CHEST_DROP					// Sandık içgörü sistemi.
#define ENABLE_CLEAR_CHAT						// Chat temizleme
#define ENABLE_CHAT_STOP						// Chat durdur
#define ENABLE_SHIFT_CHAT_STOP					// Shift Chat durdur
#define ENABLE_STATUS_UP_RENEWAL				// Hızlı Statü
#define ENABLE_FAST_ATTACH_ITEMS_SYSTEM			// sağ tıkla ticaret
#define ENABLE_NEW_MAX_COUNT					// eşya limit sistemi
#define ENABLE_INSTANT_PICKUP_SYSTEM			// Hızlı item toplama
#define ENABLE_AUTO_PICKUP_SYSTEM				// Oto İtem Toplama

#define ENABLE_MOUSEWHEEL_EVENT					// Scrollbar
#define ENABLE_MOUSEWHEEL_EVENT_OLD				// Scrollbar

#define ENABLE_FLASH_APPLICATION				// Pm geldiğinde client bildiri
#define ENABLE_CHANNEL_INFO_UPDATE				// Kanal yenileme
#define ENABLE_SPECIAL_STORAGE					// K envanteri
#define __AUTO_SKILL_READER__					// Oto Skill 
#define ENABLE_SWITCHBOT						// Efsun Botu
#define ENABLE_IMAGE_CLIP_RECT					// İmage Scroll
#define __BL_CLIP_MASK__
#define ENABLE_ITEM_UPDATE_OPTIMIZATION			// İtem paket optimizasyon
#define ENABLE_ON_TARGET_HIDE					// Pet, binek sağ click ve bayrak gizleme
#define ENABLE_NO_COLLISION_NPC_PET_MOUNT		// Npc, Pet, binek içinden geçme
#define ENABLE_SAFEZONE_STOP_COLLISION			// Korumalı alanlarda birbirinin içinden geçme
#define ENABLE_REFINE_RENEWAL					// Hızlı + basma
#define ENABLE_CHAT_FLASH						// Chat Editline
#define ENABLE_ITEM_DELETE_SYSTEM				// Toplu İtem sil
#define ENABLE_DROP_DIALOG_EXTENDED_SYSTEM		// Yere item sil sat
#define ENABLE_BOSS_EFFECT_SYSTEM				// Patron efekti
#define ENABLE_PERMA_SET						// Perma Affectler
#define __BL_HIDE_EFFECT__						// skill buff gizleme
#define ENABLE_ANTI_EXP							// Anti Exp
#define ENABLE_OFFER_SYSTEM						// Teklif
#define ENABLE_FAST_ATTACH_ITEMS_SYSTEM			// Depo hızlı item aktarma
#define ENABLE_TRADE_DELETE_SLOT_EFFECT			// Slot Effect
#define ENABLE_GRAPHIC_ON_OFF					// graphic performance
#define ENABLE_FAST_OPEN_CHEST 					// Sandık  açımı
#define ENABLE_FOG_FIX							// sis aç kapat
#define ENABLE_SHOW_NIGHT_SYSTEM				// gece gündüz aç kapat
#define ENABLE_FOV_OPTION						// Kamera Modu
#define ENABLE_MESSENGER_BLOCK					// oyuncu engelleme
#define ENABLE_HIDE_MOB_SYSTEM					// mob detay
#define ENABLE_KOSTUM_SAC_BINEK_PET_EFSUNLAMA	// Efsunlama
#define ENABLE_COSTUME_WEAPON_ATTR				// Silah Efsunlama
#define ENABLE_STYLE_ATTRIBUTE_SYSTEM			// Kadim Efsun
#define ENABLE_GIVE_TIME_COSTUME_SYSTEM			// Kostüm süre uzatma sistemi
#define ENABLE_NEW_RING							// Yeni slotlar
#define ENABLE_COSTUME_PET_SYSTEM				// Giymeli Pet
#define ENABLE_EXCHANGE_RENEWAL					// Ticaret Ekran
#define ENABLE_PASIF_SKILL						// Pasif Skiller
#define ENABLE_NEW_ATTRIBUTE_TYPES				// Yeni efsun tipleri
#define ENABLE_HIDE_COSTUME_PART				// Kostüm Gizleme sistemi
#define ENABLE_PET_MOUNT_PACKET_PERFORMANCE		// Pet Binek Performans
#define ENABLE_MEMORY_LEAK
#define ENABLE_FULL_YANG						// Sınırsız Yang
#define ENABLE_EXTRA_SOCKET_SYSTEM				// 4.Slot Sistemi
#define ENABLE_EFFECT_SCALE						// Effect gizleme

#define ENABLE_CUBE_RENEWAL_WORLDARD			// Offical Cube Sistemi
#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
	#define CUBE_RENEWAL_RELOAD_FIX				// Offical Arındırma Cube Reload Sistemi
#endif

#define ENABLE_SMITH_EFFECT_SYSTEM				// +Basma Effecti
#define INSIDE_RENDER							// Window animation
#define __RANKING_SYSTEM__						// Global Rank
#define __AUTO_SELL__							// Otomatik satma
#define ENABLE_OFFLINESHOP_DEBUG				// offlineshop debug
#define ENABLE_DISCORD_STUFF					// Discord mesaj
#define ENABLE_NEW_GAMEOPTION					// oyun seçenekleri
#define ENABLE_PM_ALL_SEND_SYSTEM				// Toplu pm
#define ENABLE_BAKIM_DUYURU						// Bakım Duyurusu
#define ENABLE_EXYGO_FPS_BOOST
#define __ARE_YOU_IN_OR_NOT__					// Var mısın Yokmusun?
#define ENABLE_EXCHANGE_LOG						// Ticaret log

#define __WHISPER_FUTURES__						// pm kaydet
#define ENABLE_NEW_LOADING						// yeni loading penceresi
//#define ENABLE_FPS							// fps opt.
#define RENEWAL_HWID_BLOCK						// Hwid ban.
#define ENABLE_LOG_STATE						// itemci log
#define RENEWAL_DISCORD_BLOCK					// discord eklenti
#define ENABLE_HILE_ENGEL						// hile engel
#define ENABLE_CYPOR_ENGEL						// cypor engel
#define ENABLE_SIKITUT_ENGEL					// sqitut engel
#define ENABLE_PERFORMANCE_REWORK				// performans
#define ENABLE_SECURITY_FISHING					// Balık tutarken damage
#define ENABLE_REFINE_LUCK						// + basma şans
#define ENABLE_SKILL_TABLE_RENEW				// skill_proto -> skilltable
#define ENABLE_KADIM_SILAH						// Kadim Silah
#define ENABLE_FIX_MAP_DUEL_SKILL				// Vs Turnuvası
#define ENABLE_ENTITY_PRELOADING				// Load Entity Information from Server ( Prevent Frame Spikes )
#define __RENEWAL_NOTICE__						// New notice aspect.

#define ENABLE_IKASHOP_RENEWAL
#define ENABLE_IKASHOP_ENTITIES
#define ENABLE_LARGE_DYNAMIC_PACKETS
#define ENABLE_SHOP_SEARCH_SYSTEM
#define ENABLE_IKASHOP_AVG_AND_MINMAX_PRICE

#define ENABLE_GUILD_WAR_OPTION_SYSTEM			// Lonca savaş ayarları
#define ENABLE_GUILD_STATISTICS_SYSTEM			// Lonca istatistik sistemi

#define ENABLE_SKILL_TIME						// Beceri süre uzatma sistemi
#define __RENEWAL_BRAVE_CAPE__					// Oto pelerin sistemi
#define ENABLE_BATTLE_PASS						// Battle Pass
#define ENABLE_TRADABLE_ICON					// TR Tipi slot marking sistemi
#define ENABLE_TELEPORTION_WIZARD_SYSTEM		// Işınlanma sihirbazı sistemi
#define ENABLE_PVM_BONUS						// Pvm efsunlar
#define ENABLE_CAPTCHA_SELECT_SYSTEM			// Bot kontrol

#define __AUTO_HUNT__							// Otomatik Av
#define ENABLE_AFFECT_CHECK
#define ENABLE_MULTI_FARM_BLOCK					// Multi farm
#define ENABLE_MULTI_HUNT_BLOCK					// Multi hunt
#define ENABLE_MULTI_FISH_BLOCK					// Multi fish

#define ENABLE_EVOLUTION_SYSTEM					// Evrim sistemi
#define ENABLE_CHANGELOOK_SYSTEM				// Tr yansıtma sistemi
#define ENABLE_LOCKED_SYSTEM					// İtem kilit
#define __BACK_DUNGEON__						// Zindan geri dönüş
#define NEW_MOB_TIMER							// Saatli Boss Sistemi
#define ENABLE_PERMA_BLEND						// Perma şebnem-sular
#define ENABLE_FARM_BLEND						// Farm şebnem
#define ENABLE_COSTUME_SET_BONUS				// Kostüm set bonus
#define ENABLE_ALIGNMENT_BONUS					// Derece sistemi
#define ENABLE_PLAYER_TAG_SYSTEM				// Tag sistemi
#define ENABLE_BLACK_MARKET_SYSTEM				// Piyasa npc sistemi
#define ENABLE_DEFENCE_SHIELD					// Pvp koruma kalkanı
#define ENABLE_MESSENGER_TEAM					// Takım listesi arkadaş penceresinde

#define ENABLE_RENDER_TARGET					// Gr2 ön izleme
#ifdef ENABLE_RENDER_TARGET
	// #define ENABLE_WIKI_RENDER_TARGET		// Gr2 ön izleme Wiki
#endif

#define BOSS_DMG_RANKING						// Boss Damage Sıralama
#ifdef BOSS_DMG_RANKING
	#define	MAX_PLAYERS_IN_TABLE 10
#endif
#define SPIN_WHEEL

#define ENABLE_REWARD_SYSTEM					//İlkler Listesi

#define EXCHANGE_CHECKIN_UPDATE					// Ticaret sağ tık ile item koyma

#define ENABLE_PCBAN_SYSTEM						// Pc ban sistemi

#define ENABLE_STANDING_MOUNT					// Kaykay sistemi
#ifdef ENABLE_STANDING_MOUNT
	#define SURFBOARD 40003						// Sorf tahtası sistemi
#endif

#define ENABLE_AUTO_ITEM_SELL_SYSTEM			// Otomatik Satış8B9Af34wWcog8B9Af34wWcog

///----------EVENTS----------///
#define ENABLE_EVENT_MANAGER					// Otomatik Event
#define ENABLE_WHEEL_OF_FORTUNE					// Çarkıfelek
#define ENABLE_TANAKA_EVENT						// Tanaka Eventi
#define ENABLE_PUBLISHER_SYSTEM					// Yayıncı sistemi
#define ENABLE_OKEY_KART_EVENT					//Okey kart sistemi
///----------EVENTS----------///

///----------PERFORMANCE----------///
#define ENABLE_POLY_ALL_MOBS_DOG				// Köpek modu
#define ENABLE_CPP_PSM							// PlayerSettingModule
#define CEF_BROWSER								// CEF Browser
#define ENABLE_FAST_BLEND_ALPHA					// Mobların Hızlı Kaybolması Yada Hızlı Doğması
#define ENABLE_TEXT_PROBLEM_FIX					// Text kasma fix
#define ENABLE_FIX_MOBS_LAG						// Fix mob lags FPS

#define ENABLE_GIF_SUPPORT						// Gif desteği
///----------PERFORMANCE----------///

//#define ENABLE_RASCAL_ANTICHEAT

// for debug: print received packets
// #define ENABLE_PRINT_RECV_PACKET_DEBUG

// ### New System Defines - Extended Version ###
//////////////////////////////////////////////////////////////////////////

// Cynex: mobil - windows'a bagli ozellikler kapali (paket yapisina dokunmuyorlar)
#ifdef CYNEX_MOBILE
	#undef ENABLE_DISCORD_RPC

	#undef ENABLE_HIYUS_FIX
	#undef ENABLE_HILE_ENGEL
	#undef ENABLE_CYPOR_ENGEL
	#undef ENABLE_SIKITUT_ENGEL

	#undef ENABLE_RASCAL_ANTICHEAT
#endif
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
