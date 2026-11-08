from __future__ import division
import app
import math
import player
import systemSetting
import wndMgr
import ui
import uiMobileCompat

from _weakref import proxy

BASE_TEXTURE = "mobile_ui/akravon/hud/hud_joystick.png"
KNOB_TEXTURE = "mobile_ui/akravon/hud/hud_knob.png"
BASE_TEXTURE_SIZE = 400.0
KNOB_TEXTURE_SIZE = 160.0
FALLBACK_FILL_TEXTURE = "d:/ymir work/ui/minimap_image_filter.dds"
FALLBACK_RING_TEXTURE = "d:/ymir work/ui/minimap/minimap.sub"
FALLBACK_FILL_SIZE = 128.0
FALLBACK_RING_WIDTH = 136.0
FALLBACK_RING_HEIGHT = 137.0
FALLBACK_BASE_COLOR = (0.05, 0.05, 0.08, 0.45)
FALLBACK_KNOB_COLOR = (0.78, 0.78, 0.78, 0.80)
FALLBACK_KNOB_ACTIVE_COLOR = (0.35, 0.62, 0.98, 0.90)
KNOB_COLOR = (0.9, 0.9, 0.9, 0.62)
BASE_IDLE_COLOR = (1.0, 1.0, 1.0, 0.45)
BASE_ACTIVE_COLOR = (1.0, 1.0, 1.0, 0.88)
KNOB_ACTIVE_COLOR = (1.0, 1.0, 1.0, 1.0)

LOCK_DISC_TEXTURE = "mobile_ui/akravon/hud/hud_lock_disc.png"
LOCK_TEXTURE_SIZE = 128.0
LOCK_ICON_TEXTURES = {"idle" : "mobile_ui/akravon/hud/lock_open.png", "armed" : "mobile_ui/akravon/hud/lock_closed.png", "active" : "mobile_ui/akravon/hud/lock_closed.png"}
LOCK_ICON_SIZE = 128.0
LOCK_IDLE_COLOR = (1.0, 1.0, 1.0, 0.9)
LOCK_ARMED_COLOR = (1.0, 0.92, 0.74, 1.0)
LOCK_ACTIVE_COLOR = (1.0, 0.82, 0.46, 1.0)

NATIVE_REGION = 2
FORWARD_ROTATION = 0.0


