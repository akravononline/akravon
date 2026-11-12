# -*- coding: utf-8 -*-
# AKRAVON: depo / nesne marketi / sifre pencereleri - envanterle ayni dil (skin/build_akravon_safebox.py)
from __future__ import division
import wndMgr
import safebox
import ui
import uiMobileStyle
import uiAkravonInventory

IMG = "d:/ymir work/ui/akravon_legacy/"
GOLD = uiMobileStyle.Color(222, 184, 110)
TEXT = uiMobileStyle.Color(250, 236, 210)
TAB_ON = uiMobileStyle.Color(40, 24, 8)	# altin sekmede koyu yazi (market deposuyla ayni)
TAB_OFF = uiMobileStyle.Color(176, 160, 128)
CELL = 58	# envanterle ayni
GRID_X = 20
GRID_Y = 116
GRID_W = 348
W = 388
H = 538
COLS = 6
ROWS = 6


class Owner(object):
	def __init__(self, window):
		self.window = window
		self.handle = None

	def Step(self):
		if self.handle:
			self.handle.Drag()


TEXTS = {
	"ChangePasswordButton" : "ŞİFREYİ YENİLE",
	"ExitButton" : "KAPAT",
	"accept_button" : "GİR",
	"cancel_button" : "ÇIK",
	"normal_button" : "EŞYA DEPOSU",
	"nesne_button" : "NESNE MARKETİ",
}


