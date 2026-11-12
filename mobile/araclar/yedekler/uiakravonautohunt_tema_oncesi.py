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
W, H = 620, 500
SIDE = 150
X0 = 170
RW = 424
TRACK = RW - 28
GOLD = uiMobileStyle.Color(222, 184, 110)
TEXT = uiMobileStyle.Color(232, 226, 210)
DIM = uiMobileStyle.Color(140, 132, 116)
DARK = uiMobileStyle.Color(24, 16, 6)
TEXT_ON = uiMobileStyle.Color(255, 255, 255)
GREEN = uiMobileStyle.Color(120, 220, 120)
RED = uiMobileStyle.Color(200, 110, 96)
CFG = "akravon_otoav.cfg"
TABS = ("Av", "İksir", "Şebnem", "Sular")
DEWS = ((50821, "Kırmızı"), (50822, "Pembe"), (50823, "Sarı"), (50824, "Yeşil"), (50825, "Mavi"), (50826, "Beyaz"))
# AKRAVON: 2008-2011 sular (hepsi 3 dk = 180 sn etkili); oto av her 180 sn'de bir yeniden icer (orijinal kullanim suresi)
WATERS = ((50801, "Şeftali Çiçeği Suyu", "Fiziksel gücü artırır"), (50802, "Çanıçiçeği Suyu", "Gücü +5 artırır"), (50813, "Sim Suyu", "%10 isabet şansı"), (50814, "Dok Suyu", "%10 kritik vuruş şansı"),
	(50817, "Zin Suyu", "Saldırı değeri +50"), (50818, "Sambo Suyu", "Savunma +70"), (50819, "Mong Suyu", "Büyü savunması %5"), (50820, "Hwal Suyu", "Saldırı hızı +3"))
