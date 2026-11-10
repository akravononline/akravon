# -*- coding: utf-8 -*-
# AKRAVON: gorev kutusu - soldaki kagit mektuplar yerine tek yuvarlak gorev dugmesi (ferman ikonu) + sayi rozeti;
# yeni gorev gelince kirmizi uyari halkasi; basinca pencerelerle ayni tahtada "Gorevler" tablosu acilir,
# satira basinca oyunun kendi gorevi (event.QuestButtonClick) calisir.
import app
import ui
import wndMgr
import uiMobileStyle
import uiMobileChrome
import uiMobileHud
import uiAkravon
import quest

from _weakref import proxy

BOARD_PATH = "mobile_ui/akravon/hud/menu_board_%s.png"
BOARD_CORNER = 12
BUTTON_SIZE = 52
BUTTON_TOP_RATIO = 0.29	# eski mektuplarin yeri (170 / 600)
BOARD_WIDTH = 480
BOARD_PAD = 16
TITLE_HEIGHT = 54
ROW_HEIGHT = 62
ROW_GAP = 6
MAX_ROWS = 6
SCROLL_STEP = 2
SCROLL_W = 44
TITLE_TEXT = "Görevler"
LABEL_TEXT = "Görev"
EMPTY_TEXT = "Bekleyen görev yok"
TITLE_FONT = "Marcellus:22"	# depo / nesne market basligiyla ayni
ROW_FONT = "Lato:17"
NAME_FONT = "LatoBd:19"
INFO_FONT = "Lato:16"
VALUE_FONT = "Marcellus:28"
LABEL_FONT = "LatoBd:14"
LABEL_COLOR = uiMobileStyle.Color(250, 228, 176)
ICON_PATH = "mobile_ui/akravon/hud/btn_quest_normal_s.png"
ICON_ASSET = 72.0
ICON_SIZE = 30
INVENTORY_IMG = "d:/ymir work/ui/akravon_legacy/"

hud = None


class Board9(ui.Window):
	# envanter / ok tablosu tahtasiyla ayni cizim (9 parca, kose 12 px)
	def __init__(self):
		ui.Window.__init__(self)
		self.pieces = {}
		self.sizes = {}
		for key in ("c", "t", "b", "l", "r", "tl", "tr", "bl", "br"):
			image = ui.ExpandedImageBox()
			image.SetParent(self)
			image.AddFlag("not_pick")
			image.LoadImage(BOARD_PATH % key)
			image.Show()
			self.pieces[key] = image
			self.sizes[key] = (max(1, image.GetWidth()), max(1, image.GetHeight()))

	def __del__(self):
		ui.Window.__del__(self)

	def SetSize(self, width, height):
		ui.Window.SetSize(self, width, height)
		c = BOARD_CORNER
		mw = max(1, width - c * 2)
		mh = max(1, height - c * 2)
		for (key, x, y, w, h) in (("c", c, c, mw, mh), ("t", c, 0, mw, c), ("b", c, height - c, mw, c),
				("l", 0, c, c, mh), ("r", width - c, c, c, mh), ("tl", 0, 0, c, c), ("tr", width - c, 0, c, c),
				("bl", 0, height - c, c, c), ("br", width - c, height - c, c, c)):
			image = self.pieces[key]
			(aw, ah) = self.sizes[key]
			image.SetPosition(x, y)
			image.SetScale(float(w) / aw, float(h) / ah)


