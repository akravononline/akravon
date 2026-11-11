# -*- coding: utf-8 -*-
# AKRAVON: buyuk harita ekranin ortasinda acilir; haritada bir yere dokununca harita kapanir, oyun dunyasinda yerde altin ayak
# izleri yolu gosterir ve karakter engellerden kacarak kendiliginden o noktaya yuruyor.
# Joystick / saldiri dokunusu yuruyusu iptal eder. Yol: background.FindPath (arazi engel verisinde A*).
from __future__ import division
import math
import app
import background
import chr
import miniMap
import net
import player
import ui
import wndMgr

ARRIVE_DISTANCE = 160.0		# cm: bu kadar yaklasinca vardi
WAYPOINT_DISTANCE = 140.0	# cm: ara noktaya bu kadar yaklasinca siradaki
FOOT_SPACING = 95.0			# cm: iki iz arasi
FOOT_SIDE = 22.0			# cm: sol / sag sapma
FOOT_MAX = 40
STUCK_SECONDS = 2.5
STUCK_DISTANCE = 50.0
REPLAN_SECONDS = 6.0

target = None
path = []
pathIndex = 0
lastProgressTime = 0.0
lastProgressPos = (0.0, 0.0)
lastPlanTime = 0.0
atlasWindow = None
handler = None
footCounter = 0


class Renderer(ui.Window):
	# haritanin kendisini motor cizer (RenderAtlas); biz yalniz konum ve olcek veririz
	def __init__(self):
		ui.Window.__init__(self)
		self.AddFlag("not_pick")

	def OnUpdate(self):
		miniMap.UpdateAtlas()

	def OnRender(self):
		(x, y) = self.GetGlobalPosition()
		miniMap.RenderAtlas(float(x), float(y))


class MapCatcher(ui.Window):
	# haritaya dokunma: gidilecek nokta
	def __init__(self, owner):
		ui.Window.__init__(self)
		self.SetParent(owner)
		self.owner = owner
		self.Show()

	def OnMouseLeftButtonUp(self):
		self.owner.OnMapClick()


PAD = 14
TITLE_H = 46
GAP = 14			# harita ile bilgi paneli arasi (ayirici ortada)
PANEL_W = 260		# sag bilgi paneli genisligi
ATLAS_SCALE = 1.5	# harita buyutme (1.0 = eski boyut); iki platformda sabit, ekran boyuna bagli degil
TITLE_TEXT = "Genel Görünüm Haritası"
INFO_TITLE = "Otomatik Yürüyüş"
INFO_TEXT = "Haritada gitmek istediğiniz yere dokunun, karakteriniz engellerden kaçarak otomatik olarak oraya yürür."
INFO_FONT = "Lato:21"
LINE_H = 29
LINE_COLOR = (96, 74, 42)


