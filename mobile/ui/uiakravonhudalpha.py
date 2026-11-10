# AKRAVON: joystick ve saldiri dugmesi seffafligi (oyuncu ayarlar > kontroller'den secer)
# Dosyada saklanir: "joy=100" / "atk=100"  (yuzde, 20..100; 100 = eski varsayilan gorunum)
FILE = "akravon_hudalpha.cfg"
cache = {}


def _Load():
	if cache:
		return
	cache.update({"joy" : 100, "atk" : 100})
	try:
		for line in open(FILE):
			parts = line.strip().split("=")
			if len(parts) == 2 and parts[0] in cache:
				cache[parts[0]] = max(20, min(100, int(parts[1])))
	except Exception:
		pass


def GetPercent(name):
	_Load()
	return cache.get(name, 100)


def Get(name):
	return GetPercent(name) / 100.0


def SetPercent(name, value):
	_Load()
	cache[name] = max(20, min(100, int(round(value))))
	try:
		out = open(FILE, "w")
		out.write("joy=%d\natk=%d\n" % (cache["joy"], cache["atk"]))
		out.close()
	except Exception:
		pass
