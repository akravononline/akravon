from __future__ import division
import app
import chat
import wndMgr
import uiScriptLocale
import ui
import uiChat
import uiMobileCompat
import uiMobileStyle

from _weakref import proxy
PANEL_WIDTH = 600
PHONE_PANEL_WIDTH = 760	# AKRAVON: telefonda genis panel (uzun satirlar tasmasin)
PANEL_HEIGHT = 250
HEADER_HEIGHT = 44
INPUT_ROW_HEIGHT = 46
INPUT_BOTTOM_PAD = 6
TAB_HEIGHT = 28
TAB_FONT = "LatoBd:17"
TAB_PADDING = 14
TAB_GAP = 6
TAB_LEFT = 14
INPUT_FONT = "Lato:15"
COLLAPSE_SIZE = 28
LINE_PADDING_X = 18
LINE_PADDING_Y = 6
FIELD_HEIGHT = 34
FIELD_TEXT_X = 12
BUTTON_SIZE = 34
INPUT_GAP = 8
INPUT_SIDE = 14
INPUT_TOP = 6
OPEN_DURATION = 0.15
CLOSE_DURATION = 0.11
COLLAPSED_HEIGHT = 38
MOBILE_CHAT_SET = 2
FLOAT_CHAT_SET = 3
FLOAT_WIDTH = 560
CHAT_MAX = 512
CHAT_USER_MAX = 120	# AKRAVON: esya baglantisi (~80 karakter) sigsin
LINK_SUPPRESS_SECONDS = 0.6
TAP_SLOP = 14
TOP_ASSET = "mobile_ui/akravon/hud/chat_dock_top.png"
BODY_ASSET = "mobile_ui/akravon/hud/chat_dock_body.png"
TOP_ASSET_SIZE = (1200.0, 88.0)
TOP_HEIGHT = 44
BODY_ASSET_HEIGHT = 200.0
GRIP_HEIGHT = 14
def _DetectIOS():
	# iOS Python da "linux2" der; uygulama klasoru /var/mobile/Containers altinda
	try:
		import os
		return "/Containers/" in os.getcwd() or "/var/mobile/" in os.getcwd()
	except Exception:
		return False
IS_IOS = _DetectIOS()
KEYBOARD_BOTTOM_RATIO = 0.575 if IS_IOS else 0.46	# klavye acikken panelin alt kenari (ekran yuksekliginin orani)
MAX_BOTTOM_GAP = 70

DIVIDER_COLOR = uiMobileStyle.Color(110, 86, 44)
TAB_TEXT_COLOR = uiMobileStyle.Color(150, 138, 116)
PLACEHOLDER_COLOR = uiMobileStyle.Color(120, 106, 84)
FIELD_COLOR = uiMobileStyle.Color(5, 9, 18)
FIELD_BORDER = uiMobileStyle.Color(112, 88, 50)
FIELD_BORDER_FOCUS = uiMobileStyle.Color(206, 166, 98)

# AKRAVON: kanal renkleri belirgin ayrilir (satirlar ve sekme yazilari ayni rengi kullanir)
# AKRAVON: orijinal Metin2 sohbet renkleri (PythonChat.cpp varsayilanlari): normal konusma isim + mesaj beyaz
LINE_COLORS = (
	("CHAT_TYPE_TALKING", (255, 255, 255)),
	("CHAT_TYPE_INFO", (255, 200, 200)),
	("CHAT_TYPE_NOTICE", (255, 230, 186)),
	("CHAT_TYPE_PARTY", (138, 255, 242)),
	("CHAT_TYPE_GUILD", (231, 216, 255)),
	("CHAT_TYPE_SHOUT", (168, 255, 213)),
	("CHAT_TYPE_WHISPER", (74, 225, 74)),
)

TABS = (
	("MOBILE_CHAT_ALL", None, "", (226, 212, 180)),
	("MOBILE_CHAT_NORMAL", "CHAT_TYPE_TALKING", "", (255, 255, 255)),
	("MOBILE_CHAT_PARTY", "CHAT_TYPE_PARTY", "#", (138, 255, 242)),
	("MOBILE_CHAT_GUILD", "CHAT_TYPE_GUILD", "%", (231, 216, 255)),
	("MOBILE_CHAT_SHOUT", "CHAT_TYPE_SHOUT", "!", (168, 255, 213)),
)
COMMAND_MARKS = "#%!/"

