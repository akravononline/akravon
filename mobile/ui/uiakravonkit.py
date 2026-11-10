# -*- coding: utf-8 -*-
# AKRAVON: tasarim kiti - eski (orijinal) pencereleri yuklenirken bizim hazir parcalarla giydirir:
#   tahta  = envanter / ekipmanla ayni menu_board (altin kenar + lacivert)
#   baslik = Marcellus altin, kapat = kare carpi (inv_close)
#   yazili dugme = depo dugmesi gorseli (sb_btn230) + Marcellus krem BUYUK harf
# Yeni cizim yok: yalniz mevcut gorseller. Bu dosyada Turkce harf YOK (cp949 ile okunur).
import ui
import uiMobileStyle

STYLE_SCRIPTS = set((
	"exchangedialog.py",	# ticaret
	"shopdialog.py",	# npc dukkani
	"refinedialog.py",	# demirci yukseltme
	"restartdialog.py",	# olunce yeniden dogma
	"pickmoneydialog.py",	# para bolme
	"newpickmoneydialog.py",	# hizli al / para bolme (yeni)
	"pickitemdialog.py",	# yigin bolme
	"whisperdialog.py",	# fisilti
	"guildwindow.py",	# lonca ana penceresi
	"guildwindow_guildinfopage.py",
	"guildwindow_guildinfopage_eu.py",
	"declareguildwardialog.py",	# lonca savasi ilani
	"changegradenamedialog.py",	# lonca rutbe adi
	"guildwindow_guildskillpage.py",
))

IMG = "d:/ymir work/ui/akravon_legacy/"
BOARD = "mobile_ui/akravon/hud/menu_board_%s.png"
BUTTON = IMG + "sb_btn230_%d.png"
BUTTON_SIZE = (230.0, 44.0)
CLOSE = (IMG + "inv_close_1.png", IMG + "inv_close_1.png", IMG + "inv_close_3.png")
GOLD = uiMobileStyle.Color(222, 184, 110)
TEXT = uiMobileStyle.Color(250, 236, 210)
CORNER = 12


class BoardSkin(ui.Window):
	def __init__(self):
		ui.Window.__init__(self)
		self.AddFlag("not_pick")
		self.pieces = {}
		for key in ("c", "t", "b", "l", "r", "tl", "tr", "bl", "br"):
			image = ui.ExpandedImageBox()
			image.SetParent(self)
			image.AddFlag("not_pick")
			image.LoadImage(BOARD % key)
			image.Show()
			self.pieces[key] = (image, max(1, image.GetWidth()), max(1, image.GetHeight()))

	def __del__(self):
		ui.Window.__del__(self)

	def SetSize(self, width, height):
		ui.Window.SetSize(self, width, height)
		c = CORNER
		mw = max(1, width - c * 2)
		mh = max(1, height - c * 2)
		for (key, x, y, w, h) in (("c", c, c, mw, mh), ("t", c, 0, mw, c), ("b", c, height - c, mw, c),
				("l", 0, c, c, mh), ("r", width - c, c, c, mh), ("tl", 0, 0, c, c), ("tr", width - c, 0, c, c),
				("bl", 0, height - c, c, c), ("br", width - c, height - c, c, c)):
			(image, aw, ah) = self.pieces[key]
			image.SetPosition(x, y)
			image.SetScale(float(w) / aw, float(h) / ah)


def _Children(window):
	result = []
	for child in getattr(window, "Children", []) or []:
		if child:
			result.append(child)
	return result


BOARD_STEP = 24
BOARD_MIN_H = 288
BOARD_MAX_H = 624
BOARD_FIXED = ((290, 558), (376, 356), (700, 520), (720, 480), (460, 230))	# npc dukkani
BOARD_WIDTHS = (380,)	# demirci: yukseklik 24 adimla