class QuestRow(uiMobileStyle.Pressable):
	# genis dikdortgen satir: ustte gorev adi, altta ne yapilacagi / kalan sayi, sagda buyuk sayac
	def __init__(self, parent, width):
		uiMobileStyle.Pressable.__init__(self, width, ROW_HEIGHT)
		self.SetParent(parent)
		self.rowWidth = width
		self.box = uiAkravon.Box()
		self.box.SetParent(self)
		self.box.SetSize(width, ROW_HEIGHT)
		self.box.Show()
		uiAkravon.SkinBox(self.box, "picker_row_normal", "plate_gold_normal")
		self.name = uiMobileStyle.CreateText(self, "", 18, 11, uiMobileStyle.Color(250, 236, 210), NAME_FONT)
		self.info = uiMobileStyle.CreateText(self, "", 18, 38, uiMobileStyle.Color(222, 184, 110), INFO_FONT)
		self.value = uiMobileStyle.CreateText(self, "", width - 20, 18, uiMobileStyle.Color(255, 214, 120), VALUE_FONT, "right")
		self.entry = None
		self.nameText = None

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.box = None
		self.name = None
		self.info = None
		self.value = None
		self.entry = None
		self.Hide()

	def SetWidth(self, width):
		if width == self.rowWidth:
			return
		self.rowWidth = width
		self.SetSize(width, ROW_HEIGHT)
		self.box.SetSize(width, ROW_HEIGHT)
		self.value.SetPosition(width - 20, 18)
		self.nameText = None

	def RefreshVisual(self):
		if getattr(self, "box", None):
			uiAkravon.SetSkinState(self.box, self.pressed)

	def Fit(self, text, font, limit):
		if uiMobileStyle.TextWidth(text, font) <= limit:
			return text
		while len(text) > 1 and uiMobileStyle.TextWidth(text + "..", font) > limit:
			text = text[:-1]
		return text + ".."

	def Bind(self, entry):
		self.entry = entry
		(kind, key, name, info, value) = entry
		valueWidth = uiMobileStyle.TextWidth(value, VALUE_FONT) + 16 if value else 0
		limit = self.rowWidth - 36 - valueWidth
		if name != self.nameText:
			self.nameText = name
			self.name.SetText(self.Fit(name, NAME_FONT, limit))
		self.info.SetText(self.Fit(info, INFO_FONT, limit))
		self.value.SetText(value)


class ArrowButton(uiMobileStyle.Pressable):
	def __init__(self, parent, x, y, prefix, event):
		uiMobileStyle.Pressable.__init__(self, SCROLL_W, SCROLL_W)
		self.SetParent(parent)
		self.SetPosition(x, y)
		self.prefix = prefix
		self.image = ui.ExpandedImageBox()
		self.image.SetParent(self)
		self.image.AddFlag("not_pick")
		self.image.LoadImage(INVENTORY_IMG + prefix + "_1.png")
		self.image.Show()
		self.SetEvent(event)

	def RefreshVisual(self):
		if getattr(self, "image", None):
			self.image.LoadImage(INVENTORY_IMG + self.prefix + ("_3.png" if self.pressed else "_1.png"))

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.image = None


