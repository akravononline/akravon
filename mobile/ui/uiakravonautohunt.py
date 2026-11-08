# -*- coding: utf-8 -*-
# AKRAVON: mobil oto av paneli (yalniz canavar; metin / patron / toplama / otomatik dirilme yok)
# motor: player.AkravonAutoHunt(hpYuzde, spYuzde, "vnum:sn,...") / player.AkravonAutoHuntStop() / player.IsAutoHunting()
from __future__ import division
import app
import player
import wndMgr
import ui
import uiMobileStyle

IMG = "d:/ymir work/ui/akravon_legacy/"
W, H = 540, 476
SIDE = 150
X0 = 170
RW = 346
GOLD = uiMobileStyle.Color(222, 184, 110)
TEXT = uiMobileStyle.Color(232, 226, 210)
DIM = uiMobileStyle.Color(140, 132, 116)
DARK = uiMobileStyle.Color(24, 16, 6)
GREEN = uiMobileStyle.Color(120, 220, 120)
RED = uiMobileStyle.Color(200, 110, 96)
CFG = "akravon_otoav.cfg"
TABS = ("Av", "İksir", "Şebnem")
DEWS = ((50821, "Kırmızı"), (50822, "Pembe"), (50823, "Sarı"), (50824, "Yeşil"), (50825, "Mavi"), (50826, "Beyaz"))
FREQS = ((0, "Süre bitince"), (300, "Her 5 dk"), (600, "Her 10 dk"), (900, "Her 15 dk"))
DEATH_SECONDS = 60

panel = None


def Toggle():
	global panel
	if panel is None:
		try:
			panel = AutoHuntPanel()
		except Exception:
			import traceback
			open("akravon_hata.txt", "w").write(traceback.format_exc())
			raise
	if panel.IsShow():
		panel.Close()
	else:
		panel.Open()


def ToggleHunt():
	# AKRAVON: HUD oto av dugmesi - dogrudan baslat / durdur (ayarlar kaydedilmis dosyadan)
	CancelInterrupt()
	if IsRunning():
		player.AkravonAutoHuntStop()
	else:
		s = Settings()
		player.AkravonAutoHunt(s.hp if s.hpOn else 0, s.sp if s.spOn else 0, s.DewArgument())
	if panel is not None:
		panel.Refresh()
	return IsRunning()


# ---------------------------------------------------------------- joystick kesintisi
# Oto av acikken joystick'e dokunulursa oto av durur; yuvarlak oto av dugmesinin icinde kirmizi 4'ten geri sayim.
# Sayim bitince oto av KENDILIGINDEN BASLAMAZ: tekrar istenirse dugmeye yeniden dokunulur.
COUNTDOWN = 4.0
interrupted = False
countStart = None
overlay = None
overlayOwner = None


