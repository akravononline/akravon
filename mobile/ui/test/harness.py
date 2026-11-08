# -*- coding: ascii -*-
# Cynex: mobil arayuz python sinamasi - motor modulleri sahte, barbaros root + mobil dosyalar gercek
import sys, os, imp, traceback, __builtin__
import _weakref
if not hasattr(_weakref, '_remove_dead_weakref'):
	def _remove_dead_weakref(d, key):
		try:
			if d[key]() is None:
				del d[key]
		except KeyError:
			pass
	_weakref._remove_dead_weakref = _remove_dead_weakref

DATA = os.environ.get('CYNEX_UI_DATA', 'C:/CynexMobilBuild/mobile_ui_data')
B2 = 'C:/CynexMobilBuild/b2pack/root'
LOCALE = 'C:/CynexMobilBuild/b2pack/locale'

# ---------------------------------------------------------------- dosya erisimi (pack gibi)
INDEX = {}
for base in (B2, DATA):
	for root, dirs, files in os.walk(base):
		for f in files:
			rel = os.path.relpath(os.path.join(root, f), base).replace('\\', '/').lower()
			INDEX[rel] = os.path.join(root, f)
for root, dirs, files in os.walk(LOCALE):
	for f in files:
		rel = os.path.relpath(os.path.join(root, f), LOCALE).replace('\\', '/').lower()
		INDEX.setdefault(rel, os.path.join(root, f))

def Key(name):
	k = name.replace('\\', '/').lower()
	if len(k) > 2 and k[1] == ':' and k[2] == '/':
		k = k[3:]
	return k.lstrip('/')

def PackExist(name):
	return Key(name) in INDEX

def PackGet(name):
	return open(INDEX[Key(name)], 'rb').read()

real_open = open
class PackFile(object):
	def __init__(self, data, mode):
		if 'b' not in mode:
			data = data.replace('\r\n', '\n')
		self.data = data
	def read(self, n=-1):
		if n < 0:
			d, self.data = self.data, ''
			return d
		d, self.data = self.data[:n], self.data[n:]
		return d
	def readline(self):
		i = self.data.find('\n')
		return self.read(i + 1 if i >= 0 else -1)
	def readlines(self):
		lines = []
		while self.data:
			lines.append(self.readline())
		return lines
	def __iter__(self):
		return iter(self.readlines())
	def close(self):
		pass
	def __enter__(self):
		return self
	def __exit__(self, *a):
		return False

def PackOpen(name, mode='r', *args):
	if ('r' in mode) and PackExist(name) and not os.path.isabs(name):
		return PackFile(PackGet(name), mode)
	return real_open(name, mode, *args)

__builtin__.open = PackOpen
__builtin__.old_open = real_open
__builtin__.new_open = PackOpen

def ExecFile(fileName, dict):
	src = PackOpen(fileName, 'r').read()
	code = compile(src, fileName, 'exec')
	exec code in dict
__builtin__.execfile = ExecFile

# ---------------------------------------------------------------- sahte motor
class Zero(int):
	def __new__(cls, name=''):
		o = int.__new__(cls, 0)
		o._n = name
		return o
	def __iter__(self):
		return iter((0, 0, 0, 0)[:2])
	def __getitem__(self, i):
		return 0
	def __call__(self, *args, **kw):
		return CALL(self._n, args)
	def __repr__(self):
		return '<%s>' % self._n

class Values(object):
	def __init__(self):
		self.handles = 1000
		self.time = 0.0
		self.touches = []
VAL = Values()
CALLS = {}
LOADED = set()

WIN = {}
def W(h):
	return WIN.setdefault(h, {'x': 0, 'y': 0, 'w': 0, 'h': 0, 'p': 0, 'show': False})

def CALL(name, args):
	CALLS[name] = CALLS.get(name, 0) + 1
	short = name.split('.', 1)[1]
	mod = name.split('.', 1)[0]
	if mod == 'wndMgr' and args:
		h = args[0]
		for a in args[1:]:
			if isinstance(a, str) and a.lower().endswith(('.png', '.tga', '.sub', '.dds', '.jpg')):
				LOADED.add(a)
		if short == 'SetText' and len(args) > 1:
			W(h)['text'] = args[1]; return None
		if short == 'GetText':
			return W(h).get('text', '')
		if short == 'SetWindowSize':
			W(h)['w'], W(h)['h'] = int(args[1]), int(args[2]); return None
		if short == 'SetWindowPosition':
			W(h)['x'], W(h)['y'] = int(args[1]), int(args[2]); return None
		if short == 'SetParent':
			W(h)['p'] = args[1]; return None
		if short == 'Show':
			W(h)['show'] = True; return None
		if short == 'Hide':
			W(h)['show'] = False; return None
		if short == 'IsShow':
			return W(h)['show']
		if short == 'GetWindowWidth':
			return W(h)['w']
		if short == 'GetWindowHeight':
			return W(h)['h']
		if short == 'GetWindowLocalPosition':
			return (W(h)['x'], W(h)['y'])
		if short == 'GetWindowGlobalPosition':
			x = y = 0
			while h:
				x += W(h)['x']; y += W(h)['y']; h = W(h)['p']
			return (x, y)
	if short.startswith('Register'):
		VAL.handles += 1
		return VAL.handles
	if short in ('GetScreenWidth',):
		return 1280
	if short in ('GetScreenHeight',):
		return 720
	if name == 'app.GetTime' or name == 'app.GetGlobalTime':
		VAL.time += 0.05
		return VAL.time
	if name == 'app.GetGlobalTimeStamp':
		return 1000
	if name == 'app.GetTouchCount':
		return len(VAL.touches)
	if name == 'app.GetTouchPoint':
		i = args[0]
		return VAL.touches[i][:3] if i < len(VAL.touches) else None
	if name == 'app.GetTouchPointEx':
		i = args[0]
		return VAL.touches[i] if i < len(VAL.touches) else None
	if name == 'app.IsMobile':
		return 1
	if name == 'systemSetting.IsMobileHud':
		return 1
	if name == 'systemSetting.GetJoystickDeadZone':
		return 0.15
	if name in ('systemSetting.GetCameraReturnPitch',):
		return 14.0
	if name in ('systemSetting.GetCameraDragSens', 'systemSetting.GetPinchZoomSens'):
		return 1.0
	if name == 'systemSetting.GetTouchSizeLimits':
		return (80.0, 560.0)
	if name == 'player.GetName':
		return 'Cynex'
	if name == 'player.GetStatus':
		return 50
	if name == 'player.GetLocalQuickSlot':
		return (args[0] % 3, 1)
	if name == 'net.GetServerInfo':
		return 'Cynex, CH1'
	if name == 'net.GetAccountCharacterSlotDataString':
		return 'Karakter'
	if name == 'net.GetAccountCharacterSlotDataInteger':
		return 1
	if name == 'net.GetEmpireID':
		return 1
	if name == 'app.GetCamera':
		return (1550.0, 27.0, 0.0, 100.0)
	if name == 'player.GetSkillCoolTime' or name == 'wndMgr.GetSlotCoolTime':
		return (1.0, 0.5)
	if short.startswith('GetText') and short.endswith('Size') or short in ('GetTextSize',):
		return (40, 12)
	if short in ('GetWindowGlobalPosition', 'GetWindowLocalPosition', 'GetMousePosition', 'GetWindowRect'):
		return (0, 0)
	if short in ('GetText', 'GetName'):
		return ''
	if short == 'pack.Exist' or name == 'pack.Exist':
		return PackExist(args[0])
	return Zero(name + '()')