# esya baglantisi istegi: sohbet paneli envanterden bir esya secilmesini bekler (uiinventory yamasi TakeItemSlot cagirir)
linkRequester = None
linkSuppressUntil = 0.0
activePanel = None


def IsTyping():
	panel = activePanel
	try:
		return bool(panel and panel.IsTyping())
	except ReferenceError:
		return False


def TakeItemSlot(player, slot):
	global linkRequester, linkSuppressUntil
	now = app.GetTime()
	if linkRequester is None:
		# ayni dokunus hem SelectItemSlot hem UseItemSlot'i cagirir: ikincisi esyayi kullanmasin
		return now < linkSuppressUntil
	requester = linkRequester
	linkRequester = None
	linkSuppressUntil = now + LINK_SUPPRESS_SECONDS
	try:
		if player.GetItemIndex(slot) != 0:
			requester.InsertLink(player.GetItemLink(slot))
		else:
			requester.InsertLink("")
	except ReferenceError:
		pass
	return True


def ApplyLineColors():
	for (typeName, (red, green, blue)) in LINE_COLORS:
		chatType = getattr(chat, typeName, None)
		if chatType is not None:
			chat.SetChatColor(chatType, red, green, blue)


def GetAllModes():
	return list(uiChat.ChatLogWindow.CHAT_MODE_INDEX)


class ChatInput(uiChat.ChatLine):
	def __init__(self):
		uiChat.ChatLine.__init__(self)
		self.prefix = ""
		# masaustu uyari satiri (CHAT_OVERLAY_NOTICE) bosken EditLine'in her SetText("")'inde yeniden gosteriliyor ve
		# "Mesaj yaz" yer tutucusunun ustune biniyordu: bos metin gosterilse de cizilmez
		self.overLay.SetText("")
		self.overLay.Hide()

	def SetPrefix(self, prefix):
		self.prefix = prefix

	def OnIMEReturn(self):
		text = self.GetText()
		if self.prefix and text.strip() and text[0] not in COMMAND_MARKS:
			self.SetText(self.prefix + text)
		result = uiChat.ChatLine.OnIMEReturn(self)
		if IS_IOS and self.IsFocus():
			self.KillFocus()	# AKRAVON: iPhone klavyesi gonderince kapaniyor; panel de hemen yerine insin
		return result


