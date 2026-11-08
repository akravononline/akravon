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
ATTACK_GROWTH = 1.15

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
RING_ANGLES = (180.0, 202.5, 225.0, 247.5, 270.0, 292.5)	# AKRAVON: 5 slot saldirinin tam solundan tam ustune esit aralikla
OUTER_RING_ANGLES = (206.0, 256.0)
PAGE_BUTTON_ANGLE = 150.0	# dondur: saldirinin sol alti; ayar ondan hemen asagida (ayni yay)
EDIT_BUTTON_ANGLE = 124.0
SKILL_SLOT_MAX = 8
SKILL_SLOT_DEFAULT = 5

UTILITY_SIZE = 54
UTILITY_GAP = 6
UTILITY_COUNT = 3
MINIMAP_RESERVED_WIDTH = 160
MINIMAP_RESERVED_HEIGHT = 214

STATUS_BAR_HEIGHT = 70
NOTIFICATION_GAP = 12
NOTIFICATION_HEIGHT = 48
MENU_BUTTON_SIZE = 52
MENU_BUTTON_GAP = 8
MENU_BUTTON_COUNT = 4
STATUS_BAR_WIDTH = 262
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
COOL_FONT = "LatoBd:24"	# AKRAVON: tum arayuzle ayni aile (Lato)
COOL_FONT_HALF = 13
COOL_COLOR = grp.GenerateColor(0.98, 0.84, 0.50, 1.0)	# kabartmali altin
COOL_SHADOW = grp.GenerateColor(0.16, 0.05, 0.02, 0.85)
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
	pageSize = int(round(smallBase * (pageOverride[2] / 100.0 if pageOverride else 1.0)))
	editSize = int(round(smallBase * (editOverride[2] / 100.0 if editOverride else 1.0)))
	# AKRAVON: kenarlardan telefonda rahat tutulacak kadar uzak (ekran oraninda)
	leftMargin = max(int(round(LEFT_MARGIN * factor)), int(screenWidth * 0.10)) + SAFE_LEFT	# joystick biraz saga
	rightMargin = max(int(round(RIGHT_MARGIN * factor)), int(screenWidth * 0.075)) + SAFE_RIGHT	# saldiri sag kenara yakin; beceriler onun solunda yay seklinde
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
	innerRadius = ringAttackSize / 2.0 + slotSize / 2.0 + SLOT_RING_GAP * ringFactor * skillScale
	# Reserve space for the attached plaque, including diagonal neighbours.
	innerRadius = max(innerRadius, slotSize * 1.22 / (2.0 * math.sin(math.radians(11.25))))
	outerRadius = innerRadius + slotSize + SLOT_RING_STEP * ringFactor * skillScale
	rightExtent = max([innerRadius * math.cos(math.radians(a)) for a in RING_ANGLES] +
		[outerRadius * math.cos(math.radians(a)) for a in OUTER_RING_ANGLES]) + slotSize / 2.0
	bottomExtent = max([innerRadius * math.sin(math.radians(a)) for a in RING_ANGLES] +
		[outerRadius * math.sin(math.radians(a)) for a in OUTER_RING_ANGLES]) + slotSize * 0.61
	ringCenterX = min(ringCenterX, screenWidth - SCREEN_EDGE - rightExtent)
	ringCenterY = min(ringCenterY, screenHeight - SCREEN_EDGE - bottomExtent - int(slotSize * 0.45))

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
	nextY = utilityTop
	for name in UTILITY_NAMES:
		override = Override(name)
		size = int(round(utilityBase * (override[2] / 100.0 if override else 1.0)))
		utilitySizes[name] = size
		position = (screenWidth - SCREEN_EDGE - size, nextY)
		nextY += size + UTILITY_GAP
		if override:
			position = (int(round(override[0] * screenWidth / 1000.0 - size / 2.0)), int(round(override[1] * screenHeight / 1000.0 - size / 2.0)))
			position = (max(SCREEN_EDGE, min(screenWidth - SCREEN_EDGE - size, position[0])), max(SCREEN_EDGE, min(screenHeight - SCREEN_EDGE - size, position[1])))
		utilities.append(position)

	pagePosition = PlaceOnRing(ringCenterX, ringCenterY, innerRadius, PAGE_BUTTON_ANGLE, pageSize)
	pagePosition = (max(SCREEN_EDGE, min(screenWidth - SCREEN_EDGE - pageSize, pagePosition[0])), max(SCREEN_EDGE, min(screenHeight - SCREEN_EDGE - pageSize, pagePosition[1])))	# ekran disina tasmasin
	if pageOverride:
		pagePosition = (int(round(pageOverride[0] * screenWidth / 1000.0 - pageSize / 2.0)), int(round(pageOverride[1] * screenHeight / 1000.0 - pageSize / 2.0)))
		pagePosition = (max(SCREEN_EDGE, min(screenWidth - SCREEN_EDGE - pageSize, pagePosition[0])), max(SCREEN_EDGE, min(screenHeight - SCREEN_EDGE - pageSize, pagePosition[1])))
	editPosition = PlaceOnRing(ringCenterX, ringCenterY, innerRadius, EDIT_BUTTON_ANGLE, editSize)
	editPosition = (max(SCREEN_EDGE, min(screenWidth - SCREEN_EDGE - editSize, editPosition[0])), max(SCREEN_EDGE, min(screenHeight - SCREEN_EDGE - editSize, editPosition[1])))
	if editOverride:
		editPosition = (int(round(editOverride[0] * screenWidth / 1000.0 - editSize / 2.0)), int(round(editOverride[1] * screenHeight / 1000.0 - editSize / 2.0)))
		editPosition = (max(SCREEN_EDGE, min(screenWidth - SCREEN_EDGE - editSize, editPosition[0])), max(SCREEN_EDGE, min(screenHeight - SCREEN_EDGE - editSize, editPosition[1])))

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
		"slotSize" : slotSize,
		"slotSizes" : slotSizes,
		"pageButton" : (pagePosition, pageSize),
		"editButton" : (editPosition, editSize),
		"utilities" : (utilities, utilityBase),
		"utilitySizes" : utilitySizes,
		"runLock" : (RUN_LOCK_SIZE, RUN_LOCK_GAP),
		"statusBar" : (SCREEN_EDGE, TOP_MARGIN),
		"notificationY" : notificationY,
		"menuBar" : (screenWidth - SAFE_RIGHT - MINIMAP_RESERVED_WIDTH - WINDOW_GAP - menuWidth, TOP_MARGIN, MENU_BUTTON_SIZE, MENU_BUTTON_GAP),
		"eventRow" : (screenWidth - SAFE_RIGHT - MINIMAP_RESERVED_WIDTH - WINDOW_GAP, TOP_MARGIN + (MENU_BUTTON_SIZE + MENU_BUTTON_GAP) * 3),
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
			self.cooldownShadow.SetFontName(COOL_FONT)
		except Exception:
			pass
		self.cooldownShadow.SetPackedFontColor(COOL_SHADOW)
		self.cooldownShadow.SetHorizontalAlignCenter()
		self.cooldownShadow.SetPosition(size // 2 + 1, size // 2 - COOL_FONT_HALF + 2)
		self.cooldownShadow.Hide()

		self.cooldownText = ui.TextLine()
		self.cooldownText.SetParent(self)
		self.cooldownText.AddFlag("not_pick")
		try:
			self.cooldownText.SetFontName(COOL_FONT)
		except Exception:
			pass
		self.cooldownText.SetOutline()
		self.cooldownText.SetPackedFontColor(COOL_COLOR)
		self.cooldownText.SetHorizontalAlignCenter()
		self.cooldownText.SetPosition(size // 2, size // 2 - COOL_FONT_HALF)
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
		# AKRAVON: basildi efekti - halka kisa sure parlar
		self.flashUntil = app.GetTime() + 0.2
		if self.ring:
			wndMgr.SetDiffuseColor(self.ring.hWnd, 1.0, 0.9, 0.45, 1.0)

	def UpdateFlash(self):
		if getattr(self, "flashUntil", 0.0) and app.GetTime() >= self.flashUntil:
			self.flashUntil = 0.0
			if self.ring:
				wndMgr.SetDiffuseColor(self.ring.hWnd, 1.0, 1.0, 1.0, 1.0)

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
		self.OnUpdate()

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
		angle = progress * 180.0 if progress < 0.5 else (progress - 1.0) * 180.0
		alpha = abs(1.0 - 2.0 * progress)
		for slot in self.arc.slots:
			slot.SetPageTransition(angle, alpha)

	def CancelTransition(self):
		if self.transitionStart is None:
			return
		self.transitionStart = None
		self.face.SetRotation(0)
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