class Module(object):
	def __init__(self, name):
		self.__name__ = name
	def __getattr__(self, k):
		if k.startswith('__') and k.endswith('__') and k[2:-2].lower() == k[2:-2]:
			raise AttributeError(k)
		v = Zero('%s.%s' % (self.__name__, k))
		object.__setattr__(self, k, v)
		return v

ENGINE = ('app', 'wndMgr', 'grp', 'grpImage', 'grpText', 'player', 'net', 'chat', 'systemSetting', 'skill', 'chr',
	'chrmgr', 'dbg', 'ime', 'snd', 'item', 'guild', 'quest', 'messenger', 'background', 'textTail', 'miniMap', 'effect',
	'event', 'exchange', 'safebox', 'shop', 'nonplayer', 'fly', 'profiler', 'ServerStateChecker', 'udp',
	'cube_renewal', 'ikashop', 'switchbot', 'acce', 'renderTarget', 'changelook', 'cpsm', 'privateItemShop',
	'eventMgr', 'wndText', 'wndSlot', 'grpThing', 'rootlib', '_winreg', 'binascii')
for name in ENGINE:
	sys.modules[name] = Module(name)

pack = Module('pack')
pack.Exist = PackExist
pack.Get = PackGet
sys.modules['pack'] = pack

dbgmod = sys.modules['dbg']
ERRORS = []
def TraceError(msg):
	ERRORS.append(msg)
	print 'TraceError:', msg
dbgmod.TraceError = TraceError
dbgmod.Trace = lambda *a: None
dbgmod.Tracen = lambda *a: None
dbgmod.LogBox = lambda *a: None

# Cynex: sozluk yerlesimleri icin dogal sabitler
app = sys.modules['app']
for k in ('IS_MOBILE', 'TOUCH_API_VERSION'):
	pass
app.TOUCH_API_VERSION = 2
app.IS_MOBILE = 1
app.loggined = False
app.__AUTO_HUNT__ = 0
net = sys.modules['net']
net.EMPIRE_A, net.EMPIRE_B, net.EMPIRE_C = 1, 2, 3
player = sys.modules['player']
player.SLOT_TYPE_NONE, player.SLOT_TYPE_INVENTORY, player.SLOT_TYPE_SKILL, player.SLOT_TYPE_EMOTION, player.SLOT_TYPE_QUICK_SLOT = 0, 1, 2, 3, 4
player.METIN_SOCKET_MAX_NUM = 3
chat = sys.modules['chat']
chat.CHAT_TYPE_TALKING, chat.CHAT_TYPE_INFO, chat.CHAT_TYPE_NOTICE, chat.CHAT_TYPE_PARTY, chat.CHAT_TYPE_GUILD = 0, 1, 2, 3, 4
chat.CHAT_TYPE_COMMAND, chat.CHAT_TYPE_SHOUT, chat.CHAT_TYPE_WHISPER = 5, 6, 7

# ---------------------------------------------------------------- import (system.py gibi)
old_import = __builtin__.__import__
def HybridImport(name, globals=None, locals=None, fromlist=None, level=-1):
	filename = name + '.py'
	if PackExist(filename) and not name in ('ui_harness',):
		if name in sys.modules:
			return sys.modules[name]
		src = PackOpen(filename, 'r').read()
		code = compile(src, filename, 'exec')
		module = imp.new_module(name)
		sys.modules[name] = module
		try:
			exec code in module.__dict__
		except:
			del sys.modules[name]
			raise
		return module
	return old_import(name, globals, locals, fromlist)
__builtin__.__import__ = HybridImport
__builtin__.__DEBUG__ = 0
__builtin__.__USE_CYTHON__ = 0
__builtin__.__USE_EXTRA_CYTHON__ = 0
__builtin__.__COMMAND_LINE__ = ''

__builtin__.TRUE = 1
__builtin__.FALSE = 0
__builtin__.NEW_PACK_FILE = True
__builtin__.old_len = len
