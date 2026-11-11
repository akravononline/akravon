# -*- coding: utf-8 -*-
# AKRAVON: orijinal Metin2 sohbet penceresini (uiChat.ChatWindow) kendi stilimize cevirir; orijinal dosya degismez.
#  - ust suslu (ejderha) cubuk -> sade lacivert / altin cubuk
#  - alt satirdaki 6 kucuk dugme kaldirilir; yalniz BAGLANTI ve GONDER kalir (buyuk, dokunmatik)
#  - Normal / Grup / Lonca / Bagir dugmesi ve yazi kutusu buyur
#  - giris satiri ekranin en altina yapisik (iPhone klavyesi acikken klavyenin ustune cikar)
from __future__ import division
import app
import chat
import grp
import uiChat
import ui
import wndMgr
import uiMobileStyle
import uiMobileChat
import uiMobileChrome

ROW = 52			# giris satiri yuksekligi
BUTTON = 44
MODE_WIDTH = 96
SIDE = 6
GAP = 6
BAR_HEIGHT = 22		# orijinal ust cubuk yuksekligi (btnChatSizing)
FONT = "Lato:20"
MAX_WIDTH = 760
MESSAGE_LINES = 5	# yazarken gorunen mesaj satiri
VIEW_LINES = 7	# kapaliyken akan mesaj satiri
VIEW_LIFT = 6
BAR_DROP = 12	# kapaliyken siyah zemin metinlerden biraz asagida biter
EDIT_GAP = 16	# yazarken mesajlar ile yazi satiri arasi	# kapaliyken mesajlar alt kenara yapismasin / ilk satir kesilmesin
MODES = (("Normal", "", (250, 236, 210)), ("Grup", "#", (138, 255, 242)), ("Lonca", "%", (231, 216, 255)), ("Bağır", "!", (168, 255, 213)))
LINE_COLOR = grp.GenerateColor(1.0, 1.0, 1.0, 1.0)
KEEP_HIDDEN = ("btnSendWhisper", "btnChatLog", "btnClearChat", "btnStopChat", "btnOtoChat")

states = {}


class LinkTarget(object):
	# uiMobileChat.TakeItemSlot'un bekledigi arayuz: envanterde dokunulan esyanin baglantisini yazi kutusuna ekler
	def __init__(self, chatLine):
		self.chatLine = chatLine

	def InsertLink(self, link):
		if link:
			self.chatLine.SetText(self.chatLine.GetText() + link)
		self.chatLine.SetFocus()


