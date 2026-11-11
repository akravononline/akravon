# -*- coding: utf-8 -*-
# AKRAVON: sohbet - sifirdan, mobil oyun icin; orijinal Metin2 mesaj akisi (karartmali, solan satirlar) + bizim tasarim dilimiz.
#
#  KAPALI : joystigin altinda sohbet dugmesi (okunmamis rozeti); yaninda gelen mesajlar karartili akar, sonra solar.
#  ACIK   : ust sade lacivert/altin serit (SOHBET + kapat), mesaj alani Metin2'nin siyah yari saydam zemini (surukleyerek kaydirilir),
#           altta TEK satir: [MOD] [yazi kutusu] [baglanti] [gonder]. Satir ekranin dibine yapisik; iPhone klavyesi acilinca
#           klavyenin ustune cikar, klavye kapaninca sohbet de kapanir.
#  MOD    : dokundukca Normal > Grup > Lonca > Bagir; # ! % isareti kutuya yazilmaz, gonderirken eklenir.
from __future__ import division
import app
import math
import chat
import grp
import ui
import wndMgr
import uiMobileStyle
import uiMobileChat
import uiMobileChrome

from _weakref import proxy

# ---- olculer (telefon / buyuk ekran) ----
PHONE = wndMgr.GetScreenHeight() <= 700
STEP = 24 if PHONE else 18			# mesaj satir adimi
FONT_SIZE = 18 if PHONE else 14
ROW = 54 if PHONE else 44			# alt satir yuksekligi
BTN = 46 if PHONE else 38
MODE_W = 104 if PHONE else 84
TOP_H = 32							# ust serit
AREA_LINES = 6						# acikken gorunen mesaj satiri
GAP = 14							# mesaj alani ile yazi satiri arasi
SIDE = 8
PAD = 6
SHEET_MAX_W = 700
BUBBLE_W = 72
BUBBLE_H = 54
BUBBLE_X = 34	# joystik bilinmiyorsa yedek konum
BUBBLE_BOTTOM = 6	# alt kenara yapisik (yalniz kucuk pay)
BUBBLE_GAP = 12	# joystik ile balon arasi
FLOAT_W = 560
FLOAT_LINES = 5
SET_LOG = uiMobileChat.MOBILE_CHAT_SET
SET_VIEW = uiMobileChat.FLOAT_CHAT_SET
AUTO_CLOSE = 0.45					# klavye kapandiktan sonra sohbetin kapanma gecikmesi
BAR_BACK = uiMobileStyle.Color(0, 0, 0, 160)	# Metin2 siyah zemini
VIEW_START = grp.GenerateColor(0.0, 0.0, 0.0, 0.0)
VIEW_END = grp.GenerateColor(0.0, 0.0, 0.0, 0.75)
GOLD = uiMobileStyle.Color(222, 184, 110)
EDGE = uiMobileStyle.Color(150, 118, 58)
NAVY = uiMobileStyle.Color(12, 20, 40, 242)
MODES = (
	("Normal", "", (255, 255, 255)),	# orijinal Metin2 sohbet renkleri (PythonChat.cpp)
	("Grup", "#", (138, 255, 242)),
	("Lonca", "%", (231, 216, 255)),
	("Bağır", "!", (168, 255, 213)),
)


def _EnableAll(chatID):
	for mode in uiMobileChat.GetAllModes():
		chat.EnableChatMode(chatID, mode)


class ViewLines(ui.Window):
	# kapaliyken akan mesajlar: orijinal Metin2 gibi karartili; satirlar zamanla solar
	def __init__(self):
		ui.Window.__init__(self, "UI")
		self.AddFlag("not_pick")
		self.chatID = chat.CreateChatSet(SET_VIEW)
		chat.SetStep(self.chatID, STEP)
		chat.SetBoardState(self.chatID, chat.BOARD_STATE_VIEW)
		_EnableAll(self.chatID)
		self.boxHeight = FLOAT_LINES * STEP

	def __del__(self):
		ui.Window.__del__(self)

	def OnRender(self):
		(x, y) = self.GetGlobalPosition()
		width = self.GetWidth()
		chat.SetHeight(self.chatID, max(0, self.boxHeight - STEP))
		chat.ArrangeShowingChat(self.chatID)
		chat.SetPosition(self.chatID, x + 12, y + self.boxHeight)
		chat.Update(self.chatID)
		visible = chat.GetVisibleLineCount(self.chatID)
		if visible > 0:
			textHeight = min(self.boxHeight, visible * STEP + 12)
			grp.RenderGradationBar(x, y + self.boxHeight - textHeight, width, textHeight, VIEW_START, VIEW_END)
		chat.Render(self.chatID)


