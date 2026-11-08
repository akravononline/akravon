# Cynex: py2 sinamasi - py2host.exe <pc istemci python27.dll> <python27 kok> -B test_hud.py
import sys, os, traceback
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import harness
from harness import VAL, CALLS, ERRORS

app = sys.modules['app']
app.GetLocalePath = lambda: 'locale/tr'
app.GetLocaleServiceName = lambda: ''
app.GetLocaleName = lambda: 'tr'
app.GetDefaultCodePage = lambda: 1254
WORK = os.path.join(os.environ.get('TEMP', '.'), 'cynex_ui_test')
if not os.path.isdir(WORK):
	os.makedirs(WORK)
os.chdir(WORK)
for _n in ('mobile_hud.cfg', 'touch.cfg'):
	if os.path.exists(_n):
		os.remove(_n)

STEP = [None]
def Step(name):
	STEP[0] = name
	print '==', name

def Run(fn):
	try:
		fn()
		return True
	except Exception:
		print 'HATA', STEP[0]
		traceback.print_exc()
		return False

ok = True
Step('import ui')
ok &= Run(lambda: __import__('ui'))
import ui

Step('import mobile modules')
def ImportAll():
	for name in ('uiMobileCompat', 'uiMobileAssets', 'uiMobileStyle', 'uiMobileWidgets', 'uiVirtualJoystick',
			'uiAttackButton', 'uiMobileChat', 'uiMobileChrome', 'uiMobileHud', 'uiMobileHudController', 'uiMobileSettings'):
		__import__(name)
ok &= Run(ImportAll)

import uiMobileHud, uiMobileHudController, uiMobileSettings, uiMobileStyle

# ---------------------------------------------------------------- sahte oyun nesneleri
class FakeWindow(ui.Window):
	def __init__(self):
		ui.Window.__init__(self)
	def __del__(self):
		ui.Window.__del__(self)

class FakeTaskBar(ui.Window):
	def __init__(self):
		ui.Window.__init__(self)
		self.tooltipHP = FakeWindow()
		self.tooltipSP = FakeWindow()
		self.tooltipST = FakeWindow()
		self.tooltipEXP = FakeWindow()
		self.refreshed = 0
	def __del__(self):
		ui.Window.__del__(self)
	def RefreshQuickSlot(self):
		self.refreshed += 1
	def OnUseSkill(self, slot, cool):
		pass
	def OnActivateSkill(self, slot):
		pass
	def OnDeactivateSkill(self, slot):
		pass
	def SelectItemQuickSlot(self, i):
		pass
	def AddQuickSlot(self, i):
		pass
	def OverInItem(self, i):
		pass
	def OverOutItem(self):
		pass

class FakeChat(ui.Window):
	def __init__(self):
		ui.Window.__init__(self)
		self.edit = False
	def __del__(self):
		ui.Window.__del__(self)
	def IsEditMode(self):
		return self.edit
	def CloseChat(self):
		self.edit = False
	def Refresh(self):
		pass

class FakeGameButton(object):
	def __init__(self):
		self.gameButtonDict = {}
		for k in ("STATUS", "SKILL", "QUEST", "HELP", "BUILD", "EXIT_OBSERVER"):
			self.gameButtonDict[k] = FakeWindow()

class FakeMiniMap(object):
	def __init__(self):
		self.OpenWindow = FakeWindow()
		self.CloseWindow = FakeWindow()
		self.MiniMapWindow = FakeWindow()
		for n in ("AtlasShowButton", "MiniMapHideButton", "ScaleUpButton", "ScaleDownButton", "MiniMapShowButton"):
			b = FakeWindow()
			b.Show()
			setattr(self, n, b)
	def ToggleAtlasWindow(self): pass
	def HideMiniMap(self): pass
	def ShowMiniMap(self): pass
	def ScaleUp(self): pass
	def ScaleDown(self): pass

class FakeInterface(object):
	def __init__(self):
		self.wndTaskBar = FakeTaskBar()
		self.wndChat = FakeChat()
		self.wndGameButton = FakeGameButton()
		self.wndMiniMap = FakeMiniMap()
		self.wndInventory = FakeWindow()
		self.wndCharacter = FakeWindow()
		self.dlgSystem = FakeWindow()
		self.wndMessenger = FakeWindow()
	def ToggleCharacterWindowStatusPage(self): pass
	def ToggleInventoryWindow(self): self.wndInventory.Show()
	def ToggleMessenger(self): pass
	def ItemShopSendOpen(self): pass
	def ToggleSystemDialog(self): pass
	def OpenAutoHunt(self): pass
	def MakeHyperlinkTooltip(self, h): pass

