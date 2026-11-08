from __future__ import division
# Cynex: mobil ayarlar - kamera, joystick, saldiri dugmesi (Menu dugmesine uzun bas)
import app
import systemSetting
import wndMgr
import uiScriptLocale
import ui
import uiMobileHud
import uiMobileStyle

PANEL_WIDTH = 520
PADDING = 24
TITLE_TOP = 22
ROW_TOP = 70
ROW_HEIGHT = 50
STEP_BUTTON = 44
STEP_SIGN_FONT = "AlegreyaSansBold:30"	# Cynex: + / - isaretleri marcellus ile cok ince kaliyordu
VALUE_WIDTH = 110
CLOSE_SIZE = 30

# (ad, okuyucu, yazici, alt, ust, adim, sifir = otomatik, hud yeniden kurulur)
ROWS = (
	("MOBILE_OPT_CAMERA_PITCH", "GetCameraReturnPitch", "SetCameraReturnPitch", 0.0, 45.0, 1.0, False, False),
	("MOBILE_OPT_CAMERA_SENS", "GetCameraDragSens", "SetCameraDragSens", 0.25, 3.0, 0.25, False, False),
	("MOBILE_OPT_PINCH_SENS", "GetPinchZoomSens", "SetPinchZoomSens", 0.25, 3.0, 0.25, False, False),
	("MOBILE_OPT_JOYSTICK_SIZE", "GetJoystickSize", "SetJoystickSize", 80.0, 320.0, 20.0, True, True),
	("MOBILE_OPT_JOYSTICK_DEADZONE", "GetJoystickDeadZone", "SetJoystickDeadZone", 0.0, 0.5, 0.05, False, False),
	("MOBILE_OPT_ATTACK_SIZE", "GetAttackButtonSize", "SetAttackButtonSize", 80.0, 320.0, 20.0, True, True),
)


def Locale(name):
	return getattr(uiScriptLocale, name, name)