class ModeButton(uiMobileStyle.Pressable):
	def __init__(self, width, height):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.field = uiMobileChat.InsetField(width, height)
		self.field.SetParent(self)
		self.field.Show()
		self.label = uiMobileStyle.CreateText(self, "", width // 2, height // 2 - 11, uiMobileStyle.TEXT_COLOR, "LatoBd:19" if PHONE else "LatoBd:15", "center")

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.field = None
		self.label = None

	def SetMode(self, index):
		(name, prefix, color) = MODES[index]
		self.label.SetText(name)
		self.label.SetPackedFontColor(uiMobileStyle.Color(color[0], color[1], color[2]))


class ChatButton(uiMobileStyle.Pressable):
	# AKRAVON: "Gorev" ikonu gibi - arkasi yok, ikon + altinda "Sohbet"; yeni mesajda kirmizi yanip soner
	ICON_W = 40
	LABEL_COLOR = uiMobileStyle.Color(250, 228, 176)

	def __init__(self):
		self.iconHeight = self.ICON_W
		uiMobileStyle.Pressable.__init__(self, self.ICON_W + 14, self.iconHeight + 22)
		self.icon = ui.ExpandedImageBox()
		self.icon.SetParent(self)
		self.icon.AddFlag("not_pick")
		self.icon.LoadImage("mobile_ui/akravon/hud/chat_icon.png")
		scale = self.ICON_W / float(max(1, self.icon.GetWidth()))
		self.icon.SetScale(scale, scale)
		self.icon.SetPosition(7, 0)
		self.icon.Show()
		self.label = uiMobileStyle.CreateText(self, "Sohbet", self.GetWidth() // 2, self.iconHeight + 2, self.LABEL_COLOR, "LatoBd:15", "center", True)
		self.alert = False
		self.unread = 0
		# sayi rozeti: ikonun sag ust kosesi; rakam arttikca genisler (9, 10, 99+)
		dark = uiMobileStyle.Color(70, 10, 12)
		red = uiMobileStyle.Color(214, 48, 48)
		self.badgeParts = (
			uiMobileStyle.CreateBar(self, dark, 0, 0, 1, 1),	# dis cerceve (koseleri kesik)
			uiMobileStyle.CreateBar(self, dark, 0, 0, 1, 1),
			uiMobileStyle.CreateBar(self, red, 0, 0, 1, 1),	# ic dolgu
			uiMobileStyle.CreateBar(self, red, 0, 0, 1, 1),
		)
		for part in self.badgeParts:
			part.Hide()
		self.badgeText = uiMobileStyle.CreateText(self, "", 0, 0, uiMobileStyle.BRIGHT_COLOR, "LatoBd:12", "center", True)
		self.badgeText.Hide()
		self.badgeShown = None

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.icon = None
		self.label = None
		self.badgeParts = ()
		self.badgeText = None

	def __SetBadge(self, count):
		text = "99+" if count > 99 else str(count)
		if text == self.badgeShown:
			return
		self.badgeShown = text
		height = 18
		width = 18 + 6 * (len(text) - 1)
		right = 7 + self.ICON_W + 3	# ikonun sag ust kosesine oturur
		x = right - width
		y = -1
		(outerA, outerB, innerA, innerB) = self.badgeParts
		outerA.SetPosition(x + 1, y)
		outerA.SetSize(width - 2, height)
		outerB.SetPosition(x, y + 1)
		outerB.SetSize(width, height - 2)
		innerA.SetPosition(x + 2, y + 1)
		innerA.SetSize(width - 4, height - 2)
		innerB.SetPosition(x + 1, y + 2)
		innerB.SetSize(width - 2, height - 4)
		self.badgeText.SetPosition(x + width // 2, y + height // 2 - 7)
		self.badgeText.SetText(text)
		for part in self.badgeParts:
			part.Show()
		self.badgeText.Show()

	def SetUnreadCount(self, count):
		self.unread = max(0, count)
		self.alert = self.unread > 0
		if self.alert:
			self.__SetBadge(self.unread)
		else:
			self.badgeShown = None
			for part in self.badgeParts:
				part.Hide()
			self.badgeText.Hide()
		self.__ApplyTint()

	def RefreshVisual(self):
		self.__ApplyTint()

	def __ApplyTint(self):
		# okunmamis mesaj: ikon boyanmaz (bej kalir), yalniz kose rozetindeki kirmizi sayi gosterir
		if getattr(self, "icon", None):
			shade = 0.78 if self.pressed else 1.0
			wndMgr.SetDiffuseColor(self.icon.hWnd, shade, shade, shade, 1.0)


class Grip(ui.Window):
	# ust seridin tutamagi: asagi / yukari surukleyince mesaj alani buyur / kuculur
	def __init__(self, owner, width, height):
		ui.Window.__init__(self, "UI")
		self.owner = proxy(owner)
		self.SetSize(width, height)
		self.dragging = False
		self.startY = 0
		self.startArea = 0

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		self.owner = None
		self.Hide()

	def OnMouseLeftButtonDown(self):
		self.dragging = True
		self.startY = wndMgr.GetMousePosition()[1]
		self.startArea = self.owner.areaHeight
		self.owner.HoldKeyboard(True)	# surukleme sirasinda klavye / sohbet kapanmasin
		return True

	def OnMouseLeftButtonUp(self):
		if self.dragging:
			self.dragging = False
			self.owner.HoldKeyboard(False)
		return True

	def OnUpdate(self):
		if not self.dragging:
			return
		try:
			delta = self.startY - wndMgr.GetMousePosition()[1]
			self.owner.SetArea(self.startArea + delta)
		except ReferenceError:
			self.dragging = False


class Sheet(ui.Window):
	# acik sohbet: ust serit + siyah mesaj alani + tek satir giris
	def __init__(self, interface, closeEvent):
		ui.Window.__init__(self, "UI")
		self.interface = proxy(interface) if interface else None
		self.closeEvent = closeEvent
		self.width = min(wndMgr.GetScreenWidth() - 24, SHEET_MAX_W)
		self.baseArea = AREA_LINES * STEP + 16
		self.areaHeight = self.baseArea
		self.totalHeight = TOP_H + self.areaHeight + GAP + ROW
		self.SetSize(self.width, self.totalHeight)
		ui.EnableSingleTap(self)
		self.mode = 0
		self.bottom = wndMgr.GetScreenHeight()
		self.keyboard = False
		self.openedAt = 0.0
		self.lostSince = None
		self.ignoreUntil = 0.0
		self.refocusAt = None
		self.hadFocus = False
		self.linkPending = None

		self.chatID = chat.CreateChatSet(SET_LOG)
		chat.SetStep(self.chatID, STEP)
		chat.SetBoardState(self.chatID, chat.BOARD_STATE_LOG)
		_EnableAll(self.chatID)

		# ust serit
		self.topEdge = uiMobileStyle.CreateBar(self, EDGE, 0, 0, self.width, TOP_H)
		self.topFill = uiMobileStyle.CreateBar(self, NAVY, 0, 1, self.width, TOP_H - 2)
		self.title = uiMobileStyle.CreateText(self, "S O H B E T", self.width // 2, TOP_H // 2 - 9, GOLD, "LatoBd:16" if PHONE else "LatoBd:13", "center")
		self.handle = uiMobileStyle.CreateBar(self, GOLD, self.width // 2 - 22, 5, 44, 3)	# tutamak: sebeti tutup yukari cek
		self.grip = Grip(self, self.width, TOP_H)
		self.grip.SetParent(self)
		self.grip.SetPosition(0, 0)
		self.grip.Show()

		# mesaj alani (Metin2 siyahi)
		self.back = uiMobileStyle.CreateBar(self, BAR_BACK, 0, TOP_H, self.width, self.areaHeight + GAP + ROW)	# mesaj alani + alt satir arkasi dolu
		self.rowItems = []
		self.lines = uiMobileChat.ChatLines(self.chatID)
		self.lines.SetInterface(self.interface)
		self.lines.SetParent(self)
		self.lines.SetPosition(PAD + 6, TOP_H + 8)
		self.lines.SetSize(self.width - (PAD + 6) * 2, self.areaHeight - 12)
		self.lines.Show()

		# alt satir
		rowY = TOP_H + self.areaHeight + GAP
		fieldY = rowY + (ROW - BTN) // 2
		self.modeButton = ModeButton(MODE_W, BTN)
		self.modeButton.SetParent(self)
		self.modeButton.SetPosition(SIDE, fieldY)
		self.modeButton.SetEvent(ui.__mem_func__(self.NextMode))
		self.modeButton.SetMode(0)
		self.modeButton.Show()

		fieldX = SIDE + MODE_W + PAD
		fieldWidth = self.width - fieldX - PAD - BTN - PAD - BTN - SIDE
		self.field = uiMobileStyle.Pressable(fieldWidth, BTN)
		self.field.SetParent(self)
		self.field.SetPosition(fieldX, fieldY)
		self.field.SetEvent(ui.__mem_func__(self.FocusInput))
		self.field.Show()
		self.fieldPanel = uiMobileChat.InsetField(fieldWidth, BTN)
		self.fieldPanel.SetParent(self.field)
		self.fieldPanel.Show()

		self.input = uiMobileChat.ChatInput()
		self.input.SetParent(self.field)
		self.input.SetMax(uiMobileChat.CHAT_MAX)
		self.input.SetUserMax(uiMobileChat.CHAT_USER_MAX)
		fontName = "Lato:20" if PHONE else "Lato:15"
		self.input.SetFontName(fontName)
		self.input.SetPosition(12, BTN // 2 - int(fontName.split(":")[1]) // 2 - 2)
		self.input.SetSize(fieldWidth - 24, 26)
		self.input.SetPackedFontColor(uiMobileStyle.BRIGHT_COLOR)
		self.input.SetPlaceHolderColor(uiMobileChat.PLACEHOLDER_COLOR)
		self.input.SetPlaceHolder("Mesaj yaz...")
		self.input.SetReturnEvent(ui.__mem_func__(self.KillInputFocus))
		self.input.SetEscapeEvent(ui.__mem_func__(self.Close))
		if interface:
			self.input.BindInterface(interface)
		self.input.Show()
		self.fieldWidth = fieldWidth

		self.linkButton = uiMobileChrome.HudButton(BTN, "link")
		self.linkButton.SetParent(self)
		self.linkButton.SetPosition(fieldX + fieldWidth + PAD, fieldY)
		self.linkButton.SetEvent(ui.__mem_func__(self.RequestItemLink))
		self.linkButton.Show()

		self.sendButton = uiMobileChrome.HudButton(BTN, "send")
		self.sendButton.SetParent(self)
		self.sendButton.SetPosition(self.width - SIDE - BTN, fieldY)
		self.sendButton.SetEvent(ui.__mem_func__(self.Send))
		self.sendButton.Show()
		self.rowItems = [self.modeButton, self.field, self.linkButton, self.sendButton]

		self.Hide()
		self.SetBottom(self.bottom)

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		uiMobileChat.linkRequester = None
		self.closeEvent = None
		self.KillInputFocus()
		for widget in (self.modeButton, self.field, self.linkButton, self.sendButton):
			widget.Destroy()
		self.grip.Destroy()
		self.grip = None
		self.modeButton = None
		self.field = None
		self.linkButton = None
		self.sendButton = None
		self.fieldPanel = None
		self.input = None
		self.lines = None
		self.interface = None
		self.Hide()

	def HoldKeyboard(self, hold):
		now = app.GetTime()
		if hold:
			self.ignoreUntil = now + 60.0
			self.wasTyping = bool(self.input and self.input.IsFocus())
		else:
			self.ignoreUntil = now + 1.0
			if getattr(self, "wasTyping", False):
				self.refocusAt = now + 0.1	# dokunus odagi aldi: klavyeyi geri getir

	# ---- yukseklik (sebeti tutup yukari cekince buyur) ----
	def MaxArea(self):
		keyboardTop = int(wndMgr.GetScreenHeight() * uiMobileChat.KEYBOARD_BOTTOM_RATIO)
		return max(self.baseArea, keyboardTop - TOP_H - GAP - ROW - 24)

	def SetArea(self, height):
		height = int(max(self.baseArea, min(self.MaxArea(), height)))
		if height == self.areaHeight:
			return
		self.areaHeight = height
		self.totalHeight = TOP_H + height + GAP + ROW
		ui.Window.SetSize(self, self.width, self.totalHeight)
		self.back.SetSize(self.width, height + GAP + ROW)
		self.lines.SetSize(self.width - (PAD + 6) * 2, height - 12)
		rowY = TOP_H + height + GAP + (ROW - BTN) // 2
		for item in self.rowItems:
			(x, y) = item.GetLocalPosition()
			item.SetPosition(x, rowY)
		self.SetBottom(self.bottom)

	# ---- yer ----
	def SetBottom(self, bottom):
		self.bottom = bottom
		x = (wndMgr.GetScreenWidth() - self.width) // 2
		self.SetPosition(x, bottom - self.totalHeight)

	# ---- ac / kapat ----
	def Open(self):
		self.openedAt = app.GetTime()
		self.hadFocus = False
		self.lostSince = None
		self.lines.ResetScroll()
		self.Show()
		self.SetTop()
		self.FocusInput()

	def Close(self):
		self.KillInputFocus()
		self.Hide()
		if self.closeEvent:
			self.closeEvent()

	# ---- yazma ----
	def FocusInput(self):
		if self.input:
			self.input.SetFocus()

	def KillInputFocus(self):
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

	def NextMode(self):
		self.mode = (self.mode + 1) % len(MODES)
		self.modeButton.SetMode(self.mode)
		self.input.SetPrefix(MODES[self.mode][1])
		# dokunus yazi kutusunun odagini alir (klavye iner): kisa sure sonra odak geri verilir
		self.ignoreUntil = app.GetTime() + 1.0
		self.refocusAt = app.GetTime() + 0.12

	def RequestItemLink(self):
		# sohbet geri cekilir, envanter acilir; esya secilince envanter kapanir, sohbet + klavye geri gelir
		uiMobileChat.linkRequester = self
		self.linkPending = app.GetTime()
		self.KillInputFocus()
		self.Hide()
		try:
			if not self.interface.wndInventory.IsShow():
				self.interface.ToggleInventoryWindow()
		except Exception:
			pass
		chat.AppendChat(chat.CHAT_TYPE_INFO, "Eklemek istediğin eşyaya dokun")

	def __CloseInventory(self):
		try:
			if self.interface.wndInventory.IsShow():
				self.interface.wndInventory.Close()
		except Exception:
			pass

	def InsertLink(self, link):
		self.linkPending = None
		if link and self.input:
			self.input.SetText(self.input.GetText() + link)
		self.__CloseInventory()
		self.Open()	# sohbet + klavye geri gelir

	def LinkAborted(self):
		# esya secmeden envanter kapandi: sohbet geri gelsin
		self.linkPending = None
		uiMobileChat.linkRequester = None
		self.Open()

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def __ClampInputWidth(self):
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
		if not self.input or not self.IsShow():
			return
		now = app.GetTime()
		focused = self.input.IsFocus()
		self.fieldPanel.SetFocused(focused)
		if focused:
			self.__ClampInputWidth()
		# mod dugmesinden sonra klavyeyi geri getir
		if self.refocusAt is not None and now >= self.refocusAt:
			self.refocusAt = None
			self.input.KillFocus()
			self.input.SetFocus()
			return
		# klavye kapaninca sohbet ANINDA kapanir (once alta inip sonra kapanma yok)
		if now >= self.ignoreUntil and now - self.openedAt >= 0.8:
			if focused:
				self.hadFocus = True
			elif self.hadFocus:
				self.Close()
				return
		# klavye acikken (iOS) satir klavyenin ustune cikar; kapaliyken dibe
		shift = uiMobileChat.IS_IOS and focused
		if shift != self.keyboard:
			self.keyboard = shift
			if shift:
				self.SetBottom(int(wndMgr.GetScreenHeight() * uiMobileChat.KEYBOARD_BOTTOM_RATIO))
			else:
				self.SetBottom(wndMgr.GetScreenHeight())


class Controller(object):
	# uiMobileHudController'in bekledigi arayuz
	def __init__(self, interface):
		self.interface = proxy(interface)
		self.visible = True
		self.expanded = False
		chat.SetFontSize(FONT_SIZE)
		uiMobileChat.ApplyLineColors()
		self.view = ViewLines()
		self.view.SetSize(FLOAT_W, FLOAT_LINES * STEP)
		self.view.Show()
		self.toggle = ChatButton()
		self.toggle.SetEvent(ui.__mem_func__(self.Expand))
		self.sheet = Sheet(interface, ui.__mem_func__(self.__OnClosed))
		uiMobileChat.activePanel = proxy(self.sheet)
		self.seenLineCount = self.__LineCount()
		self.bubbleX = BUBBLE_X
		self.Place()

	def SetAnchor(self, joystick):
		# sol kenar: "Gorev" ikonunun hemen altinda; mesajlar balonun yaninda alttan yukari akar
		try:
			import uiMobileHud
			self.bubbleX = int(uiMobileHud.SCREEN_EDGE)
			questY = uiMobileHud.TOP_MARGIN + uiMobileHud.STATUS_BAR_HEIGHT + uiMobileHud.AFFECT_GAP + 2 * uiMobileHud.AFFECT_ROW_HEIGHT + 20
			self.bubbleX = int(uiMobileHud.SCREEN_EDGE) - 7	# "Gorev" ikonuyla ayni sol hiza
			self.bubbleY = int(questY + 65 + 8)	# gorev ikonu (43) + etiket (22) + bosluk
			(jx, jy, js) = joystick
			self.viewX = int(jx + js + BUBBLE_GAP)
		except Exception:
			self.bubbleX = BUBBLE_X
			self.bubbleY = None
			self.viewX = BUBBLE_X + BUBBLE_W + 8
		self.Place()

	def __LineCount(self):
		try:
			return chat.GetLineCount(self.sheet.chatID)
		except Exception:
			return 0

	def __HideDesktopChat(self):
		try:
			window = self.interface.wndChat
		except ReferenceError:
			return
		if window:
			if window.IsEditMode():
				window.CloseChat()
			if window.IsShow():
				window.Hide()

	def Restore(self):
		self.__HideDesktopChat()

	def Place(self):
		height = wndMgr.GetScreenHeight()
		y = getattr(self, "bubbleY", None)
		if y is None:
			y = height - BUBBLE_H - BUBBLE_BOTTOM
		self.toggle.SetPosition(self.bubbleX, y)
		# mesajlar ekranin altinda, joystigin sagindan baslayarak akar
		self.view.SetPosition(getattr(self, "viewX", 120), height - self.view.GetHeight() - BUBBLE_BOTTOM)

	def Collapse(self, animate = True):
		self.sheet.Close() if self.sheet.IsShow() else self.__OnClosed()

	def Expand(self):
		if not self.visible:
			return
		self.expanded = True
		self.toggle.Hide()
		self.view.Hide()
		self.toggle.SetUnreadCount(0)
		self.sheet.Open()

	def __OnClosed(self):
		self.expanded = False
		self.seenLineCount = self.__LineCount()
		if self.visible:
			self.toggle.Show()
			self.view.Show()

	def Show(self):
		self.visible = True
		if not self.expanded:
			self.toggle.Show()
			self.view.Show()

	def Hide(self):
		self.visible = False
		if self.sheet.IsShow():
			self.sheet.Close()
		self.toggle.Hide()
		self.view.Hide()

	def Update(self):
		self.__HideDesktopChat()
		sheet = self.sheet
		if sheet and sheet.linkPending is not None and app.GetTime() - sheet.linkPending > 0.6:
			try:
				inventoryOpen = self.interface.wndInventory.IsShow()
			except Exception:
				inventoryOpen = False
			if not inventoryOpen:
				sheet.LinkAborted()
		count = self.__LineCount()
		if self.expanded:
			self.seenLineCount = count
		else:
			self.toggle.SetUnreadCount(count - self.seenLineCount)

	def Destroy(self):
		if self.sheet:
			self.sheet.Destroy()
		self.sheet = None
		if self.toggle:
			self.toggle.Destroy()
		self.toggle = None
		self.view = None
		self.interface = None