WATER_SECONDS = 180
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
	if interrupted and not IsRunning():
		return IsRunning()	# AKRAVON: joystick kesintisinin 4 sn sayimi bitmeden yeniden baslatilamaz (iOS + Android)
	CancelInterrupt()
	if IsRunning():
		player.AkravonAutoHuntStop()
	elif IsMounted():
		import chat
		chat.AppendChat(chat.CHAT_TYPE_INFO, "At üzerindeyken oto av kullanılamaz.")
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
		fontSize = max(20, int(size * 0.4))	# "3.9" sigsin
		self.count = uiMobileStyle.CreateText(self, "4.0", size // 2, size // 2 - int(fontSize * 0.55), uiMobileStyle.Color(255, 235, 230), "Marcellus:%d" % fontSize, "center", True)
		self.Show()

	def SetNumber(self, number):
		# AKRAVON: saniye.salise (beceri bekleme suresiyle ayni: 3.9)
		self.count.SetText("%.1f" % max(0.0, number))


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
	if IsRunning() and IsMounted():
		player.AkravonAutoHuntStop()	# ata binince oto av durur
		if panel is not None:
			panel.Refresh()
	if not joystickActive and IsRunning() and KeyboardMoving():
		joystickActive = True	# klavye ile hareket de joystick gibi oto avi bozar (emulator / klavyeli cihaz)
	if joystickActive and IsRunning():
		player.AkravonAutoHuntStop()
		interrupted = True
		countStart = now	# AKRAVON: sayim hareket ederken de akar (4'te takili kalmaz)
		if panel is not None:
			panel.Refresh()
	ripple = getattr(button, "ripple", None) if button is not None else None
	if ripple:
		ripple.color = (1.0, 0.25, 0.2) if interrupted else (0.45, 1.0, 0.45)	# sayim sirasinda basma efekti de kirmizi
	if not interrupted or button is None:
		return
	box = __Overlay(button)
	if countStart is None:
		countStart = now
	remaining = COUNTDOWN - (now - countStart)
	if remaining <= 0.0:
		CancelInterrupt()	# oto av kapali kalir; yeniden dokunarak acilir
		return
	box.SetNumber(remaining)
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


def KeyboardMoving():
	try:
		for name in ("DIK_W", "DIK_A", "DIK_S", "DIK_D", "DIK_UP", "DIK_DOWN", "DIK_LEFT", "DIK_RIGHT"):
			if app.IsPressed(getattr(app, name)):
				return True
	except Exception:
		pass
	return False


def IsMounted():
	try:
		return bool(player.IsMountingHorse())
	except Exception:
		return False


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
		self.dews = dict((vnum, [0, 0]) for (vnum, name) in DEWS)
		for (vnum, name, desc) in WATERS:
			self.dews[vnum] = [0, 0]	# vnum -> [acik, siklik sirasi]
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
			tail = ",".join("%d:%d:%d" % (vnum, self.dews[vnum][0], self.dews[vnum][1]) for vnum in [d[0] for d in DEWS] + [w[0] for w in WATERS])
			open(CFG, "w").write("%d %d %d %d|%s" % (self.hpOn, self.hp, self.spOn, self.sp, tail))
		except Exception:
			pass

	def DewArgument(self):
		parts = ["%d:0" % vnum for (vnum, name) in DEWS if self.dews[vnum][0]]
		parts += ["%d:%d" % (vnum, WATER_SECONDS) for (vnum, name, desc) in WATERS if self.dews[vnum][0]]
		return ",".join(parts)


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


VISIBLE_ROWS = 5


ROW_STEP = 68


class Scroller(ui.Window):
	# AKRAVON: karakter penceresindeki bonus listesi gibi dogrudan kaydirma - listeye basip parmagi surukle; kisa dokunus = secim
	def __init__(self, page, count, relayout, ontap):
		ui.Window.__init__(self)
		self.SetParent(page)
		self.SetPosition(0, 24)
		self.SetSize(RW, VISIBLE_ROWS * ROW_STEP)
		KEEP.append(self)
		self.offset = 0
		self.count = count
		self.relayout = relayout
		self.ontap = ontap
		self.dragging = False
		self.moved = False
		self.dragY = 0
		self.dragOffset = 0
		self.label = Text(page, "", RW // 2, 24 + VISIBLE_ROWS * ROW_STEP + 6, DIM, "Lato:15", "center")
		self.Refresh()
		self.Show()

	def Visible(self, index):
		return self.offset <= index < self.offset + VISIBLE_ROWS

	def Refresh(self):
		last = min(self.count, self.offset + VISIBLE_ROWS)
		self.label.SetText("%d - %d / %d   (parmakla kaydır)" % (self.offset + 1, last, self.count))

	def OnMouseLeftButtonDown(self):
		self.dragging = True
		self.moved = False
		self.dragY = wndMgr.GetMousePosition()[1]
		self.dragOffset = self.offset
		return True

	def OnUpdate(self):
		if not self.dragging:
			return
		delta = self.dragY - wndMgr.GetMousePosition()[1]
		if abs(delta) > 12:
			self.moved = True
		offset = max(0, min(max(0, self.count - VISIBLE_ROWS), self.dragOffset + int(round(delta / float(ROW_STEP)))))
		if offset != self.offset:
			self.offset = offset
			self.Refresh()
			self.relayout()

	def OnMouseLeftButtonUp(self):
		if self.dragging and not self.moved:
			(mouseX, mouseY) = wndMgr.GetMousePosition()
			(left, top) = self.GetGlobalPosition()
			index = self.offset + int((mouseY - top) // ROW_STEP)
			if 0 <= index < self.count and (mouseY - top) % ROW_STEP < 62:
				self.ontap(index, mouseX - left)
		self.dragging = False
		return True

	def Destroy(self):
		self.relayout = None
		self.ontap = None
		self.Hide()


class Slider(ui.Window):
	# dokunulan yere gore %10..%90 (10'ar adim); surukleme de calisir
	def __init__(self, parent, x, y, onChange):
		ui.Window.__init__(self)
		self.SetParent(parent)
		self.SetPosition(x, y)
		self.SetSize(TRACK, 30)
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
		self.knob.SetPosition(int(ratio * (TRACK - 22)), 4)

	def __FromMouse(self):
		(mouseX, mouseY) = wndMgr.GetMousePosition()
		(left, top) = self.GetGlobalPosition()
		ratio = max(0.0, min(1.0, (mouseX - left - 11) / float(TRACK - 22)))
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
		self.board = Image(self, "oa_board", 0, 0)
		Text(self, "Otomatik Av", W // 2, 13, GOLD, "Marcellus:22", "center")
		KEEP.append(uiMobileStyle.CreateBar(self, uiMobileStyle.Color(96, 74, 42), 16, 52, W - 32, 1))
		import uiAkravonInventory
		self.close = uiAkravonInventory.CloseButton(self, W - 46, 12, self.Close)	# kare X (diger pencerelerle ayni)
		KEEP.append(self.close)
		KEEP.append(uiMobileStyle.CreateBar(self, uiMobileStyle.Color(96, 74, 42), SIDE, 66, 1, H - 90))

		self.tabs = []
		for (index, name) in enumerate(TABS):
			# AKRAVON: karakter penceresi kenar sekmesi (altin dolgu / lacivert), ok yok
			tab = Button(self, 14, 70 + index * 44, 122, 40, "oa_tab%d_off" % index)
			tab.SetEvent(self.SelectTab, index)
			self.tabs.append(tab)

		self.pages = [self.__BuildHunt(), self.__BuildPotion(), self.__BuildDew(), self.__BuildWater()]
		self.SelectTab(0)
		self.Refresh()

	# --- sayfalar ---
	def __Page(self):
		page = ui.Window()
		page.SetParent(self)
		page.AddFlag("not_pick")
		page.SetPosition(X0, 66)
		page.SetSize(RW, H - 90)
		return page

	def __Row(self, page, y, title, sub, onTap):
		row = Button(page, 0, y, RW, 50, "oa_row")
		Text(row, title, 14, 5, TEXT, "Lato:17")
		Text(row, sub, 14, 27, DIM, "Lato:15")
		row.toggle = Image(row, "oa_toggle_off", RW - 62, 12)
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
		Text(page, "İşaretli olanlar süresi bitince kendiliğinden kullanılır", 0, 2, GOLD, "LatoBd:15")
		self.dewCards = []
		self.dewScroll = Scroller(page, len(DEWS), self.__LayoutDews, self.__TapDew)
		for (index, (vnum, name)) in enumerate(DEWS):
			# AKRAVON: tek sutun satir - satira dokun = aktif / pasif, sagdaki kutu = siklik
			y = index * 68
			card = Button(self.dewScroll, 0, y, RW, 62, "oa_dew_off")
			card.AddFlag("not_pick")
			icon = Image(card, "icon/item/%d.tga" % vnum, 12, 15)
			check = Image(card, "oa_check", 36, 9)
			title = Text(card, name + " Şebnem", 62, 8, TEXT, "Lato:19")
			state = Text(card, "", 62, 34, GREEN, "LatoBd:16")
			Text(card, "Süresi bitince yeniden kullanılır", RW - 16, 20, DIM, "Lato:16", "right")
			chipText = None
			self.dewCards.append((vnum, card, icon, check, title, state, chipText))
		self.__LayoutDews()
		return page

	def __LayoutDews(self):
		for (i, entry) in enumerate(self.dewCards):
			card = entry[1]
			shown = self.dewScroll.Visible(i)
			(card.Show if shown else card.Hide)()
			if shown:
				y = (i - self.dewScroll.offset) * 68
				card.SetPosition(0, y)

	def __TapDew(self, index, x):
		vnum = DEWS[index][0]
		self.__ToggleDew(vnum)

	def __TapWater(self, index, x):
		self.__ToggleDew(WATERS[index][0])

	def __LayoutWaters(self):
		for (i, entry) in enumerate(self.waterCards):
			card = entry[1]
			shown = self.waterScroll.Visible(i)
			(card.Show if shown else card.Hide)()
			if shown:
				card.SetPosition(0, (i - self.waterScroll.offset) * 68)

	def __BuildWater(self):
		page = self.__Page()
		Text(page, "Kullanmak istediğine dokun  ·  etki süresi 3 dk, bitince yeniden içilir", 0, 2, GOLD, "LatoBd:15")
		self.waterCards = []
		self.waterScroll = Scroller(page, len(WATERS), self.__LayoutWaters, self.__TapWater)
		for (index, (vnum, name, desc)) in enumerate(WATERS):
			y = index * 68
			card = Button(self.waterScroll, 0, y, RW, 62, "oa_dew_off")
			card.AddFlag("not_pick")
			icon = Image(card, "icon/item/%d.tga" % vnum, 12, 15)
			check = Image(card, "oa_check", 36, 9)
			title = Text(card, name, 62, 8, TEXT, "Lato:19")
			state = Text(card, "", 62, 34, GREEN, "LatoBd:16")
			Text(card, desc + "  ·  3 dk", RW - 16, 20, DIM, "Lato:16", "right")
			self.waterCards.append((vnum, card, icon, check, title, state))
		self.__LayoutWaters()
		return page

	# --- olaylar ---
	def SelectTab(self, index):
		self.tab = index
		for (i, tab) in enumerate(self.tabs):
			active = i == index
			tab.SetImages("oa_tab%d_on" % i if active else "oa_tab%d_off" % i)
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
		for (vnum, card, icon, check, title, state) in self.waterCards:
			on = s.dews[vnum][0]
			card.SetImages("oa_dew_on" if on else "oa_dew_off")
			icon.SetDiffuseColor(*((1.0, 1.0, 1.0, 1.0) if on else (0.45, 0.45, 0.45, 0.7)))
			if on:
				check.Show()
			else:
				check.Hide()
			title.SetPackedFontColor(TEXT if on else DIM)
			state.SetText("AKTİF" if on else "PASİF")
			state.SetPackedFontColor(GREEN if on else RED)
		parts = []
		parts.append("HP %%%d" % s.hp if s.hpOn else "HP kapalı")
		parts.append("SP %%%d" % s.sp if s.spOn else "SP kapalı")
		parts.append("%d şebnem/su" % sum(1 for v in s.dews.values() if v[0]))
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