def Label(window, buttonName, keep, texts = None):
	button = window.GetChild(buttonName)
	label = (texts or TEXTS).get(buttonName, "")
	text = uiMobileStyle.CreateText(button, label, button.GetWidth() // 2, 12, TEXT, "Marcellus:19", "center")
	keep.append(text)
	if not hasattr(window, "akrLabels"):
		window.akrLabels = {}
	window.akrLabels[buttonName] = text
	return text


def Title(window, parent, text, width, keep, y = 13):
	keep.append(uiMobileStyle.CreateText(parent, text, width // 2, y, GOLD, "Marcellus:22", "center"))
	keep.append(uiMobileStyle.CreateBar(parent, uiMobileStyle.Color(96, 74, 42), 16, 52, width - 32, 1))


def ApplyBox(window, title, paged):
	# depo (paged) ve nesne marketi: baslik, tek satirlik sekmeler, 46 px izgara
	keep = []
	board = window.GetChild("board")
	Title(window, board, title, W, keep)
	window.akrKeep = keep
	for name in ("ChangePasswordButton", "ExitButton"):
		try:
			Label(window, name, keep)
		except Exception:
			pass
	try:
		window.GetChild("ExitButton").Hide()
	except Exception:
		pass
	owner = Owner(window)
	owner.handle = uiAkravonInventory.DragHandle(board, owner, W - 60, 52)
	window.akrOwner = owner
	window.akrSlider = uiAkravonInventory.Slider(owner)
	window.akrTabLabels = []
	window.akrStrip = None

	def SetTableSize(size):
		cols = COLS
		rows = ROWS
		slotRows = safebox.SAFEBOX_SLOT_Y_COUNT
		# sunucu sayfa sayisini (kucuk sayi) ya da yuva sayisini gonderebilir
		pages = size if size <= 5 else size // slotRows
		pageCount = 5 if paged else 1	# AKRAVON: depo 5 sayfa x 6x6 (sunucu: 6 sutunluk tek izgara, 30 satir)
		if paged:
			safebox.SAFEBOX_PAGE_SIZE = cols * rows	# sayfa = 36 yuva: sunucunun surekli izgarasindan 6 satirlik dilim
		window.pageCount = pageCount
		window.curPageIndex = 0
		for button in getattr(window, "pageButtonList", []):
			button.Hide()
		window.pageButtonList = []
		for label in window.akrTabLabels:
			label.Hide()
		window.akrTabLabels = []
		if window.akrStrip:
			window.akrStrip.Hide()
		strip = ui.ExpandedImageBox()
		strip.SetParent(board)
		strip.AddFlag("not_pick")
		strip.LoadImage(IMG + "sb_strip_%d.png" % pageCount)
		strip.SetPosition(GRID_X, 62)
		strip.Show()
		window.akrStrip = strip
		tabWidth = GRID_W // pageCount
		names = ("I", "II", "III", "IV", "V") if paged else (title,)
		for i in range(pageCount):
			button = ui.RadioButton()
			button.SetParent(board)
			button.SetUpVisual(IMG + "sb_tab_empty_%d.png" % pageCount)
			button.SetOverVisual(IMG + "sb_tab_empty_%d.png" % pageCount)
			button.SetDownVisual(IMG + "sb_tab_on_%d.png" % pageCount)
			button.SetPosition(GRID_X + i * tabWidth, 62)
			if paged:
				button.SetEvent(lambda arg = i: window.SelectPage(arg))
			button.Show()
			window.pageButtonList.append(button)
			window.akrTabLabels.append(uiMobileStyle.CreateText(board, names[i], GRID_X + i * tabWidth + tabWidth // 2, 71,
				TAB_ON if i == 0 else TAB_OFF, "Marcellus:20", "center"))
		if paged:
			window.pageButtonList[0].Down()
		item = window.wndItem
		item.SetParent(board)
		item.SetPosition(GRID_X, GRID_Y)
		item.ArrangeSlot(0, cols, rows, CELL, CELL, 0, 0)
		item.RefreshSlot()
		item.SetSlotBaseImage(IMG + "inv_cell.png", 1.0, 1.0, 1.0, 1.0)
		if hasattr(wndMgr, "SetSlotUnitSize"):
			wndMgr.SetSlotUnitSize(item.hWnd, CELL, CELL, 4)
		window.SetSize(W, H)
		window.UpdateRect()
		# envanter aciksa onun (ekipman aciksa ekipmanin) SOLUNDA, dikeyde ortada; degilse ekranin ortasinda
		sw = wndMgr.GetScreenWidth()
		sh = wndMgr.GetScreenHeight()
		x = (sw - W) // 2
		try:
			import uiToolTip
			for anchor in uiToolTip.ANCHOR_WINDOWS:
				if not anchor.IsShow():
					continue
				left = anchor.GetGlobalPosition()[0]
				try:
					equip = anchor.GetChild("EquipBoard")
					if equip.IsShow():
						left = min(left, equip.GetGlobalPosition()[0])
				except Exception:
					pass
				if left - W - 12 >= 4:
					x = left - W - 12
				break
		except Exception:
			pass
		window.SetPosition(x, max(8, (sh - H) // 2))

	window.SetTableSize = SetTableSize

	if paged:
		select = window.SelectPage

		def SelectPage(index):
			select(index)
			for (i, label) in enumerate(window.akrTabLabels):
				label.SetPackedFontColor(TAB_ON if i == index else TAB_OFF)

		window.SelectPage = SelectPage


def ApplyDialog(dialog, buttons):
	keep = []
	texts = None
	if "normal_button" not in buttons:
		texts = {"accept_button" : "KAYDET", "cancel_button" : "VAZGEÇ"}
	for name in buttons:
		try:
			Label(dialog, name, keep, texts)
		except Exception:
			pass
	try:
		title = dialog.GetChild("TitleName")
		title.SetFontName("Marcellus:22")
		title.SetPackedFontColor(GOLD)
		title.SetText(title.GetText())	# yeni yazi tipiyle yeniden olustur
		if "normal_button" not in buttons:
			# sifre degistirme penceresinde baslik metni yeniden olusmuyordu: ayri bir yazi katmani kullan
			text = title.GetText()
			title.Hide()
			keep.append(uiMobileStyle.CreateText(dialog.GetChild("board"), text, dialog.GetChild("board").GetWidth() // 2, 13, GOLD, "Marcellus:22", "center"))
	except Exception:
		pass
	board = dialog.GetChild("board")
	keep.append(uiMobileStyle.CreateBar(board, uiMobileStyle.Color(96, 74, 42), 16, 52, board.GetWidth() - 32, 1))
	dialog.akrKeep = keep

	if "normal_button" in buttons:
		ChoiceFlow(dialog, keep)


SUB = uiMobileStyle.Color(214, 204, 184)
YELLOW = uiMobileStyle.Color(255, 214, 90)
BODY_FONT = "Lato:19"	# bilgi yazilari telefonda okunur boy
PW = 340


def ChoiceFlow(dialog, keep):
	# 1. adim: Esya Deposu / Nesne Marketi sec  ->  2. adim: sifre gir (pencere her adimda icerige gore boyutlanir)
	get = dialog.GetChild
	board = get("board")
	for i in range(1, 6):
		get("Desc%d" % i).Hide()		# eski yazilar: oyunun varsayilan yazi tipi, satir sonu yok
	slot = get("password_slot")
	normal = get("normal_button")
	nesne = get("nesne_button")
	accept = get("accept_button")
	cancel = get("cancel_button")
	state = {"step" : 1}

	def Line(text, y, color = SUB):
		line = uiMobileStyle.CreateText(board, text, PW // 2, y, color, BODY_FONT, "center")
		keep.append(line)
		return line

	head1 = Line("", 72, TEXT)
	state["lines"] = []
	margin = 28
	space = uiMobileStyle.TextWidth("a a", BODY_FONT) - uiMobileStyle.TextWidth("aa", BODY_FONT)

	def Paragraph(parts, y):
		# parts: [(metin, renk)] - sozcukler dolgu genisligine gore satirlara akar, her satir ortalanir
		for old in state["lines"]:
			old.Hide()
		state["lines"] = []
		words = []
		for (text, color) in parts:
			for word in text.split(" "):
				if word:
					words.append((word, color, uiMobileStyle.TextWidth(word, BODY_FONT)))
		rows = [[]]
		used = 0
		for item in words:
			need = item[2] + (space if rows[-1] else 0)
			if rows[-1] and used + need > PW - 2 * margin:
				rows.append([])
				used = 0
				need = item[2]
			rows[-1].append(item)
			used += need
		for (i, row) in enumerate(rows):
			total = sum(w[2] for w in row) + space * (len(row) - 1)
			x = PW // 2 - total // 2
			for item in row:
				line = uiMobileStyle.CreateText(board, item[0], x, y + i * 26, item[1], BODY_FONT, "left")
				state["lines"].append(line)
				x += item[2] + space
		return len(rows)

	def Resize(height, image, top = False):
		board.LoadImage(IMG + image)
		dialog.SetSize(PW, height)
		board.SetSize(PW, height)
		try:
			if top:	# sifre: klavyenin ustunde (ekranin ust kismi)
				dialog.SetPosition((wndMgr.GetScreenWidth() - PW) // 2, uiMobileStyle.InputTop(height))
			else:
				dialog.SetCenterPosition()
		except Exception:
			pass

	def Step1():
		state["step"] = 1
		head1.SetText("Hangisini açmak istiyorsun?")
		for old in state["lines"]:
			old.Hide()
		slot.Hide()
		accept.Hide()
		cancel.Hide()
		normal.Show()
		nesne.Show()
		normal.SetPosition(22, 112)
		nesne.SetPosition(178, 112)
		normal.SetUp()
		nesne.SetUp()
		Resize(200, "sb_pw_board_1.png")
		try:	# secim adiminda klavye acilmaz
			dialog.passwordValue.KillFocus()
			import app
			if hasattr(app, "ShowKeyboard"):
				app.ShowKeyboard(0)
		except Exception:
			pass

	def Step2(name):
		state["step"] = 2
		head1.SetText("")
		count = Paragraph([("%s için gizli şifreni gir. Hiç değiştirmediysen ilk şifren " % name, TEXT), ("“000000”", YELLOW), (".", TEXT)], 70)
		count = max(2, min(3, count))
		fieldY = 70 + 22 * count + 14
		slot.Show()
		slot.SetPosition(95, fieldY)
		accept.Show()
		normal.Hide()
		nesne.Hide()
		cancel.Show()
		buttonY = fieldY + 40 + 20
		accept.SetPosition(43, buttonY)
		cancel.SetPosition(177, buttonY)
		dialog.akrCancelLabel.SetText("GERİ")
		Resize(buttonY + 44 + 24, "sb_pw_board_%d.png" % (count + 0), True)
		try:
			dialog.passwordValue.Show()
			dialog.passwordValue.SetTop()
			dialog.passwordValue.KillFocus()	# odak zaten varsa klavye yeniden acilmaz: once birak, sonra al
			dialog.passwordValue.SetFocus()
			import app
			if hasattr(app, "ShowKeyboard"):
				app.ShowKeyboard(1)
		except Exception:
			pass

	dialog.akrCancelLabel = dialog.akrLabels["cancel_button"]
	show = dialog.ShowDialog
	chooseNormal = dialog._PasswordDialog__NormalDepo
	chooseNesne = dialog._PasswordDialog__NesneDepo

	def PickNormal():
		chooseNormal()
		Step2("Eşya Deposu")

	def PickNesne():
		chooseNesne()
		Step2("Nesne Marketi")

	def OnCancel():
		if state["step"] == 2:
			Step1()
			return True
		dialog.CloseDialog()
		return True

	def ShowDialog():
		show()
		normal.SetEvent(PickNormal)
		nesne.SetEvent(PickNesne)
		cancel.SetEvent(OnCancel)
		dialog.passwordValue.OnPressEscapeKey = OnCancel
		Step1()

	dialog.ShowDialog = ShowDialog
	dialog.akrKeep.append(Step1)
