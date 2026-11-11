#-*- coding: utf-8 -*-
# AKRAVON: kendi fisilti penceremiz (orijinal uiWhisper ile ayni sinif / metot adlari, ici sifirdan)
#  - lacivert-altin tahta, baslik + ayirici + kare kapat dugmesi (baslik seridinden tasinir)
#  - mesajlar sade balonlar: gelenler solda, kendi mesajlarimiz sagda (yesilimsi), sistem mesaji ortada
#  - altta yazi alani + gonder dugmesi; yukari/asagi surukleyerek eski mesajlara bakilir
from __future__ import division
import ui
import net
import chat
import player
import app
import localeInfo
import ime
import constInfo
import chr
import messenger
import wndMgr
import time
import uiMobileStyle
import uiMobileChat
import uiMobileChrome
import uiAkravonQuest
import uiAkravonInventory

W = 480
H = 330
PAD = 14
TITLE_H = 46
INPUT_H = 44
LINE = 25
GAP = 10
FONT = "Lato:19"
TIME_FONT = "Lato:13"
TIME_H = 16
IMG = "d:/ymir work/ui/akravon_legacy/"
BUBBLE_ME = uiMobileStyle.Color(30, 78, 58, 240)
BUBBLE_OTHER = uiMobileStyle.Color(34, 46, 76, 240)
SEPARATOR_COLOR = uiMobileStyle.Color(150, 118, 58)

HISTORY = {}	# kisi adi -> [("me" | "them" | "sys", metin), ...]
DIALOGS = {}	# kisi adi -> acik pencere (yeni mesajda yenilenir)
HISTORY_MAX = 80
UNREAD = set()	# okunmamis fisilti gonderenleri (HUD zarf dugmesi yanip soner)


def Record(mode, name, line):
	items = HISTORY.setdefault(name, [])
	stamp = time.strftime("%H:%M")
	incoming = False
	if mode == chat.WHISPER_TYPE_CHAT:
		me = player.GetName()
		prefix = me + " : "
		if line.startswith(prefix):
			items.append(("me", line[len(prefix):], stamp))
		else:
			index = line.find(" : ")
			items.append(("them", line[index + 3:] if index >= 0 else line, stamp))
			incoming = True
	else:
		items.append(("sys", line, stamp))
	if len(items) > HISTORY_MAX:
		del items[0]
	dialog = DIALOGS.get(name)
	if incoming and not (dialog and dialog.IsShow()):
		UNREAD.add(name)
	if dialog:
		dialog.Refresh()


if not getattr(chat, "akravonWhisperWrapped", False):
	__original = chat.AppendWhisper

	def __AppendWhisper(mode, name, line):
		try:
			Record(mode, name, line)
		except:
			pass
		return __original(mode, name, line)

	chat.AppendWhisper = __AppendWhisper
	chat.akravonWhisperWrapped = True


def ClearHistory(name):
	if name in HISTORY:
		del HISTORY[name]


class WhisperButton(ui.Button):
	def __init__(self):
		ui.Button.__init__(self, "TOP_MOST")

	def __del__(self):
		ui.Button.__del__(self)

	def SetToolTipText(self, text, x=0, y = 32):
		ui.Button.SetToolTipText(self, text, x, y)
		self.ToolTipText.Show()

	def SetToolTipTextWithColor(self, text, color, x=0, y = 32):
		ui.Button.SetToolTipText(self, text, x, y)
		self.ToolTipText.SetPackedFontColor(color)
		self.ToolTipText.Show()

	def ShowToolTip(self):
		if 0 != self.ToolTipText:
			self.ToolTipText.Show()

	def HideToolTip(self):
		if 0 != self.ToolTipText:
			self.ToolTipText.Show()


