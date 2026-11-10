from __future__ import division
import app
import dbg
import grp
import math
import player
import skill
import item
import systemSetting
import wndMgr
import mouseModule
import constInfo
import ui
import uiMobileCompat
import uiAkravonSkillGrade
import uiAkravonSkillLayout
try:
	import uiAkravonSkillIcon
except Exception:
	uiAkravonSkillIcon = None

from _weakref import proxy

DESKTOP_CAMERA_PITCH = 27.0
DEFAULT_CAMERA_PITCH = 14.0

JOYSTICK_DEFAULT_SIZE = 200
ATTACK_DEFAULT_SIZE = 140
ATTACK_GROWTH = 1.15 * 1.3	# AKRAVON: varsayilan saldiri eskisinin %130u (kullanici ayari); ayarda %100 gorunur

LEFT_MARGIN = 60
RIGHT_MARGIN = 56
BOTTOM_MARGIN = 38
TOP_MARGIN = 10
SCREEN_EDGE = 12
BASE_TOP_MARGIN = 10
BASE_SCREEN_EDGE = 12
SAFE_LEFT = SAFE_RIGHT = SAFE_TOP = SAFE_BOTTOM = 0	# AKRAVON: centik / yuvarlak kose guvenli alani (mantiksal piksel)
WINDOW_GAP = 8

SLOT_SIZE = 58
PAGE_BUTTON_SIZE = 40
ROUND_ICON_RATIO = 0.66	# kare ikonun koseleri diskin icinde kalsin (maske orter)	# yuvarlak beceri ikonu / yuva capi (disk duz alani ~0.75)
SLOT_RING_GAP = 14
SLOT_RING_STEP = 8
# AKRAVON: saldiri butonunun etrafinda tek yay - ilk halkada 6 konum (5 beceri + "+"), disarida 2 konum daha
# AKRAVON: varsayilan - saldirinin etrafinda tek yay, 5 slot sol alttan (160) sag uste (292) esit aralikli.
# yaricap slot boyuna oranli: her ekranda (iPhone 8 Plus .. Pro Max, Android) ayni gorunur. P plakasi komsuya degmez.
# 6-8. konumlar yalniz slot sayisi artirilinca (dis yay).
RING_ANGLES = (160.0, 193.0, 226.0, 259.0, 292.0)
OUTER_RING_ANGLES = (176.5, 209.5, 242.5)
RING_LAYOUT_SPOTS = 5	# saldiri konumu varsayilan 5 slota gore hesaplanir
INNER_RADIUS_RATIO = 2.16	# yay yaricapi / slot (P plakasi dahil cakismasiz en yakin: 2.12)
OUTER_RADIUS_RATIO = 3.1
# AKRAVON: sag taraf varsayilanlari (kullanici yerlestirdi 2026-11-09): merkez = sag sinirdan (SCREEN_EDGE) uzaklik, y = ekran yuksekligi orani
UTILITY_SCALE = 1.2	# topla / oto av / at eskisinin %120'si (ayarda %100)
PAGE_SCALE = 1.3	# dondur %130
EDIT_SCALE = 1.2	# ayar %120
UTILITY_RIGHT_INSET = 26
UTILITY_FIRST_CENTER_Y = 0.345	# topla merkezi; oto av / at altinda boy + UTILITY_GAP araliklarla (buyuyunce ust uste binmez)
RIGHT_SAFE_SHARE = 0.5	# sag sutun / saldiri: centik payinin yarisi (tam pay sagda gereksiz bosluk birakiyordu)
PAGE_RIGHT_INSET = 84
PAGE_CENTER_Y = 0.655
EDIT_RIGHT_INSET = 23
EDIT_CENTER_Y = 0.809
RING_DROP_RATIO = 0.12	# alttaki bosluk kalksin: sinir en alttaki slotun P plakasi (ekran kenarindan SCREEN_EDGE)
PAGE_BUTTON_ANGLE = 140.0	# dondur: saldirinin sol alti; ayar ondan hemen asagida (ic halka)
EDIT_BUTTON_ANGLE = 115.0
SKILL_SLOT_MAX = 8
SKILL_SLOT_DEFAULT = 5

UTILITY_SIZE = 54
UTILITY_GAP = 6
UTILITY_COUNT = 3
MINIMAP_RESERVED_WIDTH = 160
MINIMAP_RESERVED_HEIGHT = 214

STATUS_BAR_HEIGHT = 78	# panel 70 + orijinal EXP plakasi
NOTIFICATION_GAP = 12
NOTIFICATION_HEIGHT = 48
MENU_BUTTON_SIZE = 52
MENU_BUTTON_GAP = 8
MENU_BUTTON_COUNT = 4
STATUS_BAR_WIDTH = 228
AFFECT_GAP = 6
AFFECT_ROW_HEIGHT = 44	# uiAffectShower MOBILE_IMAGE_STEP: 40 px ikon + 4 aralik

CHAT_PILL_WIDTH = 380
CHAT_PILL_HEIGHT = 38

RUN_LOCK_SIZE = 64
RUN_LOCK_GAP = 58

SLOT_ICON_SIZE = 32
SLOT_TEXTURE_SIZE = 256.0
DISC_TEXTURE = "mobile_ui/akravon/hud/hud_disc.png"
RING_TEXTURE = "mobile_ui/akravon/hud/hud_ring.png"
GLOW_TEXTURE = "mobile_ui/akravon/hud/hud_active_glow.png"
RIPPLE_TEXTURE = "mobile_ui/akravon/hud/hud_active_ripple.png"
ICON_MASK_TEXTURE = "mobile_ui/akravon/hud/hud_icon_mask.png"
HD_ICON_RATIO = 0.84	# HD ikon capi / yuva capi (disk kenarina kadar doldurur)
HD_ICON_TEXTURE_SIZE = 64.0
COOL_FRAMES = 36
PRESS_EFFECT_SECONDS = 0.42	# AKRAVON: beceri slotu basma efekti suresi (ikon cokmesi ilk %50)
PRESS_RING_TEXTURE = "mobile_ui/akravon/hud/hud_press_ring.png"
PRESS_DOT_TEXTURE = "mobile_ui/akravon/hud/hud_press_dot.png"
PRESS_RING_COUNT = 2
PRESS_DOT_COUNT = 8
# AKRAVON: geri sayim = oto av geri sayimiyla ayni (Marcellus, krem, cerceveli; yuva capinin %40'i)
COOL_COLOR = grp.GenerateColor(1.0, 0.922, 0.902, 1.0)
COOL_SHADOW = grp.GenerateColor(0.0, 0.0, 0.0, 0.0)


def CoolFontSize(size):
	return max(20, int(size * 0.4))


def CoolFont(size):
	return "Marcellus:%d" % CoolFontSize(size)


def CoolHalf(size):
	return int(CoolFontSize(size) * 0.55)
PAGE_TEXTURE = "mobile_ui/akravon/hud/hud_page_%d.png"
PAGE_TEXTURE_SIZE = 256.0

CONFIG_FILE_NAME = "mobile_hud.cfg"	# Cynex: config/ klasoru telefonda olmayabilir
CONFIG_DYNAMIC_JOYSTICK = "DYNAMIC_JOYSTICK"
CONFIG_SHOW_FPS = "SHOW_FPS"
CONFIG_SKILL_SLOTS = "SKILL_SLOTS"
CONFIG_SKILL_LAYOUT = "SKILL_PLAQUE_LAYOUT"
CONFIG_CHAT_HEIGHT = "CHAT_HEIGHT"
PLUS_TEXTURE = "mobile_ui/akravon/hud/hud_plus.png"

QUICK_PAGE_COUNT = 4
MOBILE_QUICK_PAGE_COUNT = 2
PAGE_TRANSITION_SECONDS = 0.20
PAGE_ROTATE_DEGREES = 40.0	# AKRAVON: sayfa degisirken yay saldirinin etrafinda bu kadar doner (yarida icerik degisir)

supportCache = None
enabledState = False
missingApiReported = False
refreshRequest = None
configCache = None


def IsSupported():
	global supportCache
	if supportCache is None:
		supportCache = getattr(app, "TOUCH_API_VERSION", 0) >= 1 and hasattr(systemSetting, "IsMobileHud")
	return supportCache


