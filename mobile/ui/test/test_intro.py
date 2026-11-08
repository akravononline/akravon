# Cynex: py2 sinamasi - py2host.exe <pc istemci python27.dll> <python27 kok> -B test_intro.py
import sys, os, traceback
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import harness
from harness import VAL, CALLS, ERRORS

app = sys.modules['app']
app.GetLocalePath = lambda: 'locale/tr'
app.GetLocaleServiceName = lambda: ''
app.GetLocaleName = lambda: 'tr'
app.GetDefaultCodePage = lambda: 1254
app.GetRandom = lambda a, b: a
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

import ui

class Popup(object):
	def Open(self, *a): pass
	def Close(self): pass

class FakeStream(object):
	def __init__(self):
		self.popupWindow = Popup()
		self.isAutoLogin = 0
		self.isAutoSelect = 0
		self.addr = ''
		self.slot = 0
		self.connected = []
	def GetConnectAddr(self): return self.addr
	def SetConnectInfo(self, *a): self.addr = a[0]
	def SetLoginInfo(self, *a): self.connected.append(a)
	def Connect(self): pass
	def SetPhaseWindow(self, w): pass
	def GetCharacterSlot(self): return self.slot
	def SetCharacterSlot(self, s): self.slot = s

ok = True

def LoginTest():
	import uiMobileLogin
	stream = FakeStream()
	w = uiMobileLogin.MobileLoginWindow(stream)
	w.Open()
	assert w.mobileLayoutReady
	for i in range(5):
		w.OnUpdate()
	w.channelButton.OnTouchTap(0, 0)
	w.OnUpdate()
	if w.picker.chips:
		w.picker.chips[0].OnTouchTap(0, 0)
	for i in range(3):
		w.OnUpdate()
	w.GetChild2("ID_EditLine").SetText("test")
	w.GetChild2("Password_EditLine").SetText("pass")
	w.GetChild2("pin_EditLine").SetText("1234")
	w.rememberToggle.OnTouchTap(0, 0)
	w.loginButton.OnTouchTap(0, 0)
	for row in w.accountPanel.rows:
		if row.IsShow():
			row.OnTouchTap(0, 0)
	w.OnAddAccount()
	w.OnDeleteAccount(1)
	print 'login connect:', stream.connected, 'addr:', stream.addr
	print 'files:', os.listdir('.')
	w.exitButton.OnTouchTap(0, 0)
	w.Close()
ok &= Run(lambda: (Step('login'), LoginTest()))

def SelectTest():
	import uiMobileSelect
	w = uiMobileSelect.MobileSelectCharacterWindow(FakeStream())
	w.Open()
	assert w.mobileReady, 'select layout'
	for i in range(3):
		w.OnUpdate()
	w.cards[1].OnTouchTap(0, 0)
	w.cards[1].OnTouchTap(0, 0)
	w.RotateSelected(10)
	w.Close()
ok &= Run(lambda: (Step('select'), SelectTest()))

def CreateTest():
	import uiMobileCreate
	w = uiMobileCreate.MobileCreateCharacterWindow(FakeStream())
	w.Open()
	assert w.mobileReady, 'create layout'
	for i in range(3):
		w.OnUpdate()
	w.tiles[2].OnTouchTap(0, 0)
	w.genderControl.items[1].OnTouchTap(0, 0)
	w.shapeControl.items[1].OnTouchTap(0, 0)
	w.OnUpdate()
	w.Close()
ok &= Run(lambda: (Step('create'), CreateTest()))

def EmpireTest():
	import uiMobileEmpire
	w = uiMobileEmpire.MobileSelectEmpireWindow(FakeStream())
	w.Open()
	assert w.mobileReady, 'empire layout'
	w.OnUpdate()
	w.mobileCards[-1].OnTouchTap(0, 0)
	w.OnUpdate()
	w.Close()
ok &= Run(lambda: (Step('empire'), EmpireTest()))

def NetworkTest():
	import networkModule
	s = networkModule.MainStream()
	w = s._MainStream__CreatePhaseWindow("uiMobileLogin", "MobileLoginWindow", None)
	assert w.__class__.__name__ == 'MobileLoginWindow', w
ok &= Run(lambda: (Step('network'), NetworkTest()))

def GameImport():
	import game
	assert hasattr(game.GameWindow, 'RefreshTouchLayout')
ok &= Run(lambda: (Step('game import'), GameImport()))

import harness as _h
_f = old_open(os.path.join(os.environ.get('TEMP', '.'), 'cynex_loaded_test_intro.txt'), 'w')
_f.write(chr(10).join(sorted(_h.LOADED)))
_f.close()
print 'SONUC', 'TAMAM' if ok else 'HATA', 'TraceError:', len(ERRORS)
for e in ERRORS:
	print '  ', e[:300]