class Bubble9(ui.Window):
	# yumusak kose balon: yalniz duz seritlerden (gorsel yok, olcek yok): dikis / kesik gorunum olmaz.
	# alttaki katman kenar rengi, ustundeki 1 px iceride dolgu rengi; koseler 4 kademe kesilir
	STEPS = (4, 2, 1, 1)
	COLORS = {
		"wsp_me": ((28, 98, 70), (96, 196, 140)),
		"wsp_other": ((34, 54, 104), (110, 146, 224)),
	}

	def __init__(self, parent, prefix):
		ui.Window.__init__(self)
		self.SetParent(parent)
		self.AddFlag("not_pick")
		(fill, edge) = self.COLORS[prefix]
		self.fill = uiMobileStyle.Color(*fill)
		self.edge = uiMobileStyle.Color(*edge)
		self.bars = []
		self.Show()

	def __Layer(self, x, y, width, height, color):
		steps = self.STEPS
		count = len(steps)
		for (index, inset) in enumerate(steps):
			self.bars.append(uiMobileStyle.CreateBar(self, color, x + inset, y + index, width - inset * 2, 1))
			self.bars.append(uiMobileStyle.CreateBar(self, color, x + inset, y + height - 1 - index, width - inset * 2, 1))
		self.bars.append(uiMobileStyle.CreateBar(self, color, x, y + count, width, height - count * 2))

	def SetSize(self, width, height):
		ui.Window.SetSize(self, width, height)
		self.bars = []
		self.__Layer(0, 0, width, height, self.edge)
		self.__Layer(1, 1, width - 2, height - 2, self.fill)


class MinButton(uiMobileStyle.Pressable):
	# kapat dugmesiyle ayni kare cerceve, ortada tire: pencereyi kenara (zarf dugmesine) alir
	def __init__(self, parent, x, y, event):
		uiMobileStyle.Pressable.__init__(self, 40, 40)
		self.SetParent(parent)
		self.SetPosition(x - 4, y - 4)
		self.image = ui.ExpandedImageBox()
		self.image.SetParent(self)
		self.image.AddFlag("not_pick")
		self.image.SetPosition(4, 4)
		self.image.LoadImage(IMG + "wsp_min_1.png")
		self.image.Show()
		self.SetEvent(event)
		self.Show()

	def RefreshVisual(self):
		if getattr(self, "image", None):
			self.image.LoadImage(IMG + ("wsp_min_3.png" if self.pressed else "wsp_min_1.png"))

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.image = None


class TitleDrag(ui.Window):
	# pencere yalniz baslik seridinden tasinir
	def __init__(self, owner, width, height):
		ui.Window.__init__(self)
		self.SetParent(owner)
		self.owner = owner
		self.SetPosition(0, 0)
		self.SetSize(width, height)
		self.dragging = False
		self.grab = (0, 0)
		self.Show()

	def OnMouseLeftButtonDown(self):
		(mx, my) = wndMgr.GetMousePosition()
		(wx, wy) = self.owner.GetGlobalPosition()
		self.grab = (mx - wx, my - wy)
		self.dragging = True

	def OnMouseLeftButtonUp(self):
		self.dragging = False

	def Drag(self):
		if not self.dragging:
			return
		(mx, my) = wndMgr.GetMousePosition()
		x = max(-W + 120, min(wndMgr.GetScreenWidth() - 120, mx - self.grab[0]))
		y = max(0, min(wndMgr.GetScreenHeight() - 90, my - self.grab[1]))
		self.owner.SetPosition(x, y)
		self.owner.userMoved = True	# elle tasindiysa her acilista yerine dondurulmez


class ScrollCatcher(ui.Window):
	# mesaj alanina basip surukleyerek eski mesajlara bakilir
	def __init__(self, owner, width, height):
		ui.Window.__init__(self)
		self.SetParent(owner)
		self.owner = owner
		self.SetSize(width, height)
		self.dragging = False
		self.lastY = 0
		self.Show()

	def OnMouseLeftButtonDown(self):
		self.dragging = True
		self.lastY = wndMgr.GetMousePosition()[1]

	def OnMouseLeftButtonUp(self):
		self.dragging = False

	def Drag(self):
		if not self.dragging:
			return
		y = wndMgr.GetMousePosition()[1]
		delta = y - self.lastY
		if delta:
			self.lastY = y
			self.owner.ScrollBy(delta)


