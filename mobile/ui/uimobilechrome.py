from __future__ import division
import app
import chat
import math
import net
import player
import wndMgr
import uiScriptLocale
import ui
import uiMobileCompat
import uiMobileStyle
import uiMobileChat

from _weakref import proxy

STATUS_WIDTH = 228
STATUS_HEIGHT = 80
# AKRAVON: konumlar durum panelinin (skin/build_akravon_player.py) 262x70 mantiksal olcegindedir
FACE_CENTER = (35, 42)
FACE_SIZE = 52
NAME_X = 78
NAME_CENTER_Y = 6
BAR_LAYOUT = {"hp" : (70, 20, 154, 17), "sp" : (70, 39, 154, 16), "st" : (70, 58, 154, 10)}	# orijinal Metin2 gauge.sub yuvalari (x1.5)
EXP_PLATE_POSITION = (87, 73)
EXP_PLATE_SCALE = 0.8	# orijinal Metin2 EXP plakasi (4 kure), mantiksal olcek
EXP_ORB_CENTERS = (15, 39, 64, 90)
EXP_ORB_Y = 17
EXP_ORB_SIZE = 19	# x, y, genislik, yukseklik
def PhoneFont(name, size):
	# AKRAVON: telefonda mantiksal ekran ~600 yuksek; kucuk HUD yazilari ayni fiziksel boyutta kalsin diye buyutulur
	height = wndMgr.GetScreenHeight()
	if height < 900:
		size = int(round(size * (900.0 / height) * 0.82))
	return "%s:%d" % (name, size)


BAR_FONTS = {"hp" : PhoneFont("LatoBd", 13), "sp" : PhoneFont("LatoBd", 13)}
# AKRAVON: HD HUD gorselleri (2 kat, yarim olcekle cizilir)
HUD_PATH = "mobile_ui/akravon/hud/%s.png"
HUD_BUTTON_ASSET = 136.0
HUD_SMALL_LIMIT = 44
HUD_MID_LIMIT = 70
STATUS_REFRESH_INTERVAL = 0.2
LOW_HP_RATIO = 0.3
GAUGE_FRAMES = {"hp" : 7, "sp" : 7, "st" : 7}	# orijinal Metin2 hareketli cubuk kareleri
GAUGE_FRAME_SECONDS = 0.11

LOW_HP_TEXT_COLOR = uiMobileStyle.Color(255, 176, 150)
BAR_TEXT_COLOR = uiMobileStyle.Color(246, 236, 214)

MENU_BUTTONS = ("inventory", "itemshop", "character", "more")
MENU_MORE_BUTTONS = ("messenger", "system")
UTILITY_BUTTONS = ("pickup", "autohunt", "mount")
HUD_BUTTON_SIZES = (50, 54, 60, 68)
HUD_BUTTON_LABELS = {}	# AKRAVON: butonlarda yazi yok, yalniz ikon
HUD_LABEL_FONT = PhoneFont("LatoBd", 14)
HUD_LABEL_BOTTOM = 17
HUD_LABEL_COLOR = uiMobileStyle.Color(240, 207, 135)
HUD_ALERT_ASSET = "hudbtn_alert_%d"
HUD_ALERT_SPEED = 5.0
HUD_ALERT_MIN_ALPHA = 0.25
# renk (r, g, b), yanip sonme hizi (0 = sabit), en dusuk saydamlik
HUD_RING_STYLES = {"alert": (1.0, 0.3, 0.22, 4.0, 0.35), "run": (0.45, 1.0, 0.55, 2.0, 0.55), "open": (1.0, 0.96, 0.88, 0, 0.8)}

CHAT_TITLE_TEXT = "S O H B E T"
CHAT_TITLE_COLOR = uiMobileStyle.Color(190, 176, 148)
CHAT_TITLE_ACTIVE_COLOR = uiMobileStyle.Color(240, 228, 200)
BADGE_LIMIT = 9
SCREEN_GAP = 8
FLOAT_LINE_GAP = 8

NOTIFICATION_ORDER = ("QUEST", "HELP", "BUILD", "EXIT_OBSERVER")
HIDDEN_NOTIFICATIONS = ("STATUS", "SKILL")
NOTIFICATION_STEP = 48


def FormatNumber(value):
	return "{:,}".format(int(value)).replace(",", ".")


