# -*- coding: utf-8 -*-
# AKRAVON: sohbet cubugu - alt kenara yapisik tek satir: [MOD] [yazi kutusu] [esya baglantisi] [gonder]
# Mesajlari oyunun orijinal sohbet penceresi (wndChat) gosterir; bu cubuk yalniz yazma kismidir.
# MOD dugmesi dokundukca Normal -> Grup -> Lonca -> Bagir doner. Yazarken (iOS klavyesi) cubuk klavyenin ustune cikar.
import app
import chat
import ui
import wndMgr
import uiMobileStyle
import uiMobileChat
import uiMobileChrome

from _weakref import proxy

BAR_W = 560
BAR_H = 56
FIELD_H = 46
BTN = 46
SIDE = 8
GAP = 6
BOTTOM_GAP = 6
MODE_W = 96
FONT = "Lato:19"
MODES = (
	("Normal", "", (250, 236, 210)),
	("Grup", "#", (138, 255, 242)),
	("Lonca", "%", (231, 216, 255)),
	("Ba\xf0\xfdr", "!", (168, 255, 213)),
)


class ModeButton(uiMobileStyle.Pressable):
	def __init__(self, width, height):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.box = uiMobileChat.InsetField(width, height)
		self.box.SetParent(self)
		self.box.Show()
		self.label = uiMobileStyle.CreateText(self, "", width // 2, height // 2 - 11, uiMobileStyle.Color(MODES[0][2][0], MODES[0][2][1], MODES[0][2][2]), "LatoBd:18", "center")
		self.index = 0
		self.SetMode(0)

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.box = None
		self.label = None

	def SetMode(self, index):
		self.index = index % len(MODES)
		(name, prefix, color) = MODES[self.index]
		self.label.SetText(uiMobileStyle.TrUpper(name))
		self.label.SetPackedFontColor(uiMobileStyle.Color(color[0], color[1], color[2]))


class ChatBar(ui.Window):
	def __init__(self, interface):
		ui.Window.__init__(self, "UI")
		self.interface = proxy(interface) if interface else None
		self.width = min(BAR_W, wndMgr.GetScreenWidth() - 20)
		self.SetSize(self.width, BAR_H)
		ui.EnableSingleTap(self)
		self.keyboardShift = False
		uiMobileChat.ApplyLineColors()

		self.frame = uiMobileChat.DockFrame()
		self.frame.SetParent(self)
		self.frame.SetSize(self.width, BAR_H)
		self.frame.Show()

		y = (BAR_H - FIELD_H) // 2
		self.mode = ModeButton(MODE_W, FIELD_H)
		self.mode.SetParent(self)
		self.mode.SetPosition(SIDE, y)
		self.mode.SetEvent(ui.__mem_func__(self.NextMode))
		self.mode.Show()

		fieldX = SIDE + MODE_W + GAP
		fieldW = self.width - fieldX - GAP - BTN - GAP - BTN - SIDE
		self.field = uiMobileStyle.Pressable(fieldW, FIELD_H)
		self.field.SetParent(self)
		self.field.SetPosition(fieldX, y)
		self.field.SetEvent(ui.__mem_func__(self.FocusInput))
		self.field.Show()
		self.fieldPanel = uiMobileChat.InsetField(fieldW, FIELD_H)
		self.fieldPanel.SetParent(self.field)
		self.fieldPanel.Show()

		self.input = uiMobileChat.ChatInput()
		self.input.SetParent(self.field)
		self.input.SetMax(uiMobileChat.CHAT_MAX)
		self.input.SetUserMax(uiMobileChat.CHAT_USER_MAX)
		self.input.SetFontName(FONT)
		self.input.SetPosition(12, FIELD_H // 2 - 12)
		self.input.SetSize(fieldW - 24, 26)
		self.input.SetPackedFontColor(uiMobileStyle.BRIGHT_COLOR)
		self.input.SetPlaceHolderColor(uiMobileChat.PLACEHOLDER_COLOR)
		self.input.SetPlaceHolder("Mesaj yaz...")
		self.input.SetReturnEvent(ui.__mem_func__(self.KillInputFocus))
		self.input.SetEscapeEvent(ui.__mem_func__(self.KillInputFocus))
		if interface:
			self.input.BindInterface(interface)
		self.input.Show()

		self.linkButton = uiMobileChrome.HudButton(BTN, "link")
		self.linkButton.SetParent(self)
		self.linkButton.SetPosition(fieldX + fieldW + GAP, y)
		self.linkButton.SetEvent(ui.__mem_func__(self.RequestItemLink))
		self.linkButton.Show()

		self.sendButton = uiMobileChrome.HudButton(BTN, "send")
		self.sendButton.SetParent(self)
		self.sendButton.SetPosition(self.width - SIDE - BTN, y)
		self.sendButton.SetEvent(ui.__mem_func__(self.Send))
		self.sendButton.Show()

		uiMobileChat.activePanel = proxy(self)
		self.SetMode(0)
		self.PlaceHome()
		self.Show()

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		if uiMobileChat.linkRequester is not None:
			uiMobileChat.linkRequester = None
		self.KillInputFocus()
		for widget in (self.mode, self.field, self.linkButton, self.sendButton):
			widget.Destroy()
		self.mode = None
		self.field = None
		self.linkButton = None
		self.sendButton = None
		self.input = None
		self.fieldPanel = None
		self.frame = None
		self.interface = None
		self.Hide()

	# ---- mod ----
	def SetMode(self, index):
		self.mode.SetMode(index)
		(name, prefix, color) = MODES[self.mode.index]
		self.input.SetPrefix(prefix)

	def NextMode(self):
		self.SetMode(self.mode.index + 1)

	# ---- yerlesim ----
	def PlaceHome(self):
		x = (wndMgr.GetScreenWidth() - self.width) // 2
		self.homeBottom = wndMgr.GetScreenHeight() - BOTTOM_GAP
		self.SetPosition(x, self.homeBottom - BAR_H)

	def PlaceKeyboard(self):
		x = (wndMgr.GetScreenWidth() - self.width) // 2
		bottom = int(wndMgr.GetScreenHeight() * uiMobileChat.KEYBOARD_BOTTOM_RATIO)
		self.SetPosition(x, bottom - BAR_H)

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
		else:
			self.FocusInput()

	def RequestItemLink(self):
		# envanteri ac; dokunulan esya baglanti olarak yazi kutusuna eklenir (uiMobileChat.TakeItemSlot)
		uiMobileChat.linkRequester = self
		try:
			if not self.interface.wndInventory.IsShow():
				self.interface.ToggleInventoryWindow()
		except Exception:
			pass
		chat.AppendChat(chat.CHAT_TYPE_INFO, "Eklemek istedi\xf0in e\xfeyaya dokun")

	def InsertLink(self, link):
		if link and self.input:
			self.input.SetText(self.input.GetText() + link)
		self.FocusInput()

	def OnPressEscapeKey(self):
		self.KillInputFocus()
		return True

	def OnUpdate(self):
		if not self.input:
			return
		focused = self.input.IsFocus()
		self.fieldPanel.SetFocused(focused)
		shift = uiMobileChat.IS_IOS and focused
		if shift != self.keyboardShift:
			self.keyboardShift = shift
			if shift:
				self.PlaceKeyboard()
			else:
				self.PlaceHome()


class Controller(object):
	# uiMobileHudController'in bekledigi arayuz (Place / Collapse / Restore / Show / Hide / Update / Destroy)
	def __init__(self, interface):
		self.interface = proxy(interface)
		self.bar = ChatBar(interface)

	def __GetChat(self):
		try:
			return self.interface.wndChat
		except ReferenceError:
			return None

	def Restore(self):
		window = self.__GetChat()
		if window:
			window.Show()
			window.Refresh()

	def Place(self):
		pass

	def Collapse(self):
		pass

	def Expand(self):
		pass

	def Update(self):
		# orijinal sohbetin mesaj satirlari cubugun hemen ustunde, cubukla ayni sol kenardan baslar
		window = self.__GetChat()
		if not window or not self.bar:
			return
		(barX, barY) = self.bar.GetGlobalPosition()
		try:
			window.SetPosition(barX, max(0, barY - window.GetHeight() - 4))
		except Exception:
			pass

	def Show(self):
		self.Restore()
		if self.bar:
			self.bar.Show()

	def Hide(self):
		if self.bar:
			self.bar.Hide()

	def Destroy(self):
		if self.bar:
			self.bar.Destroy()
		self.bar = None
		self.interface = None