class QuestBoard(ui.Window):
	def __init__(self, owner):
		ui.Window.__init__(self, "UI")
		self.owner = owner
		self.board = Board9()
		self.board.SetParent(self)
		self.board.AddFlag("not_pick")
		self.board.Show()
		self.title = uiMobileStyle.CreateText(self, TITLE_TEXT, BOARD_WIDTH // 2, 13, uiMobileStyle.Color(222, 184, 110), TITLE_FONT, "center")
		self.title.SetPosition(BOARD_WIDTH // 2, 15)
		self.divider = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(96, 74, 42), BOARD_PAD, TITLE_HEIGHT, BOARD_WIDTH - BOARD_PAD * 2, 1)
		import uiAkravonInventory
		self.close = uiAkravonInventory.CloseButton(self, BOARD_WIDTH - BOARD_PAD - 34, 11, ui.__mem_func__(self.Hide))
		self.empty = uiMobileStyle.CreateText(self, EMPTY_TEXT, BOARD_WIDTH // 2, TITLE_HEIGHT + 24, uiMobileStyle.TEXT_COLOR, ROW_FONT, "center")
		self.rows = []
		self.entries = []
		self.signature = None
		self.offset = 0
		listWidth = BOARD_WIDTH - BOARD_PAD * 2 - SCROLL_W - 10
		self.listHeight = MAX_ROWS * (ROW_HEIGHT + ROW_GAP) - ROW_GAP
		for index in range(MAX_ROWS):
			row = QuestRow(self, listWidth)
			row.SetPosition(BOARD_PAD, TITLE_HEIGHT + 12 + index * (ROW_HEIGHT + ROW_GAP))
			row.SetEvent(ui.__mem_func__(self.__OnRow), index)
			self.rows.append(row)
		scrollX = BOARD_WIDTH - BOARD_PAD - SCROLL_W
		self.upButton = ArrowButton(self, scrollX, TITLE_HEIGHT + 12, "qs_up", ui.__mem_func__(self.ScrollUp))
		self.downButton = ArrowButton(self, scrollX, TITLE_HEIGHT + 12 + self.listHeight - SCROLL_W, "qs_dn", ui.__mem_func__(self.ScrollDown))
		trackTop = TITLE_HEIGHT + 12 + SCROLL_W + 6
		self.trackHeight = self.listHeight - (SCROLL_W + 6) * 2
		self.track = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(10, 16, 32, 220), scrollX + SCROLL_W // 2 - 3, trackTop, 6, self.trackHeight)
		self.thumb = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(176, 140, 74, 255), scrollX + SCROLL_W // 2 - 3, trackTop, 6, 20)
		self.trackTop = trackTop
		self.counter = uiMobileStyle.CreateText(self, "", BOARD_WIDTH // 2, 0, uiAkravon.HINT_COLOR, INFO_FONT, "center")
		self.SetSize(BOARD_WIDTH, TITLE_HEIGHT + 12 + self.listHeight + BOARD_PAD + 18)
		self.board.SetSize(BOARD_WIDTH, TITLE_HEIGHT + 12 + self.listHeight + BOARD_PAD + 18)
		self.counter.SetPosition(BOARD_WIDTH // 2, TITLE_HEIGHT + 12 + self.listHeight + 6)
		self.Hide()

	def __del__(self):
		ui.Window.__del__(self)

	def SetEntries(self, entries):
		signature = [(entry[0], entry[1], entry[2]) for entry in entries]
		if signature != self.signature:
			self.signature = signature
			self.offset = 0
		self.entries = entries
		self.Layout()

	def __Scroll(self, step):
		self.offset += step
		self.Layout()

	def ScrollUp(self):
		self.__Scroll(-SCROLL_STEP)

	def ScrollDown(self):
		self.__Scroll(SCROLL_STEP)

	def OnRunMouseWheel(self, length):
		self.__Scroll(-1 if length > 0 else 1)
		return True

	def Layout(self):
		count = len(self.entries)
		self.offset = max(0, min(self.offset, max(0, count - MAX_ROWS)))
		for (index, row) in enumerate(self.rows):
			position = self.offset + index
			if position < count:
				row.Bind(self.entries[position])
				row.Show()
			else:
				row.Hide()
		scrollable = count > MAX_ROWS
		fullWidth = BOARD_WIDTH - BOARD_PAD * 2
		for row in self.rows:
			row.SetWidth(fullWidth - SCROLL_W - 10 if scrollable else fullWidth)
		for widget in (self.upButton, self.downButton, self.track, self.thumb):
			if scrollable:
				widget.Show()
			else:
				widget.Hide()
		if scrollable:
			thumbHeight = max(24, int(self.trackHeight * MAX_ROWS / float(count)))
			span = self.trackHeight - thumbHeight
			maxOffset = count - MAX_ROWS
			self.thumb.SetSize(6, thumbHeight)
			self.thumb.SetPosition(self.thumb.GetLocalPosition()[0], self.trackTop + int(span * self.offset / float(maxOffset)))
		if count:
			self.empty.Hide()
			first = self.offset + 1
			last = min(count, self.offset + MAX_ROWS)
			self.counter.SetText("%d - %d / %d" % (first, last, count) if scrollable else "")
		else:
			self.empty.Show()
			self.counter.SetText("")

	def __OnRow(self, index):
		position = self.offset + index
		if position < len(self.entries) and self.owner:
			self.owner.Open(self.entries[position])

	def OnPressEscapeKey(self):
		self.Hide()
		return True

	def Destroy(self):
		for row in self.rows:
			row.Destroy()
		self.rows = []
		for widget in (self.upButton, self.downButton):
			widget.Destroy()
		if self.close:
			self.close.Destroy()
		self.close = None
		self.owner = None
		self.Hide()


class QuestButton(uiMobileStyle.Pressable):
	# diskli yuvarlak dugme yerine buyuk ferman ikonu; "Gorev" yazisi ikonun ALTINDA; sayi rozeti sag ustte
	ICON_W = 46

	def __init__(self):
		self.iconHeight = int(self.ICON_W * 138 / 128.0)
		uiMobileStyle.Pressable.__init__(self, self.ICON_W + 14, self.iconHeight + 22)
		self.icon = ui.ExpandedImageBox()
		self.icon.SetParent(self)
		self.icon.AddFlag("not_pick")
		self.icon.LoadImage("mobile_ui/akravon/hud/quest_icon.png")
		self.icon.SetScale(self.ICON_W / 128.0, self.ICON_W / 128.0)
		self.icon.SetPosition(7, 0)
		self.icon.Show()
		self.label = uiMobileStyle.CreateText(self, LABEL_TEXT, self.GetWidth() // 2, self.iconHeight + 2, LABEL_COLOR, "LatoBd:15", "center", True)
		self.badge = uiMobileStyle.CreateText(self, "", self.GetWidth() - 8, 0, uiMobileStyle.Color(255, 120, 100), "LatoBd:21", "center", True)
		self.alert = False

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.icon = None
		self.label = None
		self.badge = None

	def SetAlert(self, alert):
		self.alert = alert
		if not alert and getattr(self, "icon", None):
			wndMgr.SetDiffuseColor(self.icon.hWnd, 1.0, 1.0, 1.0, 1.0)

	def RefreshVisual(self):
		if getattr(self, "icon", None):
			shade = 0.78 if self.pressed else 1.0
			wndMgr.SetDiffuseColor(self.icon.hWnd, shade, shade, shade, 1.0)

	def OnUpdate(self):
		if self.alert and getattr(self, "icon", None) and not self.pressed:
			import math
			pulse = 0.5 + 0.5 * (0.5 + 0.5 * math.sin(app.GetTime() * 6.0))
			wndMgr.SetDiffuseColor(self.icon.hWnd, 1.0, 1.0, 1.0, pulse)


class QuestHud(object):
	def __init__(self, interface):
		self.interface = proxy(interface)
		self.seenCount = 0
		self.count = 0
		x = uiMobileHud.SCREEN_EDGE + 6
		y = int(wndMgr.GetScreenHeight() * BUTTON_TOP_RATIO)
		self.button = QuestButton()
		self.button.SetPosition(x, y)
		self.button.SetEvent(ui.__mem_func__(self.Toggle))
		self.label = self.button.label
		self.badge = self.button.badge
		self.board = QuestBoard(proxy(self))
		self.boardX = x + QuestButton.ICON_W + 24
		self.board.SetPosition(self.boardX, uiMobileHud.SCREEN_EDGE)
		self.originalArrange = None
		self.__Hook()
		self.Refresh()

	def __Hook(self):
		interface = self.interface
		original = getattr(interface, "_Interface__ArrangeQuestButton", None)
		if not original:
			return
		self.originalArrange = original
		weakSelf = proxy(self)

		def Arrange():
			original()
			try:
				weakSelf.Refresh()
			except ReferenceError:
				pass
		interface._Interface__ArrangeQuestButton = Arrange

	def __Unhook(self):
		try:
			if "_Interface__ArrangeQuestButton" in self.interface.__dict__:
				del self.interface.__dict__["_Interface__ArrangeQuestButton"]
		except (ReferenceError, AttributeError):
			pass

	def __Buttons(self):
		try:
			return list(getattr(self.interface, "questButtonList", []) or [])
		except ReferenceError:
			return []

	def Entries(self):
		# (tur, anahtar, ad, bilgi, sayi): bekleyen mektuplar once, sonra devam eden gorevler (kalan sayi ile)
		entries = []
		for button in self.__Buttons():
			name = getattr(button, "name", "") or "?"
			entries.append(("new", button, name, "Yeni g\xf6rev - ba\xfelamak i\xe7in dokun", ""))
		try:
			total = quest.GetQuestCount()
		except Exception:
			total = 0
		for index in range(total):
			try:
				(name, icon, counterName, counterValue) = quest.GetQuestData(index)
				questIndex = quest.GetQuestIndex(index)
				(lastName, lastTime) = quest.GetQuestLastTime(index)
			except Exception:
				continue
			info = counterName if counterName else "Devam ediyor"
			value = str(counterValue) if counterName else ""
			if lastName and lastTime > 0:
				info += "   %s: %d dk %d sn" % (lastName, lastTime // 60, lastTime % 60)
			entries.append(("active", questIndex, name, info, value))
		return entries

	def Refresh(self):
		buttons = self.__Buttons()
		for button in buttons:
			if button.IsShow():
				button.Hide()
		entries = self.Entries()
		self.count = len(entries)
		self.badge.SetText("")	# sayi yazilmaz: gorev gelince simge yanip soner
		if self.count > 0:
			self.button.Show()
		else:
			self.button.Hide()
			self.board.Hide()
		if self.count < self.seenCount:
			self.seenCount = self.count
		self.button.SetAlert(self.count > self.seenCount and not self.board.IsShow())
		if self.board.IsShow():
			self.board.SetEntries(entries)

	def Toggle(self):
		if self.board.IsShow():
			self.board.Hide()
			return
		entries = self.Entries()
		self.board.SetEntries(entries)
		self.board.SetPosition(self.boardX, max(uiMobileHud.SCREEN_EDGE, (wndMgr.GetScreenHeight() - self.board.GetHeight()) // 2))
		self.board.Show()
		self.board.SetTop()
		self.seenCount = len(entries)
		self.button.SetAlert(False)

	def Open(self, entry):
		# satira dokunulunca: yeni mektup -> oyunun kendi gorev penceresi; devam eden gorev -> gorev ozeti (karakter penceresindeki gibi)
		(kind, key, name, info, value) = entry
		self.board.Hide()
		import constInfo
		constInfo.AKRAVON_QUEST_TITLE = name or ""
		import event
		if kind == "new":
			method = getattr(self.interface, "_Interface__StartQuest", None)
			if method:
				method(key)
			else:
				event.QuestButtonClick(key.index)
		else:
			event.QuestButtonClick(-2147483648 + key)
		self.Refresh()

	def Update(self):
		# eski mektuplar baska yollardan (ShowAllQuestButton) yeniden gorunurse gizle; sayi / sayac degistiyse yenile
		buttons = self.__Buttons()
		now = app.GetTime()
		if len(buttons) != self.count - self.activeCount() or any(button.IsShow() for button in buttons):
			self.Refresh()
		elif self.board.IsShow() and now >= getattr(self, "nextTick", 0):
			self.nextTick = now + 0.5
			self.Refresh()

	def activeCount(self):
		try:
			return quest.GetQuestCount()
		except Exception:
			return 0

	def Show(self):
		if self.count > 0:
			self.button.Show()

	def Hide(self):
		self.button.Hide()
		self.board.Hide()

	def Destroy(self):
		self.__Unhook()
		self.board.Destroy()
		self.board = None
		self.button.Destroy()
		self.button = None
		self.label = None
		self.badge = None
		try:
			for button in self.__Buttons():
				button.Show()
		except Exception:
			pass
		self.interface = None


def Install(interface):
	global hud
	Destroy()
	try:
		hud = QuestHud(interface)
	except Exception as error:
		import dbg
		dbg.TraceError("uiAkravonQuest.Install: %s" % error)
		hud = None


def Update():
	if hud:
		hud.Update()


def SetVisible(visible):
	if hud:
		if visible:
			hud.Show()
		else:
			hud.Hide()


def Destroy():
	global hud
	if hud:
		try:
			hud.Destroy()
		except Exception:
			pass
	hud = None