def HudButtonSize(size):
	return min(HUD_BUTTON_SIZES, key=lambda candidate: abs(candidate - size))


PRESS_SECONDS = 0.42
PRESS_RING_TEXTURE = HUD_PATH % "hud_press_ring"
PRESS_DOT_TEXTURE = HUD_PATH % "hud_press_dot"
PRESS_DOT_COUNT = 8


class PressRipple(object):
	# AKRAVON: beceri slotlariyla ayni basma efekti - disa yayilan iki halka + etrafa sacilan kabarciklar (ust katman)
	def __init__(self, color = (1.0, 0.9, 0.6)):
		self.color = color	# halka / kabarcik rengi (varsayilan altin; oto av yesil)
		self.rings = [self.__Image(PRESS_RING_TEXTURE) for i in range(2)]
		self.dots = [self.__Image(PRESS_DOT_TEXTURE) for i in range(PRESS_DOT_COUNT)]
		self.start = None

	def __Image(self, fileName):
		image = ui.ExpandedImageBox()
		image.AddFlag("not_pick")
		image.AddFlag("float")
		image.LoadImage(fileName)
		image.Hide()
		return image

	def Start(self):
		self.start = app.GetTime()

	def Update(self, owner):
		if self.start is None:
			return
		progress = (app.GetTime() - self.start) / PRESS_SECONDS
		if progress >= 1.0 or not owner.IsShow():
			self.start = None
			self.Hide()
			return
		size = owner.GetWidth()
		(gx, gy) = owner.GetGlobalPosition()
		cx = gx + size / 2.0
		cy = gy + size / 2.0
		for (i, ring) in enumerate(self.rings):
			p = (progress - i * 0.18) / 0.82
			if p <= 0.0 or p >= 1.0:
				ring.Hide()
				continue
			drawn = size * (0.92 + 1.05 * p)
			ring.SetScale(drawn / 256.0, drawn / 256.0)
			ring.SetPosition(int(round(cx - drawn / 2.0)), int(round(cy - drawn / 2.0)))
			(r, g, b) = self.color
			wndMgr.SetDiffuseColor(ring.hWnd, r, g, b, 1.0 - p * 0.85)
			ring.Show()
			ring.SetTop()
		for (k, dot) in enumerate(self.dots):
			angle = math.radians(k * (360.0 / PRESS_DOT_COUNT) + 20.0 * (k % 2))
			distance = size * (0.42 + (0.55 + 0.12 * (k % 3)) * progress)
			dotSize = size * (0.21 - 0.09 * progress) * (0.8 + 0.2 * (k % 2))
			dot.SetScale(dotSize / 64.0, dotSize / 64.0)
			dot.SetPosition(int(round(cx + math.cos(angle) * distance - dotSize / 2.0)), int(round(cy + math.sin(angle) * distance - dotSize / 2.0 - size * 0.12 * progress)))
			(r, g, b) = self.color
			wndMgr.SetDiffuseColor(dot.hWnd, r, g, b, 1.0 - progress * 0.9)
			dot.Show()
			dot.SetTop()

	def Hide(self):
		for image in self.rings + self.dots:
			image.Hide()