class StepperRow(object):
	def __init__(self, parent, y, spec, owner):
		(self.labelName, self.getter, self.setter, self.low, self.high, self.step, self.allowAuto, self.rebuild) = spec
		self.owner = owner
		self.label = uiMobileStyle.CreateText(parent, Locale(self.labelName), PADDING, y + 15, uiMobileStyle.SOFT_COLOR, uiMobileStyle.FONT_MEDIUM)
		right = PANEL_WIDTH - PADDING
		self.plus = uiMobileStyle.PillButton(STEP_BUTTON, STEP_BUTTON - 6, "+", None, "field", uiMobileStyle.GOLD_COLOR, STEP_SIGN_FONT)
		self.plus.SetParent(parent)
		self.plus.SetPosition(right - STEP_BUTTON, y + 3)
		self.plus.SetEvent(ui.__mem_func__(self.__Change), 1)
		self.plus.Show()
		self.value = uiMobileStyle.CreateText(parent, "", right - STEP_BUTTON - VALUE_WIDTH // 2, y + 15, uiMobileStyle.BRIGHT_COLOR, uiMobileStyle.FONT_MEDIUM, "center")
		self.minus = uiMobileStyle.PillButton(STEP_BUTTON, STEP_BUTTON - 6, "-", None, "field", uiMobileStyle.GOLD_COLOR, STEP_SIGN_FONT)
		self.minus.SetParent(parent)
		self.minus.SetPosition(right - STEP_BUTTON * 2 - VALUE_WIDTH, y + 3)
		self.minus.SetEvent(ui.__mem_func__(self.__Change), -1)
		self.minus.Show()
		self.Refresh()

	def Destroy(self):
		self.plus.Destroy()
		self.minus.Destroy()
		self.plus = None
		self.minus = None
		self.label = None
		self.value = None
		self.owner = None

	def __Get(self):
		function = getattr(systemSetting, self.getter, None)
		if function is None:
			return 0.0
		try:
			return float(function())
		except Exception:
			return 0.0

	def __Change(self, direction):
		current = self.__Get()
		if self.allowAuto and current <= 0.0:
			current = self.low if direction > 0 else 0.0
		else:
			current += self.step * direction
			if self.allowAuto and current < self.low:
				current = 0.0
			else:
				current = max(self.low, min(self.high, current))
		function = getattr(systemSetting, self.setter, None)
		if function:
			function(current)
		if self.rebuild:
			uiMobileHud.RequestRefresh()
		self.Refresh()

	def Refresh(self):
		current = self.__Get()
		if self.allowAuto and current <= 0.0:
			text = Locale("MOBILE_OPT_AUTO")
		elif self.labelName == "MOBILE_OPT_CAMERA_PITCH":
			text = "%d" % int(round(current))
		elif self.labelName == "MOBILE_OPT_JOYSTICK_DEADZONE":
			text = "%%%d" % int(round(current * 100))
		elif self.step < 1.0:
			text = "x%.2f" % current
		else:
			text = "%d" % int(round(current))
		self.value.SetText(text)


class MobileSettingsDialog(ui.Window):
	def __init__(self):
		ui.Window.__init__(self, "TOP_MOST")
		self.rows = []
		self.toggles = []
		height = ROW_TOP + len(ROWS) * ROW_HEIGHT + 2 * 40 + PADDING
		self.SetSize(PANEL_WIDTH, height)
		self.panel = uiMobileStyle.CreateOrnatePanel(self, 0, 0, PANEL_WIDTH, height)
		self.title = uiMobileStyle.CreateText(self, Locale("MOBILE_SETTINGS_TITLE"), PANEL_WIDTH // 2, TITLE_TOP, uiMobileStyle.GOLD_COLOR, uiMobileStyle.FONT_LARGE, "center")
		self.closeButton = uiMobileStyle.StateImageButton("close_30")
		self.closeButton.SetParent(self)
		self.closeButton.SetPosition(PANEL_WIDTH - CLOSE_SIZE - 14, 16)
		self.closeButton.SetEvent(ui.__mem_func__(self.Close))
		self.closeButton.Show()

		y = ROW_TOP
		for spec in ROWS:
			self.rows.append(StepperRow(self, y, spec, self))
			y += ROW_HEIGHT

		y += 6
		self.cameraReturn = self.__AddToggle(Locale("MOBILE_OPT_CAMERA_RETURN"), bool(self.__Call("IsCameraReturn", 1)), self.__OnCameraReturn, y)
		y += 40
		self.dynamicJoystick = self.__AddToggle(Locale("MOBILE_OPT_DYNAMIC_JOYSTICK"), uiMobileHud.IsDynamicJoystick(), self.__OnDynamicJoystick, y)

		self.SetPosition((wndMgr.GetScreenWidth() - PANEL_WIDTH) // 2, max(10, (wndMgr.GetScreenHeight() - height) // 2))
		self.Hide()

	def __del__(self):
		ui.Window.__del__(self)

	def __Call(self, name, default):
		function = getattr(systemSetting, name, None)
		if function is None:
			return default
		try:
			return function()
		except Exception:
			return default

	def __AddToggle(self, text, value, event, y):
		toggle = uiMobileStyle.Toggle(text, value)
		toggle.SetParent(self)
		toggle.SetPosition(PADDING, y)
		toggle.SetChangeEvent(ui.__mem_func__(event))
		toggle.Show()
		self.toggles.append(toggle)
		return toggle

	def __OnCameraReturn(self, value):
		function = getattr(systemSetting, "SetCameraReturn", None)
		if function:
			function(1 if value else 0)

	def __OnDynamicJoystick(self, value):
		if value != uiMobileHud.IsDynamicJoystick():
			uiMobileHud.SetDynamicJoystick(value)
			uiMobileHud.RequestRefresh()

	def Destroy(self):
		for row in self.rows:
			row.Destroy()
		for toggle in self.toggles:
			toggle.Destroy()
		self.rows = []
		self.toggles = []
		self.cameraReturn = None
		self.dynamicJoystick = None
		if self.closeButton:
			self.closeButton.Destroy()
		self.closeButton = None
		self.panel = None
		self.title = None
		self.Hide()

	def Open(self):
		for row in self.rows:
			row.Refresh()
		self.cameraReturn.SetValue(bool(self.__Call("IsCameraReturn", 1)))
		self.dynamicJoystick.SetValue(uiMobileHud.IsDynamicJoystick())
		self.Show()
		self.SetTop()

	def Close(self):
		self.Hide()

	def Toggle(self):
		if self.IsShow():
			self.Close()
		else:
			self.Open()

	def OnPressEscapeKey(self):
		self.Close()
		return True


dialog = None


def Toggle():
	global dialog
	if dialog is None:
		dialog = MobileSettingsDialog()
	dialog.Toggle()


def Hide():
	if dialog:
		dialog.Close()