class WhisperDialog(ui.ScriptWindow):

	def __init__(self, eventMinimize, eventClose):
		ui.ScriptWindow.__init__(self)
		self.targetName = ""
		self.eventMinimize = eventMinimize
		self.eventClose = eventClose
		self.eventAcceptTarget = None
		self.interface = None
		self.placed = False
		self.userMoved = False
		self.refocusAt = 0.0
		self.scrollOffset = 0
		self.items = []
		self.built = False

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def LoadDialog(self):
		self.SetSize(W, H)
		self.AddFlag("float")
		self.board = uiAkravonQuest.Board9()
		self.board.SetParent(self)
		self.board.SetPosition(0, 0)
		self.board.SetSize(W, H)
		self.board.AddFlag("not_pick")
		self.board.Show()

		self.titleName = uiMobileStyle.CreateText(self, "", W // 2, 11, uiMobileStyle.GOLD_COLOR, "Marcellus:22", "center")
		self.divider = uiMobileStyle.CreateBar(self, SEPARATOR_COLOR, PAD, TITLE_H, W - PAD * 2, 1)
		self.titleDrag = TitleDrag(self, W - 108, TITLE_H - 2)
		self.closeButton = uiAkravonInventory.CloseButton(self, W - PAD - 38, 8, ui.__mem_func__(self.Close))
		self.minButton = MinButton(self, W - PAD - 38 - 44, 8, ui.__mem_func__(self.Minimize))

		# alici adi girisi (hedefsiz acilis)
		self.nameField = uiMobileChat.InsetField(W - PAD * 2 - 90, 36)
		self.nameField.SetParent(self)
		self.nameField.SetPosition(PAD, TITLE_H + 12)
		self.titleNameEdit = ui.EditLine()
		self.titleNameEdit.SetParent(self)
		self.titleNameEdit.SetPosition(PAD + 12, TITLE_H + 12 + 6)
		self.titleNameEdit.SetSize(W - PAD * 2 - 114, 26)
		self.titleNameEdit.SetMax(24)
		self.titleNameEdit.SetFontName("Lato:19")
		self.titleNameEdit.SetPackedFontColor(0xFFFFFFFF)
		self.titleNameEdit.SetReturnEvent(ui.__mem_func__(self.AcceptTarget))
		self.titleNameEdit.SetEscapeEvent(ui.__mem_func__(self.Close))
		self.acceptButton = uiMobileChrome.HudButton(36, "send")
		self.acceptButton.SetParent(self)
		self.acceptButton.SetPosition(W - PAD - 36, TITLE_H + 12)
		self.acceptButton.SetEvent(ui.__mem_func__(self.AcceptTarget))
		self.nameField.Hide()
		self.titleNameEdit.Hide()
		self.acceptButton.Hide()

		# mesaj alani
		self.areaY = TITLE_H + 8
		self.areaH = H - self.areaY - INPUT_H - PAD * 2
		self.areaW = W - PAD * 2
		self.area = ui.Window()
		self.area.SetParent(self)
		self.area.SetPosition(PAD, self.areaY)
		self.area.SetSize(self.areaW, self.areaH)
		self.area.AddFlag("not_pick")
		self.area.Show()
		self.catcher = ScrollCatcher(self, self.areaW, self.areaH)
		self.catcher.SetPosition(PAD, self.areaY)

		# yazi alani
		inputY = H - PAD - INPUT_H
		fieldWidth = W - PAD * 2 - INPUT_H - 8
		self.fieldPanel = uiMobileChat.InsetField(fieldWidth, INPUT_H)
		self.fieldPanel.SetParent(self)
		self.fieldPanel.SetPosition(PAD, inputY)
		self.fieldPanel.Show()
		self.chatLine = ui.EditLine()
		self.chatLine.SetParent(self)
		self.chatLine.SetPosition(PAD + 12, inputY + INPUT_H // 2 - 11)
		self.chatLine.SetSize(fieldWidth - 24, 26)
		self.chatLine.SetMax(120)
		self.chatLine.SetFontName("Lato:19")
		self.chatLine.SetPackedFontColor(0xFFFFFFFF)
		self.chatLine.SetReturnEvent(ui.__mem_func__(self.SendWhisper))
		self.chatLine.SetEscapeEvent(ui.__mem_func__(self.Minimize))
		self.chatLine.Show()
		self.fieldTap = uiMobileStyle.Pressable(fieldWidth, INPUT_H)
		self.fieldTap.SetParent(self)
		self.fieldTap.SetPosition(PAD, inputY)
		self.fieldTap.SetEvent(ui.__mem_func__(self.FocusInput))
		self.fieldTap.Show()
		self.sendButton = uiMobileChrome.HudButton(INPUT_H, "send")
		self.sendButton.SetParent(self)
		self.sendButton.SetPosition(W - PAD - INPUT_H, inputY)
		self.sendButton.SetEvent(ui.__mem_func__(self.SendWhisper))
		self.sendButton.Show()
		self.built = True

	def Destroy(self):
		if self.targetName and DIALOGS.get(self.targetName) is self:
			del DIALOGS[self.targetName]
		self.eventMinimize = None
		self.eventClose = None
		self.eventAcceptTarget = None
		self.items = []
		self.ClearDictionary()

	# ------------------------------------------------------------ mesaj cizimi
	def Refresh(self):
		if not self.built:
			return
		self.items = []
		messages = HISTORY.get(self.targetName, []) if self.targetName else []
		maxText = int(self.areaW * 0.70)
		heights = []
		for (kind, text, stamp) in messages:
			lines = uiMobileStyle.WrapLines(text, maxText, FONT)
			extra = 0 if kind == "sys" else TIME_H
			heights.append((kind, lines, stamp, len(lines) * LINE + 16 + extra))
		total = 0
		for (kind, lines, stamp, height) in heights:
			total += height + GAP
		self.maxScroll = max(0, total - self.areaH)
		self.scrollOffset = max(0, min(self.scrollOffset, self.maxScroll))
		bottom = self.areaH + self.scrollOffset
		for (kind, lines, stamp, height) in reversed(heights):
			top = bottom - height
			bottom = top - GAP
			if top < 0 or top + height > self.areaH:
				continue	# tam sigmayan mesajlar cizilir (alanin disina tasan olmaz)
			width = 0
			for line in lines:
				width = max(width, uiMobileStyle.TextWidth(line, FONT))
			if kind == "sys":
				for (index, line) in enumerate(lines):
					self.items.append(uiMobileStyle.CreateText(self.area, line, self.areaW // 2, top + 8 + index * LINE, uiMobileStyle.MUTED_COLOR, FONT, "center"))
				continue
			boxWidth = max(width + 28, uiMobileStyle.TextWidth(stamp, TIME_FONT) + 28)
			x = self.areaW - boxWidth if kind == "me" else 0
			bubble = Bubble9(self.area, "wsp_me" if kind == "me" else "wsp_other")
			bubble.SetPosition(x, top)
			bubble.SetSize(boxWidth, height)
			self.items.append(bubble)
			for (index, line) in enumerate(lines):
				self.items.append(uiMobileStyle.CreateText(self.area, line, x + 14, top + 8 + index * LINE, uiMobileStyle.BRIGHT_COLOR, FONT, "left"))
			self.items.append(uiMobileStyle.CreateText(self.area, stamp, x + boxWidth - 12, top + height - TIME_H - 3, uiMobileStyle.SOFT_COLOR, TIME_FONT, "right"))

	def ScrollBy(self, delta):
		old = self.scrollOffset
		self.scrollOffset = max(0, min(getattr(self, "maxScroll", 0), self.scrollOffset + delta))
		if self.scrollOffset != old:
			self.Refresh()

	def OnUpdate(self):
		if not self.placed:
			self.placed = True
			self.SetPosition((wndMgr.GetScreenWidth() - W) // 2, 16)	# klavye acilinca ustte kalsin
		if self.built:
			self.titleDrag.Drag()
			self.catcher.Drag()
			if self.refocusAt and app.GetTime() >= self.refocusAt:
				self.refocusAt = 0.0
				if self.IsShow() and self.targetName:
					self.FocusInput()	# gonderince klavye kapanmasin: kullanici kucultene / kapatana kadar acik kalir

	# ------------------------------------------------------------ interface arayuzu
	def OpenWithTarget(self, targetName):
		chat.CreateWhisper(targetName)
		self.targetName = targetName
		DIALOGS[targetName] = self
		UNREAD.discard(targetName)
		self.titleName.SetText(targetName)
		self.nameField.Hide()
		self.titleNameEdit.Hide()
		self.acceptButton.Hide()
		self.area.Show()
		self.catcher.Show()
		self.fieldPanel.Show()
		self.chatLine.Show()
		self.sendButton.Show()
		self.scrollOffset = 0
		self.Refresh()
		if not self.userMoved:
			self.placed = False	# arayuz acilista konumu degistirebiliyor: ortaya geri al
		self.SetTop()	# Arkadaslar vb. pencerelerin arkasinda kalmasin
		self.chatLine.SetFocus()

	def OpenWithoutTarget(self, event):
		self.eventAcceptTarget = event
		self.targetName = 0
		self.titleName.SetText("")
		self.titleNameEdit.SetText("")
		self.nameField.Show()
		self.titleNameEdit.Show()
		self.acceptButton.Show()
		self.area.Hide()
		self.catcher.Hide()
		self.fieldPanel.Hide()
		self.chatLine.Hide()
		self.sendButton.Hide()
		self.titleNameEdit.SetFocus()

	def SetGameMasterLook(self):
		pass

	def ResizeWhisperDialog(self):
		pass

	def SetWhisperDialogSize(self, width, height):
		pass

	def AddFrind(self):
		net.SendMessengerAddByNamePacket(self.targetName)

	def Minimize(self):
		self.titleNameEdit.KillFocus()
		self.chatLine.KillFocus()
		self.refocusAt = 0.0
		self.Hide()
		if None != self.eventMinimize:
			self.eventMinimize(self.targetName)

	def Close(self):
		chat.ClearWhisper(self.targetName)
		ClearHistory(self.targetName)
		UNREAD.discard(self.targetName)
		self.titleNameEdit.KillFocus()
		self.chatLine.KillFocus()
		self.refocusAt = 0.0
		self.Hide()
		if None != self.eventClose:
			self.eventClose(self.targetName)

	def blockTarget(self):
		net.SendMessengerAddBlockByNamePacket(self.targetName)

	def unblockTarget(self):
		messenger.RemoveBlock(constInfo.ME_KEY)
		net.SendMessengerRemoveBlockPacket(constInfo.ME_KEY, self.targetName)

	def AcceptTarget(self):
		name = self.titleNameEdit.GetText()
		if len(name) <= 0:
			self.Close()
			return
		if None != self.eventAcceptTarget:
			self.titleNameEdit.KillFocus()
			self.eventAcceptTarget(name)

	def FocusInput(self):
		self.chatLine.KillFocus()
		self.chatLine.SetFocus()

	def SendWhisper(self):
		text = self.chatLine.GetText()
		if len(text) <= 0 or not self.targetName:
			return
		if self.targetName in ("[Sistem]", "[Biyolog]"):
			return
		if net.IsInsultIn(text):
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.CHAT_INSULT_STRING)
			return
		net.SendWhisperPacket(self.targetName, text)
		self.chatLine.SetText("")
		self.scrollOffset = 0
		chat.AppendWhisper(chat.WHISPER_TYPE_CHAT, self.targetName, player.GetName() + " : " + text)
		self.refocusAt = app.GetTime() + 0.12

	def OnTop(self):
		if self.targetName and self.built:
			self.chatLine.SetFocus()

	def BindInterface(self, interface):
		self.interface = interface

	def OnPressEscapeKey(self):
		self.Minimize()
		return True