class RunLockIndicator(ui.Window):
	def __init__(self, size):
		ui.Window.__init__(self, "UI")
		self.AddFlag("not_pick")
		self.SetSize(size, size)

		self.disc = ui.ExpandedImageBox()
		self.disc.SetParent(self)
		self.disc.AddFlag("not_pick")
		self.disc.LoadImage(LOCK_DISC_TEXTURE)
		self.disc.SetScale(size / LOCK_TEXTURE_SIZE, size / LOCK_TEXTURE_SIZE)
		self.disc.Show()

		self.icon = ui.ExpandedImageBox()
		self.icon.SetParent(self)
		self.icon.AddFlag("not_pick")
		self.icon.Show()

		self.state = None
		self.SetState("idle")

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		self.disc = None
		self.icon = None

	def SetState(self, state):
		if state == self.state:
			return
		self.state = state
		color = {"idle" : LOCK_IDLE_COLOR, "armed" : LOCK_ARMED_COLOR, "active" : LOCK_ACTIVE_COLOR}[state]
		self.icon.LoadImage(LOCK_ICON_TEXTURES[state])
		iconScale = self.GetWidth() * 0.62 / LOCK_ICON_SIZE
		self.icon.SetScale(iconScale, iconScale)
		iconWidth = int(LOCK_ICON_SIZE * iconScale)
		self.icon.SetPosition((self.GetWidth() - iconWidth) // 2, (self.GetHeight() - iconWidth) // 2)
		wndMgr.SetDiffuseColor(self.disc.hWnd, *color)
		wndMgr.SetDiffuseColor(self.icon.hWnd, *color)


class VirtualJoystick(ui.Window):
	KNOB_RATIO = 78.0 / 190.0
	BODY_RATIO = 0.96
	RETURN_EASING = 0.55

	def __init__(self, gameWindow, size, zone = None, runLock = None):
		ui.Window.__init__(self, "UI")

		self.gameWindow = proxy(gameWindow) if gameWindow else None
		self.baseSize = int(size)
		self.knobSize = int(self.baseSize * self.KNOB_RATIO)
		self.radius = (self.baseSize - self.knobSize) / 2.0

		self.zone = zone
		self.homeX = 0
		self.homeY = 0
		self.baseX = 0
		self.baseY = 0

		self.knobOffsetX = 0.0
		self.knobOffsetY = 0.0
		self.isActive = False
		self.isMoving = False
		self.isSteering = False
		self.touchId = None
		self.mouseActive = False
		self.runLocked = False
		self.runLockArmed = False

		self.SetWindowName("VirtualJoystick")
		uiMobileCompat.SetTouchNative(self)
		ui.EnableSingleTap(self)

		self.baseWindow = ui.Window()
		self.baseWindow.SetParent(self)
		self.baseWindow.AddFlag("not_pick")
		self.baseWindow.SetSize(self.baseSize, self.baseSize)
		self.baseWindow.Show()

		self.usesMobileArt = False
		self.ringImage = None
		self.baseImage = self.__CreateImage(BASE_TEXTURE, self.baseSize / BASE_TEXTURE_SIZE, self.baseSize / BASE_TEXTURE_SIZE)
		if self.baseImage:
			self.usesMobileArt = True
			self.knobImage = self.__CreateImage(KNOB_TEXTURE, self.knobSize / KNOB_TEXTURE_SIZE, self.knobSize / KNOB_TEXTURE_SIZE)
		else:
			self.baseImage = self.__CreateImage(FALLBACK_FILL_TEXTURE, self.baseSize / FALLBACK_FILL_SIZE, self.baseSize / FALLBACK_FILL_SIZE)
			if self.baseImage:
				wndMgr.SetDiffuseColor(self.baseImage.hWnd, *FALLBACK_BASE_COLOR)
			self.ringImage = self.__CreateImage(FALLBACK_RING_TEXTURE, self.baseSize / FALLBACK_RING_WIDTH, self.baseSize / FALLBACK_RING_HEIGHT)
			self.knobImage = self.__CreateImage(FALLBACK_FILL_TEXTURE, self.knobSize / FALLBACK_FILL_SIZE, self.knobSize / FALLBACK_FILL_SIZE)

		self.lockIndicator = None
		self.lockSize = 0
		self.lockGap = 0
		if runLock:
			(self.lockSize, self.lockGap) = runLock
			self.lockIndicator = RunLockIndicator(self.lockSize)
			self.lockIndicator.SetParent(self)
			self.lockIndicator.Hide()

		self.knobActiveState = None
		self.__PlaceKnob()
		self.__RefreshKnobState()

	def __del__(self):
		ui.Window.__del__(self)

	def __CreateImage(self, fileName, scaleX, scaleY):
		try:
			image = ui.ExpandedImageBox()
			image.SetParent(self.baseWindow)
			image.AddFlag("not_pick")
			image.LoadImage(fileName)
			image.SetScale(scaleX, scaleY)
			image.SetPosition(0, 0)
			image.Show()
			return image
		except Exception:
			return None

	def Place(self, x, y):
		if self.zone:
			(zoneX, zoneY, zoneWidth, zoneHeight) = self.zone
			self.SetPosition(zoneX, zoneY)
			self.SetSize(zoneWidth, zoneHeight)
			self.homeX = x - zoneX
			self.homeY = y - zoneY
		else:
			self.SetPosition(x, y)
			self.SetSize(self.baseSize, self.baseSize)
			self.homeX = 0
			self.homeY = 0
		self.__MoveBase(self.homeX, self.homeY)

	def __MoveBase(self, x, y):
		self.baseX = int(x)
		self.baseY = int(y)
		self.baseWindow.SetPosition(self.baseX, self.baseY)
		if self.lockIndicator:
			self.lockIndicator.SetPosition(self.baseX + (self.baseSize - self.lockSize) // 2, self.baseY - self.lockGap - self.lockSize)

	def Destroy(self):
		self.Stop()
		self.gameWindow = None
		self.baseImage = None
		self.ringImage = None
		self.knobImage = None
		if self.lockIndicator:
			self.lockIndicator.Destroy()
			self.lockIndicator = None
		self.baseWindow = None
		self.Hide()

	def Stop(self):
		self.mouseActive = False
		self.isActive = False
		self.touchId = None
		self.runLocked = False
		self.runLockArmed = False
		self.__StopMovement()
		self.knobOffsetX = 0.0
		self.knobOffsetY = 0.0
		if self.zone:
			self.__MoveBase(self.homeX, self.homeY)
		self.__PlaceKnob()
		self.__RefreshKnobState()
		self.__RefreshLockState()

	def __GetCenter(self):
		(x, y) = self.GetGlobalPosition()
		return (x + self.baseX + self.baseSize / 2.0, y + self.baseY + self.baseSize / 2.0)

	def __IsInsideBase(self, pointX, pointY):
		(centerX, centerY) = self.__GetCenter()
		return math.hypot(pointX - centerX, pointY - centerY) <= self.baseSize / 2.0 * self.BODY_RATIO

	def __IsInsideWindow(self, pointX, pointY):
		(x, y) = self.GetGlobalPosition()
		return x <= pointX < x + self.GetWidth() and y <= pointY < y + self.GetHeight()

	def __CanCapture(self, pointX, pointY):
		if self.zone:
			return self.__IsInsideWindow(pointX, pointY)
		return self.__IsInsideBase(pointX, pointY)

	def __IsInsideLock(self, pointX, pointY):
		if not self.lockIndicator:
			return False
		(x, y) = self.lockIndicator.GetGlobalPosition()
		margin = 6	# AKRAVON: kilit yalnizca bilincli olarak ona surukleyince (kendiliginden kilitlenmesin)
		return x - margin <= pointX < x + self.lockSize + margin and y - margin <= pointY < y + self.lockSize + margin

	def __Begin(self, pointX, pointY):
		self.runLocked = False
		self.isActive = True
		if self.zone:
			(x, y) = self.GetGlobalPosition()
			half = self.baseSize // 2
			baseX = min(max(0, pointX - x - half), self.GetWidth() - self.baseSize)
			baseY = min(max(self.lockSize + self.lockGap, pointY - y - half), self.GetHeight() - self.baseSize)
			self.__MoveBase(baseX, baseY)
		self.__Steer(pointX, pointY)

	def __End(self):
		self.isActive = False
		self.touchId = None
		self.mouseActive = False
		if self.runLockArmed:
			self.runLockArmed = False
			self.runLocked = True
			self.knobOffsetX = 0.0
			self.knobOffsetY = -self.radius
		else:
			self.__StopMovement()
			if self.zone:
				self.__MoveBase(self.homeX, self.homeY)
		self.__RefreshLockState()

	def OnMouseLeftButtonDown(self):
		(mouseX, mouseY) = wndMgr.GetMousePosition()
		if not self.__CanCapture(mouseX, mouseY):
			return True
		self.mouseActive = True
		self.__Begin(mouseX, mouseY)
		return True

	def OnMouseLeftButtonUp(self):
		if self.mouseActive:
			self.__End()
		return True

	def __FindTouch(self, touchId):
		for index in range(app.GetTouchCount()):
			point = app.GetTouchPoint(index)
			if point and point[0] == touchId:
				return point
		return None

	def __CaptureTouch(self):
		for index in range(app.GetTouchCount()):
			point = app.GetTouchPointEx(index)
			if not point or point[3] != NATIVE_REGION:
				continue
			if self.__CanCapture(point[1], point[2]):
				return point
		return None

	def OnUpdate(self):
		if self.mouseActive:
			(mouseX, mouseY) = wndMgr.GetMousePosition()
			self.__Steer(mouseX, mouseY)
		elif self.isActive:
			point = self.__FindTouch(self.touchId)
			if point:
				self.__Steer(point[1], point[2])
			else:
				self.__End()
		else:
			point = self.__CaptureTouch()
			if point:
				self.touchId = point[0]
				self.__Begin(point[1], point[2])

		if self.runLocked:
			player.MoveToDirection(FORWARD_ROTATION)
			self.isMoving = True
		elif not self.isActive and (self.knobOffsetX != 0.0 or self.knobOffsetY != 0.0):
			self.knobOffsetX *= self.RETURN_EASING
			self.knobOffsetY *= self.RETURN_EASING
			if abs(self.knobOffsetX) < 1.0 and abs(self.knobOffsetY) < 1.0:
				self.knobOffsetX = 0.0
				self.knobOffsetY = 0.0

		self.__PlaceKnob()
		self.__RefreshKnobState()

	def __Steer(self, pointX, pointY):
		(centerX, centerY) = self.__GetCenter()
		deltaX = pointX - centerX
		deltaY = pointY - centerY

		distance = math.hypot(deltaX, deltaY)
		if self.radius > 0 and distance > self.radius:
			scale = self.radius / distance
			deltaX *= scale
			deltaY *= scale
			distance = self.radius

		self.knobOffsetX = deltaX
		self.knobOffsetY = deltaY

		armed = self.__IsInsideLock(pointX, pointY)
		if armed != self.runLockArmed:
			self.runLockArmed = armed
			self.__RefreshLockState()

		self.__ApplyMovement(deltaX, deltaY, distance)

	def __ApplyMovement(self, deltaX, deltaY, distance):
		if self.radius <= 0:
			return

		deadZone = self.radius * systemSetting.GetJoystickDeadZone()
		if distance <= 0.0 or distance < deadZone:
			self.__StopMovement()
			return

		if not self.isSteering:
			self.isSteering = True
			self.__ReleaseTarget()

		player.MoveToDirection(self.__GetRotation(deltaX, deltaY))
		self.isMoving = True

	def __GetRotation(self, deltaX, deltaY):
		length = math.hypot(deltaX, deltaY)
		if length == 0.0:
			return 0.0
		dot = max(-1.0, min(1.0, -deltaY / length))
		rotation = math.degrees(math.acos(dot))
		if deltaX < 0.0:
			rotation = 360.0 - rotation
		return (360.0 - rotation) % 360.0

	def __ReleaseTarget(self):
		player.ClearTarget()
		if self.gameWindow is None:
			return
		try:
			self.gameWindow.CloseTargetBoard()
		except ReferenceError:
			self.gameWindow = None

	def __StopMovement(self):
		self.isSteering = False
		if self.isMoving:
			self.isMoving = False
			player.EndKeyWalkingImmediately()

	def __PlaceKnob(self):
		if not self.knobImage:
			return
		x = int(self.baseSize / 2.0 + self.knobOffsetX - self.knobSize / 2.0)
		y = int(self.baseSize / 2.0 + self.knobOffsetY - self.knobSize / 2.0)
		self.knobImage.SetPosition(x, y)

	def __RefreshKnobState(self):
		if not self.knobImage:
			return
		active = self.isActive or self.runLocked
		if self.knobActiveState == active:
			return
		self.knobActiveState = active
		if self.usesMobileArt:
			color = KNOB_ACTIVE_COLOR if active else KNOB_COLOR
		else:
			color = FALLBACK_KNOB_ACTIVE_COLOR if active else FALLBACK_KNOB_COLOR
		wndMgr.SetDiffuseColor(self.knobImage.hWnd, *color)
		# AKRAVON: joystick bosta yari saydam, dokununca belirginlesir
		if self.usesMobileArt and self.baseImage:
			wndMgr.SetDiffuseColor(self.baseImage.hWnd, *(BASE_ACTIVE_COLOR if active else BASE_IDLE_COLOR))

	def __RefreshLockState(self):
		if not self.lockIndicator:
			return
		# AKRAVON: kilit yalnizca joystick tutulurken veya kilitliyken gorunur
		if not (self.isActive or self.runLocked):
			self.lockIndicator.Hide()
			self.lockIndicator.SetState("idle")
			return
		if self.runLocked:
			self.lockIndicator.SetState("active")
		elif self.runLockArmed:
			self.lockIndicator.SetState("armed")
		else:
			self.lockIndicator.SetState("idle")
		self.lockIndicator.Show()