def Refresh():
	global enabledState, missingApiReported
	requested = False
	if hasattr(systemSetting, "IsMobileHud"):
		try:
			requested = bool(systemSetting.IsMobileHud())
		except Exception:
			requested = False

	if requested and not IsSupported():
		if not missingApiReported:
			missingApiReported = True
			dbg.TraceError("uiMobileHud: MOBIL_HUD is on but the client has no touch API, HUD disabled")
		requested = False

	enabledState = requested
	return enabledState


def IsEnabled():
	return enabledState


def SetEnabled(value):
	if hasattr(systemSetting, "SetMobileHud"):
		systemSetting.SetMobileHud(1 if int(value) else 0)
		systemSetting.SaveConfig()
	return Refresh()


def SetDoubleTapGate(enabled):
	if IsSupported() and hasattr(app, "SetIkiDokunusKapisi"):
		app.SetIkiDokunusKapisi(1 if enabled else 0)


def IsCameraMode():
	if hasattr(player, "IsCameraMode"):
		try:
			return bool(player.IsCameraMode())
		except Exception:
			return False
	return False


def IsCameraSettingSupported():
	return getattr(app, "TOUCH_API_VERSION", 0) >= 2


def GetCameraPitch():
	if IsCameraSettingSupported():
		try:
			return float(systemSetting.GetCameraReturnPitch())
		except Exception:
			pass
	return DESKTOP_CAMERA_PITCH


def BindRefreshRequest(function):
	global refreshRequest
	refreshRequest = function


def RequestRefresh():
	if refreshRequest is None:
		return
	try:
		refreshRequest()
	except ReferenceError:
		pass


def IsSettingsCategoryVisible():
	if not IsSupported():
		return False
	try:
		return bool(systemSetting.IsMobileHud())
	except Exception:
		return False


def LoadConfig():
	global configCache
	if configCache is not None:
		return configCache

	configCache = {}
	try:
		for line in old_open(CONFIG_FILE_NAME, "r").read().splitlines():
			tokens = line.split()
			if len(tokens) == 2:
				configCache[tokens[0]] = tokens[1]
	except Exception:
		pass
	return configCache


def SaveConfig():
	config = LoadConfig()
	try:
		old_open(CONFIG_FILE_NAME, "w").write("".join("%s %s\n" % (key, value) for (key, value) in sorted(config.items())))
	except Exception:
		dbg.TraceError("uiMobileHud: cannot write %s" % CONFIG_FILE_NAME)


def IsDynamicJoystick():
	return LoadConfig().get(CONFIG_DYNAMIC_JOYSTICK, "0") == "1"


def SetDynamicJoystick(enabled):
	LoadConfig()[CONFIG_DYNAMIC_JOYSTICK] = "1" if enabled else "0"
	SaveConfig()


# Mini haritanin altindaki "FPS: x" (uiminimap); varsayilan acik.
def IsShowFps():
	return LoadConfig().get(CONFIG_SHOW_FPS, "1") == "1"


def SetShowFps(enabled):
	LoadConfig()[CONFIG_SHOW_FPS] = "1" if enabled else "0"
	SaveConfig()


def GetChatHeight(default):
	try:
		return int(LoadConfig().get(CONFIG_CHAT_HEIGHT, default))
	except ValueError:
		return default


def SetChatHeight(height):
	LoadConfig()[CONFIG_CHAT_HEIGHT] = str(int(height))
	SaveConfig()


def GetSkillSlotCount():
	config = LoadConfig()
	if config.get(CONFIG_SKILL_LAYOUT) != "1":
		config[CONFIG_SKILL_LAYOUT] = "1"
		config[CONFIG_SKILL_SLOTS] = str(SKILL_SLOT_DEFAULT)
		SaveConfig()
	try:
		count = int(config.get(CONFIG_SKILL_SLOTS, SKILL_SLOT_DEFAULT))
	except ValueError:
		count = SKILL_SLOT_DEFAULT
	return max(SKILL_SLOT_DEFAULT, min(SKILL_SLOT_MAX, count))


def SetSkillSlotCount(count):
	LoadConfig()[CONFIG_SKILL_SLOTS] = str(max(SKILL_SLOT_DEFAULT, min(SKILL_SLOT_MAX, int(count))))
	SaveConfig()


def GetScreenFactor(screenHeight):
	# AKRAVON: telefonda mantiksal yukseklik ~600; dugmeler/slot halkasi 720'ye gore tasarlandi, oransal kucultulur (alt sinir 0.78)
	return max(0.78, screenHeight / 720.0)


def GetUIScale():
	function = getattr(wndMgr, "GetUIScale", None)
	if function is None:
		return 1.0
	try:
		scale = float(function())
	except Exception:
		return 1.0
	return scale if scale > 0.0 else 1.0


def GetSettingSize(getterName):
	function = getattr(systemSetting, getterName, None)
	if function is None:
		return 0
	try:
		size = float(function())
	except Exception:
		return 0
	if size <= 0.0:
		return 0
	return int(round(size / GetUIScale()))


def GetAttackSettingSize(drawnSize):
	size = int(drawnSize / ATTACK_GROWTH) + 1
	while size > 1 and int(size * ATTACK_GROWTH + 1e-6) > drawnSize:
		size -= 1
	return size


def PlaceOnRing(centerX, centerY, radius, angle, size):
	radians = math.radians(angle)
	x = int(round(centerX + radius * math.cos(radians) - size / 2.0))
	y = int(round(centerY + radius * math.sin(radians) - size / 2.0))
	return (x, y)


def UpdateSafeArea(screenWidth, screenHeight):
	# fiziksel piksel -> mantiksal piksel; en az 12 px kose payi (yuvarlak koseli ekranlar)
	global TOP_MARGIN, SCREEN_EDGE, SAFE_LEFT, SAFE_RIGHT, SAFE_TOP, SAFE_BOTTOM
	insets = (0, 0, 0, 0)
	physical = 0
	try:
		import systemSetting
		insets = systemSetting.GetSafeInsets()
		physical = systemSetting.GetDisplaySize()[1]
	except Exception:
		pass
	k = (screenHeight / float(physical)) if physical > 0 else 1.0
	SAFE_LEFT = int(round(insets[0] * k))
	SAFE_TOP = int(round(insets[1] * k))
	SAFE_RIGHT = int(round(insets[2] * k))
	SAFE_BOTTOM = int(round(insets[3] * k))
	SCREEN_EDGE = BASE_SCREEN_EDGE + max(SAFE_LEFT, SAFE_RIGHT, SAFE_BOTTOM)
	TOP_MARGIN = BASE_TOP_MARGIN + SAFE_TOP


# AKRAVON: kullanicinin tasidigi / boyutlandirdigi HUD ogeleri (merkez: ekranin binde'si, olcek: yuzde)
# Her oge bagimsizdir (PUBG Mobile gibi): joystick, saldiri dugmesi, topla / oto av / at; beceri slotlari ortak olcekli,
# konumlari SKILL_POSITION_i ile ayri ayri saklanir.
HUD_OVERRIDE_PREFIX = "HUD_"
UTILITY_NAMES = ("pickup", "autohunt", "mount")
HUD_KEYS = ("joystick", "attack", "page", "edit") + UTILITY_NAMES
editing = False
editSelection = None
hudController = None


def SetController(controller):
	global hudController
	hudController = controller


def GetController():
	return hudController


def IsEditing():
	return editing


def SetEditing(value):
	global editing
	editing = bool(value)


def GetEditSelection():
	return editSelection


def SetEditSelection(key):
	global editSelection
	editSelection = key


def GetHudOverride(name):
	value = LoadConfig().get(HUD_OVERRIDE_PREFIX + name)
	if not value:
		return None
	try:
		(x, y, scale) = [int(part) for part in value.split(",")]
	except ValueError:
		return None
	return (max(0, min(1000, x)), max(0, min(1000, y)), max(40, min(200, scale)))


def SetHudOverride(name, x, y, scale, save = True):
	LoadConfig()[HUD_OVERRIDE_PREFIX + name] = "%d,%d,%d" % (x, y, scale)
	if save:
		SaveConfig()


def ClearHudOverride(name, save = True):
	LoadConfig().pop(HUD_OVERRIDE_PREFIX + name, None)
	if save:
		SaveConfig()