class FakeGame(object):
	def __init__(self):
		self.affectShower = FakeWindow()
	def CloseTargetBoard(self):
		pass

def HudTest():
	game = FakeGame()
	interface = FakeInterface()
	uiMobileHud.Refresh()
	assert uiMobileHud.IsEnabled()
	hud = uiMobileHudController.MobileHudController(game, interface)
	for i in range(30):
		hud.Update()
	# Cynex: joystick ve saldiri dokunuslari (bolge 2)
	joy = hud.joystick
	(jx, jy) = (60 + 100, 720 - 20 - 100)
	VAL.touches = [(5, jx + 50, jy - 30, 2)]
	for i in range(5):
		joy.OnUpdate()
		hud.attackButton.OnUpdate()
	assert CALLS.get('player.MoveToDirection', 0) > 0, CALLS
	VAL.touches = []
	joy.OnUpdate()
	assert CALLS.get('player.EndKeyWalkingImmediately', 0) > 0
	(ax, ay) = hud.attackButton.GetGlobalPosition()
	VAL.touches = [(7, ax + 10, ay + 10, 2)]
	hud.attackButton.OnUpdate()
	VAL.touches = []
	hud.attackButton.OnUpdate()
	# Cynex: dokunma olaylari (motorun cagirdigi gibi)
	slot = hud.skillArc.slots[0]
	slot.OnTouchTap(0, 0)
	slot.OnTouchLongPress(0, 0)
	hud.skillArc.pageButton.OnTouchTap(0, 0)
	for name in ("character", "inventory", "messenger", "itemshop", "system"):
		hud.menuBar.GetButton(name).OnTouchTap(0, 0)
	hud.menuBar.GetButton("system").OnTouchLongPress(0, 0)
	uiMobileSettings.dialog.rows[0].plus.OnTouchTap(0, 0)
	uiMobileSettings.dialog.rows[3].plus.OnTouchTap(0, 0)
	uiMobileSettings.dialog.rows[3].minus.OnTouchTap(0, 0)
	uiMobileSettings.dialog.cameraReturn.OnTouchTap(0, 0)
	uiMobileSettings.dialog.dynamicJoystick.OnTouchTap(0, 0)
	uiMobileSettings.dialog.closeButton.OnTouchTap(0, 0)
	for name in ("pickup", "mount"):
		hud.utilities.GetButton(name).OnTouchTap(0, 0)
	hud.statusBar.OnTouchTap(0, 0)
	hud.chatToggle.OnTouchTap(0, 0)
	for i in range(20):
		hud.Update()
		hud.chatController.panel.OnUpdate()
	hud.chatController.panel.tabs[2].OnTouchTap(0, 0)
	hud.chatController.panel.field.OnTouchTap(0, 0)
	hud.chatController.panel.Send()
	hud.chatController.panel.collapseButton.OnTouchTap(0, 0)
	for i in range(20):
		hud.Update()
		hud.chatController.panel.OnUpdate()
	interface.wndChat.edit = True
	hud.Update()
	interface.wndTaskBar.RefreshQuickSlot()
	interface.wndTaskBar.OnUseSkill(1, 3.0)
	for b in hud.minimapButtons:
		b.OnTouchTap(0, 0)
	interface.wndMiniMap.MiniMapWindow.OnTouchTap(0, 0)
	hud.Destroy(True)
	assert 'RefreshQuickSlot' not in interface.wndTaskBar.__dict__
	hud = uiMobileHudController.MobileHudController(game, interface)
	hud.Update()
	hud.Destroy(False)
ok &= Run(lambda: (Step('hud'), HudTest()))

import harness as _h
_f = old_open(os.path.join(os.environ.get('TEMP', '.'), 'cynex_loaded_test_hud.txt'), 'w')
_f.write(chr(10).join(sorted(_h.LOADED)))
_f.close()
print 'SONUC', 'TAMAM' if ok else 'HATA', 'TraceError:', len(ERRORS)