def _BoardFile(width, height):
	# bilgisayarda envanterle ayni ciziminden uretilmis tek parca tahta (akrboard_WxH.png); yoksa None
	if (width, height) in BOARD_FIXED:
		return "mobile_ui/akravon/hud/akrboard_%dx%d.png" % (width, height)
	if width in BOARD_WIDTHS:
		snapped = max(BOARD_MIN_H, min(BOARD_MAX_H, ((height - BOARD_MIN_H + BOARD_STEP - 1) // BOARD_STEP) * BOARD_STEP + BOARD_MIN_H))
		return "mobile_ui/akravon/hud/akrboard_%dx%d.png" % (width, snapped), snapped
	return None


def _LayoutBoard(board):
	# eski tahta parcalari gizli; yerine tek parca hazir tahta gorseli (katman ustune katman yok)
	for piece in (getattr(board, "Corners", []) or []) + (getattr(board, "Lines", []) or []):
		piece.Hide()
	width = board.GetWidth()
	height = board.GetHeight()
	found = _BoardFile(width, height)
	if found is None:
		return
	fileName = found[0] if isinstance(found, tuple) else found
	# en alttaki katman = tahtanin kendi zemini (Base): yeni gorsel buraya, icerik ustunde kalir
	base = getattr(board, "Base", None)
	if base is None or not hasattr(base, "LoadImage"):
		return
	base.LoadImage(fileName)
	if hasattr(base, "SetRenderingRect"):
		base.SetRenderingRect(0.0, 0.0, 0.0, 0.0)
	base.SetScale(1.0, 1.0)
	base.SetPosition(0, 0)
	base.Show()
	title = getattr(board, "akrTitle", None)
	if title:
		title.SetPosition(width // 2, 13)
	closeInfo = getattr(board, "akrClose", None)
	if closeInfo:
		(close, tbx, tby) = closeInfo
		close.SetPosition(width - tbx - 12 - close.GetWidth(), 10 - tby)
	divider = getattr(board, "akrDivider", None)
	if divider:
		divider.SetSize(max(1, width - 32), 1)


def _StyleBoard(board, keep):
	original = board.SetSize

	def SetSize(width, height):
		original(width, height)
		_LayoutBoard(board)	# boyut degisince de bizim tahta kalsin
	board.SetSize = SetSize
	_LayoutBoard(board)


def _StyleTitleBar(titleBar, keep):
	for name in ("imgLeft", "imgCenter", "imgRight"):
		image = getattr(titleBar, name, None)
		if image:
			image.Hide()
	close = getattr(titleBar, "btnClose", None)
	if close:
		close.SetUpVisual(CLOSE[0])
		close.SetOverVisual(CLOSE[1])
		close.SetDownVisual(CLOSE[2])
		board = getattr(titleBar, "akrBoard", None)
		(tbx, tby) = titleBar.GetLocalPosition()
		if board is not None:
			close.SetPosition(board.GetWidth() - tbx - 12 - close.GetWidth(), 10 - tby)	# envanterle ayni: icerde
			board.akrClose = (close, tbx, tby)
		else:
			close.SetPosition(max(0, titleBar.GetWidth() - close.GetWidth()), 0)
	board = getattr(titleBar, "akrBoard", None)
	for child in _Children(titleBar):
		if isinstance(child, ui.TextLine):
			text = child.GetText()
			child.Hide()
			if board is None:
				continue
			label = uiMobileStyle.CreateText(board, text, board.GetWidth() // 2, 13, GOLD, "Marcellus:22", "center")
			board.akrTitle = label
			keep.append(label)

			def SetText(newText, label = label):
				label.SetText(newText)
			child.SetText = SetText


def _StyleQuestDialog(window):
	# NPC gorev konusmasi: eski kalin seffaf panel yerine giris penceresindeki (uiAkravon.Frame) cerceve
	try:
		board = window.GetChild("board")
		for piece in list(getattr(board, "Corners", [])) + list(getattr(board, "Lines", [])):
			piece.Hide()
		if getattr(board, "Base", None):
			board.Base.Hide()
		import uiAkravon
		skin = BoardSkin()	# desenli lacivert doku (menu_board), hafif seffaf
		skin.SetParent(board)
		skin.SetPosition(2, 2)
		skin.SetSize(max(1, board.GetWidth() - 4), max(1, board.GetHeight() - 4))
		for key in skin.pieces:
			skin.pieces[key][0].SetAlpha(0.93)
		skin.Show()
		frame = uiAkravon.Frame(True)	# altin cerceve, kose susleri, elmaslar (zemin yok)
		frame.SetParent(board)
		frame.SetPosition(0, 0)
		frame.fill.Hide()
		frame.SetSize(board.GetWidth(), board.GetHeight())
		frame.Show()
		board.akrFrame = frame
		board.akrSkin = skin
		original = board.SetSize

		def SetSize(width, height):
			original(width, height)
			frame.SetSize(board.GetWidth(), board.GetHeight())
			skin.SetSize(max(1, board.GetWidth() - 4), max(1, board.GetHeight() - 4))
		board.SetSize = SetSize
	except Exception:
		import dbg
		import traceback
		dbg.TraceError("uiAkravonKit gorev cercevesi: %s" % traceback.format_exc())


def _StylePlainPopup(window):
	# lonca vb. "UIScript/PopupDialog.py"yi duz ScriptWindow olarak yukler: yazi ve dugme etiketi bizim stile baglanmaz -> uicommon.PopupDialog ile ayni baglama
	try:
		message = window.GetChild("message")
		window.board = window.GetChild("board")
		window.accceptButton = window.GetChild("accept")
		import uiAkravonPopup
		uiAkravonPopup.Style(window, 1)
		message.SetText = lambda text: window.SetText(text)
	except Exception:
		import dbg
		import traceback
		dbg.TraceError("uiAkravonKit popup: %s" % traceback.format_exc())


def _StyleBoardTitle(board, keep):
	# board_with_titlebar: baslik cubugu ve cikis dugmesi kodda olusur (eleman listesinde yok) -> kare carpi + Marcellus altin baslik
	titleBar = getattr(board, "titleBar", None)
	name = getattr(board, "titleName", None)
	if titleBar is None or name is None or getattr(board, "akrTitle", None):
		return
	titleBar.akrBoard = board
	_StyleTitleBar(titleBar, keep)
	text = name.GetText()
	name.Hide()
	label = uiMobileStyle.CreateText(board, text, board.GetWidth() // 2, 13, GOLD, "Marcellus:22", "center")
	board.akrTitle = label
	keep.append(label)

	def SetTitleName(newText):
		label.SetText(newText)

	def SetTitleColor(color):
		pass
	board.SetTitleName = SetTitleName
	board.SetTitleColor = SetTitleColor


def _StyleButton(button, keep, ownSkin = False):
	try:
		text = button.GetText()
	except Exception:
		return
	if not text:
		return
	width = button.GetWidth()
	height = button.GetHeight()
	if width <= 0 or height <= 0:
		return
	if getattr(button, "ButtonText", None):
		button.ButtonText.Hide()
	if height < 40 and not ownSkin:	# eski kucuk dugme: depo dugmesi gorseli ustune (kendi gorselli dugmelerde yok)
		skin = ui.ExpandedImageBox()
		skin.SetParent(button)
		skin.AddFlag("not_pick")
		skin.LoadImage(BUTTON % 1)
		skin.SetScale(width / BUTTON_SIZE[0], height / BUTTON_SIZE[1])
		skin.SetPosition(0, 0)
		skin.Show()
		keep.append(skin)
	size = 19 if height >= 40 else max(12, min(19, int(height * 0.62)))
	color = GOLD if getattr(button, "akrName", "") in GOLD_LABEL_BUTTONS else TEXT
	label = uiMobileStyle.CreateText(button, uiMobileStyle.TrUpper(text), width // 2, max(0, (height - size) // 2 - 1), color, "Marcellus:%d" % size, "center")
	keep.append(label)

	def SetText(newText, height = 4):
		label.SetText(uiMobileStyle.TrUpper(newText))
	button.SetText = SetText


GOLD_LABEL_BUTTONS = set(("UploadGuildMarkButton", "UploadGuildSymbolButton"))
OWN_SKIN_SCRIPTS = set(("guildwindow_guildinfopage.py", "guildwindow_guildinfopage_eu.py", "declareguildwardialog.py", "guildwindow.py", "changegradenamedialog.py", "guildwindow_guildskillpage.py"))	# dugmeler kendi gorselini tasir
DIVIDER_Y = {"shopdialog.py" : 51, "refinedialog.py" : 58, "guildwindow.py" : 52, "declareguildwardialog.py" : 52, "changegradenamedialog.py" : 52}
DIVIDER_SCRIPTS = set(("shopdialog.py", "refinedialog.py", "guildwindow.py", "declareguildwardialog.py", "changegradenamedialog.py"))	# envanter gibi baslik alti cizgi


def _ShopAfter(window):
	import wndMgr
	if hasattr(wndMgr, "SetSlotUnitSize"):
		wndMgr.SetSlotUnitSize(window.GetChild("ItemSlot").hWnd, 50, 50, 4)	# envanterle ayni 50 px kutucuk
	try:
		import uiAkravonBulkSell
		uiAkravonBulkSell.Attach(window)	# toplu satis
	except Exception:
		import dbg
		import traceback
		dbg.TraceError("uiAkravonBulkSell: " + traceback.format_exc())

REFINE_EXTRA_W = 50
REFINE_SHIFT = 23	# materyal satirlari ozellik grubuyla ayni hizada
REFINE_EXTRA_H = 78
REFINE_TOP = 72	# icerik baslik cizgisinin altindan baslar
REFINE_TIP_WIDTH = 230	# ozellik kutusu: sikismaz ama cok genis degil
REFINE_SLOT = 64	# hedef esya kutucugu (buyuk)
REFINE_GAP = 10


def _RefineAfter(window):
	# demirci: depo dugmeleri (sb_btn110), daha genis / yuksek tahta, yazilar dugmelerin ustunde
	root = IMG + "sb_btn140_%d.png"
	for (name, x) in (("AcceptButton", -80), ("CancelButton", 80)):
		try:
			button = window.GetChild(name)
		except Exception:
			continue
		button.SetUpVisual(root % 1)
		button.SetOverVisual(root % 2)
		button.SetDownVisual(root % 3)
		button.SetWindowVerticalAlignTop()
		button.SetPosition(x, 200)
	shifted = set()
	original = window.UpdateDialog

	def UpdateDialog():
		_WidenRefineTip(window)
		original()
		board = window.board
		width = 380
		tipHeight = window.toolTip.GetHeight()
		costY = REFINE_TOP + tipHeight + 20
		buttonsY = costY + 42
		height = buttonsY + 44 + 28
		for (name, x) in (("AcceptButton", 190 - 80 - 70), ("CancelButton", 190 + 80 - 70)):
			try:
				button = window.GetChild(name)
				button.SetWindowHorizontalAlignLeft()
				button.SetWindowVerticalAlignTop()
				button.SetPosition(x, buttonsY)
			except Exception:
				pass
		try:
			cost = window.GetChild("Cost")
			cost.SetWindowHorizontalAlignLeft()
			cost.SetWindowVerticalAlignTop()
			cost.SetPosition(190, costY)	# yazi kendi icinde ortali: x = tahta ortasi
		except Exception:
			pass
		try:
			window.GetChild("SuccessPercentage").Hide()
		except Exception:
			pass
		height = max(BOARD_MIN_H, min(BOARD_MAX_H, ((height - BOARD_MIN_H + BOARD_STEP - 1) // BOARD_STEP) * BOARD_STEP + BOARD_MIN_H))
		board.SetSize(width, height)
		window.SetSize(width, height)
		for child in getattr(window, "children", []) or []:
			if id(child) in shifted:
				continue
			shifted.add(id(child))
			try:
				(x, y) = child.GetLocalPosition()
				child.SetPosition(x + REFINE_SHIFT, y)
			except Exception:
				pass
		toolTip = getattr(window, "toolTip", None)
		if toolTip:
			groupWidth = REFINE_SLOT + REFINE_GAP + REFINE_TIP_WIDTH
			toolTip.SetPosition((380 - groupWidth) // 2 + REFINE_SLOT + REFINE_GAP, REFINE_TOP)
		_RefineTarget(window)
		_HideRefineCheck(window)
		import wndMgr
		window.SetPosition((wndMgr.GetScreenWidth() - width) // 2, max(10, (wndMgr.GetScreenHeight() - height) // 2))
	window.UpdateDialog = UpdateDialog
	originalOpen = window.Open

	def Open(*args):
		result = originalOpen(*args)
		_HideRefineCheck(window)
		return result
	window.Open = Open


def _WidenRefineTip(window):
	# ozellik kutusu: dar varsayilan (190) yerine genis; ilk acilista icerik yeniden yazilir
	import player
	tip = getattr(window, "toolTip", None)
	if not tip or getattr(tip, "TOOL_TIP_WIDTH", 0) == REFINE_TIP_WIDTH:
		return
	tip.TOOL_TIP_WIDTH = REFINE_TIP_WIDTH
	vnum = getattr(window, "vnum", 0)
	pos = getattr(window, "targetItemPos", 0)
	if not vnum:
		return
	try:
		metinSlot = [player.GetItemMetinSocket(pos, i) for i in xrange(player.METIN_SOCKET_MAX_NUM)]
		attrSlot = [player.GetItemAttribute(pos, i) for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)]
		tip.ClearToolTip()
		tip.AddRefineItemData(vnum, metinSlot, attrSlot)
		window.dialogHeight = tip.GetHeight() + 46
	except Exception:
		pass


def _RefineTarget(window):
	# hedef esya: envanterle ayni dolu kutucuk (inv_cell, 50 px, motorun kutucuk cizimi), solda ustten hizali
	import item
	for slot in getattr(window, "slotList", []) or []:
		slot.Hide()
	image = getattr(window, "itemImage", None)
	if image:
		image.Hide()
	vnum = getattr(window, "vnum", 0)
	if not vnum:
		return
	holder = getattr(window, "akrTargetSlot", None)
	if holder is None:
		holder = ui.Window()
		holder.SetParent(window)
		holder.AddFlag("not_pick")
		window.akrTargetSlot = holder
		window.akrTargetParts = []
	for part in window.akrTargetParts:
		part.Hide()
	window.akrTargetParts = []
	item.SelectItem(vnum)
	(xSize, ySize) = item.GetItemSize()
	ySize = max(1, min(3, ySize))
	cell = ui.ExpandedImageBox()	# envanterle ayni oyuk kutu, esyanin boyuna gore TEK parca (ortada cizgi yok)
	cell.SetParent(holder)
	cell.AddFlag("not_pick")
	cell.LoadImage("mobile_ui/akravon/hud/akrslot_%d.png" % ySize)
	cell.SetPosition(0, 0)
	cell.Show()
	window.akrTargetParts.append(cell)
	icon = ui.ExpandedImageBox()	# ikon 32 x (32 * satir): 2 kat buyutulunce kutularla birebir ayni boy
	icon.SetParent(holder)
	icon.AddFlag("not_pick")
	icon.LoadImage(item.GetIconImageFileName())
	icon.SetScale(REFINE_SLOT / 32.0, REFINE_SLOT / 32.0)
	icon.SetPosition(0, 0)
	icon.Show()
	window.akrTargetParts.append(icon)
	holder.SetSize(REFINE_SLOT, REFINE_SLOT * ySize)
	holder.SetPosition((380 - (REFINE_SLOT + REFINE_GAP + REFINE_TIP_WIDTH)) // 2, REFINE_TOP + 4)
	holder.Show()


def _HideRefineCheck(window):
	# "Pencere kapanmasin" secenegi yok: gizli ve hep kapali (pencere yukseltmeden sonra kapanir)
	check = getattr(window, "checkBoxRefine", None)
	if not check:
		return
	for name in ("SetChecked", "SetCheck"):
		method = getattr(check, name, None)
		if method:
			try:
				method(False)
			except Exception:
				pass
	check.Hide()
	check.Show = lambda *args: None

AFTER = {"shopdialog.py" : _ShopAfter, "refinedialog.py" : _RefineAfter}


def StyleWindow(window, name = ""):
	keep = getattr(window, "akrKitKeep", None)
	if keep is None:
		keep = []
		window.akrKitKeep = keep
	for (key, element) in getattr(window, "ElementDictionary", {}).items():
		try:
			element.akrName = key
		except Exception:
			pass
	elements = list(getattr(window, "ElementDictionary", {}).values())
	mainBoard = None
	for element in elements:
		if isinstance(element, (ui.Board, ui.ThinBoard)):
			_StyleBoard(element, keep)
			if mainBoard is None or element.GetWidth() * element.GetHeight() > mainBoard.GetWidth() * mainBoard.GetHeight():
				mainBoard = element
	if mainBoard is not None and name in DIVIDER_SCRIPTS:
		divider = uiMobileStyle.CreateBar(mainBoard, uiMobileStyle.Color(96, 74, 42), 16, DIVIDER_Y.get(name, 46), mainBoard.GetWidth() - 32, 1)
		mainBoard.akrDivider = divider
		keep.append(divider)
	for element in elements:
		if isinstance(element, ui.BoardWithTitleBar):
			_StyleBoardTitle(element, keep)
	for element in elements:
		if isinstance(element, ui.TitleBar):
			element.akrBoard = mainBoard
			_StyleTitleBar(element, keep)
	after = AFTER.get(name)
	if after:
		after(window)
	for element in elements:
		if isinstance(element, ui.Button) and not isinstance(element, ui.TitleBar):
			_StyleButton(element, keep, name in OWN_SKIN_SCRIPTS)


def Install():
	if getattr(ui, "akrKitInstalled", False):
		return
	ui.akrKitInstalled = True
	original = ui.PythonScriptLoader.LoadScriptFile

	def LoadScriptFile(self, window, FileName):
		result = original(self, window, FileName)
		name = FileName.replace("\\", "/").split("/")[-1].lower()
		if name == "questdialog.py":
			_StyleQuestDialog(window)
		if name == "popupdialog.py" and type(window) is ui.ScriptWindow:
			_StylePlainPopup(window)
		if name in STYLE_SCRIPTS:
			try:
				StyleWindow(window, name)
			except Exception:
				import dbg
				import traceback
				dbg.TraceError("uiAkravonKit %s: %s" % (name, traceback.format_exc()))
		return result
	ui.PythonScriptLoader.LoadScriptFile = LoadScriptFile
