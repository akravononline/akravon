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
STATUS_HEIGHT = 96
# AKRAVON: konumlar durum panelinin (skin/build_akravon_player.py) 262x70 mantiksal olcegindedir
FACE_CENTER = (34, 35)
FACE_SIZE = 58
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
# calisiyor efekti: iksir kabarciklari (baslangic fazi, tur suresi sn, yatay konum [yaricap orani], boy, salinim)
RUN_BUBBLES = ()	# icerideki kabarciklar kaldirildi (referans: sadece kenar isiltisi + disarida minik noktalar)
# dis kabarciklar: dugme kenarindan disari dogru yukselir (baslangic fazi, tur suresi sn, cikis acisi [derece, 0 = sag, -90 = yukari], boy)
RUN_OUT_BUBBLES = tuple(((i * 0.618) % 1.0, 1.3 + (i % 5) * 0.3, (i * 137.5) % 360.0 - 180.0, 0.7 + (i % 4) * 0.25) for i in range(36))
RUN_BUBBLE_ASSET = 16.0
RUN_GLOW_ALPHA = 0.9	# kenar isiltisi sabit (yanip sonmez)
RUN_COLOR = (0.45, 1.0, 0.55)
RUN_TINT = (0.72, 1.0, 0.78)	# oto av aktifken dugme ikonunun hafif yesil tonu
HUD_RING_STYLES = {"alert": (1.0, 0.3, 0.22, 4.0, 0.35), "run": (0.45, 1.0, 0.55, 0, 0.45), "open": (1.0, 0.96, 0.88, 0, 0.8)}

CHAT_TITLE_TEXT = "S O H B E T"
CHAT_TITLE_COLOR = uiMobileStyle.Color(190, 176, 148)
CHAT_TITLE_ACTIVE_COLOR = uiMobileStyle.Color(240, 228, 200)
BADGE_LIMIT = 9
SCREEN_GAP = 8
STRIP_WIDTH = 420
FLOAT_LINE_GAP = 8

NOTIFICATION_ORDER = ("QUEST", "HELP", "BUILD", "EXIT_OBSERVER")
HIDDEN_NOTIFICATIONS = ("STATUS", "SKILL")
NOTIFICATION_STEP = 48


def FormatNumber(value):
	return "{:,}".format(int(value)).replace(",", ".")


def HudButtonSize(size):
	return min(HUD_BUTTON_SIZES, key=lambda candidate: abs(candidate - size))


