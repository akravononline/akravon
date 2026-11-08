# -*- coding: utf-8 -*-
# AKRAVON: HUD duzenleme (PUBG Mobile gibi) - gercek HUD gorunur; joystick, saldiri dugmesi, her beceri slotu ve
# topla / oto av / at dugmeleri ayri ayri tasinir ve boyutlandirilir. Karakter HUD'u ve harita HUD'u sabit kalir.
# Konumlar ekranin binde'si olarak saklanir -> her telefon / ekranda orantili calisir.
from __future__ import division
import ui
import wndMgr
import uiMobileStyle
import uiMobileHud
import uiAkravon

LABELS = {
	"joystick" : "Joystick",
	"attack" : "Saldırı",
	"pickup" : "Topla",
	"autohunt" : "Oto Av",
	"mount" : "At",
	"page" : "Sayfa",
	"edit" : "Ayar",
}
SCALE_MIN = 50
SCALE_MAX = 180
SCALE_STEP = 10

FILL = uiMobileStyle.Color(60, 110, 200, 40)
FILL_SELECTED = uiMobileStyle.Color(60, 130, 230, 110)
LINE = uiMobileStyle.Color(215, 180, 110, 255)
LINE_SELECTED = uiMobileStyle.Color(120, 200, 255, 255)
TEXT = uiMobileStyle.Color(250, 236, 210)
FONT_LABEL = "Lato:16"
FONT_TITLE = "Marcellus:22"
FONT_BUTTON = "Marcellus:20"
BAR_HEIGHT = 46
EDGE = 6

editor = None
snapshot = None


def GetName(key):
	if key.startswith("skill"):
		return "Beceri %d" % (int(key[5:]) + 1)
	return LABELS.get(key, key)