def ClearHudLayout():
	# tum HUD konumlari / olcekleri (beceri slotlari dahil) varsayilana doner
	config = LoadConfig()
	for key in list(config.keys()):
		if key.startswith(HUD_OVERRIDE_PREFIX) or key.startswith("SKILL_POSITION_"):
			del config[key]
	SaveConfig()


def GetTouchLayout(screenWidth, screenHeight, useOverrides = True):
	UpdateSafeArea(screenWidth, screenHeight)
	factor = GetScreenFactor(screenHeight)

	def Override(name):
		return GetHudOverride(name) if useOverrides else None

	joystickOverride = Override("joystick")
	attackOverride = Override("attack")
	skillScale = 1.0
	attackScale = attackOverride[2] / 100.0 if attackOverride else 1.0

	joystickSize = GetSettingSize("GetJoystickSize") or int(round(JOYSTICK_DEFAULT_SIZE * factor))
	if joystickOverride:
		joystickSize = int(round(joystickSize * joystickOverride[2] / 100.0))
	attackBase = GetSettingSize("GetAttackButtonSize") or ATTACK_DEFAULT_SIZE * max(1.0, factor) * 1.07
	ringAttackSize = int(attackBase * ATTACK_GROWTH + 1e-6)	# beceri halkasi bu olcuye gore dizilir (saldiri dugmesi ayri olceklenir)
	attackSize = int(ringAttackSize * attackScale + 1e-6)

	ringFactor = max(1.0, factor) * 1.3	# beceri slotlari / halka telefonda kucuk kalmasin
	slotSize = int(round(SLOT_SIZE * ringFactor * skillScale))
	slotSizes = []
	for index in range(SKILL_SLOT_MAX):
		slotOverride = Override("skill%d" % index)
		slotSizes.append(int(round(slotSize * (slotOverride[2] / 100.0 if slotOverride else 1.0))))
	utilityBase = int(round(MENU_BUTTON_SIZE * max(factor, 1.0)))	# yardimci dugmeler diger HUD dugmeleriyle ayni boyda
	smallBase = int(round(utilityBase * 0.8))	# donder / ayar dugmeleri bir kademe kucuk
	pageOverride = Override("page")
	editOverride = Override("edit")
	pageSize = int(round(smallBase * PAGE_SCALE * (pageOverride[2] / 100.0 if pageOverride else 1.0)))
	editSize = int(round(smallBase * EDIT_SCALE * (editOverride[2] / 100.0 if editOverride else 1.0)))
	# AKRAVON: kenarlardan telefonda rahat tutulacak kadar uzak (ekran oraninda)
	leftMargin = max(int(round(LEFT_MARGIN * factor)), int(screenWidth * 0.10)) + SAFE_LEFT	# joystick biraz saga
	rightMargin = max(int(round(RIGHT_MARGIN * factor)), int(screenWidth * 0.075)) + int(SAFE_RIGHT * RIGHT_SAFE_SHARE)	# saldiri sag kenara yakin; beceriler onun solunda yay seklinde
	rightEdge = BASE_SCREEN_EDGE + int(SAFE_RIGHT * RIGHT_SAFE_SHARE)	# sag sutun / saldiri icin kenar payi
	bottomMargin = max(int(round(BOTTOM_MARGIN * factor)), int(screenHeight * 0.11)) + SAFE_BOTTOM	# joystick ve saldiri biraz yukarida

	joystickX = leftMargin
	joystickY = screenHeight - bottomMargin - joystickSize
	if joystickOverride:
		joystickX = int(round(joystickOverride[0] * screenWidth / 1000.0 - joystickSize / 2.0))
		joystickY = int(round(joystickOverride[1] * screenHeight / 1000.0 - joystickSize / 2.0))
		joystickX = max(SCREEN_EDGE, min(screenWidth - SCREEN_EDGE - joystickSize, joystickX))
		joystickY = max(SCREEN_EDGE, min(screenHeight - SCREEN_EDGE - joystickSize, joystickY))

	# varsayilan saldiri merkezi = beceri halkasinin merkezi
	ringCenterX = screenWidth - rightMargin - ringAttackSize / 2.0
	ringCenterY = screenHeight - bottomMargin - ringAttackSize / 2.0
	# ic halka saldirinin gorunen kenarina yakin (gorsel ~%80), slotlar birbirine ve plakalara degmez
	innerRadius = max(ringAttackSize * 0.4 + slotSize / 2.0 + 4, slotSize * INNER_RADIUS_RATIO)
	outerRadius = max(innerRadius + slotSize * 0.9, slotSize * OUTER_RADIUS_RATIO)
	ringSpots = ([(innerRadius, a) for a in RING_ANGLES] + [(outerRadius, a) for a in OUTER_RING_ANGLES])[:RING_LAYOUT_SPOTS]
	rightExtent = max([r * math.cos(math.radians(a)) for (r, a) in ringSpots]) + slotSize / 2.0
	bottomExtent = max([r * math.sin(math.radians(a)) for (r, a) in ringSpots]) + slotSize * 0.61
	ringCenterX = min(ringCenterX, screenWidth - rightEdge - rightExtent)
	# AKRAVON: yay + saldiri asagida; en alttaki slotun P plakasi ve saldiri ekrandan tasmaz.
	# SCREEN_EDGE yan centigi da icerir: dikeyde yalniz taban pay + alt guvenli alan (cift sayilinca yay yukarida kaliyordu)
	bottomEdge = BASE_SCREEN_EDGE + SAFE_BOTTOM
	bottomExtent = max(bottomExtent, ringAttackSize * 0.45)
	ringCenterY = min(ringCenterY + int(screenHeight * RING_DROP_RATIO), screenHeight - bottomEdge - bottomExtent)

	centerX = ringCenterX
	centerY = ringCenterY
	if attackOverride:
		centerX = attackOverride[0] * screenWidth / 1000.0
		centerY = attackOverride[1] * screenHeight / 1000.0
		centerX = max(SCREEN_EDGE + attackSize / 2.0, min(screenWidth - SCREEN_EDGE - attackSize / 2.0, centerX))
		centerY = max(SCREEN_EDGE + attackSize / 2.0, min(screenHeight - SCREEN_EDGE - attackSize / 2.0, centerY))
	attackX = int(round(centerX - attackSize / 2.0))
	attackY = int(round(centerY - attackSize / 2.0))

	slotPositions = [PlaceOnRing(ringCenterX, ringCenterY, innerRadius, angle, slotSize) for angle in RING_ANGLES]
	slotPositions += [PlaceOnRing(ringCenterX, ringCenterY, outerRadius, angle, slotSize) for angle in OUTER_RING_ANGLES]

	# AKRAVON: sag kenardaki yardimci dugmeler (topla / oto av / at); her biri ayri tasinir ve boyutlanir
	utilityTop = int(MINIMAP_RESERVED_HEIGHT * min(1.0, factor + 0.1))
	utilities = []
	utilitySizes = {}
	nextCenterY = screenHeight * UTILITY_FIRST_CENTER_Y
	for name in UTILITY_NAMES:
		override = Override(name)
		size = int(round(utilityBase * UTILITY_SCALE * (override[2] / 100.0 if override else 1.0)))
		utilitySizes[name] = size
		position = (int(round(screenWidth - rightEdge - UTILITY_RIGHT_INSET - size / 2.0)), int(round(nextCenterY - size / 2.0)))
		position = (min(screenWidth - rightEdge - size, position[0]), position[1])
		nextCenterY += size + UTILITY_GAP
		if override:
			position = (int(round(override[0] * screenWidth / 1000.0 - size / 2.0)), int(round(override[1] * screenHeight / 1000.0 - size / 2.0)))
			position = (max(SCREEN_EDGE, min(screenWidth - rightEdge - size, position[0])), max(SCREEN_EDGE, min(screenHeight - SCREEN_EDGE - size, position[1])))
		utilities.append(position)

	# AKRAVON: dondur / ayar sag sutunda, topla / oto av / at dugmelerinin altinda ayni hizada (saldiriya binmez)
	pagePosition = (int(round(screenWidth - rightEdge - PAGE_RIGHT_INSET - pageSize / 2.0)), int(round(screenHeight * PAGE_CENTER_Y - pageSize / 2.0)))
	pagePosition = (max(SCREEN_EDGE, min(screenWidth - rightEdge - pageSize, pagePosition[0])), max(SCREEN_EDGE, min(screenHeight - SCREEN_EDGE - pageSize, pagePosition[1])))	# ekran disina tasmasin
	if pageOverride:
		pagePosition = (int(round(pageOverride[0] * screenWidth / 1000.0 - pageSize / 2.0)), int(round(pageOverride[1] * screenHeight / 1000.0 - pageSize / 2.0)))
		pagePosition = (max(SCREEN_EDGE, min(screenWidth - rightEdge - pageSize, pagePosition[0])), max(SCREEN_EDGE, min(screenHeight - SCREEN_EDGE - pageSize, pagePosition[1])))
	editPosition = (int(round(screenWidth - rightEdge - EDIT_RIGHT_INSET - editSize / 2.0)), int(round(screenHeight * EDIT_CENTER_Y - editSize / 2.0)))
	editPosition = (max(SCREEN_EDGE, min(screenWidth - rightEdge - editSize, editPosition[0])), max(SCREEN_EDGE, min(screenHeight - SCREEN_EDGE - editSize, editPosition[1])))
	if editOverride:
		editPosition = (int(round(editOverride[0] * screenWidth / 1000.0 - editSize / 2.0)), int(round(editOverride[1] * screenHeight / 1000.0 - editSize / 2.0)))
		editPosition = (max(SCREEN_EDGE, min(screenWidth - rightEdge - editSize, editPosition[0])), max(SCREEN_EDGE, min(screenHeight - SCREEN_EDGE - editSize, editPosition[1])))

	affectY = TOP_MARGIN + STATUS_BAR_HEIGHT + AFFECT_GAP
	notificationY = affectY + AFFECT_ROW_HEIGHT + NOTIFICATION_GAP
	menuWidth = MENU_BUTTON_COUNT * MENU_BUTTON_SIZE + (MENU_BUTTON_COUNT - 1) * MENU_BUTTON_GAP

	pillWidth = CHAT_PILL_WIDTH
	pillX = int((screenWidth - pillWidth) / 2)
	pillY = screenHeight - CHAT_PILL_HEIGHT	# ekranin dibine yapisik

	zoneY = notificationY + NOTIFICATION_HEIGHT + NOTIFICATION_GAP

	return {
		"joystick" : (joystickX, joystickY, joystickSize),
		"attack" : (attackX, attackY, attackSize),
		"slots" : slotPositions,
		"ringCenter" : (ringCenterX, ringCenterY),
		"slotSize" : slotSize,
		"slotSizes" : slotSizes,
		"pageButton" : (pagePosition, pageSize),
		"editButton" : (editPosition, editSize),
		"utilities" : (utilities, utilityBase),
		"utilitySizes" : utilitySizes,
		"runLock" : (RUN_LOCK_SIZE, RUN_LOCK_GAP),
		"statusBar" : (SCREEN_EDGE, TOP_MARGIN),
		"notificationY" : notificationY,
		"menuBar" : (screenWidth - int(SAFE_RIGHT * RIGHT_SAFE_SHARE) - MINIMAP_RESERVED_WIDTH - WINDOW_GAP - menuWidth - 8, TOP_MARGIN + 18, MENU_BUTTON_SIZE, MENU_BUTTON_GAP),
		"eventRow" : (screenWidth - int(SAFE_RIGHT * RIGHT_SAFE_SHARE) - MINIMAP_RESERVED_WIDTH - WINDOW_GAP, TOP_MARGIN + (MENU_BUTTON_SIZE + MENU_BUTTON_GAP) * 3),
		"affectToggle" : (SCREEN_EDGE + STATUS_BAR_WIDTH + WINDOW_GAP, TOP_MARGIN + 4),
		"affectShower" : (SCREEN_EDGE, affectY),
		"chatPill" : (pillX, pillY, pillWidth, CHAT_PILL_HEIGHT),
		"joystickZone" : (0, zoneY, max(joystickX + joystickSize, pillX - WINDOW_GAP), screenHeight - zoneY),
	}