PRESS_SECONDS = 0.42
PRESS_SINK_SECONDS = 0.26	# dugme ice cokup geri gelme suresi
PRESS_SINK_DEPTH = 0.14	# en fazla kucuklme orani
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
		if progress < 0.0 or progress >= 1.0 or not owner.IsShow():	# progress < 0: isinlanmada oyun saati sifirlandi
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
		self.akrName = name
		try:	# TANI: isinlanmada dugme sayisi / boyutu
			import dbg
			dbg.TraceError("HUDBTN olustu %s size=%s suffix=%s scale=%.3f t=%.1f" % (name, size, self.suffix, self.assetScale, app.GetTime()))
		except Exception:
			pass
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
		self.flashStart = -10.0	# efektsiz basma geri bildirimi: dokununca dugme kisa sure kararir
		self.flashed = False

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
		self.__HideRunFx()

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
		if kind == "run" and not getattr(self, "runBubbles", None):
			glow = ui.ExpandedImageBox()
			glow.AddFlag("not_pick")
			glow.AddFlag("float")
			glow.LoadImage(HUD_PATH % "btn_run_glow")
			glow.SetScale(self.alertScale, self.alertScale)
			self.runGlow = glow
			self.runBubbles = []
			for index in range(len(RUN_BUBBLES) + len(RUN_OUT_BUBBLES)):
				bubble = ui.ExpandedImageBox()
				bubble.AddFlag("not_pick")
				bubble.AddFlag("float")
				bubble.LoadImage(HUD_PATH % "btn_run_dot")
				self.runBubbles.append(bubble)
		if not self.alert:
			return
		if kind:
			self.alert.Show()
			self.OnUpdate()
		else:
			self.alert.Hide()
			self.__HideRunFx()

	def EnablePressRipple(self, color = (1.0, 0.9, 0.6)):
		if not getattr(self, "ripple", None):
			self.ripple = PressRipple(color)
		else:
			self.ripple.color = color

	def __HideRunFx(self):
		glow = getattr(self, "runGlow", None)
		if glow:
			glow.Hide()
		for bubble in getattr(self, "runBubbles", None) or ():
			bubble.Hide()

	def __UpdateRunFx(self, left, top):
		# iksir kabarciklari: alttan yukari salinarak yukselir, ortada belirgin, uclarda solar; her karede kayar (akici)
		now = app.GetTime()
		glow = self.runGlow
		glow.SetPosition(left, top)
		wndMgr.SetDiffuseColor(glow.hWnd, RUN_COLOR[0], RUN_COLOR[1], RUN_COLOR[2], RUN_GLOW_ALPHA)
		glow.Show()
		scale = self.alertScale
		center = 88.0 * scale	# 176 tuvalin merkezi
		radius = 68.0 * scale	# dugme yaricapi
		for (index, bubble) in enumerate(self.runBubbles):
			if index >= len(RUN_BUBBLES):
				# dis kabarcik: kenardan disari, hafif yukari suzulerek
				(phase, period, angle, sizeFactor) = RUN_OUT_BUBBLES[index - len(RUN_BUBBLES)]
				t = ((now / period) + phase) % 1.0
				rad = math.radians(angle)
				distance = radius * (1.04 + 0.5 * t)
				x = distance * math.cos(rad) + radius * 0.05 * math.sin((t * 2.0 + phase) * 6.2832)
				y = distance * math.sin(rad) - radius * 0.08 * t
				alpha = (math.sin(t * 3.14159) ** 0.8) * 0.9
				if alpha <= 0.02:
					bubble.Hide()
					continue
				size = RUN_BUBBLE_ASSET * scale * 1.35 * sizeFactor * (1.0 - 0.3 * t)
				factor = size / RUN_BUBBLE_ASSET
				bubble.SetScale(factor, factor)
				bubble.SetPosition(int(left + center + x - size / 2.0), int(top + center + y - size / 2.0))
				wndMgr.SetDiffuseColor(bubble.hWnd, RUN_COLOR[0], RUN_COLOR[1], RUN_COLOR[2], alpha)
				bubble.Show()
				continue
			(phase, period, xFactor, sizeFactor, wobble) = RUN_BUBBLES[index]
			t = ((now / period) + phase) % 1.0
			y = radius * (0.86 - 1.72 * t)
			x = radius * (xFactor + wobble * math.sin((t * 1.6 + phase) * 6.2832))
			distance = math.sqrt(x * x + y * y) / radius
			edge = max(0.0, min(1.0, (0.94 - distance) / 0.26))
			alpha = (math.sin(t * 3.14159) ** 0.7) * edge * 0.95
			if alpha <= 0.02:
				bubble.Hide()
				continue
			size = RUN_BUBBLE_ASSET * scale * 0.55 * sizeFactor * (0.75 + 0.45 * t)
			factor = size / RUN_BUBBLE_ASSET
			bubble.SetScale(factor, factor)
			bubble.SetPosition(int(left + center + x - size / 2.0), int(top + center + y - size / 2.0))
			wndMgr.SetDiffuseColor(bubble.hWnd, RUN_COLOR[0], RUN_COLOR[1], RUN_COLOR[2], alpha)
			bubble.Show()

	def OnUpdate(self):
		# isinlanma / harita yuklemesi ikonu yeniden yukleyip olcegi 1.0'a sifirlayabilir (dugme dev gibi buyur): 0.5 sn'de bir olcek dogrulanir
		now = app.GetTime()
		if self.image and not getattr(self, "ripple", None) and not self.flashed and now - self.flashStart > PRESS_SINK_SECONDS and now - getattr(self, "scaleCheckAt", 0.0) > 0.5:
			self.scaleCheckAt = now
			if getattr(self, "akrName", "") == "inventory" and now - getattr(self, "akrLogAt", 0.0) > 2.0:
				self.akrLogAt = now
				try:
					import dbg
					(gx, gy) = self.GetGlobalPosition()
					dbg.TraceError("HUDINV t=%.1f pencere=%sx%s ikon=%sx%s scale=%.3f gorunur=%s konum=%s,%s durum=%s" % (now, self.GetWidth(), self.GetHeight(), self.image.GetWidth(), self.image.GetHeight(), self.assetScale, self.IsShow(), gx, gy, self.state))
				except Exception:
					pass
			try:
				if self.GetWidth() > 90 or self.image.GetWidth() > 90:
					import dbg
					dbg.TraceError("HUDBTN BUYUK %s pencere=%s ikon=%s scale=%.3f" % (getattr(self, "akrName", "?"), self.GetWidth(), self.image.GetWidth(), self.assetScale))
			except Exception:
				pass
			self.image.SetScale(self.assetScale, self.assetScale)
		ripple = getattr(self, "ripple", None)
		if ripple:
			ripple.Update(self)
		elif self.image:
			# basma: dugme ice dogru cokup (kararip) yumusakca geri gelir
			p = (app.GetTime() - self.flashStart) / PRESS_SINK_SECONDS
			if p < 0.0:	# isinlanmada oyun saati sifirlanir: eski basis zamani gelecekte kalir, olcek 95 kat buyurdu
				self.flashStart = -10.0
				p = 99.0
			if p < 1.0:
				if p < 0.35:
					depth = p / 0.35
				else:
					q = (p - 0.35) / 0.65
					depth = 1.0 - q + 0.08 * math.sin(q * 3.14159)
				factor = 1.0 - PRESS_SINK_DEPTH * depth
				shade = 1.0 - 0.3 * max(0.0, depth)
				self.flashed = True
				self.image.SetScale(self.assetScale * factor, self.assetScale * factor)
				offset = int(round(self.GetWidth() * (1.0 - factor) / 2.0))
				self.image.SetPosition(offset, offset)
				wndMgr.SetDiffuseColor(self.image.hWnd, shade, shade, shade, 1.0)
			elif self.flashed:
				self.flashed = False
				self.image.SetScale(self.assetScale, self.assetScale)
				self.image.SetPosition(0, 0)
				wndMgr.SetDiffuseColor(self.image.hWnd, 1.0, 1.0, 1.0, 1.0)
		if not self.alert:
			return
		kind = self.__RingKind()
		if ripple and self.image:
			# oto av calisirken dugmenin ici hafif yesilimsi
			tinted = (kind == "run")
			if tinted != getattr(self, "runTinted", False):
				self.runTinted = tinted
				if tinted:
					wndMgr.SetDiffuseColor(self.image.hWnd, RUN_TINT[0], RUN_TINT[1], RUN_TINT[2], 1.0)
				else:
					wndMgr.SetDiffuseColor(self.image.hWnd, 1.0, 1.0, 1.0, 1.0)
		if not kind or not self.IsShow():
			self.alert.Hide()
			self.__HideRunFx()
			return
		(x, y) = self.GetGlobalPosition()
		offset = int(round(20 * self.alertScale))
		self.alert.SetPosition(x - offset, y - offset)
		self.alert.Show()
		if getattr(self, "runBubbles", None):
			if kind == "run":
				self.__UpdateRunFx(x - offset, y - offset)
			else:
				self.__HideRunFx()
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
		if self.pressed:
			wndMgr.SetDiffuseColor(self.image.hWnd, 0.55, 0.55, 0.55, 1.0)
		else:
			wndMgr.SetDiffuseColor(self.image.hWnd, 1.0, 1.0, 1.0, 1.0)

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
		else:
			self.flashStart = now
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
		# AKRAVON: eski lacivert "bos kisim" resmi kaldirildi; yuva panel gorselinin icinde
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