class EditHandle(ui.Window):
	def __init__(self, owner, key):
		ui.Window.__init__(self, "TOP_MOST")
		self.owner = owner
		self.key = key
		self.dragging = False
		self.dragOffset = (0, 0)
		self.box = uiAkravon.Box()
		self.box.SetParent(self)
		self.box.Show()
		text = key[5:] and str(int(key[5:]) + 1) if key.startswith("skill") else LABELS.get(key, key)
		self.label = uiMobileStyle.CreateText(self, text, 0, 0, TEXT, FONT_LABEL, "center")
		self.SetSelected(False)

	def Destroy(self):
		self.owner = None
		self.box = None
		self.label = None
		self.Hide()

	def SetRect(self, x, y, width, height):
		self.SetPosition(x, y)
		self.SetSize(width, height)
		self.box.SetSize(width, height)
		self.label.SetPosition(width // 2, max(0, height // 2 - 9))

	def SetSelected(self, selected):
		self.box.SetFillColor(FILL_SELECTED if selected else FILL)
		self.box.SetLineColor(LINE_SELECTED if selected else LINE)

	def OnMouseLeftButtonDown(self):
		(mouseX, mouseY) = wndMgr.GetMousePosition()
		(x, y) = self.GetGlobalPosition()
		self.dragOffset = (mouseX - x, mouseY - y)
		self.dragging = True
		self.owner.Select(self.key)
		return True

	def OnMouseLeftButtonUp(self):
		self.dragging = False
		return True

	def OnUpdate(self):
		if not self.dragging:
			return
		(mouseX, mouseY) = wndMgr.GetMousePosition()
		self.owner.Move(self.key, mouseX - self.dragOffset[0], mouseY - self.dragOffset[1])


class BarButton(uiMobileStyle.Pressable):
	def __init__(self, width, text, event):
		uiMobileStyle.Pressable.__init__(self, width, BAR_HEIGHT)
		self.box = uiAkravon.Box()
		self.box.SetParent(self)
		self.box.SetSize(width, BAR_HEIGHT)
		self.box.SetFillColor(uiMobileStyle.Color(10, 16, 34, 230))
		self.box.SetLineColor(LINE)
		self.box.Show()
		self.label = uiMobileStyle.CreateText(self, text, width // 2, BAR_HEIGHT // 2 - 12, TEXT, FONT_BUTTON, "center")
		self.SetEvent(event)


class HudEditor(ui.Window):
	def __init__(self, controller):
		ui.Window.__init__(self, "TOP_MOST")
		self.controller = controller
		self.screenWidth = wndMgr.GetScreenWidth()
		self.screenHeight = wndMgr.GetScreenHeight()
		self.SetSize(self.screenWidth, self.screenHeight)
		self.SetPosition(0, 0)

		# bos alanlara dokunmalar oyuna gitmesin
		self.blocker = uiMobileStyle.Pressable(self.screenWidth, self.screenHeight)
		self.blocker.SetParent(self)
		self.blocker.SetPosition(0, 0)
		self.blocker.Show()

		self.rects = {}
		self.initial = {}
		self.order = []
		self.handles = {}
		for (key, x, y, width, height) in controller.GetEditRects():
			self.rects[key] = [x, y, width, height]
			self.initial[key] = (x, y, width, height)
			self.order.append(key)
			handle = EditHandle(self, key)
			handle.SetRect(x, y, width, height)
			handle.Show()
			self.handles[key] = handle

		self.selected = uiMobileHud.GetEditSelection()
		if self.selected not in self.rects:
			self.selected = "attack"
		self.__BuildBar()
		self.Select(self.selected)
		self.Show()
		self.SetTop()
		for handle in self.handles.values():
			handle.SetTop()
		self.__RaiseBar()

	def Destroy(self):
		for handle in self.handles.values():
			handle.Destroy()
		self.handles = {}
		self.blocker = None
		self.bar = []
		self.Hide()

	# ------------------------------------------------------------ secim / tasima

	def Select(self, key):
		self.selected = key
		uiMobileHud.SetEditSelection(key)
		for (name, handle) in self.handles.items():
			handle.SetSelected(name == key)
		self.handles[key].SetTop()
		self.__RaiseBar()
		self.__RefreshTitle()

	def Move(self, key, x, y):
		rect = self.rects[key]
		x = max(EDGE, min(self.screenWidth - EDGE - rect[2], x))
		y = max(EDGE, min(self.screenHeight - EDGE - rect[3], y))
		rect[0] = x
		rect[1] = y
		self.handles[key].SetRect(x, y, rect[2], rect[3])
		self.controller.MoveHudElement(key, x, y)

	def __Moved(self, key):
		rect = self.rects[key]
		initial = self.initial[key]
		return abs(rect[0] - initial[0]) > 1 or abs(rect[1] - initial[1]) > 1

	# ------------------------------------------------------------ kayit

	def __ScaleKey(self, key):
		return key

	def __Persist(self):
		# tasinan ogelerin konumunu (ekran binde'si) kaydet
		anyMoved = False
		for key in self.order:
			if key.startswith("skill"):
				anyMoved = anyMoved or self.__Moved(key)
				continue
			if not self.__Moved(key):
				continue
			rect = self.rects[key]
			current = uiMobileHud.GetHudOverride(key)
			scale = current[2] if current else 100
			centerX = rect[0] + rect[2] / 2.0
			centerY = rect[1] + rect[3] / 2.0
			uiMobileHud.SetHudOverride(key, int(round(centerX * 1000.0 / self.screenWidth)), int(round(centerY * 1000.0 / self.screenHeight)), scale, False)
		if anyMoved:
			self.controller.SaveSkillPositions()
		uiMobileHud.SaveConfig()

	def __Rescale(self, direction):
		key = self.selected
		self.__Persist()
		scaleKey = self.__ScaleKey(key)
		current = uiMobileHud.GetHudOverride(scaleKey)
		scale = max(SCALE_MIN, min(SCALE_MAX, (current[2] if current else 100) + direction * SCALE_STEP))
		if scaleKey.startswith("skill"):
			(cx, cy) = (0, 0)
		elif current:
			(cx, cy) = (current[0], current[1])
		else:
			rect = self.rects[key]
			cx = int(round((rect[0] + rect[2] / 2.0) * 1000.0 / self.screenWidth))
			cy = int(round((rect[1] + rect[3] / 2.0) * 1000.0 / self.screenHeight))
		uiMobileHud.SetHudOverride(scaleKey, cx, cy, scale)
		self.__Rebuild()

	def __Rebuild(self):
		# boyut degisince HUD yeniden kurulur; duzenleyici yeni HUD ile kendiliginden acilir
		self.controller = None
		self.Destroy()
		uiMobileHud.RequestRefresh()

	def __Reset(self):
		uiMobileHud.ClearHudLayout()
		self.__Rebuild()

	def __Save(self):
		self.__Persist()
		self.__Finish()

	def __Cancel(self):
		global snapshot
		if snapshot is not None:
			config = uiMobileHud.LoadConfig()
			config.clear()
			config.update(snapshot)
			uiMobileHud.SaveConfig()
		self.__Finish()

	def __Finish(self):
		global snapshot
		snapshot = None
		uiMobileHud.SetEditing(False)
		uiMobileHud.SetEditSelection(None)
		self.controller = None
		self.Destroy()
		uiMobileHud.RequestRefresh()

	# ------------------------------------------------------------ cubuk

	def __BuildBar(self):
		specs = (("-", 54, lambda: self.__Rescale(-1)), ("+", 54, lambda: self.__Rescale(1)), ("Sıfırla", 104, self.__Reset), ("İptal", 92, self.__Cancel), ("Kaydet", 104, self.__Save))
		gap = 8
		total = sum([spec[1] for spec in specs]) + gap * (len(specs) - 1)
		x = (self.screenWidth - total) // 2
		y = 36
		self.bar = []
		self.title = uiMobileStyle.CreateText(self, "", self.screenWidth // 2, 8, TEXT, FONT_TITLE, "center", True)
		for (text, width, event) in specs:
			button = BarButton(width, text, event)
			button.SetParent(self)
			button.SetPosition(x, y)
			button.Show()
			self.bar.append(button)
			x += width + gap

	def __RaiseBar(self):
		for button in self.bar:
			button.SetTop()

	def __RefreshTitle(self):
		key = self.selected
		current = uiMobileHud.GetHudOverride(self.__ScaleKey(key))
		scale = current[2] if current else 100
		self.title.SetText("%s  •  %%%d" % (GetName(key), scale))

	def OnPressEscapeKey(self):
		self.__Cancel()
		return True


def Open():
	# ayarlardan: duzenleme modu acilir; denetleyici bir sonraki karede Resume() cagirir
	global snapshot
	snapshot = dict(uiMobileHud.LoadConfig())
	uiMobileHud.SetEditSelection(None)
	uiMobileHud.SetEditing(True)


def Resume():
	global editor
	controller = uiMobileHud.GetController()
	if controller is None:
		return
	if editor is not None:
		editor.Destroy()
	editor = HudEditor(controller)


def Destroy():
	global editor
	if editor is not None:
		editor.Destroy()
	editor = None
