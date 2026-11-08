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


class AtlasHandler(object):
	def __init__(self, atlas):
		self.atlas = atlas

	def OnClick(self):
		atlas = self.atlas
		if not atlas or not atlas.AtlasMainWindow:
			return
		(mouseX, mouseY) = wndMgr.GetMousePosition()
		info = miniMap.GetAtlasInfo(mouseX, mouseY)
		(posX, posY) = (info[2], info[3])
		if chr.IsGameMaster(player.GetMainCharacterIndex()):
			net.SendChatPacket("/go %s %s" % (str(posX), str(posY)))
			return
		if posX <= 0 and posY <= 0:
			return
		if SetTarget(posX * 100.0, posY * 100.0):
			atlas.Hide()


def Install(miniMapWindow):
	global atlasWindow, handler
	atlas = getattr(miniMapWindow, "AtlasWindow", None)
	if not atlas or getattr(atlas, "akravonInstalled", False):
		atlasWindow = atlas
		return
	atlas.akravonInstalled = True
	atlasWindow = atlas
	originalShow = atlas.Show

	def Show(*args):
		originalShow()
		Center(atlas)

	atlas.Show = Show
	handler = AtlasHandler(atlas)
	atlas.board.SetOnMouseLeftButtonUpEvent(ui.__mem_func__(handler.OnClick))


def Center(atlas):
	width = wndMgr.GetScreenWidth()
	height = wndMgr.GetScreenHeight()
	atlas.SetPosition(max(0, (width - atlas.GetWidth()) // 2), max(0, (height - atlas.GetHeight()) // 2))


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
