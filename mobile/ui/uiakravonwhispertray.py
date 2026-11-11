# -*- coding: utf-8 -*-
# AKRAVON: gelen fisilti zarflari - Sohbet ikonunun ALTINDA, Gorev / Sohbet ile ayni sutunda ve ayni fontta.
# Okunmamis mesajda zarf yesil yanip soner; GM / sistem kendi ikonunu korur.
# Cokme guvenligi: tepsi bir kez olusturulan sabit sayida parcadan olusur (SLOTS). Oyunun kendi fisilti dugmeleri
# (interface.whisperButtonList) yalniz OKUNUR ve gizlenir; onlara hicbir pencere eklenmez, silinmeleri tepsiyi etkilemez.
from __future__ import division
import math
import app
import ui
import wndMgr
import uiMobileStyle
import uiMobileHud

SLOTS = 5
COLUMN = 54		# Gorev / Sohbet ikon sutunu genisligi
ICON_H = 40
LABEL_Y = 42
STEP = 64
ICON = "mobile_ui/akravon/hud/mail_icon.png"
ICON_GREEN = "mobile_ui/akravon/hud/mail_icon_green.png"
GM_ICON = "chat_eklenti/gm_logo.tga"
LABEL_COLOR = uiMobileStyle.Color(250, 228, 176)

tray = None


class TraySlot(uiMobileStyle.Pressable):
	def __init__(self, parent, owner):
		uiMobileStyle.Pressable.__init__(self, COLUMN, LABEL_Y + 22)
		self.SetParent(parent)
		self.owner = owner
		self.icon = ui.ExpandedImageBox()
		self.icon.SetParent(self)
		self.icon.AddFlag("not_pick")
		self.icon.LoadImage(ICON)
		self.icon.Show()
		self.green = ui.ExpandedImageBox()
		self.green.SetParent(self)
		self.green.AddFlag("not_pick")
		self.green.LoadImage(ICON_GREEN)
		self.green.Hide()
		self.label = uiMobileStyle.CreateText(self, "", COLUMN // 2, LABEL_Y, LABEL_COLOR, "LatoBd:15", "center", True)
		self.name = None
		self.isGM = False
		self.kind = "mail"
		self.SetEvent(ui.__mem_func__(self.OnTap))
		self.Hide()

	def OnTap(self):
		if self.name and self.owner:
			self.owner.Open(self.name)

	def Bind(self, name, isGM):
		if name != self.name:
			self.name = name
			shown = name if len(name) <= 9 else name[:8] + ".."
			self.label.SetText(shown)
		kind = "gm" if isGM else "mail"
		if kind != self.kind:
			self.kind = kind
			self.icon.LoadImage(GM_ICON if isGM else ICON)
		self.isGM = isGM
		x = (COLUMN - self.icon.GetWidth()) // 2
		y = (ICON_H - self.icon.GetHeight()) // 2
		self.icon.SetPosition(max(0, x), max(0, y))
		self.green.SetPosition(0, 0)

	def Pulse(self, active, now):
		if active and not self.isGM:
			wndMgr.SetDiffuseColor(self.green.hWnd, 1.0, 1.0, 1.0, 0.5 + 0.5 * math.sin(now * 6.0))
			self.green.Show()
		else:
			self.green.Hide()


class Tray(ui.Window):
	def __init__(self, interface):
		ui.Window.__init__(self)
		self.AddFlag("float")
		self.SetSize(COLUMN, STEP * SLOTS)
		self.interface = interface
		self.slots = [TraySlot(self, self) for i in range(SLOTS)]
		self.Show()

	def Open(self, name):
		interface = self.interface
		if interface is None:
			return
		for button in list(getattr(interface, "whisperButtonList", None) or []):
			if str(getattr(button, "name", "")) == name:
				interface.ShowWhisperDialog(button)
				return
		interface.OpenWhisperDialog(name)


def Update(interface, chatController):
	global tray
	if tray is None:
		tray = Tray(interface)
	tray.interface = interface
	buttons = list(getattr(interface, "whisperButtonList", None) or [])
	entries = []
	for button in buttons:
		try:
			button.Hide()	# oyunun kendi dugmesi gizli: tepsi cizer
			tip = getattr(button, "ToolTipText", None)
			if tip:
				tip.Hide()
		except Exception:
			pass
		name = str(getattr(button, "name", ""))
		if "DUYURU" in name.upper():
			continue
		entries.append(name)
	try:
		import uiWhisper
		unread = uiWhisper.UNREAD
		uiWhisper.WatchLink()
	except Exception:
		unread = ()
	x = getattr(chatController, "bubbleX", int(uiMobileHud.SCREEN_EDGE) - 7)
	y = getattr(chatController, "bubbleY", None)
	if y is None:
		y = 260
	tray.SetPosition(x, y + 62 + 8)
	now = app.GetTime()
	for (index, slot) in enumerate(tray.slots):
		if index < len(entries):
			name = entries[index]
			isGM = False
			try:
				isGM = bool(interface.IsGameMasterName(name)) or name in ("[Sistem]", "[Efsun Botu]")
			except Exception:
				pass
			slot.SetPosition(0, index * STEP)
			slot.Bind(name, isGM)
			slot.Pulse(name in unread, now)
			slot.Show()
		else:
			slot.Hide()


def Destroy():
	global tray
	if tray is not None:
		tray.Hide()
	tray = None