class ExpSquares(object):
	# AKRAVON: HP/SP cubuklarinin hemen altinda 4 kare; her kare deneyimin %25'ini doldurur
	COUNT = 4
	WIDTH = 53
	GAP = 4
	HEIGHT = 16
	X = 0
	Y = 73

	def __init__(self, parent):
		self.ratio = -1.0
		self.fills = []
		self.parts = []	# pencere nesneleri saklanmazsa silinir
		for index in range(self.COUNT):
			x = self.X + index * (self.WIDTH + self.GAP)
			self.parts.append(HudImage(parent, "exp_slot", x, self.Y))
			fill = HudImage(parent, "exp_fill", x + 1, self.Y + 1)
			fill.Hide()
			self.fills.append(fill)

	FRAMES = 16
	FRAME_SECONDS = 0.07

	def Animate(self, now):
		# cok yumusak 'nefes alma' isiltisi: gorsel degistirmez, yalniz parlaklik (takilma olmaz)
		if self.ratio <= 0.0:
			return
		import math
		shade = 0.86 + 0.14 * (0.5 + 0.5 * math.sin(now * 2.2))
		for fill in self.fills:
			if fill.IsShow():
				wndMgr.SetDiffuseColor(fill.hWnd, shade, shade, shade, 1.0)
		return
		frame = int(now / self.FRAME_SECONDS) % self.FRAMES + 1
		if frame == getattr(self, "frame", 0):
			return
		self.frame = frame
		for (index, fill) in enumerate(self.fills):
			if fill.IsShow():
				part = max(0.0, min(1.0, self.ratio * self.COUNT - index))
				fill.LoadImage(HUD_PATH % ("exp_fill_%d" % frame))
				fill.SetScale(0.5, 0.5)
				fill.SetRenderingRect(0.0, 0.0, part - 1.0, 0.0)

	def SetValue(self, current, maximum):
		ratio = max(0.0, min(1.0, float(current) / maximum)) if maximum > 0 else 1.0
		if ratio == self.ratio:
			return ratio
		self.ratio = ratio
		for (index, fill) in enumerate(self.fills):
			part = max(0.0, min(1.0, ratio * self.COUNT - index))
			if part <= 0.0:
				fill.Hide()
				continue
			fill.SetRenderingRect(0.0, 0.0, part - 1.0, 0.0)
			fill.Show()
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
		self.expGauge = ExpSquares(self)	# AKRAVON: HP/SP altinda 4 EXP karesi

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
		for gauge in (self.hpGauge, self.spGauge, self.stGauge, self.expGauge):
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

		# AKRAVON: yalniz dugme - altin cerceveli lacivert kare + sohbet ikonu (mesajlar yanindaki seritte akar)
		self.edge = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(150, 118, 58), 0, 0, width, height)
		self.fill = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(12, 20, 40, 235), 2, 2, width - 4, height - 4)
		self.frame = None
		self.icon = HudImage(self, "chat_icon", (width - 22) // 2, (height - 22) // 2 + 1)
		self.title = None
		self.arrow = None
		self.arrowLeft = width - 14

		self.badge = HudImage(self, "chat_badge_dot", width - 22, 2)
		self.badge.Hide()
		self.badgeText = uiMobileStyle.CreateText(self, "", width - 22 + 11, 2 + 6, uiMobileStyle.BRIGHT_COLOR, "LatoBd:14", "center")
		self.badgeText.Hide()
		self.badgeValue = 0

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.frame = None
		self.edge = None
		self.fill = None
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
		self.boxHeight = 0

	def __del__(self):
		ui.Window.__del__(self)

	def SetBoxHeight(self, height):
		# serit yuksekligi: yalniz sigan satir sayisi gosterilir; satirlar seridin altindan yukari dizilir
		self.boxHeight = height

	def OnRender(self):
		(x, y) = self.GetGlobalPosition()
		if self.boxHeight > 0:
			step = max(1, chat.GetLineStep(self.chatID))
			chat.SetHeight(self.chatID, max(0, self.boxHeight - step))
			chat.ArrangeShowingChat(self.chatID)
			chat.SetPosition(self.chatID, x, y + self.boxHeight)
		else:
			chat.SetPosition(self.chatID, x, y)
		chat.Update(self.chatID)
		chat.Render(self.chatID)


class ChatStrip(uiMobileStyle.Pressable):
	# AKRAVON: sohbet dugmesinin sagindaki karartilmis serit; son mesajlar burada akar, dokununca panel acilir
	def __init__(self, width, height):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.back = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(0, 0, 0, 0), 0, 0, width, height)	# gorunmez dokunma alani: sabit siyah serit yok, mesajlar orijinal Metin2 gibi gelince gorunur
		self.line = None

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.back = None
		self.line = None


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
		self.strip = ChatStrip(min(uiMobileChat.FLOAT_WIDTH, STRIP_WIDTH), toggle.GetHeight() + 3 * 24)
		self.strip.SetEvent(ui.__mem_func__(self.Expand))
		self.strip.Show()
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
		x = max(SCREEN_GAP, min(wndMgr.GetScreenWidth() - SCREEN_GAP - uiMobileChat.PANEL_WIDTH, toggleX))	# panel dugmeden saga dogru acilir
		self.panel.SetHomePosition(x, toggleY + self.toggle.GetHeight())
		self.panel.SetCollapsedBox(toggleX, toggleY, self.toggle.GetWidth(), self.toggle.GetHeight())
		boxHeight = self.toggle.GetHeight() + 3 * 24
		self.strip.SetPosition(toggleX + self.toggle.GetWidth() + 6, toggleY + self.toggle.GetHeight() - boxHeight)
		self.floating.SetBoxHeight(boxHeight)
		self.floating.SetPosition(toggleX + self.toggle.GetWidth() + 12, toggleY + self.toggle.GetHeight() - boxHeight)

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
		if self.strip:
			self.strip.Destroy()
		self.strip = None
		self.panel = None
		self.toggle = None
		self.interface = None

	def Expand(self):
		self.expanded = True
		self.toggle.Hide()
		self.floating.Hide()
		self.strip.Hide()
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
			self.strip.Show()

	def Hide(self):
		self.visible = False
		if self.expanded:
			self.Collapse(False)
		elif self.panel:
			self.panel.CloseNow()
		self.toggle.Hide()
		self.floating.Hide()
		self.strip.Hide()

	def Show(self):
		self.visible = True
		if not self.expanded and not (self.panel and self.panel.IsShow()):
			self.toggle.Show()
			self.floating.Show()
			self.strip.Show()

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