class SkillSlot(ui.Window):
	def __init__(self, slotIndex, size, taskBar):
		ui.Window.__init__(self, "UI")

		self.slotIndex = slotIndex
		self.taskBar = taskBar
		self.selectSkill = None
		self.contentKey = None
		self.pageTransition = None
		self.transitionIconName = None
		self.SetSize(size, size)
		self.SetWindowName("MobileSkillSlot%d" % slotIndex)
		self.AddFlag("float")	# SkillArc.SetTop: C paneli acikken onun ustunde
		ui.EnableSingleTap(self)

		# yuvarlak ikon diskin duz ic alanina gore buyur (ic 56 -> 40, dis 48 -> 36); birim API'si yoksa 32
		iconSize = SLOT_ICON_SIZE
		if hasattr(wndMgr, "SetSlotUnitSize"):
			iconSize = max(SLOT_ICON_SIZE, int(size * ROUND_ICON_RATIO) // 2 * 2)
		iconOffset = (size - iconSize) // 2

		self.disc = self.__CreateImage(DISC_TEXTURE, size)

		self.slot = ui.GridSlotWindow()
		self.slot.SetParent(self)
		self.slot.AddFlag("not_pick")
		self.slot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
		self.slot.ArrangeSlot(slotIndex, 1, 1, iconSize, iconSize, 0, 0)
		if iconSize != SLOT_ICON_SIZE:
			wndMgr.SetSlotUnitSize(self.slot.hWnd, iconSize, iconSize, 1)
		# telefonda beceri ikonlari yuvarlak tasarimli gelir (C++ skill_round): sogutma / etkin halka da daire. PC kare.
		if uiMobileCompat.IsMobile() and hasattr(wndMgr, "SetSlotRoundIcons"):
			wndMgr.SetSlotRoundIcons(self.slot.hWnd, 1)
		self.slot.SetPosition(iconOffset, iconOffset)
		self.slot.Show()

		maskPx = max(40, min(160, int(round(size * GetUIScale() / 2.0)) * 2))
		self.iconMask = self.__CreateImage("mobile_ui/akravon/hud/hud_icon_mask_%d.png" % maskPx, size, maskPx)
		if self.iconMask is None:
			self.iconMask = self.__CreateImage(ICON_MASK_TEXTURE, size)

		# HD yuvarlak beceri ikonu: oyunun kendi ikonu 2x yumusak buyutulmus + daire (skin/build_akravon_skillicons.py)
		self.hdSize = int(size * HD_ICON_RATIO) // 2 * 2
		self.hdIcon = ui.ExpandedImageBox()
		self.hdIcon.SetParent(self)
		self.hdIcon.AddFlag("not_pick")
		self.hdIcon.SetPosition((size - self.hdSize) // 2, (size - self.hdSize) // 2)
		self.hdIcon.Hide()
		self.hdIconName = None
		# AKRAVON: kullanimdaki esya (iksir) gostergesi - maske ve ikonun ustunde mavi dalgalanan parilti (yalniz esya slotu)
		self.itemActive = False
		self.slotSize = size
		self.glowSize = int(size * 0.86) // 2 * 2
		self.activeGlow = self.__CreateImage(GLOW_TEXTURE, self.glowSize, 128)
		if self.activeGlow:
			self.activeGlow.SetPosition((size - self.glowSize) // 2, (size - self.glowSize) // 2)
			self.activeGlow.Hide()
		# yuvarlak dalga seritleri (daire icinde, envanterdeki aktif slotla ayni dalga)
		self.ripples = []
		radius = int(size * 0.43)
		strips = 44
		stripH = max(2, (2 * radius) // strips)
		for k in range(strips):
			yy = -radius + k * stripH + stripH / 2.0
			half = int(math.sqrt(max(0.0, radius * radius - yy * yy)))
			if half <= 0:
				continue
			bar = ui.Bar()
			bar.SetParent(self)
			bar.AddFlag("not_pick")
			bar.SetPosition(size // 2 - half, int(size // 2 + yy - stripH / 2.0))
			bar.SetSize(2 * half, stripH)
			bar.SetColor(0x00000000)
			bar.Hide()
			self.ripples.append((k, bar))
		self.ring = self.__CreateImage(RING_TEXTURE, size)
		# AKRAVON: basma efekti - disa yayilan altin parilti (ikonun ustunde)
		self.pressGlow = self.__CreateImage(GLOW_TEXTURE, size, 128)
		if self.pressGlow:
			self.pressGlow.Hide()

		# dolan daire (mobile_ui/akravon/cool/cool_<px>_<kare>.png, skin/build_akravon_skillicons.py)
		self.coolPx = max(44, min(100, int(round(self.hdSize * GetUIScale() / 4.0)) * 4))
		self.coolFrame = -1
		self.coolImage = ui.ExpandedImageBox()
		self.coolImage.SetParent(self)
		self.coolImage.AddFlag("not_pick")
		self.coolImage.SetPosition((size - self.hdSize) // 2, (size - self.hdSize) // 2)
		self.coolImage.Hide()

		self.cooldownShadow = ui.TextLine()
		self.cooldownShadow.SetParent(self)
		self.cooldownShadow.AddFlag("not_pick")
		try:
			self.cooldownShadow.SetFontName(CoolFont(size))
		except Exception:
			pass
		self.cooldownShadow.SetPackedFontColor(COOL_SHADOW)
		self.cooldownShadow.SetHorizontalAlignCenter()
		self.cooldownShadow.SetPosition(size // 2 + 1, size // 2 - CoolHalf(size) + 2)
		self.cooldownShadow.Hide()

		self.cooldownText = ui.TextLine()
		self.cooldownText.SetParent(self)
		self.cooldownText.AddFlag("not_pick")
		try:
			self.cooldownText.SetFontName(CoolFont(size))
		except Exception:
			pass
		self.cooldownText.SetOutline()
		self.cooldownText.SetPackedFontColor(COOL_COLOR)
		self.cooldownText.SetHorizontalAlignCenter()
		self.cooldownText.SetPosition(size // 2, size // 2 - CoolHalf(size))
		self.cooldownText.Hide()
		self.cooldownValue = ""
		self.hasCooldownQuery = hasattr(self.slot, "GetSlotCoolTime")
		self.gradeBadge = uiAkravonSkillGrade.SkillGradeBadge(self, size)
		self.transitionIcon = ui.ExpandedImageBox()
		self.transitionIcon.SetParent(self)
		self.transitionIcon.AddFlag("not_pick")
		self.transitionIcon.SetPosition((size - self.hdSize) // 2, (size - self.hdSize) // 2)
		self.transitionIcon.Hide()

		self.SetTouchTapEvent(ui.__mem_func__(self.__OnTap))
		self.SetTouchLongPressEvent(ui.__mem_func__(self.__OnLongPress))

	def __del__(self):
		ui.Window.__del__(self)

	def __CreateImage(self, fileName, size, textureSize = SLOT_TEXTURE_SIZE):
		try:
			image = ui.ExpandedImageBox()
			image.SetParent(self)
			image.AddFlag("not_pick")
			image.LoadImage(fileName)
			image.SetScale(float(size) / textureSize, float(size) / textureSize)
			image.SetPosition(0, 0)
			image.Show()
			return image
		except Exception:
			return None

	def Destroy(self):
		self.selectSkill = None
		self.taskBar = None
		self.eventTouchTap = None
		self.eventTouchLongPress = None
		self.disc = None
		self.slot = None
		self.ring = None
		self.activeGlow = None
		self.pressGlow = None
		for image in getattr(self, "pressRings", []) + getattr(self, "pressDots", []):
			image.Hide()
		self.pressRings = []
		self.pressDots = []
		self.ripples = []
		self.iconMask = None
		self.hdIcon = None
		self.cooldownText = None
		self.cooldownShadow = None
		self.coolImage = None
		self.gradeBadge = None
		self.transitionIcon = None
		self.Hide()

	def __CallTaskBar(self, name, *args):
		if self.taskBar is None:
			return None
		try:
			return getattr(self.taskBar, name)(*args)
		except ReferenceError:
			return None

	def Flash(self):
		# AKRAVON: basildi efekti - ikon icine coker ve kararir, halka altin yanar, altin parilti disa yayilip soner
		now = app.GetTime()
		self.flashStart = now
		self.flashUntil = now + PRESS_EFFECT_SECONDS
		if self.ring:
			wndMgr.SetDiffuseColor(self.ring.hWnd, 1.0, 0.85, 0.4, 1.0)
		self.__ApplyPress(0.0)

	def __PressImage(self, fileName):
		image = ui.ExpandedImageBox()
		image.AddFlag("not_pick")
		image.AddFlag("float")	# ust katman: slotun disina tasan dalga kirpilmaz
		image.LoadImage(fileName)
		image.Hide()
		return image

	def __ApplyRipple(self, progress):
		if not hasattr(self, "pressRings"):
			try:
				self.pressRings = [self.__PressImage(PRESS_RING_TEXTURE) for i in range(PRESS_RING_COUNT)]
				self.pressDots = [self.__PressImage(PRESS_DOT_TEXTURE) for i in range(PRESS_DOT_COUNT)]
			except Exception:
				self.pressRings = []
				self.pressDots = []
		size = self.slotSize
		(gx, gy) = self.GetGlobalPosition()
		cx = gx + size / 2.0
		cy = gy + size / 2.0
		visible = progress < 1.0 and self.IsShow()
		for (i, ring) in enumerate(self.pressRings):
			p = (progress - i * 0.18) / 0.82
			if not visible or p <= 0.0 or p >= 1.0:
				ring.Hide()
				continue
			drawn = size * (0.92 + 1.05 * p)
			ring.SetScale(drawn / 256.0, drawn / 256.0)
			ring.SetPosition(int(round(cx - drawn / 2.0)), int(round(cy - drawn / 2.0)))
			wndMgr.SetDiffuseColor(ring.hWnd, 1.0, 0.9, 0.6, 1.0 - p * 0.85)
			ring.Show()
			ring.SetTop()
		for (k, dot) in enumerate(self.pressDots):
			if not visible:
				dot.Hide()
				continue
			angle = math.radians(k * (360.0 / max(1, PRESS_DOT_COUNT)) + 20.0 * (k % 2))
			distance = size * (0.42 + (0.55 + 0.12 * (k % 3)) * progress)
			dotSize = size * (0.21 - 0.09 * progress) * (0.8 + 0.2 * (k % 2))
			x = cx + math.cos(angle) * distance - dotSize / 2.0
			y = cy + math.sin(angle) * distance - dotSize / 2.0 - size * 0.12 * progress	# kabarcik hafif yukari suzulur
			dot.SetScale(dotSize / 64.0, dotSize / 64.0)
			dot.SetPosition(int(round(x)), int(round(y)))
			wndMgr.SetDiffuseColor(dot.hWnd, 1.0, 0.92, 0.7, 1.0 - progress * 0.9)
			dot.Show()
			dot.SetTop()

	def __ApplyPress(self, progress):
		self.__ApplyRipple(progress)
		size = self.slotSize
		wave = math.sin(math.pi * min(1.0, progress * 2.0))
		if self.hdIcon and self.hdIcon.IsShow():
			factor = 1.0 - 0.17 * wave
			drawn = self.hdSize * factor
			self.hdIcon.SetScale(drawn / HD_ICON_TEXTURE_SIZE, drawn / HD_ICON_TEXTURE_SIZE)
			self.hdIcon.SetPosition(int(round((size - drawn) / 2.0)), int(round((size - drawn) / 2.0)))
			shade = 1.0 - 0.3 * wave
			wndMgr.SetDiffuseColor(self.hdIcon.hWnd, shade, shade, shade, 1.0)
		glow = getattr(self, "pressGlow", None)
		if glow:
			if progress >= 1.0:
				glow.Hide()
			else:
				glowSize = size * (0.7 + 0.3 * min(1.0, progress * 2.0))
				glow.SetScale(glowSize / 128.0, glowSize / 128.0)
				glow.SetPosition(int(round((size - glowSize) / 2.0)), int(round((size - glowSize) / 2.0)))
				wndMgr.SetDiffuseColor(glow.hWnd, 1.0, 0.86, 0.45, max(0.0, 1.0 - progress * 2.0))
				glow.Show()
				glow.SetTop()

	def UpdateFlash(self):
		until = getattr(self, "flashUntil", 0.0)
		if not until:
			return
		now = app.GetTime()
		if now >= until:
			self.flashUntil = 0.0
			self.__ApplyPress(1.0)
			if self.hdIcon:
				self.hdIcon.SetScale(self.hdSize / HD_ICON_TEXTURE_SIZE, self.hdSize / HD_ICON_TEXTURE_SIZE)
				self.hdIcon.SetPosition((self.slotSize - self.hdSize) // 2, (self.slotSize - self.hdSize) // 2)
				wndMgr.SetDiffuseColor(self.hdIcon.hWnd, 1.0, 1.0, 1.0, 1.0)
			if self.ring:
				wndMgr.SetDiffuseColor(self.ring.hWnd, 1.0, 1.0, 1.0, 1.0)
			return
		self.__ApplyPress((now - self.flashStart) / PRESS_EFFECT_SECONDS)

	def __Use(self):
		if self.pageTransition is not None:
			return
		self.Flash()
		if player.GetLocalQuickSlot(self.slotIndex)[0] == player.SLOT_TYPE_NONE:
			if self.selectSkill:
				self.selectSkill(self.slotIndex)
			return
		player.RequestUseLocalQuickSlot(self.slotIndex)

	def __Take(self):
		if self.pageTransition is not None:
			return
		(slotType, slotPosition) = player.GetLocalQuickSlot(self.slotIndex)
		if player.SLOT_TYPE_NONE == slotType:
			return
		self.__CallTaskBar("SelectItemQuickSlot", self.slotIndex)

	def __Drop(self):
		if self.pageTransition is not None:
			return
		controller = mouseModule.mouseController
		if player.SLOT_TYPE_QUICK_SLOT == controller.GetAttachedType() \
				and controller.GetAttachedSlotNumber() == player.LocalQuickSlotIndexToGlobalQuickSlotIndex(self.slotIndex):
			controller.DeattachObject()
			return
		self.__CallTaskBar("AddQuickSlot", self.slotIndex)

	def __OnTap(self, x, y):
		if mouseModule.mouseController.isAttached():
			self.__Drop()
		else:
			self.__Use()

	def __OnLongPress(self, x, y):
		if not mouseModule.mouseController.isAttached():
			self.__Take()

	def OnMouseLeftButtonDown(self):
		if mouseModule.mouseController.isAttached():
			self.__Drop()
		elif app.IsPressed(app.DIK_LSHIFT) or app.IsPressed(app.DIK_RSHIFT):
			self.__Take()
		else:
			self.__Use()
		return True

	def OnMouseLeftButtonUp(self):
		if mouseModule.mouseController.isAttached() and wndMgr.IsDragging():
			if self.IsIn():
				self.__Drop()
				return True
			return False
		return True

	def OnMouseRightButtonDown(self):
		if mouseModule.mouseController.isAttached():
			mouseModule.mouseController.DeattachObject()
		else:
			self.__Use()
		return True

	def OnMouseOverIn(self):
		if not uiMobileCompat.IsMobile():
			self.__CallTaskBar("OverInItem", self.slotIndex)

	def OnMouseOverOut(self):
		self.__CallTaskBar("OverOutItem")

	def RefreshContent(self):
		index = self.slotIndex
		slot = self.slot
		(slotType, position) = player.GetLocalQuickSlot(index)
		hdName = None
		self.itemActive = False
		self.gradeBadge.Hide()
		contentKey = (slotType, position)
		if self.contentKey != contentKey:
			slot.SetSlotCoolTime(index, 0)
			slot.DeactivateSlot(index)
			self.contentKey = contentKey

		if player.SLOT_TYPE_NONE == slotType:
			slot.ClearSlot(index)

		elif player.SLOT_TYPE_INVENTORY == slotType:
			itemIndex = player.GetItemIndex(position)
			itemCount = player.GetItemCount(position)
			slot.DeactivateSlot(index)

			if constInfo.IS_AUTO_POTION(itemIndex):
				sockets = [player.GetItemMetinSocket(position, socket) for socket in range(player.METIN_SOCKET_MAX_NUM)]
				if 0 != int(sockets[0]):
					self.itemActive = True	# tek gosterge: maskenin ustundeki mavi parilti (motor halkasi kullanilmaz)
				else:
					slot.DeactivateSlot(index)

			if itemIndex and itemCount > 0:
				slot.SetItemSlot(index, itemIndex, 0)
				self.gradeBadge.RefreshCount(itemCount)
			else:
				slot.ClearSlot(index)

		elif player.SLOT_TYPE_SKILL == slotType:
			skillIndex = player.GetSkillIndex(position)
			if 0 == skillIndex:
				slot.ClearSlot(index)
			else:
				if skill.SKILL_TYPE_GUILD == skill.GetSkillType(skillIndex):
					import guild
					skillGrade = 0
					skillLevel = guild.GetSkillLevel(position)
				else:
					skillGrade = player.GetSkillGrade(position)
					skillLevel = player.GetSkillLevel(position)

				slot.SetSkillSlotNew(index, skillIndex, skillGrade, skillLevel)
				hdName = uiAkravonSkillIcon.GetIcon(skillIndex, skillGrade) if uiAkravonSkillIcon else None
				slot.SetSlotCount(index, 0)
				self.gradeBadge.Refresh(skillGrade, skillLevel)

				if player.IsSkillCoolTime(position):
					(coolTime, elapsedTime) = player.GetSkillCoolTime(position)
					slot.SetSlotCoolTime(index, coolTime, elapsedTime)

				if player.IsSkillActive(position):
					slot.ActivateSlot(index)
				else:
					slot.DeactivateSlot(index)

		elif player.SLOT_TYPE_EMOTION == slotType:
			import uiAkravonActionIcon
			slot.SetEmotionSlot(index, position)
			slot.SetSlotCount(index, 0)
			hdName = uiAkravonActionIcon.GetEmotionIcon(position)

		slot.RefreshSlot()
		self.__SetHDIcon(hdName)
		if self.pageTransition is not None:
			self.SetPageTransition(*self.pageTransition)

	def SetPageTransition(self, angle, alpha):
		self.pageTransition = (angle, alpha)
		name = self.hdIconName
		slotType, position = player.GetLocalQuickSlot(self.slotIndex)
		if slotType == player.SLOT_TYPE_INVENTORY:
			vnum = player.GetItemIndex(position)
			if vnum:
				item.SelectItem(vnum)
				name = item.GetIconImageFileName()
		if name:
			if name != self.transitionIconName:
				self.transitionIcon.LoadImage(name)
				self.transitionIcon.SetScale(float(self.hdSize) / self.transitionIcon.GetWidth(),
					float(self.hdSize) / self.transitionIcon.GetHeight())
				self.transitionIcon.SetOrigin(self.hdSize / 2.0, self.hdSize / 2.0)
				self.transitionIconName = name
			self.transitionIcon.SetRotation(0)	# donerken ekran ortasinda yabanci simge belirmesin: yalniz solma
			wndMgr.SetDiffuseColor(self.transitionIcon.hWnd, 1.0, 1.0, 1.0, alpha)
			self.transitionIcon.Show()
		else:
			self.transitionIcon.Hide()
		for widget in (self.slot, self.hdIcon, self.gradeBadge, self.coolImage,
				self.cooldownText, self.cooldownShadow):
			widget.Hide()

	def EndPageTransition(self):
		self.pageTransition = None
		self.transitionIcon.Hide()
		self.transitionIconName = None
		self.slot.Show()
		self.RefreshContent()

	def __SetHDIcon(self, name):
		if not self.hdIcon:
			return
		if not name:
			self.hdIcon.Hide()
			self.hdIconName = None
			return
		if name != self.hdIconName:
			try:
				self.hdIcon.LoadImage(name)
				self.hdIcon.SetScale(self.hdSize / HD_ICON_TEXTURE_SIZE, self.hdSize / HD_ICON_TEXTURE_SIZE)
			except Exception:
				self.hdIcon.Hide()
				self.hdIconName = None
				return
			self.hdIconName = name
		self.hdIcon.Show()

	def __UpdateCoolCircle(self, fraction):
		if not self.coolImage:
			return
		frame = int(math.ceil(min(1.0, fraction) * COOL_FRAMES)) if fraction > 0.0 else 0
		if frame == self.coolFrame:
			return
		self.coolFrame = frame
		if frame <= 0:
			self.coolImage.Hide()
			return
		try:
			self.coolImage.LoadImage("mobile_ui/akravon/cool/cool_%d_%02d.png" % (self.coolPx, frame))
			scale = float(self.hdSize) / self.coolPx
			self.coolImage.SetScale(scale, scale)
			self.coolImage.Show()
		except Exception:
			self.coolImage.Hide()

	def IsSkillSlot(self, skillSlotIndex):
		(slotType, position) = player.GetLocalQuickSlot(self.slotIndex)
		return player.SLOT_TYPE_SKILL == slotType and skillSlotIndex == position

	def __UpdateActiveGlow(self):
		glow = self.activeGlow
		if not glow:
			return
		if not self.itemActive or self.pageTransition is not None:
			if glow.IsShow():
				glow.Hide()
			for (k, bar) in self.ripples:
				if bar.IsShow():
					bar.Hide()
			return
		t = app.GetTime()
		# sabit hafif taban + disa dogru yayilan 3 dalga halkasi (elektrik dalgalanmasi)
		if glow.IsShow():
			glow.Hide()
		for (k, bar) in self.ripples:
			wave = 0.5 + 0.5 * math.sin(t * 4.0 - k * 0.28)
			wave2 = 0.5 + 0.5 * math.sin(t * 2.3 + k * 0.15 + 1.3)
			a = int(255 * (0.16 + 0.26 * (0.6 * wave + 0.4 * wave2)))
			bar.SetColor((a << 24) | (115 << 16) | (184 << 8) | 255)
			if not bar.IsShow():
				bar.Show()

	def OnUpdate(self):
		self.__UpdateActiveGlow()
		if self.pageTransition is not None:
			return
		if not self.hasCooldownQuery or not self.cooldownText:
			return

		remaining = 0.0
		coolTime = 0.0
		try:
			(coolTime, elapsedTime) = self.slot.GetSlotCoolTime(self.slotIndex)
			if coolTime > 0.0:
				remaining = coolTime - elapsedTime
		except Exception:
			remaining = 0.0
		self.__UpdateCoolCircle(remaining / coolTime if remaining > 0.05 and coolTime > 0.0 else 0.0)

		if remaining > 0.05:
			text = "%.1f" % remaining	# AKRAVON: saniye.salise (38.4)
			if text != self.cooldownValue:
				self.cooldownValue = text
				self.cooldownText.SetText(text)
				self.cooldownShadow.SetText(text)
			if not self.cooldownText.IsShow():
				self.cooldownShadow.Show()
				self.cooldownText.Show()
		elif self.cooldownText.IsShow():
			self.cooldownText.Hide()
			self.cooldownShadow.Hide()
			self.cooldownValue = ""


class QuickPageButton(ui.Window):
	def __init__(self, size, arc):
		ui.Window.__init__(self, "UI")
		self.SetSize(size, size)
		self.SetWindowName("MobileQuickPageButton")
		self.AddFlag("float")
		self.arc = arc
		self.transitionStart = None
		self.transitionSwitched = False
		ui.EnableSingleTap(self)

		self.faceScale = size / PAGE_TEXTURE_SIZE
		self.face = ui.ExpandedImageBox()
		self.face.SetParent(self)
		self.face.AddFlag("not_pick")
		self.face.LoadImage("mobile_ui/akravon/hud/hud_skill_pages.png")
		self.face.SetScale(self.faceScale, self.faceScale)
		self.face.SetPosition(0, 0)
		self.face.SetOrigin(size / 2.0, size / 2.0)
		self.face.Show()

		self.pageIndex = -1
		self.SetTouchTapEvent(ui.__mem_func__(self.__OnTap))
		self.Refresh()

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		self.eventTouchTap = None
		self.arc = None
		self.face = None
		self.Hide()

	def __NextPage(self):
		if self.transitionStart is not None or self.arc.editor.editing:
			return
		self.arc.picker.Hide()
		self.transitionStart = app.GetTime()
		self.transitionSwitched = False
		self.targetPage = (player.GetQuickPage() + 1) % MOBILE_QUICK_PAGE_COUNT
		self.basePositions = [slot.GetLocalPosition() for slot in self.arc.slots]
		self.OnUpdate()

	def __RotateArc(self, degrees):
		# gorunen slotlar saldiri merkezinin etrafinda birlikte doner (kullanici tasidiysa kendi konumlari etrafinda)
		center = getattr(self.arc, "ringCenter", None)
		positions = getattr(self, "basePositions", None)
		if not center or not positions:
			return
		(cx, cy) = center
		radians = math.radians(degrees)
		(cosA, sinA) = (math.cos(radians), math.sin(radians))
		for (index, slot) in enumerate(self.arc.slots):
			if index >= len(positions):
				break
			size = slot.GetWidth()
			(x, y) = positions[index]
			dx = x + size / 2.0 - cx
			dy = y + size / 2.0 - cy
			nx = cx + dx * cosA - dy * sinA - size / 2.0
			ny = cy + dx * sinA + dy * cosA - size / 2.0
			slot.SetPosition(int(round(nx)), int(round(ny)))

	def OnUpdate(self):
		if self.transitionStart is None:
			return
		progress = min(1.0, max(0.0, (app.GetTime() - self.transitionStart) / PAGE_TRANSITION_SECONDS))
		if progress >= 0.5 and not self.transitionSwitched:
			self.transitionSwitched = True
			player.SetQuickPage(self.targetPage)
			self.arc.RefreshQuickSlot()
		self.face.SetRotation(progress * 360.0)
		if progress >= 1.0:
			self.CancelTransition()
			return
		# ilk yari: yay saat yonunde doner ve gider; yarida yeni sayfa; ikinci yari: diger yandan gelip yerine oturur
		if progress < 0.5:
			phase = progress / 0.5
			degrees = PAGE_ROTATE_DEGREES * phase * phase
		else:
			phase = (progress - 0.5) / 0.5
			degrees = -PAGE_ROTATE_DEGREES * (1.0 - phase) * (1.0 - phase)
		self.__RotateArc(degrees)

	def CancelTransition(self):
		if self.transitionStart is None:
			return
		self.transitionStart = None
		self.face.SetRotation(0)
		self.__RotateArc(0.0)
		self.basePositions = None
		for slot in self.arc.slots:
			slot.EndPageTransition()
		self.Refresh()

	def __OnTap(self, x, y):
		self.__NextPage()

	def OnMouseLeftButtonDown(self):
		self.__NextPage()
		return True

	def Refresh(self):
		pageIndex = player.GetQuickPage()
		if pageIndex == self.pageIndex:
			return
		self.pageIndex = pageIndex


class PlusSlot(ui.Window):
	# AKRAVON: sonraki bos beceri yuvasinin yerinde durur; dokun = yuva ekle, uzun bas = son yuvayi kaldir
	def __init__(self, size, onAdd, onRemove):
		ui.Window.__init__(self, "UI")
		self.SetSize(size, size)
		self.SetWindowName("MobileSkillPlus")
		self.AddFlag("float")
		ui.EnableSingleTap(self)
		self.onAdd = onAdd
		self.onRemove = onRemove

		self.face = ui.ExpandedImageBox()
		self.face.SetParent(self)
		self.face.AddFlag("not_pick")
		self.face.LoadImage(PLUS_TEXTURE)
		self.face.SetScale(size / SLOT_TEXTURE_SIZE, size / SLOT_TEXTURE_SIZE)
		self.face.Show()
		wndMgr.SetDiffuseColor(self.face.hWnd, 1.0, 1.0, 1.0, 0.8)

		self.SetTouchTapEvent(ui.__mem_func__(self.__OnTap))
		self.SetTouchLongPressEvent(ui.__mem_func__(self.__OnLongPress))

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		self.eventTouchTap = None
		self.eventTouchLongPress = None
		self.onAdd = None
		self.onRemove = None
		self.face = None
		self.Hide()

	def __OnTap(self, x, y):
		if self.onAdd:
			self.onAdd()

	def __OnLongPress(self, x, y):
		if self.onRemove:
			self.onRemove()

	def OnMouseLeftButtonDown(self):
		if self.onAdd:
			self.onAdd()
		return True


class SkillArc(object):
	def __init__(self, taskBar, layout):
		self.slots = []
		self.shown = False
		self.positions = layout["slots"]
		self.defaultPositions = list(self.positions)
		self.ringCenter = layout.get("ringCenter")
		self.slotSize = layout["slotSize"]
		slotSize = layout["slotSize"]
		weakTaskBar = proxy(taskBar)
		slotSizes = layout.get("slotSizes") or [slotSize] * SKILL_SLOT_MAX
		for (index, (x, y)) in enumerate(self.positions):
			slot = SkillSlot(index, slotSizes[index], weakTaskBar)
			slot.selectSkill = ui.__mem_func__(self.SelectSkill)
			slot.SetPosition(x, y)
			self.slots.append(slot)

		((x, y), size) = layout["pageButton"]
		self.pageButton = QuickPageButton(size, proxy(self))
		self.pageButton.SetPosition(x, y)

		self.plusSlot = PlusSlot(slotSize, ui.__mem_func__(self.AddSlot), ui.__mem_func__(self.RemoveSlot))
		self.count = GetSkillSlotCount()
		self.LoadPositions()
		editPosition, editSize = layout.get("editButton", ((wndMgr.GetScreenWidth() - size - 12, 400), size))
		self.editor = uiAkravonSkillLayout.LayoutEditor(self, slotSize, editPosition, editSize)
		self.picker = uiAkravonSkillLayout.QuickSlotPicker(self)

		self.RefreshQuickSlot()

	def Destroy(self):
		self.CancelPageTransition()
		if self.editor.editing:
			self.SavePositions()
		self.editor.Destroy()
		self.editor = None
		self.picker.Destroy()
		self.picker = None
		for slot in self.slots:
			slot.Destroy()
		self.slots = []
		if self.pageButton:
			self.pageButton.Destroy()
			self.pageButton = None
		if self.plusSlot:
			self.plusSlot.Destroy()
			self.plusSlot = None

	def AddSlot(self):
		if self.pageButton.transitionStart is not None:
			return
		if self.count < SKILL_SLOT_MAX:
			self.count += 1
			self.__PlaceExtraSlot(self.count - 1)
			SetSkillSlotCount(self.count)
			self.SavePositions()
			self.__Arrange()
			self.SelectSkill(self.count - 1)

	def SelectSkill(self, index):
		if self.pageButton.transitionStart is not None:
			return
		self.picker.Open(index)

	def CancelPageTransition(self):
		self.pageButton.CancelTransition()

	def RemoveSlot(self):
		if self.pageButton.transitionStart is not None:
			return
		if self.count > SKILL_SLOT_DEFAULT:
			self.count -= 1
			SetSkillSlotCount(self.count)
			self.__Arrange()

	def __Arrange(self):
		# gorunen yuvalar ilk "count" konumu doldurur, "+" bir sonraki konumda durur
		for (index, slot) in enumerate(self.slots):
			if self.shown and index < self.count:
				slot.Show()
			else:
				slot.Hide()
		self.ArrangePlus()
		self.editor.Refresh()

	def ArrangePlus(self):
		if False and self.shown and self.count < SKILL_SLOT_MAX:	# AKRAVON: sabit 5 slot; ekleme / cikarma ayar panelinden
			(x, y) = self.positions[self.count]
			if any(uiAkravonSkillLayout.Overlaps((x, y), slot.GetLocalPosition(), self.slotSize)
					for slot in self.slots[:self.count]):
				x = (wndMgr.GetScreenWidth() + 324) // 2 + 8
				y = 72
			self.plusSlot.SetPosition(x, y)
			self.plusSlot.Show()
		else:
			self.plusSlot.Hide()

	def __PlaceExtraSlot(self, index):
		position = self.slots[index].GetLocalPosition()
		if not any(uiAkravonSkillLayout.Overlaps(position, slot.GetLocalPosition(), self.slotSize)
				for slot in self.slots[:index]):
			return
		step = self.slotSize + 8
		for y in range(120, wndMgr.GetScreenHeight() - int(self.slotSize * 1.11), step + 8):
			for x in range(20, wndMgr.GetScreenWidth() - self.slotSize, step):
				if not any(uiAkravonSkillLayout.Overlaps((x, y), slot.GetLocalPosition(), self.slotSize)
						for slot in self.slots[:index]):
					self.slots[index].SetPosition(x, y)
					return
		dbg.TraceError("uiMobileHud: no free position for skill slot %d" % index)

	def LoadPositions(self):
		width, height = wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight()
		if LoadConfig().get("SKILL_LAYOUT_REPAIR") != "1":
			for i in range(SKILL_SLOT_MAX):
				LoadConfig().pop("SKILL_POSITION_%d" % i, None)
			LoadConfig()["SKILL_LAYOUT_REPAIR"] = "1"
			LoadConfig()[CONFIG_SKILL_SLOTS] = str(SKILL_SLOT_DEFAULT)
			self.count = SKILL_SLOT_DEFAULT
			SaveConfig()
		for i, slot in enumerate(self.slots):
			value = LoadConfig().get("SKILL_POSITION_%d" % i)
			if value is None:
				continue
			try:
				x, y = [float(v) for v in value.split(",")]
				if not (0.0 <= x <= 1.0 and 0.0 <= y <= 1.0):
					raise ValueError("outside screen")
				slot.SetPosition(*uiAkravonSkillLayout.ClampPosition(x * width, y * height,
					self.slotSize, width, height))
			except ValueError:
				dbg.TraceError("uiMobileHud: invalid saved skill position %d: %s" % (i, value))
		for i in range(self.count):
			self.__PlaceExtraSlot(i)

	def SavePositions(self):
		width, height = float(wndMgr.GetScreenWidth()), float(wndMgr.GetScreenHeight())
		for i, slot in enumerate(self.slots):
			x, y = slot.GetLocalPosition()
			LoadConfig()["SKILL_POSITION_%d" % i] = "%.6f,%.6f" % (x / width, y / height)
		SaveConfig()

	def ResetPositions(self):
		for i, position in enumerate(self.defaultPositions):
			self.slots[i].SetPosition(*position)
			LoadConfig().pop("SKILL_POSITION_%d" % i, None)
		self.count = SKILL_SLOT_DEFAULT
		SetSkillSlotCount(self.count)
		self.__Arrange()

	def Show(self):
		self.shown = True
		self.__Arrange()
		self.pageButton.Show()

	def Hide(self):
		self.CancelPageTransition()
		self.picker.Hide()
		if self.editor.editing:
			self.editor.Toggle()
		self.shown = False
		self.__Arrange()
		self.pageButton.Hide()

	def UpdateFlash(self):
		for slot in self.slots:
			slot.UpdateFlash()

	def SetTop(self):
		for slot in self.slots:
			slot.SetTop()
		self.pageButton.SetTop()
		self.plusSlot.SetTop()
		self.editor.Refresh()

	def CleanQuickSlots(self):
		# AKRAVON: slota girmemesi gereken (kutu, ozel esya vb.) eskiden eklenmis esyalar bir kez temizlenir
		for index in range(len(self.slots)):
			try:
				(slotType, position) = player.GetLocalQuickSlot(index)
				if slotType == player.SLOT_TYPE_INVENTORY:
					vnum = player.GetItemIndex(position)
					if vnum and not item.CanAddToQuickSlotItem(vnum):
						player.RequestDeleteGlobalQuickSlot(player.LocalQuickSlotIndexToGlobalQuickSlotIndex(index))
			except Exception:
				pass

	def RefreshQuickSlot(self):
		if not getattr(self, 'cleaned', False) and player.GetStatus(player.LEVEL) > 0:
			self.cleaned = True
			self.CleanQuickSlots()
		for slot in self.slots:
			slot.RefreshContent()
		if self.pageButton:
			self.pageButton.Refresh()

	def OnUseSkill(self, usedSlotIndex, coolTime):
		for slot in self.slots:
			if slot.IsSkillSlot(usedSlotIndex):
				slot.slot.SetSlotCoolTime(slot.slotIndex, coolTime)
				return

	def OnActivateSkill(self, usedSlotIndex):
		for slot in self.slots:
			if slot.IsSkillSlot(usedSlotIndex):
				slot.slot.ActivateSlot(slot.slotIndex)
				return

	def OnDeactivateSkill(self, usedSlotIndex):
		for slot in self.slots:
			if slot.IsSkillSlot(usedSlotIndex):
				slot.slot.DeactivateSlot(slot.slotIndex)
				return
