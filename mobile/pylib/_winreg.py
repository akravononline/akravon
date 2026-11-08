# Cynex: android'de kayit defteri yok - _winreg ayni arayuzle data/registry.json'a yazar
import json
import os
import __builtin__

if not hasattr(__builtin__, "WindowsError"):
	class WindowsError(OSError):
		pass
	__builtin__.WindowsError = WindowsError
else:
	WindowsError = __builtin__.WindowsError

error = WindowsError

HKEY_CLASSES_ROOT = 0x80000000
HKEY_CURRENT_USER = 0x80000001
HKEY_LOCAL_MACHINE = 0x80000002
HKEY_USERS = 0x80000003
HKEY_CURRENT_CONFIG = 0x80000005

KEY_QUERY_VALUE = 0x0001
KEY_SET_VALUE = 0x0002
KEY_CREATE_SUB_KEY = 0x0004
KEY_ENUMERATE_SUB_KEYS = 0x0008
KEY_READ = 0x20019
KEY_WRITE = 0x20006
KEY_EXECUTE = 0x20019
KEY_ALL_ACCESS = 0xF003F
KEY_WOW64_64KEY = 0x0100
KEY_WOW64_32KEY = 0x0200

REG_NONE = 0
REG_SZ = 1
REG_EXPAND_SZ = 2
REG_BINARY = 3
REG_DWORD = 4
REG_MULTI_SZ = 7
REG_QWORD = 11

_FILE = "data/registry.json"
_data = None


def _load():
	global _data
	if _data is None:
		try:
			f = open(_FILE, "r")
			_data = json.load(f)
			f.close()
		except Exception:
			_data = {}
	return _data


def _save():
	try:
		f = open(_FILE + ".tmp", "w")
		json.dump(_load(), f)
		f.close()
		os.rename(_FILE + ".tmp", _FILE)
	except Exception:
		pass


class HKEYType(object):
	def __init__(self, path):
		self.path = path
		self.handle = id(self)

	def Close(self):
		pass

	def Detach(self):
		return self.handle

	def __enter__(self):
		return self

	def __exit__(self, *args):
		return False

	def __int__(self):
		return self.handle


def _path(key, sub):
	base = key.path if isinstance(key, HKEYType) else str(key)
	sub = (sub or "").replace("/", "\\").strip("\\").lower()
	return base + ("\\" + sub if sub else "")


def CreateKey(key, sub_key):
	p = _path(key, sub_key)
	d = _load()
	if p not in d:
		d[p] = {}
		_save()
	return HKEYType(p)


def CreateKeyEx(key, sub_key, reserved=0, access=KEY_WRITE):
	return CreateKey(key, sub_key)


def OpenKey(key, sub_key, reserved=0, access=KEY_READ):
	p = _path(key, sub_key)
	if p not in _load():
		raise WindowsError(2, "The system cannot find the file specified")
	return HKEYType(p)


OpenKeyEx = OpenKey


def CloseKey(hkey):
	pass


def QueryValueEx(key, value_name):
	values = _load().get(key.path)
	name = (value_name or "").lower()
	if values is None or name not in values:
		raise WindowsError(2, "The system cannot find the file specified")
	v = values[name]
	return v[0], v[1]


def QueryValue(key, sub_key):
	return QueryValueEx(OpenKey(key, sub_key), "")[0]


def SetValueEx(key, value_name, reserved, type, value):
	d = _load()
	d.setdefault(key.path, {})[(value_name or "").lower()] = [value, type]
	_save()


def SetValue(key, sub_key, type, value):
	SetValueEx(CreateKey(key, sub_key), "", 0, type, value)


def DeleteValue(key, value_name):
	values = _load().get(key.path)
	name = (value_name or "").lower()
	if values is None or name not in values:
		raise WindowsError(2, "The system cannot find the file specified")
	del values[name]
	_save()


def DeleteKey(key, sub_key):
	p = _path(key, sub_key)
	d = _load()
	if p not in d:
		raise WindowsError(2, "The system cannot find the file specified")
	del d[p]
	_save()


def EnumValue(key, index):
	values = _load().get(key.path, {})
	names = sorted(values.keys())
	if index >= len(names):
		raise WindowsError(259, "No more data is available")
	v = values[names[index]]
	return names[index], v[0], v[1]


def EnumKey(key, index):
	prefix = key.path + "\\"
	subs = sorted(set(p[len(prefix):].split("\\")[0] for p in _load().keys() if p.startswith(prefix)))
	if index >= len(subs):
		raise WindowsError(259, "No more data is available")
	return subs[index]


def ConnectRegistry(computer_name, key):
	return HKEYType(str(key))


def FlushKey(key):
	_save()