class HudButton(ui.Window):
	def __init__(self, size, name):
		ui.Window.__init__(self, "UI")
		self.SetSize(size, size)
		ui.EnableSingleTap(self)

		self.event = None
		self.active = False
		self.pressed = False
		self.alert = None
		self.alertOn = False
		self.assetPrefix = HUD_PATH % ("btn_%s" % name)
		if size <= HUD_SMALL_LIMIT:
			(self.suffix, assetSize) = ("_s", 72.0)
		elif size <= HUD_MID_LIMIT:
			(self.suffix, assetSize) = ("_m", 112.0)
		else:
			(self.suffix, assetSize) = ("", HUD_BUTTON_ASSET)
		self.assetScale = float(size) / assetSize
		self.alertScale = float(size) / HUD_BUTTON_ASSET
		self.state = None

		self.image = ui.ExpandedImageBox()
		self.image.SetParent(self)
		self.image.AddFlag("not_pick")
		self.image.Show()
		self.__Refresh()

		labelName = HUD_BUTTON_LABELS.get(name)
		self.label = uiMobileStyle.CreateText(self, getattr(uiScriptLocale, labelName, "") if labelName else "", size // 2, size - HUD_LABEL_BOTTOM, HUD_LABEL_COLOR, HUD_LABEL_FONT, "center", True)

		self.SetTouchTapEvent(ui.__mem_func__(self.__OnTap))

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		ripple = getattr(self, "ripple", None)
		if ripple:
			ripple.Hide()
		self.ripple = None
		self.event = None
		self.longPressEvent = None
		self.eventTouchTap = None
		self.eventTouchLongPress = None
		self.image = None
		self.label = None
		if self.alert:
			self.alert.Hide()
		self.alert = None
		self.alertOn = False
		self.Hide()

	def SetEvent(self, event):
		self.event = event

	# Cynex: uzun basis ikinci islev (Menu: mobil ayarlar)
	def SetLongPressEvent(self, event):
		self.longPressEvent = event
		self.SetTouchLongPressEvent(ui.__mem_func__(self.__OnLongPress))

	def __OnLongPress(self, x, y):
		event = getattr(self, "longPressEvent", None)
		if event:
			event()

	# AKRAVON: vurgu halkasi - "alert" kirmizi hizli yanip soner (onemli), "run" yesil yavas nabiz (calisiyor),
	# "open" beyaz sabit (pencere acik). oncelik: alert > run > open
	def SetAlert(self, enabled):
		self.alertOn = enabled
		self.__RefreshRing()

	def Hide(self):
		ui.Window.Hide(self)
		if getattr(self, "alert", None):
			self.alert.Hide()

	def SetRunning(self, enabled):
		self.running = enabled
		self.__RefreshRing()

	def SetHighlight(self, enabled):
		self.active = enabled
		self.__Refresh()
		self.__RefreshRing()

	def __RingKind(self):
		if self.alertOn:
			return "alert"
		if getattr(self, "running", False):
			return "run"
		if self.active:
			return "open"
		return None

	def __RefreshRing(self):
		kind = self.__RingKind()
		if kind and not self.alert:
			# ust seviye pencere: dugmenin alt ogesi olsa kenarda kirpilir, dis isilti gorunmez
			self.alert = ui.ExpandedImageBox()
			self.alert.AddFlag("not_pick")
			self.alert.AddFlag("float")
			self.alert.LoadImage(HUD_PATH % "btn_alert")
			self.alert.SetScale(self.alertScale, self.alertScale)
		if not self.alert:
			return
		if kind:
			self.alert.Show()
			self.OnUpdate()
		else:
			self.alert.Hide()

	def EnablePressRipple(self, color = (1.0, 0.9, 0.6)):
		if not getattr(self, "ripple", None):
			self.ripple = PressRipple(color)

	def OnUpdate(self):
		ripple = getattr(self, "ripple", None)
		if ripple:
			ripple.Update(self)
		if not self.alert:
			return
		kind = self.__RingKind()
		if not kind or not self.IsShow():
			self.alert.Hide()
			return
		(x, y) = self.GetGlobalPosition()
		offset = int(round(20 * self.alertScale))
		self.alert.SetPosition(x - offset, y - offset)
		self.alert.Show()
		(red, green, blue, speed, low) = HUD_RING_STYLES[kind]
		alpha = low
		if speed:
			phase = (math.sin(app.GetTime() * speed) + 1.0) / 2.0
			alpha = low + (1.0 - low) * phase
		wndMgr.SetDiffuseColor(self.alert.hWnd, red, green, blue, alpha)

	def SetIcon(self, name):
		self.assetPrefix = HUD_PATH % ("btn_%s" % name)
		self.state = None
		self.__Refresh()

	def __Refresh(self):
		if not self.image:
			return
		if self.pressed:
			state = "down"
		elif self.active:
			state = "active"
		else:
			state = "normal"
		if state == self.state:
			return
		self.state = state
		self.image.LoadImage(self.assetPrefix[:-4] + "_%s%s.png" % (state, self.suffix))
		self.image.SetScale(self.assetScale, self.assetScale)

	def __Run(self):
		# AKRAVON: iPhone'da tek dokunus hem dokunma hem fare tiklamasi olarak gelebiliyor: 0.35 sn icinde ikinci calisma yutulur (ac-kapa hatasi)
		now = app.GetTime()
		import dbg
		if now - getattr(self, "lastRunTime", -10.0) < 0.35:
			return
		self.lastRunTime = now
		ripple = getattr(self, "ripple", None)
		if ripple:
			ripple.Start()
		if self.event:
			self.event()

	def __OnTap(self, x, y):
		import dbg
		self.__Run()

	def OnMouseLeftButtonDown(self):
		self.pressed = True
		self.__Refresh()
		return True

	def OnMouseLeftButtonUp(self):
		wasPressed = self.pressed
		self.pressed = False
		self.__Refresh()
		if wasPressed and self.IsIn():
			import dbg
			self.__Run()
		return True

	def OnMouseOverOut(self):
		if self.pressed:
			self.pressed = False
			self.__Refresh()


def HudImage(parent, name, x, y, scaleX = 0.5, scaleY = 0.5):
	image = ui.ExpandedImageBox()
	image.SetParent(parent)
	image.AddFlag("not_pick")
	image.LoadImage(HUD_PATH % name)
	image.SetScale(scaleX, scaleY)
	image.SetPosition(x, y)
	image.Show()
	return image


class Gauge(object):
	# AKRAVON: panelin kendi yuvasina oturan dolgu (referanstan kesilmis); kind None = kapali
	def __init__(self, parent, kind, showText):
		self.enabled = kind is not None
		self.ratio = -1.0
		self.text = None
		if not self.enabled:
			return
		x, y, width, height = BAR_LAYOUT[kind]
		self.kind = kind
		self.frame = 0
		self.bg = HudImage(parent, "status_%s_bg" % kind, x, y)	# bos kisim: koyu oluk
		self.fill = HudImage(parent, "status_%s" % kind, x, y)
		if showText:
			self.text = uiMobileStyle.CreateText(parent, "", x + width // 2, y + height // 2 - 6, BAR_TEXT_COLOR, BAR_FONTS[kind], "center", True)

	def Animate(self, now):
		# orijinal Metin2 cubugu: kareler sirayla doner (isiltili akis); doluluk korunur
		frames = GAUGE_FRAMES.get(getattr(self, "kind", None), 0)
		if not self.enabled or frames <= 1 or self.ratio <= 0.0:
			return
		frame = int(now / GAUGE_FRAME_SECONDS) % frames + 1
		if frame == self.frame:
			return
		self.frame = frame
		self.fill.LoadImage(HUD_PATH % ("status_%s_%d" % (self.kind, frame)))
		self.fill.SetScale(0.5, 0.5)
		self.fill.SetRenderingRect(0.0, 0.0, self.ratio - 1.0, 0.0)

	def SetValue(self, current, maximum):
		ratio = 0.0
		if maximum > 0:
			ratio = max(0.0, min(1.0, float(current) / maximum))
		if not self.enabled:
			return ratio
		if ratio != self.ratio:
			self.ratio = ratio
			self.fill.SetRenderingRect(0.0, 0.0, ratio - 1.0, 0.0)
		if self.text:
			self.text.SetText("%s / %s" % (FormatNumber(min(current, maximum)), FormatNumber(maximum)))
		return ratio


class ExpOrbs(object):
	# AKRAVON: orijinal Metin2 EXP gostergesi - 4 kure, her biri deneyimin %25'ini doldurur
	def __init__(self, parent):
		(px, py) = EXP_PLATE_POSITION
		scale = EXP_PLATE_SCALE / 2.0
		self.plate = HudImage(parent, "exp_plate", px, py, scale, scale)
		self.orbs = []
		self.ratio = -1.0
		for center in EXP_ORB_CENTERS:
			size = EXP_ORB_SIZE * EXP_PLATE_SCALE
			orb = HudImage(parent, "exp_orb", int(round(px + center * EXP_PLATE_SCALE - size / 2.0)), int(round(py + EXP_ORB_Y * EXP_PLATE_SCALE - size / 2.0)), scale, scale)
			orb.Hide()
			self.orbs.append(orb)

	def SetValue(self, current, maximum):
		ratio = max(0.0, min(1.0, float(current) / maximum)) if maximum > 0 else 1.0	# azami seviye: tam dolu
		if ratio == self.ratio:
			return ratio
		self.ratio = ratio
		for (index, orb) in enumerate(self.orbs):
			part = max(0.0, min(1.0, ratio * len(self.orbs) - index))
			if part <= 0.0:
				orb.Hide()
				continue
			orb.SetRenderingRect(0.0, 0.0, part - 1.0, 0.0)
			orb.Show()
		return ratio


class StatusBar(ui.Window):
	def __init__(self):
		ui.Window.__init__(self, "UI")
		self.SetSize(STATUS_WIDTH, STATUS_HEIGHT)
		ui.EnableSingleTap(self)

		self.event = None
		self.nextRefreshTime = 0.0
		self.race = -1
		self.lowHealth = None

		self.frame = HudImage(self, "status_panel", 0, 0)

		self.face = ui.ExpandedImageBox()
		self.face.SetParent(self)
		self.face.AddFlag("not_pick")
		self.face.SetPosition(FACE_CENTER[0] - FACE_SIZE // 2, FACE_CENTER[1] - FACE_SIZE // 2)
		self.face.Show()

		self.nameText = uiMobileStyle.CreateText(self, "", NAME_X, NAME_CENTER_Y - 9, uiMobileStyle.BRIGHT_COLOR, PhoneFont("LatoBd", 16), "left", True)

		self.hpGauge = Gauge(self, "hp", False)	# AKRAVON: cubuklarin ustunde yazi yok (orijinal Metin2 gibi)
		self.spGauge = Gauge(self, "sp", False)
		self.stGauge = Gauge(self, "st", False)
		self.expGauge = Gauge(self, None, False)	# AKRAVON: EXP gostergesi kaldirildi (yeri yok)

		self.SetTouchTapEvent(ui.__mem_func__(self.__OnTap))
		self.RefreshStatus()

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		self.event = None
		self.eventTouchTap = None
		self.frame = None
		self.face = None
		self.nameText = None
		self.hpGauge = None
		self.spGauge = None
		self.stGauge = None
		self.expGauge = None
		self.Hide()

	def SetEvent(self, event):
		self.event = event

	def __OnTap(self, x, y):
		if self.event:
			self.event()

	def OnMouseLeftButtonDown(self):
		return True

	def OnMouseLeftButtonUp(self):
		if self.event and self.IsIn():
			self.event()
		return True

	def __RefreshFace(self):
		race = player.GetRace()
		if race == self.race:
			return
		self.race = race
		try:
			import uiCharacter
			fileName = uiCharacter.FACE_IMAGE_DICT[race]
		except Exception:
			return
		face = "status_face_%s" % fileName.replace("\\", "/").split("/")[-1].split(".")[0].lower()
		if uiMobileStyle.HasAsset(face):
			uiMobileStyle.LoadAsset(self.face, face)
			self.face.SetScale(FACE_SIZE / 52.0, FACE_SIZE / 52.0)
			return
		self.face.LoadImage(fileName)
		width = max(1, self.face.GetWidth())
		height = max(1, self.face.GetHeight())
		self.face.SetScale(float(FACE_SIZE) / width, float(FACE_SIZE) / height)

	def RefreshStatus(self):
		self.__RefreshFace()

		currentHP = player.GetStatus(player.HP)
		maximumHP = player.GetStatus(player.MAX_HP)
		currentEXP = player.GetStatus(player.EXP)
		nextEXP = player.GetStatus(player.NEXT_EXP)

		healthRatio = self.hpGauge.SetValue(currentHP, maximumHP)
		self.spGauge.SetValue(player.GetStatus(player.SP), player.GetStatus(player.MAX_SP))
		self.stGauge.SetValue(player.GetStatus(player.STAMINA), player.GetStatus(player.MAX_STAMINA))
		self.expGauge.SetValue(currentEXP, nextEXP)

		lowHealth = maximumHP > 0 and healthRatio < LOW_HP_RATIO
		if lowHealth != self.lowHealth:
			self.lowHealth = lowHealth
			if self.hpGauge.text:
				self.hpGauge.text.SetPackedFontColor(LOW_HP_TEXT_COLOR if lowHealth else BAR_TEXT_COLOR)

		self.nameText.SetText(player.GetName())

	def OnUpdate(self):
		now = app.GetTime()
		for gauge in (self.hpGauge, self.spGauge, self.stGauge):
			if gauge:
				gauge.Animate(now)
		if now < self.nextRefreshTime:
			return
		self.nextRefreshTime = now + STATUS_REFRESH_INTERVAL
		self.RefreshStatus()


class ButtonRow(object):
	def __init__(self, names, size, sizes = None):
		self.buttons = {}
		self.order = []
		for name in names:
			button = HudButton((sizes or {}).get(name, size), name)
			self.buttons[name] = button
			self.order.append(name)

	def Destroy(self):
		for button in self.buttons.values():
			button.Destroy()
		self.buttons = {}
		self.order = []

	def SetEvent(self, name, event):
		if event is None:
			self.buttons[name].Hide()
			self.order.remove(name)
			return
		self.buttons[name].SetEvent(event)

	def GetButton(self, name):
		return self.buttons.get(name)

	def SetLongPressEvent(self, name, event):
		button = self.buttons.get(name)
		if button:
			button.SetLongPressEvent(event)

	def PlaceHorizontal(self, x, y, size, gap):
		for (index, name) in enumerate(self.order):
			self.buttons[name].SetPosition(x + index * (size + gap), y)

	def PlaceVertical(self, positions):
		for (name, (x, y)) in zip(self.order, positions):
			self.buttons[name].SetPosition(x, y)

	def Show(self):
		for name in self.order:
			self.buttons[name].Show()

	def Hide(self):
		for button in self.buttons.values():
			button.Hide()


class ChatToggle(uiMobileStyle.Pressable):
	# AKRAVON: ekranin dibine yapisik kisa sohbet seridi (ters U); solda ikon, ortada SOHBET, sagda rozet + ok
	def __init__(self, width, height):
		uiMobileStyle.Pressable.__init__(self, width, height)

		self.frame = HudImage(self, "chat_dock_closed", 0, 0)

		self.icon = HudImage(self, "chat_icon", 14, (height - 22) // 2 + 1)
		self.title = uiMobileStyle.CreateText(self, CHAT_TITLE_TEXT, width // 2, height // 2 - 6, CHAT_TITLE_COLOR, "LatoBd:14", "center")
		self.arrow = HudImage(self, "chat_arrow", width - 14 - 20, (height - 20) // 2 + 1, 0.5, 0.5)
		self.arrowLeft = width - 14 - 20

		self.badge = HudImage(self, "chat_badge_dot", self.arrowLeft - 34, (height - 22) // 2 + 1)
		self.badge.Hide()
		self.badgeText = uiMobileStyle.CreateText(self, "", self.arrowLeft - 34 + 11, height // 2 - 6, uiMobileStyle.BRIGHT_COLOR, "LatoBd:14", "center")
		self.badgeText.Hide()
		self.badgeValue = 0

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.frame = None
		self.icon = None
		self.title = None
		self.arrow = None
		self.badge = None
		self.badgeText = None

	def RefreshVisual(self):
		if not getattr(self, "title", None):
			return
		self.title.SetPackedFontColor(CHAT_TITLE_ACTIVE_COLOR if self.pressed or self.hovered else CHAT_TITLE_COLOR)

	def SetUnreadCount(self, count):
		count = max(0, count)
		if count == self.badgeValue:
			return
		self.badgeValue = count
		if count <= 0:
			self.badge.Hide()
			self.badgeText.Hide()
			return
		self.badgeText.SetText(str(min(count, BADGE_LIMIT)) + ("+" if count > BADGE_LIMIT else ""))
		self.badge.Show()
		self.badgeText.Show()


class FloatingChat(ui.Window):
	# AKRAVON: sohbet kapaliyken seridin ustunde akan son satirlar (arka plan yok, zamanla silikleserek kaybolur)
	def __init__(self, width):
		ui.Window.__init__(self, "UI")
		self.AddFlag("not_pick")
		self.SetSize(width, 1)
		self.chatID = chat.CreateChatSet(uiMobileChat.FLOAT_CHAT_SET)
		chat.SetBoardState(self.chatID, chat.BOARD_STATE_VIEW)

	def __del__(self):
		ui.Window.__del__(self)

	def OnRender(self):
		(x, y) = self.GetGlobalPosition()
		chat.SetPosition(self.chatID, x, y)
		chat.Update(self.chatID)
		chat.Render(self.chatID)


class ChatController(object):
	def __init__(self, interface, toggle):
		self.interface = proxy(interface)
		self.toggle = toggle
		self.expanded = False
		self.visible = True
		self.panel = uiMobileChat.ChatPanel(interface)
		self.panel.SetCollapseEvent(ui.__mem_func__(self.Collapse))
		self.panel.SetExpandEvent(ui.__mem_func__(self.Expand))
		self.panel.Hide()
		self.floating = FloatingChat(uiMobileChat.FLOAT_WIDTH)
		self.floating.Show()
		self.seenLineCount = self.__GetLineCount()
		self.toggle.SetEvent(ui.__mem_func__(self.Expand))

	def __GetChatWindow(self):
		try:
			return self.interface.wndChat
		except ReferenceError:
			return None

	def __GetLineCount(self):
		chatID = getattr(self.panel, "chatID", None) if self.panel else None
		if chatID is None:
			return 0
		try:
			return chat.GetLineCount(chatID)
		except Exception:
			return 0

	def Place(self):
		(toggleX, toggleY) = self.toggle.GetGlobalPosition()
		centerX = toggleX + self.toggle.GetWidth() // 2
		x = max(SCREEN_GAP, min(wndMgr.GetScreenWidth() - SCREEN_GAP - uiMobileChat.PANEL_WIDTH, centerX - uiMobileChat.PANEL_WIDTH // 2))
		self.panel.SetHomePosition(x, toggleY + self.toggle.GetHeight())
		self.panel.SetCollapsedBox(toggleX, toggleY, self.toggle.GetWidth(), self.toggle.GetHeight())
		self.floating.SetPosition(toggleX + 6, toggleY - FLOAT_LINE_GAP)

	def Restore(self):
		window = self.__GetChatWindow()
		if window:
			window.Show()
			window.Refresh()

	def Destroy(self):
		if self.panel:
			self.panel.Destroy()
		if self.floating:
			self.floating.Hide()
		self.floating = None
		self.panel = None
		self.toggle = None
		self.interface = None

	def Expand(self):
		self.expanded = True
		self.toggle.Hide()
		self.floating.Hide()
		self.panel.Open()
		self.toggle.SetUnreadCount(0)

	def Collapse(self, animate = True):
		self.expanded = False
		self.seenLineCount = self.__GetLineCount()
		if not self.panel:
			self.__OnPanelClosed()
			return
		if animate:
			self.panel.Close(ui.__mem_func__(self.__OnPanelClosed))
			return
		self.panel.CloseNow()
		self.__OnPanelClosed()

	def __OnPanelClosed(self):
		if self.toggle and self.visible and not self.expanded:
			self.toggle.Show()
			self.floating.Show()

	def Hide(self):
		self.visible = False
		if self.expanded:
			self.Collapse(False)
		elif self.panel:
			self.panel.CloseNow()
		self.toggle.Hide()
		self.floating.Hide()

	def Show(self):
		self.visible = True
		if not self.expanded and not (self.panel and self.panel.IsShow()):
			self.toggle.Show()
			self.floating.Show()

	def __ReleaseDesktopChat(self, window):
		if window.IsEditMode():
			window.CloseChat()
			if not self.expanded:
				self.Expand()
			self.panel.FocusInput()
		if window.IsShow():
			window.Hide()

	def Update(self):
		window = self.__GetChatWindow()
		if window:
			self.__ReleaseDesktopChat(window)

		lineCount = self.__GetLineCount()
		if self.expanded:
			self.seenLineCount = lineCount
		else:
			self.toggle.SetUnreadCount(lineCount - self.seenLineCount)


class NotificationLayout(object):
	def __init__(self, gameButtonWindow):
		self.window = proxy(gameButtonWindow)
		self.originalPositions = {}

	def Apply(self, x, y):
		try:
			buttons = self.window.gameButtonDict
		except ReferenceError:
			return
		for (index, name) in enumerate(NOTIFICATION_ORDER):
			button = buttons.get(name)
			if not button:
				continue
			if name not in self.originalPositions:
				self.originalPositions[name] = button.GetLocalPosition()
			button.SetPosition(x + index * NOTIFICATION_STEP, y)

	def HidePointButtons(self):
		try:
			buttons = self.window.gameButtonDict
		except ReferenceError:
			return
		for name in HIDDEN_NOTIFICATIONS:
			button = buttons.get(name)
			if button and button.IsShow():
				button.Hide()

	def Restore(self):
		try:
			buttons = self.window.gameButtonDict
		except ReferenceError:
			return
		for (name, (x, y)) in self.originalPositions.items():
			button = buttons.get(name)
			if button:
				button.SetPosition(x, y)
		self.originalPositions = {}


def RunMount():
	net.SendChatPacket("/ride")


def RunPickup():
	player.PickCloseItem()