class ChatTab(uiMobileStyle.Pressable):
	def __init__(self, text, color):
		width = uiMobileStyle.TextWidth(text, TAB_FONT) + TAB_PADDING * 2
		uiMobileStyle.Pressable.__init__(self, width, TAB_HEIGHT)
		self.color = uiMobileStyle.Color(*color)
		# secili sekme: kanal renginde koyu cam zemin + ince ust cizgi + kalin alt cizgi
		tint = uiMobileStyle.Color(color[0] // 7 + 8, color[1] // 7 + 12, color[2] // 7 + 22)
		self.glass = uiMobileStyle.CreateBar(self, tint, 0, 2, width, TAB_HEIGHT - 2)
		self.glass.Hide()
		self.top = uiMobileStyle.CreateBar(self, self.color, 6, 1, width - 12, 1)
		self.top.Hide()
		self.underline = uiMobileStyle.CreateBar(self, self.color, 4, TAB_HEIGHT - 3, width - 8, 3)
		self.underline.Hide()
		self.label = uiMobileStyle.CreateText(self, text, width // 2, TAB_HEIGHT // 2 - 10, self.color, TAB_FONT, "center", True)
		self.selected = False

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.glass = None
		self.top = None
		self.underline = None
		self.label = None

	def SetSelected(self, selected):
		self.selected = selected
		self.RefreshVisual()

	def RefreshVisual(self):
		if not getattr(self, "label", None):
			return
		self.label.SetPackedFontColor(self.color)
		for part in (self.glass, self.top, self.underline):
			if self.selected:
				part.Show()
			else:
				part.Hide()


class InsetField(ui.Window):
	# koyu girinti + ince bronz kenar; odaktayken kenar acilir
	def __init__(self, width, height):
		ui.Window.__init__(self)
		self.AddFlag("not_pick")
		self.SetSize(width, height)
		self.fill = uiMobileStyle.CreateBar(self, FIELD_COLOR, 0, 0, width, height)
		self.borders = [
			uiMobileStyle.CreateBar(self, FIELD_BORDER, 0, 0, width, 1),
			uiMobileStyle.CreateBar(self, FIELD_BORDER, 0, height - 1, width, 1),
			uiMobileStyle.CreateBar(self, FIELD_BORDER, 0, 0, 1, height),
			uiMobileStyle.CreateBar(self, FIELD_BORDER, width - 1, 0, 1, height),
		]
		self.focused = False

	def __del__(self):
		ui.Window.__del__(self)

	def SetFocused(self, focused):
		if focused == self.focused:
			return
		self.focused = focused
		for bar in self.borders:
			bar.SetColor(FIELD_BORDER_FOCUS if focused else FIELD_BORDER)


class DockFrame(ui.Window):
	# acik panelin gorseli: sabit ust parca + dikeyde esneyen tek govde (dikis ve bosluk olusmaz)
	def __init__(self):
		ui.Window.__init__(self)
		self.AddFlag("not_pick")
		self.top = ui.ExpandedImageBox()
		self.top.SetParent(self)
		self.top.AddFlag("not_pick")
		self.top.LoadImage(TOP_ASSET)
		self.top.Show()
		self.body = ui.ExpandedImageBox()
		self.body.SetParent(self)
		self.body.AddFlag("not_pick")
		self.body.LoadImage(BODY_ASSET)
		self.body.SetPosition(0, TOP_HEIGHT - 1)
		self.body.Hide()

	def __del__(self):
		ui.Window.__del__(self)

	def SetSize(self, width, height):
		ui.Window.SetSize(self, width, height)
		scaleX = width / TOP_ASSET_SIZE[0]
		self.top.SetScale(scaleX, 0.5)
		if height < TOP_HEIGHT:
			self.top.SetRenderingRect(0.0, 0.0, 0.0, -(1.0 - height / float(TOP_HEIGHT)))
		else:
			self.top.SetRenderingRect(0.0, 0.0, 0.0, 0.0)
		bodyHeight = height - TOP_HEIGHT + 1
		if bodyHeight > 0:
			self.body.SetScale(scaleX, bodyHeight / float(BODY_ASSET_HEIGHT))
			self.body.Show()
		else:
			self.body.Hide()


class GripArea(ui.Window):
	# panelin ust seridi: yukari surukleyince pencere yukselir (gecmis mesajlari okumak icin), asagi surukleyince alcalir
	def __init__(self, width, height, getHeight, setHeight, finish):
		ui.Window.__init__(self)
		self.SetSize(width, height)
		self.getHeight = getHeight
		self.setHeight = setHeight
		self.finish = finish
		self.dragging = False
		self.startY = 0
		self.startHeight = 0

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		self.getHeight = None
		self.setHeight = None
		self.finish = None
		self.Hide()

	def OnMouseLeftButtonDown(self):
		self.dragging = True
		self.startY = wndMgr.GetMousePosition()[1]
		self.startHeight = self.getHeight()
		return True

	def OnMouseLeftButtonUp(self):
		if self.dragging:
			self.dragging = False
			self.finish()
		return True

	def OnUpdate(self):
		if self.dragging:
			self.setHeight(self.startHeight + self.startY - wndMgr.GetMousePosition()[1])


class ChatLines(ui.Window):
	def __init__(self, chatID):
		ui.Window.__init__(self)
		self.chatID = chatID
		self.interface = None
		self.dragging = False
		self.dragStartY = 0
		self.dragStartX = 0
		self.dragStartPosition = 1.0
		self.position = 1.0
		self.pressLink = ""

	def __del__(self):
		ui.Window.__del__(self)

	def SetInterface(self, interface):
		self.interface = interface

	def ResetScroll(self):
		self.position = 1.0
		chat.SetEndPos(self.chatID, self.position)

	def OnMouseLeftButtonDown(self):
		self.dragging = True
		(self.dragStartX, self.dragStartY) = wndMgr.GetMousePosition()
		self.dragStartPosition = self.position
		# AKRAVON: dokunulan yerdeki bag (esya / oyuncu ismi) basma aninda okunur
		self.pressLink = ui.GetHyperlink()
		return True

	def OnMouseLeftButtonUp(self):
		self.dragging = False
		(x, y) = wndMgr.GetMousePosition()
		moved = abs(x - self.dragStartX) + abs(y - self.dragStartY)
		link = self.pressLink or ui.GetHyperlink()
		self.pressLink = ""
		if link and moved < TAP_SLOP and self.interface:
			try:
				self.interface.MakeHyperlinkTooltip(link)
			except Exception:
				pass
		return True

	def OnUpdate(self):
		if not self.dragging:
			return
		hidden = chat.GetLineCount(self.chatID) - chat.GetVisibleLineCount(self.chatID)
		if hidden <= 0:
			return
		step = max(1, chat.GetLineStep(self.chatID))
		movedLines = (wndMgr.GetMousePosition()[1] - self.dragStartY) / float(step)
		position = max(0.0, min(1.0, self.dragStartPosition - movedLines / hidden))
		if position != self.position:
			self.position = position
			chat.SetEndPos(self.chatID, position)

	def OnRender(self):
		(x, y) = self.GetGlobalPosition()
		height = self.GetHeight()
		# motor "yukseklik + 1 satir" sigdirir: fazladan satir basligin ustune tasiyordu
		step = max(1, chat.GetLineStep(self.chatID))
		chat.SetHeight(self.chatID, max(0, height - step))
		chat.ArrangeShowingChat(self.chatID)
		chat.SetPosition(self.chatID, x, y + height)
		chat.Update(self.chatID)
		chat.Render(self.chatID)


class ModeButton(uiMobileStyle.Pressable):
	# AKRAVON: sol dugme - dokundukca Normal > Grup > Lonca > Bagir doner (hem yazma hem gorunen sekme)
	def __init__(self, width, height):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.box = InsetField(width, height)
		self.box.SetParent(self)
		self.box.Show()
		self.label = uiMobileStyle.CreateText(self, "", width // 2, height // 2 - 12, uiMobileStyle.BRIGHT_COLOR, "LatoBd:18", "center")

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.box = None
		self.label = None

	def SetMode(self, text, color):
		self.label.SetText(uiMobileStyle.TrUpper(text))
		self.label.SetPackedFontColor(uiMobileStyle.Color(color[0], color[1], color[2]))


MODE_NAMES = ("Normal", "Grup", "Lonca", "Bağır")


class ChatPanel(ui.Window):
	def __init__(self, interface):
		import uiMobileChrome
		ui.Window.__init__(self, "UI")
		# AKRAVON: telefonda (kucuk mantiksal ekran) sohbet yazisi buyuk, acikken yalniz ~4 satir gorunur (goz yorgunlugu)
		global PANEL_WIDTH, PANEL_HEIGHT, HEADER_HEIGHT, INPUT_ROW_HEIGHT, FIELD_HEIGHT, BUTTON_SIZE, TAB_HEIGHT, TAB_FONT, TAB_PADDING, TAB_GAP, INPUT_FONT, COLLAPSE_SIZE
		self.chatStep = 15
		if wndMgr.GetScreenHeight() <= 700:
			PANEL_WIDTH = PHONE_PANEL_WIDTH
			chat.SetFontSize(18)
			self.chatStep = 24
			# AKRAVON: telefonda dokunmatik boyutlar - yazi alani, gonder / baglanti dugmeleri ve sekmeler buyuk
			HEADER_HEIGHT = 58
			INPUT_ROW_HEIGHT = 62
			FIELD_HEIGHT = 48
			BUTTON_SIZE = 48
			COLLAPSE_SIZE = 36
			TAB_HEIGHT = 40
			TAB_FONT = "LatoBd:19"
			TAB_PADDING = 20
			TAB_GAP = 10
			INPUT_FONT = "Lato:19"
			PANEL_HEIGHT = HEADER_HEIGHT + LINE_PADDING_Y + 4 * self.chatStep + INPUT_ROW_HEIGHT + 2
		self.SetSize(PANEL_WIDTH, PANEL_HEIGHT)
		ui.EnableSingleTap(self)
		ApplyLineColors()

		self.interface = proxy(interface) if interface else None
		global activePanel
		activePanel = proxy(self)
		self.collapseEvent = None
		self.expandEvent = None
		self.selectedTab = -1
		self.sendMode = 0	# 0 Normal, 1 Grup, 2 Lonca, 3 Bagir (yalniz yazma onegi; gorunen mesajlar hep hepsi)
		self.homeX = 0
		self.homeBottom = 0
		self.panelHeight = self.__LoadHeight()
		self.layoutHeight = self.panelHeight
		self.layoutBottom = 0
		self.keyboardShift = False

		self.chatID = chat.CreateChatSet(MOBILE_CHAT_SET)
		chat.SetStep(self.chatID, self.chatStep)
		chat.SetBoardState(self.chatID, chat.BOARD_STATE_LOG)

		self.frame = DockFrame()
		self.frame.SetParent(self)
		self.frame.SetSize(PANEL_WIDTH, self.panelHeight)
		self.frame.Show()

		# ust serit (sekmelerin altinda): yukari surukle = pencereyi buyut
		self.grip = GripArea(PANEL_WIDTH - COLLAPSE_SIZE - 24, HEADER_HEIGHT, ui.__mem_func__(self.GetPanelHeight), ui.__mem_func__(self.SetPanelHeight), ui.__mem_func__(self.__SaveHeight))
		self.grip.SetParent(self)
		self.grip.SetPosition(0, 0)
		self.grip.Show()

		self.divider = uiMobileStyle.CreateBar(self, DIVIDER_COLOR, 14, HEADER_HEIGHT - 1, PANEL_WIDTH - 28, 1)

		self.headerTitle = uiMobileStyle.CreateText(self, "S O H B E T", TAB_LEFT + 6, HEADER_HEIGHT // 2 - 10, uiMobileStyle.Color(222, 184, 110), "LatoBd:18")
		self.tabs = []
		x = TAB_LEFT
		for (index, (labelName, modeName, prefix, color)) in enumerate(TABS):
			tab = ChatTab(getattr(uiScriptLocale, labelName, labelName), color)
			tab.SetParent(self)
			tab.SetPosition(x, GRIP_HEIGHT)
			tab.SetEvent(ui.__mem_func__(self.SelectTab), index)
			tab.Hide()	# AKRAVON: ust sekmeler yok - sol mod dugmesi var
			x += tab.GetWidth() + TAB_GAP
			self.tabs.append(tab)

		self.collapseButton = uiMobileChrome.HudButton(COLLAPSE_SIZE, "collapse")
		self.collapseButton.SetParent(self)
		self.collapseButton.SetPosition(PANEL_WIDTH - COLLAPSE_SIZE - 14, (HEADER_HEIGHT - COLLAPSE_SIZE) // 2 + 1)
		self.collapseButton.SetEvent(ui.__mem_func__(self.__OnCollapse))
		self.collapseButton.Show()

		self.linesBack = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(0, 0, 0, 165), LINE_PADDING_X - 6, HEADER_HEIGHT + 2, PANEL_WIDTH - LINE_PADDING_X * 2 + 12, 40)	# orijinal Metin2 sohbet zemini (siyah, yari saydam)
		self.lines = ChatLines(self.chatID)
		self.lines.SetInterface(self.interface)
		self.lines.SetParent(self)
		self.lines.SetPosition(LINE_PADDING_X, HEADER_HEIGHT + LINE_PADDING_Y)
		self.lines.Show()

		self.modeButton = ModeButton(BUTTON_SIZE * 2, BUTTON_SIZE)
		self.modeButton.SetParent(self)
		self.modeButton.SetEvent(ui.__mem_func__(self.NextMode))
		self.modeButton.Show()

		self.linkButton = uiMobileChrome.HudButton(BUTTON_SIZE, "link")
		self.linkButton.SetParent(self)
		self.linkButton.SetEvent(ui.__mem_func__(self.RequestItemLink))
		self.linkButton.Show()

		fieldX = INPUT_SIDE + BUTTON_SIZE * 2 + INPUT_GAP
		fieldWidth = PANEL_WIDTH - fieldX - INPUT_GAP - BUTTON_SIZE - INPUT_GAP - BUTTON_SIZE - INPUT_SIDE
		self.field = uiMobileStyle.Pressable(fieldWidth, FIELD_HEIGHT)
		self.field.SetParent(self)
		self.field.SetPosition(fieldX, 0)
		self.field.SetEvent(ui.__mem_func__(self.FocusInput))
		self.field.Show()
		self.fieldPanel = InsetField(fieldWidth, FIELD_HEIGHT)
		self.fieldPanel.SetParent(self.field)
		self.fieldPanel.Show()

		self.input = ChatInput()
		self.input.SetParent(self.field)
		self.input.SetMax(CHAT_MAX)
		self.input.SetUserMax(CHAT_USER_MAX)
		self.input.SetFontName(INPUT_FONT)
		self.input.SetPosition(FIELD_TEXT_X, FIELD_HEIGHT // 2 - int(INPUT_FONT.split(":")[1]) // 2 - 2)
		self.input.SetSize(fieldWidth - FIELD_TEXT_X * 2, 26)
		self.input.SetPackedFontColor(uiMobileStyle.BRIGHT_COLOR)
		self.input.SetPlaceHolderColor(PLACEHOLDER_COLOR)
		self.input.SetPlaceHolder(getattr(uiScriptLocale, "MOBILE_CHAT_PLACEHOLDER", ""))
		self.input.SetReturnEvent(ui.__mem_func__(self.__OnEmptyReturn))
		self.input.SetEscapeEvent(ui.__mem_func__(self.__OnCollapse))
		if interface:
			self.input.BindInterface(interface)
		self.input.Show()

		self.sendButton = uiMobileChrome.HudButton(BUTTON_SIZE, "send")
		self.sendButton.SetParent(self)
		self.sendButton.SetEvent(ui.__mem_func__(self.Send))
		self.sendButton.Show()

		self.contents = [self.grip, self.divider, self.headerTitle] + [self.collapseButton, self.linesBack, self.lines, self.modeButton, self.linkButton, self.field, self.sendButton]
		self.collapsedBox = (0, 0, PANEL_WIDTH, COLLAPSED_HEIGHT)
		self.animation = None
		self.closedEvent = None
		self.pendingFocus = False

		self.__Place(self.panelHeight, 0)
		self.SelectTab(0)

	def __del__(self):
		ui.Window.__del__(self)

	def __LoadHeight(self):
		import uiMobileHud
		# AKRAVON: telefonda yeni (buyuk yazi / 4 satir) yerlesim - eski kayitli yukseklik bir kez sifirlanir
		if wndMgr.GetScreenHeight() <= 700:
			config = uiMobileHud.LoadConfig()
			if config.get("CHAT_LAYOUT") != "2":
				config["CHAT_LAYOUT"] = "2"
				config[uiMobileHud.CONFIG_CHAT_HEIGHT] = str(PANEL_HEIGHT)
				uiMobileHud.SaveConfig()
		return max(PANEL_HEIGHT, min(self.__MaxHeight(), uiMobileHud.GetChatHeight(PANEL_HEIGHT)))

	def __SaveHeight(self):
		import uiMobileHud
		uiMobileHud.SetChatHeight(self.panelHeight)

	def __MaxHeight(self):
		return wndMgr.GetScreenHeight() - MAX_BOTTOM_GAP

	def GetPanelHeight(self):
		return self.panelHeight

	def SetPanelHeight(self, height):
		height = max(PANEL_HEIGHT, min(self.__MaxHeight(), int(height)))
		if height == self.panelHeight:
			return
		self.panelHeight = height
		self.__Place(height, self.layoutBottom)

	def __Place(self, height, bottom):
		# panel alt kenara sabit; yukseklik degisince ust kenar hareket eder, icerik yeniden dizilir
		self.layoutHeight = height
		self.layoutBottom = bottom
		ui.Window.SetSize(self, PANEL_WIDTH, height)
		self.SetPosition(self.homeX, bottom - height)
		if not self.animation:
			self.frame.SetSize(PANEL_WIDTH, height)
		self.lines.SetSize(PANEL_WIDTH - LINE_PADDING_X * 2, height - INPUT_ROW_HEIGHT - 2 - HEADER_HEIGHT - LINE_PADDING_Y)
		self.linesBack.SetPosition(LINE_PADDING_X - 6, HEADER_HEIGHT + 4)
		self.linesBack.SetSize(PANEL_WIDTH - LINE_PADDING_X * 2 + 12, max(10, height - INPUT_ROW_HEIGHT - HEADER_HEIGHT - 6))
		inputY = height - FIELD_HEIGHT - INPUT_BOTTOM_PAD
		self.modeButton.SetPosition(INPUT_SIDE, inputY)
		self.field.SetPosition(INPUT_SIDE + BUTTON_SIZE * 2 + INPUT_GAP, inputY)
		self.linkButton.SetPosition(PANEL_WIDTH - INPUT_SIDE - BUTTON_SIZE * 2 - INPUT_GAP, inputY)
		self.sendButton.SetPosition(PANEL_WIDTH - INPUT_SIDE - BUTTON_SIZE, inputY)

	def Destroy(self):
		global linkRequester
		if linkRequester is not None:
			linkRequester = None
		self.collapseEvent = None
		self.expandEvent = None
		self.closedEvent = None
		self.animation = None
		self.contents = []
		self.KillInputFocus()
		for tab in self.tabs:
			tab.Destroy()
		self.tabs = []
		self.collapseButton.Destroy()
		self.sendButton.Destroy()
		self.linkButton.Destroy()
		self.modeButton.Destroy()
		self.field.Destroy()
		self.collapseButton = None
		self.sendButton = None
		self.linkButton = None
		self.modeButton = None
		self.field = None
		self.fieldPanel = None
		self.input = None
		self.lines = None
		self.linesBack = None
		self.frame = None
		self.divider = None
		self.grip.Destroy()
		self.grip = None
		self.interface = None
		self.Hide()

	def SetCollapseEvent(self, event):
		self.collapseEvent = event

	def SetExpandEvent(self, event):
		self.expandEvent = event

	def RequestItemLink(self):
		# envanteri ac, panel kapansin; envanterde dokunulan esya baglanti olarak yazi alanina eklenir
		global linkRequester
		linkRequester = self
		try:
			if not self.interface.wndInventory.IsShow():
				self.interface.ToggleInventoryWindow()
		except Exception:
			pass
		chat.AppendChat(chat.CHAT_TYPE_INFO, getattr(uiScriptLocale, "MOBILE_CHAT_PICK_ITEM", "Eklemek istedigin esyaya dokun"))
		self.__OnCollapse()

	def InsertLink(self, link):
		if self.expandEvent:
			self.expandEvent()
		if link and self.input:
			self.input.SetText(self.input.GetText() + link)

	def SetHomePosition(self, x, bottom):
		self.homeX = x
		self.homeBottom = bottom
		self.keyboardShift = False
		self.__Place(self.panelHeight, bottom)

	def SetCollapsedBox(self, x, y, width, height):
		# ekran koordinati: kapali seridin kutusu (acilis / kapanis animasyonu buradan baslar)
		self.collapsedBox = (x, y, min(width, PANEL_WIDTH), height)

	def Open(self):
		self.closedEvent = None
		progress = self.__GetProgress() if self.IsShow() else 0.0
		self.__SetContentVisible(False)
		self.animation = (app.GetTime() - progress * OPEN_DURATION, OPEN_DURATION, True)
		self.__ApplyFrame(progress)
		self.Show()
		self.SetTop()

	def Close(self, event):
		self.KillInputFocus()
		self.pendingFocus = False
		if not self.IsShow():
			self.animation = None
			if event:
				event()
			return
		progress = self.__GetProgress()
		self.__SetContentVisible(False)
		self.closedEvent = event
		self.animation = (app.GetTime() - (1.0 - progress) * CLOSE_DURATION, CLOSE_DURATION, False)

	def CloseNow(self):
		self.KillInputFocus()
		self.pendingFocus = False
		self.animation = None
		self.closedEvent = None
		self.Hide()
		self.__ApplyFrame(1.0)

	def __GetProgress(self):
		if not self.animation:
			return 1.0
		(start, duration, opening) = self.animation
		elapsed = max(0.0, min(1.0, (app.GetTime() - start) / duration))
		return elapsed if opening else 1.0 - elapsed

	def __SetContentVisible(self, visible):
		for widget in self.contents:
			if visible:
				widget.Show()
			else:
				widget.Hide()

	def __ApplyFrame(self, progress):
		eased = 1.0 - (1.0 - progress) ** 3
		(boxX, boxY, boxWidth, boxHeight) = self.collapsedBox
		relX = boxX - self.homeX
		relY = boxY - (self.layoutBottom - self.layoutHeight)
		width = int(round(boxWidth + (PANEL_WIDTH - boxWidth) * eased))
		height = int(round(boxHeight + (self.layoutHeight - boxHeight) * eased))
		self.frame.SetPosition(int(round(relX * (1.0 - eased))), int(round(relY * (1.0 - eased))))
		self.frame.SetSize(width, height)

	def __UpdateAnimation(self):
		if not self.animation:
			return
		(start, duration, opening) = self.animation
		elapsed = max(0.0, min(1.0, (app.GetTime() - start) / duration))
		self.__ApplyFrame(elapsed if opening else 1.0 - elapsed)
		if elapsed < 1.0:
			return
		self.animation = None
		if opening:
			self.__SetContentVisible(True)
			if self.pendingFocus:
				self.pendingFocus = False
				self.FocusInput()
			return
		self.Hide()
		self.__ApplyFrame(1.0)
		event = self.closedEvent
		self.closedEvent = None
		if event:
			event()

	def SelectTab(self, index):
		if index == self.selectedTab:
			return
		self.selectedTab = index
		(labelName, modeName, prefix, color) = TABS[index]
		selectedMode = getattr(chat, modeName, None) if modeName else None
		for mode in GetAllModes():
			if selectedMode is None or mode == selectedMode:
				chat.EnableChatMode(self.chatID, mode)
			else:
				chat.DisableChatMode(self.chatID, mode)
		for (position, tab) in enumerate(self.tabs):
			tab.SetSelected(position == index)
		self.ApplySendMode()
		self.lines.ResetScroll()

	def ApplySendMode(self):
		self.input.SetPrefix(TABS[self.sendMode + 1][2])
		self.modeButton.SetMode(MODE_NAMES[self.sendMode], TABS[self.sendMode + 1][3])

	def NextMode(self):
		self.sendMode = (self.sendMode + 1) % 4
		self.ApplySendMode()

	def FocusInput(self):
		import dbg
		dbg.TraceError("[TIK] FocusInput t=%.3f odak=%d" % (app.GetTime(), 1 if (self.input and self.input.IsFocus()) else 0))	# GECICI IZ
		if self.animation and self.animation[2]:
			self.pendingFocus = True
			return
		if self.input:
			self.input.SetFocus()

	def KillInputFocus(self):
		import dbg
		dbg.TraceError("[TIK] KillInputFocus t=%.3f" % app.GetTime())	# GECICI IZ
		if self.input and self.input.IsFocus():
			self.input.KillFocus()

	def IsTyping(self):
		return bool(self.input and self.input.IsFocus())

	def Send(self):
		if not self.input:
			return
		if self.input.GetText().strip():
			self.input.OnIMEReturn()
			self.lines.ResetScroll()
		else:
			self.FocusInput()

	def __OnEmptyReturn(self):
		self.KillInputFocus()

	def __OnCollapse(self):
		self.KillInputFocus()
		if self.collapseEvent:
			self.collapseEvent()

	def OnPressEscapeKey(self):
		self.__OnCollapse()
		return True

	def __ClampInputWidth(self):
		# AKRAVON: yazi kutunun disina tasmasin - kutu dolunca fazlasi kesilir (yazma siniri = kutu genisligi)
		try:
			limit = self.input.GetWidth()
			width = self.input.GetTextSize()[0]
		except Exception:
			return
		if width <= limit:
			return
		text = self.input.GetText()
		guard = 0
		while text and width > limit and guard < 64:
			text = text[:-1]
			self.input.SetText(text)
			width = self.input.GetTextSize()[0]
			guard += 1

	def OnUpdate(self):
		self.__UpdateAnimation()
		# AKRAVON: acik sohbet sag alttaki beceri / saldiri dugmelerinin altinda kalmasin
		now = app.GetTime()
		if self.IsShow() and now - getattr(self, "lastRaise", 0.0) > 0.5:
			self.lastRaise = now
			self.SetTop()
		if not self.input:
			return
		self.fieldPanel.SetFocused(self.input.IsFocus())
		if self.input.IsFocus():
			self.__ClampInputWidth()
		shift = IS_IOS and self.input.IsFocus()	# iOS: klavye paneli kapatir, yazarken panel klavyenin ustune oturur (Android: yerinde kalir)
		if shift != self.keyboardShift:
			self.keyboardShift = shift
			if shift:
				# klavye acikken: panel klavyenin ustune oturur, ekrana sigmazsa kisalir
				bottom = int(wndMgr.GetScreenHeight() * KEYBOARD_BOTTOM_RATIO)
				self.__Place(max(PANEL_HEIGHT // 2, min(self.panelHeight, bottom - 4)), bottom)
			else:
				self.__Place(self.panelHeight, self.homeBottom)
