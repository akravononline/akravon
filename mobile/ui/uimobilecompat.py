# Cynex: barbaros ui.py'ye dokunmadan mobil dokunma eklentileri
import app
import wndMgr
import ui

import uiMobileLocale


def IsMobile():
	function = getattr(app, "IsMobile", None)
	if function is None:
		return False
	try:
		return bool(function())
	except Exception:
		return False


def EnableSingleTap(window):
	if hasattr(wndMgr, "SetTekDokunus") and window and getattr(window, "hWnd", None) is not None:
		wndMgr.SetTekDokunus(window.hWnd, 1)


def SetTouchNative(window, enable = True):
	if hasattr(wndMgr, "SetTouchNative") and window and getattr(window, "hWnd", None) is not None:
		wndMgr.SetTouchNative(window.hWnd, 1 if enable else 0)


# Cynex: motor yalniz ornekte OnTouch* olan pencereyi sahiplenir; sinif metodu olsaydi her pencere sahiplenirdi
def _SetTouchTapEvent(self, event):
	self.eventTouchTap = event
	self.OnTouchTap = ui.__mem_func__(self._CynexTouchTap)


def _SetTouchLongPressEvent(self, event):
	self.eventTouchLongPress = event
	self.OnTouchLongPress = ui.__mem_func__(self._CynexTouchLongPress)


def _SetTouchLongPressCancelEvent(self, event):
	self.eventTouchLongPressCancel = event
	self.OnTouchLongPressCancel = ui.__mem_func__(self._CynexTouchLongPressCancel)


def _ClearTouchEvents(self):
	for name in ("OnTouchTap", "OnTouchLongPress", "OnTouchLongPressCancel"):
		if name in self.__dict__:
			del self.__dict__[name]
	self.eventTouchTap = None
	self.eventTouchLongPress = None
	self.eventTouchLongPressCancel = None


def _CynexTouchTap(self, x, y):
	event = getattr(self, "eventTouchTap", None)
	if event:
		event(x, y)


def _CynexTouchLongPress(self, x, y):
	event = getattr(self, "eventTouchLongPress", None)
	if event:
		event(x, y)


def _CynexTouchLongPressCancel(self):
	event = getattr(self, "eventTouchLongPressCancel", None)
	if event:
		event()


def _SetTapItemEvent(self, event, *args):
	self.eventTapItem = event
	self.eventTapItemArgs = args
	self.OnTapItem = ui.__mem_func__(self._CynexTapItem)


def _SetTapEmptySlotEvent(self, event, *args):
	self.eventTapEmptySlot = event
	self.eventTapEmptySlotArgs = args
	self.OnTapEmptySlot = ui.__mem_func__(self._CynexTapEmptySlot)


def _SetLongPressItemEvent(self, event, *args):
	self.eventLongPressItem = event
	self.eventLongPressItemArgs = args
	self.OnLongPressItem = ui.__mem_func__(self._CynexLongPressItem)


def _SetLongPressEmptySlotEvent(self, event, *args):
	self.eventLongPressEmptySlot = event
	self.eventLongPressEmptySlotArgs = args
	self.OnLongPressEmptySlot = ui.__mem_func__(self._CynexLongPressEmptySlot)


def _CynexTapItem(self, slotNumber):
	event = getattr(self, "eventTapItem", None)
	if event:
		event(slotNumber, *getattr(self, "eventTapItemArgs", ()))


def _CynexTapEmptySlot(self, slotNumber):
	event = getattr(self, "eventTapEmptySlot", None)
	if event:
		event(slotNumber, *getattr(self, "eventTapEmptySlotArgs", ()))


def _CynexLongPressItem(self, slotNumber):
	event = getattr(self, "eventLongPressItem", None)
	if event:
		event(slotNumber, *getattr(self, "eventLongPressItemArgs", ()))


def _CynexLongPressEmptySlot(self, slotNumber):
	event = getattr(self, "eventLongPressEmptySlot", None)
	if event:
		event(slotNumber, *getattr(self, "eventLongPressEmptySlotArgs", ()))


def _SetSlotScale(self, slotIndex, itemIndex, width, height, icon, sx, sy, diffuseColor = (1.0, 1.0, 1.0, 1.0)):
	if hasattr(wndMgr, "SetSlotScale"):
		wndMgr.SetSlotScale(self.hWnd, slotIndex, itemIndex, width, height, icon, diffuseColor, sx, sy)
	else:
		wndMgr.SetSlot(self.hWnd, slotIndex, itemIndex, width, height, icon, diffuseColor)


def Install():
	window = ui.Window
	if getattr(window, "_cynexTouchInstalled", False):
		return
	window._cynexTouchInstalled = True
	window.SetTouchTapEvent = _SetTouchTapEvent
	window.SetTouchLongPressEvent = _SetTouchLongPressEvent
	window.SetTouchLongPressCancelEvent = _SetTouchLongPressCancelEvent
	window.ClearTouchEvents = _ClearTouchEvents
	window._CynexTouchTap = _CynexTouchTap
	window._CynexTouchLongPress = _CynexTouchLongPress
	window._CynexTouchLongPressCancel = _CynexTouchLongPressCancel

	slot = ui.SlotWindow
	slot.SetTapItemEvent = _SetTapItemEvent
	slot.SetTapEmptySlotEvent = _SetTapEmptySlotEvent
	slot.SetLongPressItemEvent = _SetLongPressItemEvent
	slot.SetLongPressEmptySlotEvent = _SetLongPressEmptySlotEvent
	slot._CynexTapItem = _CynexTapItem
	slot._CynexTapEmptySlot = _CynexTapEmptySlot
	slot._CynexLongPressItem = _CynexLongPressItem
	slot._CynexLongPressEmptySlot = _CynexLongPressEmptySlot
	if not hasattr(slot, "SetSlotScale"):
		slot.SetSlotScale = _SetSlotScale

	ui.EnableSingleTap = EnableSingleTap
	ui.IsMobileClient = IsMobile


Install()
