from __future__ import division
import app
import wndMgr
import ui
import uiMobileCompat

from _weakref import proxy

TITLE_BAR_HIT_HEIGHT = 30
CLOSE_BUTTON_SIZE = 30


def FitButton(button, width, height):
	baseWidth = button.GetWidth()
	baseHeight = button.GetHeight()
	if baseWidth > 0 and baseHeight > 0:
		button.SetButtonScale(float(width) / baseWidth, float(height) / baseHeight)
	button.SetSize(width, height)


def FitButtonToArea(window, buttonName):
	button = window.GetChild2(buttonName)
	area = window.GetChild2(buttonName + "Area")
	if button and area:
		FitButton(button, area.GetWidth(), area.GetHeight())
	return button


def FitButtonsToAreas(window, buttonNames):
	return [FitButtonToArea(window, name) for name in buttonNames]


def EnlargeCloseButton(titleBar):
	width = titleBar.GetWidth()
	titleBar.SetSize(width, TITLE_BAR_HIT_HEIGHT)
	FitButton(titleBar.btnClose, CLOSE_BUTTON_SIZE, CLOSE_BUTTON_SIZE)
	titleBar.btnClose.SetPosition(width - CLOSE_BUTTON_SIZE, 0)


def SetButtonLabel(button, text):
	if button:
		button.SetText(text)


def SetButtonActive(button, imageName, active):
	if not button:
		return
	button.SetUpVisual(imageName % ("down" if active else "normal"))
	button.SetOverVisual(imageName % ("down" if active else "over"))
	button.SetDownVisual(imageName % "down")
	button.SetUp()


def InstallFocusGuard(editLine, group):
	target = proxy(editLine)

	def FocusWhenTouched():
		try:
			allowed = target.IsIn() or any(member.IsFocus() for member in group)
			handle = target.hWnd
		except ReferenceError:
			return
		if allowed:
			wndMgr.SetFocus(handle)

	editLine.SetFocus = FocusWhenTouched
	if editLine.IsFocus():
		editLine.KillFocus()


def GuardAutoFocus(editLines):
	group = [proxy(editLine) for editLine in editLines]
	for editLine in editLines:
		InstallFocusGuard(editLine, group)


def IsInsideWindow(window, x, y):
	if not window or not window.IsShow():
		return False
	(left, top) = window.GetGlobalPosition()
	return left <= x < left + window.GetWidth() and top <= y < top + window.GetHeight()


def GridCellAt(grid, x, y, columns, cellSize, cellCount):
	if not IsInsideWindow(grid, x, y):
		return None
	(left, top) = grid.GetGlobalPosition()
	column = int((x - left) // cellSize)
	row = int((y - top) // cellSize)
	if column < 0 or column >= columns or row < 0:
		return None
	cell = row * columns + column
	if cell >= cellCount:
		return None
	return cell


class ScrollStrip(ui.Window):
	def __init__(self, scrollBar):
		ui.Window.__init__(self)
		self.scrollBar = proxy(scrollBar)
		self.dragging = False

	def __del__(self):
		ui.Window.__del__(self)

	def OnMouseLeftButtonDown(self):
		self.dragging = True
		self.__Apply()
		return True

	def OnMouseLeftButtonUp(self):
		self.dragging = False
		return True

	def OnUpdate(self):
		if not self.dragging:
			return
		if uiMobileCompat.IsMobile() and app.GetTouchCount() == 0:
			self.dragging = False
			return
		self.__Apply()

	def __Apply(self):
		try:
			if not self.scrollBar.IsShow():
				return
			top = self.GetGlobalPosition()[1]
			self.scrollBar.SetPos(float(wndMgr.GetMousePosition()[1] - top) / max(1, self.GetHeight()))
		except ReferenceError:
			self.dragging = False


class TouchDrag(object):
	def __init__(self):
		self.Reset()

	def Reset(self):
		self.active = False
		self.touchId = None
		self.position = (0, 0)
		self.source = None

	def Begin(self, source):
		(mouseX, mouseY) = wndMgr.GetMousePosition()
		nearest = None
		for index in range(app.GetTouchCount()):
			point = app.GetTouchPoint(index)
			if not point:
				continue
			distance = abs(point[1] - mouseX) + abs(point[2] - mouseY)
			if nearest is None or distance < nearest[0]:
				nearest = (distance, point)
		if nearest is None:
			return False
		self.active = True
		self.touchId = nearest[1][0]
		self.position = (nearest[1][1], nearest[1][2])
		self.source = source
		return True

	def Update(self):
		if not self.active:
			return None
		for index in range(app.GetTouchCount()):
			point = app.GetTouchPoint(index)
			if point and point[0] == self.touchId:
				self.position = (point[1], point[2])
				return None
		released = (self.source, self.position[0], self.position[1])
		self.Reset()
		return released