class AtlasBoard(ui.Window):
	# AKRAVON: buyuk harita penceresi sifirdan: lacivert-altin tahta, baslik, kare kapat, solda harita, ortada ayirici, sagda bilgi
	def __init__(self):
		import uiMobileStyle
		import uiAkravonQuest
		import uiAkravonInventory
		ui.Window.__init__(self, "UI")
		self.AddFlag("float")
		self.frame = uiAkravonQuest.Board9()
		self.frame.SetParent(self)
		self.frame.AddFlag("not_pick")
		self.frame.Show()
		self.title = uiMobileStyle.CreateText(self, TITLE_TEXT, 100, 14, uiMobileStyle.Color(222, 184, 110), "Marcellus:22", "center")
		self.divider = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(*LINE_COLOR), PAD, TITLE_H, 100, 1)
		self.vline = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(*LINE_COLOR), 100, TITLE_H + 12, 1, 100)
		self.infoTitle = uiMobileStyle.CreateText(self, INFO_TITLE, 100, 100, uiMobileStyle.Color(222, 184, 110), "Marcellus:25", "left")
		self.infoLines = [uiMobileStyle.CreateText(self, line, 100, 100, uiMobileStyle.TEXT_COLOR, INFO_FONT, "left") for line in uiMobileStyle.WrapLines(INFO_TEXT, PANEL_W, INFO_FONT)]
		self.closeButton = uiAkravonInventory.CloseButton(self, 100, 11, ui.__mem_func__(self.Close))
		self.renderer = Renderer()
		self.renderer.SetParent(self)
		self.renderer.Show()
		self.catcher = MapCatcher(self)
		self.AtlasMainWindow = self.renderer	# AtlasHandler uyumu
		self.size = (0, 0)
		self.Hide()

	def __del__(self):
		ui.Window.__del__(self)

	def Open(self):
		try:
			(ok, sizeX, sizeY) = miniMap.GetAtlasSize()
		except Exception:
			(ok, sizeX, sizeY) = (0, 256, 256)
		if not ok or sizeX <= 0 or sizeY <= 0:
			(sizeX, sizeY) = (256, 256)
		scale = 1.0
		if hasattr(miniMap, "SetAtlasScale"):
			scale = ATLAS_SCALE
			miniMap.SetAtlasScale(float(scale))
		mapWidth = int(sizeX * scale)
		mapHeight = int(sizeY * scale)
		width = PAD + mapWidth + GAP + 1 + GAP + PANEL_W + PAD
		height = TITLE_H + 12 + mapHeight + PAD
		self.size = (width, height)
		self.SetSize(width, height)
		self.frame.SetSize(width, height)
		self.title.SetPosition(width // 2, 14)
		self.divider.SetSize(width - PAD * 2, 1)
		self.closeButton.SetPosition(width - PAD - 34, 11 - 4)
		lineX = PAD + mapWidth + GAP
		self.vline.SetPosition(lineX, TITLE_H + 12)
		self.vline.SetSize(1, height - TITLE_H - 12 - PAD)
		textX = lineX + 1 + GAP
		self.infoTitle.SetPosition(textX, TITLE_H + 14)
		for (index, line) in enumerate(self.infoLines):
			line.SetPosition(textX, TITLE_H + 14 + 42 + index * LINE_H)
		self.renderer.SetPosition(PAD, TITLE_H + 12)
		self.catcher.SetPosition(PAD, TITLE_H + 12)
		self.catcher.SetSize(mapWidth, mapHeight)
		self.SetPosition(max(0, (wndMgr.GetScreenWidth() - width) // 2), max(0, (wndMgr.GetScreenHeight() - height) // 2))
		miniMap.ShowAtlas()
		self.Show()
		self.SetTop()

	def Close(self):
		miniMap.HideAtlas()
		self.Hide()

	def OnMapClick(self):
		(mouseX, mouseY) = wndMgr.GetMousePosition()
		info = miniMap.GetAtlasInfo(mouseX, mouseY)
		(posX, posY) = (info[2], info[3])
		if posX <= 0 and posY <= 0:
			return
		if chr.IsGameMaster(player.GetMainCharacterIndex()):
			net.SendChatPacket("/go %s %s" % (str(posX), str(posY)))
			return
		if SetTarget(posX * 100.0, posY * 100.0):
			self.Close()

	def OnPressEscapeKey(self):
		self.Close()
		return True


def Install(miniMapWindow):
	# eski Metin2 harita penceresi yalniz motor kaydi icin gizli kalir; Show / Hide / IsShow bizim pencereye yonlenir
	global atlasWindow, handler
	legacy = getattr(miniMapWindow, "AtlasWindow", None)
	if not legacy or getattr(legacy, "akravonInstalled", False):
		return
	legacy.akravonInstalled = True
	board = AtlasBoard()
	atlasWindow = board
	handler = board
	legacy.Hide()
	legacy.Show = board.Open
	legacy.Hide = board.Close
	legacy.IsShow = board.IsShow


def __Plan():
	global path, pathIndex, lastPlanTime
	(px, py, pz) = player.GetMainCharacterPosition()
	lastPlanTime = app.GetTime()
	try:
		points = background.FindPath(px, py, target[0], target[1])
	except Exception:
		points = ()
	if len(points) < 2:
		return False
	path = list(points)
	pathIndex = 1
	return True


def SetTarget(x, y):
	global target, lastProgressTime, lastProgressPos
	target = (x, y)
	if not __Plan():
		target = None
		background.SetFootprints(())
		return False
	(px, py, pz) = player.GetMainCharacterPosition()
	lastProgressTime = app.GetTime()
	lastProgressPos = (px, py)
	return True


def IsWalking():
	return target is not None


def Cancel():
	global target, path
	if target is None:
		return
	target = None
	path = []
	background.SetFootprints(())
	try:
		player.EndKeyWalkingImmediately()
	except Exception:
		pass


def __Footprints(px, py):
	# oyuncunun onunden hedefe dogru, yol uzerinde ayak izleri
	global footCounter
	prints = []
	remaining = FOOT_SPACING * 0.6
	currentX, currentY = px, py
	right = 0
	for index in range(pathIndex, len(path)):
		(nx, ny) = path[index]
		segment = math.hypot(nx - currentX, ny - currentY)
		if segment < 1.0:
			continue
		dx = (nx - currentX) / segment
		dy = (ny - currentY) / segment
		angle = math.degrees(math.atan2(dx, -dy))
		travelled = remaining
		while travelled <= segment and len(prints) < FOOT_MAX:
			side = FOOT_SIDE if right else -FOOT_SIDE
			prints.append((currentX + dx * travelled - dy * side, currentY + dy * travelled + dx * side, angle, right))
			right = 1 - right
			travelled += FOOT_SPACING
		remaining = travelled - segment
		currentX, currentY = nx, ny
		if len(prints) >= FOOT_MAX:
			break
	background.SetFootprints(tuple(prints))


def Update(interrupted):
	# her karede (HUD denetleyicisi); interrupted: joystick / saldiri dokunusu
	global pathIndex, lastProgressTime, lastProgressPos
	if target is None:
		return
	if interrupted:
		Cancel()
		return
	(px, py, pz) = player.GetMainCharacterPosition()
	if math.hypot(target[0] - px, target[1] - py) < ARRIVE_DISTANCE:
		Cancel()
		return
	now = app.GetTime()

	# takildiysa yolu yeniden hesapla, yine ilerlemiyorsa birak
	if now - lastProgressTime >= STUCK_SECONDS:
		if math.hypot(px - lastProgressPos[0], py - lastProgressPos[1]) < STUCK_DISTANCE:
			if not __Plan():
				Cancel()
				return
			if now - lastPlanTime < 0.01 and getattr(Update, "replans", 0) >= 3:
				Cancel()
				return
			Update.replans = getattr(Update, "replans", 0) + 1
		else:
			Update.replans = 0
		lastProgressTime = now
		lastProgressPos = (px, py)
	elif now - lastPlanTime >= REPLAN_SECONDS:
		__Plan()

	# siradaki ara nokta
	while pathIndex < len(path) - 1 and math.hypot(path[pathIndex][0] - px, path[pathIndex][1] - py) < WAYPOINT_DISTANCE:
		pathIndex += 1
	(wx, wy) = path[pathIndex]
	player.WalkToPixelPosition(wx, wy)
	__Footprints(px, py)