class CountdownOverlay(ui.Window):
	def __init__(self, button):
		ui.Window.__init__(self, "UI")
		size = button.GetWidth()
		self.SetParent(button)
		self.AddFlag("not_pick")
		self.SetSize(size, size)
		self.SetPosition(0, 0)
		self.disc = ui.ExpandedImageBox()
		self.disc.SetParent(self)
		self.disc.AddFlag("not_pick")
		self.disc.LoadImage("mobile_ui/akravon/hud/hud_lock_disc.png")
		self.disc.SetScale(size / 128.0, size / 128.0)
		self.disc.Show()
		wndMgr.SetDiffuseColor(self.disc.hWnd, 0.85, 0.05, 0.05, 0.78)
		self.count = uiMobileStyle.CreateText(self, "4", size // 2, size // 2 - int(size * 0.28), uiMobileStyle.Color(255, 235, 230), "Marcellus:%d" % max(26, int(size * 0.55)), "center", True)
		self.Show()

	def SetNumber(self, number):
		self.count.SetText(str(number))


def __Overlay(button):
	global overlay, overlayOwner
	if overlay is None or overlayOwner is not button:
		if overlay is not None:
			overlay.Hide()
		overlay = CountdownOverlay(button)
		overlayOwner = button
	return overlay


def CancelInterrupt():
	global interrupted, countStart
	interrupted = False
	countStart = None
	if overlay is not None:
		overlay.Hide()


def Update(joystickActive, button = None):
	# her karede (HUD denetleyicisi): joystick kullanimi oto avi bozar; yuvarlak oto av dugmesinde 4..1 kirmizi sayim, sonra normal
	global interrupted, countStart
	now = app.GetTime()
	if joystickActive and IsRunning():
		player.AkravonAutoHuntStop()
		interrupted = True
		countStart = None
		if panel is not None:
			panel.Refresh()
	if not interrupted or button is None:
		return
	box = __Overlay(button)
	if joystickActive:
		countStart = None
		box.SetNumber(int(COUNTDOWN))
		box.Show()
		return
	if countStart is None:
		countStart = now
	remaining = COUNTDOWN - (now - countStart)
	if remaining <= 0.0:
		CancelInterrupt()	# oto av kapali kalir; yeniden dokunarak acilir
		return
	box.SetNumber(int(remaining) + 1 if remaining > int(remaining) else int(remaining))
	box.Show()


def Destroy():
	global panel
	global overlay, overlayOwner
	if overlay is not None:
		overlay.Hide()
	overlay = None
	overlayOwner = None
	if panel is not None:
		panel.Destroy()
	panel = None


def IsRunning():
	try:
		return player.IsAutoHunting()
	except Exception:
		return 0


KEEP = []	# AKRAVON: ui nesneleri python'da tutulmazsa motor da siler


def Text(*args):
	line = uiMobileStyle.CreateText(*args)
	KEEP.append(line)
	return line


def Image(parent, name, x, y):
	image = ui.ExpandedImageBox()
	KEEP.append(image)
	image.SetParent(parent)
	image.AddFlag("not_pick")
	image.LoadImage(name if "/" in name else IMG + name + ".png")
	image.SetPosition(x, y)
	image.Show()
	return image


class Settings(object):
	def __init__(self):
		self.hpOn, self.hp, self.spOn, self.sp = 1, 40, 1, 30
		self.dews = dict((vnum, [0, 0]) for (vnum, name) in DEWS)	# vnum -> [acik, siklik sirasi]
		try:
			(head, tail) = open(CFG).read().strip().split("|")
			(self.hpOn, self.hp, self.spOn, self.sp) = [int(v) for v in head.split()]
			for part in tail.split(","):
				(vnum, on, freq) = [int(v) for v in part.split(":")]
				if vnum in self.dews:
					self.dews[vnum] = [on, max(0, min(len(FREQS) - 1, freq))]
		except Exception:
			pass

	def Save(self):
		try:
			tail = ",".join("%d:%d:%d" % (vnum, self.dews[vnum][0], self.dews[vnum][1]) for (vnum, name) in DEWS)
			open(CFG, "w").write("%d %d %d %d|%s" % (self.hpOn, self.hp, self.spOn, self.sp, tail))
		except Exception:
			pass

	def DewArgument(self):
		return ",".join("%d:%d" % (vnum, FREQS[self.dews[vnum][1]][0]) for (vnum, name) in DEWS if self.dews[vnum][0])


class Button(uiMobileStyle.Pressable):
	# resimli dokunma alani: normal / basili resmi
	def __init__(self, parent, x, y, width, height, normal = None, down = None):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.SetParent(parent)
		self.SetPosition(x, y)
		KEEP.append(self)
		self.images = (normal, down or normal)
		self.image = Image(self, normal, 0, 0) if normal else None
		self.Show()

	def SetImages(self, normal, down = None):
		self.images = (normal, down or normal)
		self.RefreshVisual()

	def RefreshVisual(self):
		if self.image:
			self.image.LoadImage(IMG + self.images[1 if self.pressed else 0] + ".png")

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.image = None


class Slider(ui.Window):
	# dokunulan yere gore %10..%90 (10'ar adim); surukleme de calisir
	def __init__(self, parent, x, y, onChange):
		ui.Window.__init__(self)
		self.SetParent(parent)
		self.SetPosition(x, y)
		self.SetSize(318, 30)
		self.onChange = onChange
		self.dragging = False
		self.value = 50
		Image(self, "oa_track", 0, 11)
		self.fill = Image(self, "oa_fill", 0, 11)
		self.knob = Image(self, "oa_knob", 0, 4)
		self.Show()

	def SetValue(self, value):
		self.value = max(10, min(90, value))
		ratio = (self.value - 10) / 80.0
		self.fill.SetRenderingRect(0.0, 0.0, ratio - 1.0, 0.0)
		self.knob.SetPosition(int(ratio * 296), 4)

	def __FromMouse(self):
		(mouseX, mouseY) = wndMgr.GetMousePosition()
		(left, top) = self.GetGlobalPosition()
		ratio = max(0.0, min(1.0, (mouseX - left - 11) / 296.0))
		value = 10 + int(round(ratio * 8)) * 10
		if value != self.value:
			self.SetValue(value)
			self.onChange(value)

	def OnMouseLeftButtonDown(self):
		self.dragging = True
		self.__FromMouse()
		return True

	def OnMouseLeftButtonUp(self):
		self.dragging = False
		return True

	def OnUpdate(self):
		if self.dragging:
			self.__FromMouse()

	def Destroy(self):
		self.onChange = None


class AutoHuntPanel(ui.Window):
	def __init__(self):
		ui.Window.__init__(self)
		self.AddFlag("float")
		self.AddFlag("movable")
		self.SetSize(W, H)
		self.SetPosition((wndMgr.GetScreenWidth() - W) // 2, (wndMgr.GetScreenHeight() - H) // 2)
		self.settings = Settings()
		self.children = []
		self.board = Image(self, "cw_board", 0, 0)
		Text(self, "Otomatik Av", W // 2, 16, GOLD, "Marcellus:24", "center")
		self.close = Button(self, W - 44, 10, 36, 36)
		Image(self.close, "close_1", 5, 5)
		self.close.SetEvent(self.Close)
		uiMobileStyle.CreateBar(self, uiMobileStyle.Color(96, 74, 42), SIDE, 46, 1, H - 66)

		self.tabs = []
		for (index, name) in enumerate(TABS):
			# AKRAVON: karakter penceresi kenar sekmesi (altin dolgu / lacivert), ok yok
			tab = Button(self, 14, 60 + index * 52, 122, 42, "oa_tab_off")
			tab.label = Text(tab, name, 61, 11, TEXT, "LatoBd:17", "center")
			tab.SetEvent(self.SelectTab, index)
			self.tabs.append(tab)

		self.pages = [self.__BuildHunt(), self.__BuildPotion(), self.__BuildDew()]
		self.SelectTab(0)
		self.Refresh()

	# --- sayfalar ---
	def __Page(self):
		page = ui.Window()
		page.SetParent(self)
		page.AddFlag("not_pick")
		page.SetPosition(X0, 52)
		page.SetSize(RW, 320)
		return page

	def __Row(self, page, y, title, sub, onTap):
		row = Button(page, 0, y, RW, 50, "oa_row")
		Text(row, title, 14, 5, TEXT, "Lato:17")
		Text(row, sub, 14, 27, DIM, "Lato:15")
		row.toggle = Image(row, "oa_toggle_off", RW - 60, 12)
		row.SetEvent(onTap)
		return row

	def __BuildHunt(self):
		page = self.__Page()
		row = Button(page, 0, 8, RW, 50, "oa_row")
		Text(row, "Yakındaki canavarlara saldırır", 14, 6, TEXT, "Lato:17")
		Text(row, "Metin taşı ve patronlara saldırmaz", 14, 27, DIM, "Lato:15")
		row = Button(page, 0, 66, RW, 50, "oa_row")
		Text(row, "Ölünce şehre döner", 14, 6, TEXT, "Lato:17")
		Text(row, "%d sn içinde dirilmezsen" % DEATH_SECONDS, 14, 27, DIM, "Lato:15")
		Text(page, "Yalnızca envanterdeki iksir ve şebnemler kullanılır.", 0, 132, DIM, "Lato:15")
		self.summary = Text(page, "", 0, 152, GOLD, "Lato:15")
		self.status = Text(page, "", 0, 190, GREEN, "LatoBd:15")
		return page

	def __BuildPotion(self):
		page = self.__Page()
		self.potionRows = []
		for (index, (label, attribute)) in enumerate((("HP iksiri", "hp"), ("SP iksiri", "sp"))):
			y = index * 140 + 8
			row = self.__Row(page, y, label + " kullan", "Değer düşünce envanterden içer", self.__TogglePotion)
			row.SetEvent(self.__TogglePotion, attribute)
			box = Image(page, "oa_row_tall", 0, y + 58)
			Text(box, "%s yüzdesi" % label.split()[0], 14, 8, TEXT, "Lato:15")
			value = Text(box, "", RW - 14, 8, GOLD, "LatoBd:16", "right")
			slider = Slider(page, 14, y + 58 + 28, lambda v, a = attribute: self.__SetPercent(a, v))
			self.potionRows.append((attribute, row, value, slider))
		return page

	def __BuildDew(self):
		page = self.__Page()
		Text(page, "Kullanmak istediğine dokun", 0, 2, GOLD, "LatoBd:15")
		self.dewCards = []
		for (index, (vnum, name)) in enumerate(DEWS):
			# AKRAVON: tek sutun satir - satira dokun = aktif / pasif, sagdaki kutu = siklik
			y = 24 + index * 48
			card = Button(page, 0, y, RW, 44, "oa_dew_off")
			icon = Image(card, "icon/item/%d.tga" % vnum, 8, 6)
			check = Image(card, "oa_check", 30, 2)
			title = Text(card, name + " Şebnem", 50, 4, TEXT, "Lato:16")
			state = Text(card, "", 50, 23, GREEN, "LatoBd:15")
			card.SetEvent(self.__ToggleDew, vnum)
			chip = Button(page, RW - 118, y + 8, 110, 28, "oa_chip")
			chipText = Text(chip, "", 55, 5, GOLD, "Lato:15", "center")
			chip.SetEvent(self.__NextFreq, vnum)
			self.dewCards.append((vnum, card, icon, check, title, state, chipText))
		return page

	# --- olaylar ---
	def SelectTab(self, index):
		self.tab = index
		for (i, tab) in enumerate(self.tabs):
			active = i == index
			tab.SetImages("oa_tab_on" if active else "oa_tab_off")
			tab.label.SetPackedFontColor(DARK if active else TEXT)
		for (i, page) in enumerate(self.pages):
			if i == index:
				page.Show()
			else:
				page.Hide()

	def __TogglePotion(self, attribute):
		key = attribute + "On"
		setattr(self.settings, key, 0 if getattr(self.settings, key) else 1)
		self.__Changed()

	def __SetPercent(self, attribute, value):
		setattr(self.settings, attribute, value)
		self.__Changed()

	def __ToggleDew(self, vnum):
		self.settings.dews[vnum][0] ^= 1
		self.__Changed()

	def __NextFreq(self, vnum):
		self.settings.dews[vnum][1] = (self.settings.dews[vnum][1] + 1) % len(FREQS)
		self.__Changed()

	def __Changed(self):
		self.settings.Save()
		if IsRunning():
			self.__Apply()
		self.Refresh()

	def __Apply(self):
		s = self.settings
		player.AkravonAutoHunt(s.hp if s.hpOn else 0, s.sp if s.spOn else 0, s.DewArgument())

	def __OnStart(self):
		ToggleHunt()
		self.Refresh()

	def Refresh(self):
		s = self.settings
		for (attribute, row, value, slider) in self.potionRows:
			on = getattr(s, attribute + "On")
			row.toggle.LoadImage(IMG + ("oa_toggle_on" if on else "oa_toggle_off") + ".png")
			value.SetText("%%%d" % getattr(s, attribute))
			value.SetPackedFontColor(GOLD if on else DIM)
			slider.SetValue(getattr(s, attribute))
		for (vnum, card, icon, check, title, state, chipText) in self.dewCards:
			(on, freq) = s.dews[vnum]
			card.SetImages("oa_dew_on" if on else "oa_dew_off")
			icon.SetDiffuseColor(*((1.0, 1.0, 1.0, 1.0) if on else (0.45, 0.45, 0.45, 0.7)))
			if on:
				check.Show()
			else:
				check.Hide()
			title.SetPackedFontColor(TEXT if on else DIM)
			state.SetText("AKTİF" if on else "PASİF")
			state.SetPackedFontColor(GREEN if on else RED)
			chipText.SetText("< %s >" % FREQS[freq][1])
			chipText.SetPackedFontColor(GOLD if on else DIM)
		parts = []
		parts.append("HP %%%d" % s.hp if s.hpOn else "HP kapalı")
		parts.append("SP %%%d" % s.sp if s.spOn else "SP kapalı")
		parts.append("%d şebnem" % sum(1 for v in s.dews.values() if v[0]))
		self.summary.SetText("  ·  ".join(parts))
		running = IsRunning()
		self.status.SetText("Oto Av çalışıyor - HUD düğmesinden durdurabilirsin" if running else "Başlatmak için ekrandaki Oto Av düğmesine bas")
		self.status.SetPackedFontColor(GREEN if running else DIM)

	def OnUpdate(self):
		running = IsRunning()
		if running != getattr(self, "lastRunning", None):
			self.lastRunning = running
			self.Refresh()

	def Open(self):
		self.Refresh()
		self.Show()
		self.SetTop()

	def Close(self):
		self.Hide()

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def Destroy(self):
		del KEEP[:]
		for page in getattr(self, "pages", []):
			page.Hide()
		self.Hide()
		self.pages = []
		self.potionRows = []
		self.dewCards = []
