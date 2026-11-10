from __future__ import division
import app
import math
import player
import wndMgr
import ui
import uiMobileCompat

BASE_TEXTURE = "mobile_ui/akravon/hud/hud_attack.png"
PRESSED_TEXTURE = "mobile_ui/akravon/hud/hud_attack_pressed.png"
TEXTURE_SIZE = 320.0
FALLBACK_FILL_TEXTURE = "d:/ymir work/ui/minimap_image_filter.dds"
FALLBACK_RING_TEXTURE = "d:/ymir work/ui/minimap/minimap.sub"
FALLBACK_ICON_TEXTURE = "d:/ymir work/ui/game/taskbar/mouse_button_attack_01.sub"
FALLBACK_PRESSED_ICON_TEXTURE = "d:/ymir work/ui/game/taskbar/mouse_button_attack_03.sub"
FALLBACK_FILL_SIZE = 128.0
FALLBACK_RING_WIDTH = 136.0
FALLBACK_RING_HEIGHT = 137.0
FALLBACK_ICON_SIZE = 32.0
FALLBACK_ICON_SCALE = 1.75
FALLBACK_BASE_COLOR = (0.62, 0.11, 0.11, 0.50)
FALLBACK_ACTIVE_COLOR = (0.95, 0.25, 0.25, 0.75)

NATIVE_REGION = 2


class AttackButton(ui.Window):
	PRESSED_RATIO = 0.94

	def __init__(self, size):
		ui.Window.__init__(self, "UI")

		self.size = int(size)
		self.isActive = False
		self.touchId = None
		self.mouseActive = False

		self.SetSize(self.size, self.size)
		self.SetWindowName("AttackButton")
		uiMobileCompat.SetTouchNative(self)
		ui.EnableSingleTap(self)

		self.usesMobileArt = False
		self.pressedImage = None
		self.ringImage = None
		self.iconImage = None
		self.pressedIconImage = None
		self.baseImage = self.__CreateImage(BASE_TEXTURE, self.size / TEXTURE_SIZE, self.size / TEXTURE_SIZE)
		if self.baseImage:
			self.usesMobileArt = True
			ratio = self.size / TEXTURE_SIZE * self.PRESSED_RATIO
			self.pressedImage = self.__CreateImage(PRESSED_TEXTURE, ratio, ratio)
			if self.pressedImage:
				offset = int(self.size * (1.0 - self.PRESSED_RATIO) / 2.0)
				self.pressedImage.SetPosition(offset, offset)
				self.pressedImage.Hide()
		else:
			self.baseImage = self.__CreateImage(FALLBACK_FILL_TEXTURE, self.size / FALLBACK_FILL_SIZE, self.size / FALLBACK_FILL_SIZE)
			if self.baseImage:
				wndMgr.SetDiffuseColor(self.baseImage.hWnd, *FALLBACK_BASE_COLOR)
			self.ringImage = self.__CreateImage(FALLBACK_RING_TEXTURE, self.size / FALLBACK_RING_WIDTH, self.size / FALLBACK_RING_HEIGHT)

			iconSize = int(FALLBACK_ICON_SIZE * FALLBACK_ICON_SCALE)
			iconOffset = (self.size - iconSize) // 2
			self.iconImage = self.__CreateImage(FALLBACK_ICON_TEXTURE, FALLBACK_ICON_SCALE, FALLBACK_ICON_SCALE)
			self.pressedIconImage = self.__CreateImage(FALLBACK_PRESSED_ICON_TEXTURE, FALLBACK_ICON_SCALE, FALLBACK_ICON_SCALE)
			if self.iconImage:
				self.iconImage.SetPosition(iconOffset, iconOffset)
			if self.pressedIconImage:
				self.pressedIconImage.SetPosition(iconOffset, iconOffset)
				self.pressedIconImage.Hide()
		self.visualActive = False
		# oyuncunun sectigi seffaflik (ayarlar > kontroller)
		import uiAkravonHudAlpha
		fixedAlpha = uiAkravonHudAlpha.Get("atk")
		if self.usesMobileArt and fixedAlpha < 0.999:
			for image in (self.baseImage, self.pressedImage):
				if image:
					wndMgr.SetDiffuseColor(image.hWnd, 1.0, 1.0, 1.0, fixedAlpha)

	def __del__(self):
		ui.Window.__del__(self)

	def __CreateImage(self, fileName, scaleX, scaleY):
		try:
			image = ui.ExpandedImageBox()
			image.SetParent(self)
			image.AddFlag("not_pick")
			image.LoadImage(fileName)
			image.SetScale(scaleX, scaleY)
			image.SetPosition(0, 0)
			image.Show()
			return image
		except Exception:
			return None

	def Destroy(self):
		self.Stop()
		self.baseImage = None
		self.pressedImage = None
		self.ringImage = None
		self.iconImage = None
		self.pressedIconImage = None
		self.Hide()

	def Stop(self):
		self.mouseActive = False
		self.touchId = None
		self.__Release()
		self.__RefreshVisual()

	def __Press(self):
		if not self.isActive:
			self.isActive = True
			player.SetAttackKeyState(True)

	def __Release(self):
		if self.isActive:
			self.isActive = False
			player.SetAttackKeyState(False)

	def __IsInsideCircle(self, pointX, pointY):
		(x, y) = self.GetGlobalPosition()
		return math.hypot(pointX - (x + self.size / 2.0), pointY - (y + self.size / 2.0)) <= self.size / 2.0

	def OnMouseLeftButtonDown(self):
		(mouseX, mouseY) = wndMgr.GetMousePosition()
		if not self.__IsInsideCircle(mouseX, mouseY):
			return True
		self.mouseActive = True
		self.__Press()
		return True

	def OnMouseLeftButtonUp(self):
		if self.mouseActive:
			self.mouseActive = False
			self.touchId = None
			self.__Release()
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
			if self.__IsInsideCircle(point[1], point[2]):
				return point
		return None

	def OnUpdate(self):
		if self.mouseActive:
			pass
		elif self.isActive:
			if not self.__FindTouch(self.touchId):
				self.touchId = None
				self.__Release()
		else:
			point = self.__CaptureTouch()
			if point:
				self.touchId = point[0]
				self.__Press()

		self.__RefreshVisual()

	def __RefreshVisual(self):
		if self.visualActive == self.isActive:
			return
		self.visualActive = self.isActive

		if self.usesMobileArt:
			if self.pressedImage:
				if self.isActive:
					self.baseImage.Hide()
					self.pressedImage.Show()
				else:
					self.pressedImage.Hide()
					self.baseImage.Show()
			return

		if self.baseImage:
			color = FALLBACK_ACTIVE_COLOR if self.isActive else FALLBACK_BASE_COLOR
			wndMgr.SetDiffuseColor(self.baseImage.hWnd, *color)
		if self.iconImage and self.pressedIconImage:
			if self.isActive:
				self.iconImage.Hide()
				self.pressedIconImage.Show()
			else:
				self.pressedIconImage.Hide()
				self.iconImage.Show()