class Style(object):
	def __init__(self, window, interface):
		self.window = window
		self.interface = interface
		inputSet = window.chatInputSet
		self.inputSet = inputSet
		self.chatLine = inputSet.chatLine
		self.built = False
		self.shifted = False
		self.__Build()

	def __Build(self):
		window = self.window
		inputSet = self.inputSet
		# ust cubuk: ejderha resimleri gizli, yerine sade cubuk
		for name in ("imgChatBarLeft", "imgChatBarMiddle", "imgChatBarRight"):
			image = getattr(window, name, None)
			if image:
				image.Hide()
		bar = window.btnChatSizing
		self.barEdge = uiMobileStyle.CreateBar(bar, uiMobileStyle.Color(150, 118, 58), 0, 0, 10, BAR_HEIGHT)
		self.barFill = uiMobileStyle.CreateBar(bar, uiMobileStyle.Color(12, 20, 40, 240), 0, 1, 10, BAR_HEIGHT - 2)
		self.barTitle = uiMobileStyle.CreateText(bar, "S O H B E T", 0, 2, uiMobileStyle.Color(222, 184, 110), "LatoBd:16", "center")
		# mod dugmesi + yazi kutusu
		mode = inputSet.chatModeButton
		mode.SetSize(MODE_WIDTH, BUTTON)
		mode.SetPosition(SIDE, (ROW - BUTTON) // 2)
		try:
			mode.SetFontName("LatoBd:20")
		except Exception:
			pass
		line = self.chatLine
		try:
			line.SetFontName(FONT)
		except Exception:
			pass
		# gonder / baglanti dugmeleri (kendi cizimimiz)
		self.sendButton = uiMobileChrome.HudButton(BUTTON, "send")
		self.sendButton.SetParent(inputSet)
		self.sendButton.SetEvent(ui.__mem_func__(self.__Send))
		self.sendButton.Show()
		self.linkButton = uiMobileChrome.HudButton(BUTTON, "link")
		self.linkButton.SetParent(inputSet)
		self.linkButton.SetEvent(ui.__mem_func__(self.__RequestLink))
		self.linkButton.Show()
		self.linkTarget = LinkTarget(line)
		# mod: yazi kutusunun icine # ! % isareti yazilmaz; onek gonderirken eklenir
		self.mode = 0
		inputSet.OnChangeChatMode = self.NextMode
		inputSet.chatModeButton.SAFE_SetEvent(self.NextMode)	# dugme olay baglantisi olusturulurken yakalandi: yeniden bagla
		line.SAFE_SetTabEvent(self.NextMode)
		original = uiChat.ChatLine.OnIMEReturn
		style = self

		def Return():
			import dbg
			text = line.GetText()
			prefix = MODES[style.mode][1]
			if prefix and text.strip() and text[0] not in "#%!/":
				line.SetText(prefix + text)
			dbg.TraceError("ChatStyle.Return mod=%d metin=%r" % (style.mode, line.GetText()))
			try:
				return original(line)
			except Exception as error:
				dbg.TraceError("ChatStyle.Return hata: %s" % str(error))
				return True
		line.OnIMEReturn = Return
		# gonder: ayni yol (renkli isaret yok)
		line._ChatLine__ResetChat = lambda: line.SetText("")
		inputSet.OnRender = self.__RenderField
		originalRender = uiChat.ChatWindow.OnRender
		owner = self.window

		def RenderWindow():
			owner.yBar += BAR_DROP
			try:
				originalRender(owner)
			finally:
				owner.yBar -= BAR_DROP
		self.window.OnRender = RenderWindow
		self.__ApplyMode()
		self.built = True

	def __ApplyMode(self):
		button = self.inputSet.chatModeButton
		button.SetText(" ")
		text = getattr(button, "buttonText", None)
		if text:
			text.Hide()	# orijinal etiket gizli; kendi etiketimiz yazilir
		if not hasattr(self, "modeLabel"):
			self.modeLabel = uiMobileStyle.CreateText(button, "", MODE_WIDTH // 2, BUTTON // 2 - 11, uiMobileStyle.TEXT_COLOR, "LatoBd:20", "center")
		color = MODES[self.mode][2]
		self.modeLabel.SetPackedFontColor(uiMobileStyle.Color(color[0], color[1], color[2]))
		self.modeLabel.SetText(MODES[self.mode][0])

	def NextMode(self):
		self.mode = (self.mode + 1) % len(MODES)
		self.__ApplyMode()
		self.ignoreUntil = app.GetTime() + 1.0	# dugmeye basinca klavye kapanir: sohbet kapanmasin, klavye geri gelsin
		self.refocusAt = app.GetTime() + 0.12	# dugmeye basinca odak kayboldu: kisa sonra kapat-ac ile klavye geri gelir

	def __RenderField(self):
		# yazi kutusu cercevesi: mod dugmesiyle ayni yukseklik ve ayni seviye
		(x, y) = self.inputSet.GetGlobalPosition()
		fieldX = SIDE + MODE_WIDTH + GAP
		fieldWidth = self.inputSet.GetWidth() - fieldX - GAP - BUTTON - GAP - BUTTON - SIDE
		ui.RenderRoundBox(x + fieldX, y + (ROW - BUTTON) // 2, fieldWidth, BUTTON, LINE_COLOR)

	def __Send(self):
		self.chatLine.OnIMEReturn()

	def __RequestLink(self):
		self.ignoreUntil = app.GetTime() + 10.0	# envanter acilirken sohbet kapanmasin
		uiMobileChat.linkRequester = self.linkTarget
		try:
			if not self.interface.wndInventory.IsShow():
				self.interface.ToggleInventoryWindow()
		except Exception:
			pass
		chat.AppendChat(chat.CHAT_TYPE_INFO, "Eklemek istedi\xf0in e\xfeyaya dokun")

	def Update(self):
		window = self.window
		inputSet = self.inputSet
		screenWidth = wndMgr.GetScreenWidth()
		screenHeight = wndMgr.GetScreenHeight()
		for name in KEEP_HIDDEN:
			button = getattr(window, name, None)
			if button and button.IsShow():
				button.Hide()
		if inputSet.btnSend.IsShow():
			inputSet.btnSend.Hide()	# orijinal kucuk gonder dugmesi
		width = min(screenWidth, MAX_WIDTH)
		window.EDIT_LINE_HEIGHT = ROW
		editing = window.IsEditMode()
		if editing:
			window.SetSize(width, ROW)
			inputSet.SetSize(width, ROW)
		# giris satirinin yerleşimi
		inputSet.chatModeButton.SetPosition(SIDE, (ROW - BUTTON) // 2)
		fieldX = SIDE + MODE_WIDTH + GAP
		fieldWidth = width - fieldX - GAP - BUTTON - GAP - BUTTON - SIDE
		self.chatLine.SetPosition(fieldX + 12, ROW // 2 - 11)
		self.chatLine.overLay.SetText("")
		self.chatLine.overLay.Hide()
		self.chatLine.overTextLine.Hide()
		self.chatLine.SetSize(max(40, fieldWidth - 20), 24)
		self.linkButton.SetPosition(width - SIDE - BUTTON * 2 - GAP, (ROW - BUTTON) // 2)
		self.sendButton.SetPosition(width - SIDE - BUTTON, (ROW - BUTTON) // 2)
		(self.chatLine.x, self.chatLine.y, self.chatLine.width, self.chatLine.height) = self.chatLine.GetRect()
		# ust cubuk boyutu
		barWidth = max(10, window.GetWidth())
		self.barEdge.SetSize(barWidth, BAR_HEIGHT)
		self.barFill.SetSize(barWidth, BAR_HEIGHT - 2)
		self.barTitle.SetPosition(barWidth // 2, 2)
		# konum: ekranin en altina yapisik, yatay ortali; klavye acikken (iOS) klavyenin ustunde
		focused = bool(editing and self.chatLine.IsFocus())
		bottom = screenHeight
		if uiMobileChat.IS_IOS and focused:
			bottom = int(screenHeight * uiMobileChat.KEYBOARD_BOTTOM_RATIO)
		windowX = (screenWidth - width) // 2
		windowY = bottom - ROW - (EDIT_GAP if editing else VIEW_LIFT)
		self.inputSet.SetPosition(0, EDIT_GAP if editing else 0)
		window.SetPosition(windowX, windowY)
		# ust cubuk (gizliyken de yukseklik hesabinin kaynagi) mesaj alaninin ustunde sabit: yazi satirina binmez, mesajlar kesilmez
		step = max(1, chat.GetLineStep(window.chatID))
		areaHeight = (MESSAGE_LINES if editing else VIEW_LINES) * step + 14
		wantY = max(0, windowY - areaHeight - BAR_HEIGHT)
		if getattr(self, "lastBar", None) != (windowX, wantY, editing):
			self.lastBar = (windowX, wantY, editing)
			window.btnChatSizing.SetPosition(windowX, wantY)
			window.Refresh()
		# klavye kapaninca sohbet de kapanir (altta yalniz balon kalir)
		now = app.GetTime()
		if now < getattr(self, "ignoreUntil", 0.0):
			refocusAt = getattr(self, "refocusAt", None)
			if editing and refocusAt is not None and now >= refocusAt:
				self.refocusAt = None
				self.chatLine.KillFocus()
				self.chatLine.SetFocus()
		elif editing and focused:
			self.focusedBefore = True
			self.lostSince = None
		elif editing and getattr(self, "focusedBefore", False):
			if getattr(self, "lostSince", None) is None:
				self.lostSince = now
			elif now - self.lostSince > 0.35:
				self.focusedBefore = False
				self.lostSince = None
				window.CloseChat()
		else:
			self.focusedBefore = False
			self.lostSince = None


def Apply(window, interface):
	try:
		key = id(window)
		if key not in states:
			states[key] = Style(window, interface)
		return states[key]
	except Exception:
		import dbg
		dbg.TraceError("uiAkravonChatStyle.Apply hata")
		return None


def Update(window):
	state = states.get(id(window))
	if state:
		try:
			state.Update()
		except Exception as error:
			import dbg
			if not getattr(state, "traced", False):
				state.traced = True
				dbg.TraceError("ChatStyle.Update hata: %s" % str(error))


def Destroy():
	states.clear()
