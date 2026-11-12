from __future__ import division
import app
import systemSetting
import wndMgr
import ui
import uiMobileStyle
import uiAkravon
import uiMobileHud
import uiAkravonHudAlpha

# AKRAVON: oyun secenekleri - sekmeli tek pencere (Grafik / Oyun / Ses / Kontroller); ayarlar menusu ile ayni dil
PANEL_WIDTH = 800
SIDE_WIDTH = 170		# AKRAVON: sekmeler solda (karakter penceresiyle ayni dil)
SIDE_TAB_WIDTH = 142
SIDE_TAB_STEP = 58
PADDING_X = 22
TOP = 18
TITLE_HEIGHT = 36
TAB_TOP = TOP + TITLE_HEIGHT + 4
TAB_HEIGHT = 44
TAB_GAP = 4
CONTENT_TOP = TOP + TITLE_HEIGHT + 14
ROW_HEIGHT = 50
ROW_GAP = 4
CLOSE_GAP = 12
BOTTOM = 20
DIM_ALPHA = 150
CONTROL_RIGHT = 12
STEP_SIZE = 40
STEP_W = 44
VALUE_WIDTH = 110
CHOICE_HEIGHT = 40
CHOICE_GAP = 4
FONT_TITLE = "Marcellus:22"
FONT_TAB = "Marcellus:20"
FONT_ROW = "Lato:19"
FONT_VALUE = "LatoBd:18"
FONT_CHOICE = "LatoBd:17"
FONT_SIGN = "AlegreyaSansBold:28"
TAB_COLOR = uiMobileStyle.Color(176, 160, 128)
TAB_ACTIVE_COLOR = uiMobileStyle.Color(236, 206, 140)
SAVE_KEY_SOUND = "SOUND_VOLUME"


def Call(name, default, *args):
	function = getattr(systemSetting, name, None)
	if function is None:
		return default
	try:
		return function(*args)
	except Exception:
		return default


def Has(name):
	return getattr(systemSetting, name, None) is not None


# ---------------------------------------------------------------- ayar uygulayicilar

def SetFog(value):
	Call("SetFogMode", None, bool(value))
	try:
		import background
		background.SetEnvironmentFog(bool(value))
	except Exception:
		pass


def SetNight(value):
	Call("SetNightMode", None, bool(value))
	try:
		import background
		import constInfo
		if value:
			background.RegisterEnvironmentData(1, constInfo.ENVIRONMENT_NIGHT)
			background.SetEnvironmentData(1)
		else:
			background.SetEnvironmentData(0)
	except Exception:
		pass


def SetDogMode(value):
	Call("SetPolyDogMode", None, bool(value))
	try:
		import background
		background.SetPolyAllMobDog(bool(value))
	except Exception:
		pass


CAMERA_FILE = "akravon_kamera.cfg"
CAMERA_NAMES = ("Çok Yakın", "Yakın", "Normal", "Uzak", "Çok Uzak")
CAMERA_DISTANCES = (900.0, 1300.0, 1800.0, 2500.0, 3200.0)	# kademe -> kamera mesafesi (en yakin 200, en uzak 3500)
CAMERA_DEFAULT = 3	# eski davranis: 2500 ust sinir
CAMERA_LAST = 4


def GetCameraLevel():
	try:
		return max(0, min(CAMERA_LAST, int(open(CAMERA_FILE).read().strip())))
	except Exception:
		return CAMERA_DEFAULT


def ApplyCameraLevel(level, move = True):
	# parmakla uzaklastirma siniri + (move ise) kamerayi o mesafeye getirir
	level = max(0, min(CAMERA_LAST, int(level)))
	distance = CAMERA_DISTANCES[level]
	try:
		app.SetCameraMaxDistance(distance)
	except Exception:
		pass
	if move:
		try:
			(oldDistance, pitch, rotation, height) = app.GetCamera()
			app.SetCamera(distance, pitch, rotation, height)
		except Exception:
			pass


def SetCameraLevel(value):
	level = max(0, min(CAMERA_LAST, int(round(value))))
	try:
		open(CAMERA_FILE, "w").write(str(level))
	except Exception:
		pass
	ApplyCameraLevel(level)


def GetFarCamera():
	try:
		import constInfo
		return bool(constInfo.GET_CAMERA_MAX_DISTANCE_INDEX())
	except Exception:
		return False


def SetFarCamera(value):
	try:
		import constInfo
		constInfo.SET_CAMERA_MAX_DISTANCE_INDEX(1 if value else 0)
	except Exception:
		pass


def GetMusic():
	return float(Call("GetMusicVolume", 1.0))


def SetMusic(value):
	Call("SetMusicVolume", None, value)
	try:
		import snd
		import net
		snd.SetMusicVolume(value * net.GetFieldMusicVolume())
	except Exception:
		pass


def GetSound():
	try:
		return float(uiMobileHud.LoadConfig().get(SAVE_KEY_SOUND, Call("GetSoundVolume", 1)))
	except ValueError:
		return 1.0


def SetSound(value):
	Call("SetSoundVolumef", None, value)
	try:
		import snd
		snd.SetSoundVolumef(value)
	except Exception:
		pass
	uiMobileHud.LoadConfig()[SAVE_KEY_SOUND] = "%.2f" % value
	uiMobileHud.SaveConfig()


def SetDynamicJoystick(value):
	if value != uiMobileHud.IsDynamicJoystick():
		uiMobileHud.SetDynamicJoystick(value)
		uiMobileHud.RequestRefresh()


def SetSystemFloat(setter, rebuild):
	def Apply(value):
		Call(setter, None, value)
		if rebuild:
			uiMobileHud.RequestRefresh()
	return Apply


def SetHudAlpha(name):
	def Apply(value):
		uiAkravonHudAlpha.SetPercent(name, value)
		uiMobileHud.RequestRefresh()
	return Apply


def Percent(value):
	return "%%%d" % int(round(value * 100))


# satir tanimlari: ("toggle", baslik, okuyucu, yazici) / ("choice", baslik, okuyucu, yazici, [(etiket, deger)])
# ("step", baslik, okuyucu, yazici, alt, ust, adim, bicim)
# AKRAVON: grafik kalitesi hazir ayarlari: (golge, filtre, gorus, doku)
QUALITY_NAMES = ("Çok Düşük", "Düşük", "Orta", "Yüksek", "Çok Yüksek")
# (golge, filtre, gorus, doku, fps): her kademe bir oncekinden gercekten farkli
QUALITY_PRESETS = ((1, 0, 2, 0, 30), (1, 0, 1, 1, 30), (3, 1, 1, 1, 30), (3, 1, 0, 2, 60), (5, 1, 0, 2, 60))
QUALITY_FILE = "akravon_grafik.cfg"
AUTO_FILE = "akravon_grafik_oto.cfg"
CONFIG_FILE = "metin2.cfg"
DESIGN_HEIGHT = 900


def GetQuality():
	try:
		return max(0, min(4, int(open(QUALITY_FILE).read().strip())))
	except Exception:
		return 3


def IsAuto():
	try:
		return open(AUTO_FILE).read().strip() != "0"
	except Exception:
		return True	# ilk acilista otomatik


def SetAuto(value):
	try:
		open(AUTO_FILE, "w").write("1" if value else "0")
	except Exception:
		pass
	if value:
		ApplyQuality(AutoQuality())


def AutoQuality():
	# cihaza gore otomatik kademe: ekran karti ailesi ve numarasi, ekran buyuklugu
	name = str(Call("GetGpuName", "")).lower()
	digits = "".join(ch if ch.isdigit() else " " for ch in name).split()
	number = int(digits[0]) if digits else 0
	height = Call("GetDisplaySize", (0, 0))[1]
	level = 2
	if "adreno" in name:
		level = 4 if number >= 730 else (3 if number >= 650 else (2 if number >= 618 else (1 if number >= 530 else 0)))
	elif "mali" in name:
		if "-g" in name:
			level = 4 if number >= 710 else (3 if number >= 77 else (2 if number >= 68 else 1))
		else:
			level = 0
	elif "xclipse" in name or "apple" in name:
		level = 4
	elif "powervr" in name:
		level = 1
	elif "nvidia" in name or "geforce" in name or "radeon" in name or "intel" in name:
		level = 4	# masaustu / emulator
	if height > 1400:
		level = max(0, level - 1)	# cok yuksek cozunurluk = daha fazla yuk
	return level


def SetQuality(value):
	try:
		open(AUTO_FILE, "w").write("0")	# elle secim otomatigi kapatir
	except Exception:
		pass
	ApplyQuality(value)


def ManualChange():
	# tek tek ayar degisince otomatik kapanir (oyuncunun secimi ezilmesin)
	try:
		open(AUTO_FILE, "w").write("0")
	except Exception:
		pass


def ApplyFixedSettings():
	# AKRAVON: gorus acisi 30 sabit, sis yok, kopek modu yok, canavar seviyesi hep gorunur
	try:
		Call("SetFOVLevel", None, 30.0)
		SetFog(False)
		Call("SetPolyDogMode", None, False)
		Call("SetShowMobLevel", None, True)
	except Exception:
		pass


def ApplyStartupQuality():
	ApplyFixedSettings()
	# oyun her acildiginda otomatik aciksa cihaza gore secilir; degilse kayitli kademe
	if IsAuto():
		ApplyQuality(AutoQuality())
	else:
		ApplyQuality(GetQuality())


def ApplyQuality(value):
	value = max(0, min(4, int(round(value))))
	try:
		open(QUALITY_FILE, "w").write(str(value))
	except Exception:
		pass
	(shadow, postFilter, viewRange, texture, fps) = QUALITY_PRESETS[value]
	Call("SetShadowLevel", None, shadow)
	Call("SetPostFilter", None, postFilter)
	Call("SetViewRange", None, viewRange)
	Call("SetTextureQuality", None, texture)
	Call("SetFpsLimit", None, fps)
	if value == 0:
		Call("SetEffectScale", None, 0.4 * 3500.0)
		SetFog(False)


def _ReadConfigSize():
	try:
		(w, h) = (0, 0)
		for line in open(CONFIG_FILE):
			parts = line.split()
			if len(parts) >= 2 and parts[0] == "WIDTH":
				w = int(parts[1])
			elif len(parts) >= 2 and parts[0] == "HEIGHT":
				h = int(parts[1])
		return (w, h)
	except Exception:
		return (0, 0)


def GetResolution():
	# 100 = ekranin kendi cozunurlugu (keskin), 75 = %75 (daha hizli)
	(dw, dh) = Call("GetDisplaySize", (0, 0))
	(w, h) = _ReadConfigSize()
	if dh <= 0 or h <= 0:
		return 100
	return 100 if h >= min(dh, DESIGN_HEIGHT) - 2 else 75


def SetResolution(value):
	# metin2.cfg WIDTH/HEIGHT yazilir - oyun yeniden acilinca uygulanir
	(dw, dh) = Call("GetDisplaySize", (0, 0))
	if dw <= 0 or dh <= 0:
		return
	if dh > DESIGN_HEIGHT:	# arayuz mantiksal 900 piksel yukseklikte; cizim yerel cozunurlukte kalir
		(dw, dh) = (((dw * DESIGN_HEIGHT // dh) + 1) // 2 * 2, DESIGN_HEIGHT)
	if value < 100:
		(dw, dh) = (int(dw * 0.75) // 2 * 2, int(dh * 0.75) // 2 * 2)
	dw, dh = max(dw, 1280), max(dh, 720)
	try:
		lines = open(CONFIG_FILE).read().splitlines()
		out = []
		for line in lines:
			parts = line.split()
			if parts and parts[0] == "WIDTH":
				line = "WIDTH\t\t\t\t\t\t%d" % dw
			elif parts and parts[0] == "HEIGHT":
				line = "HEIGHT\t\t\t\t\t\t%d" % dh
			if parts and parts[0] == "AKRAVON_RES":
				continue
			out.append(line)
		out.append("AKRAVON_RES\t\t\t\t\t1")	# AKRAVON: kullanici secimi - motor otomatik boyutu kullanmaz
		open(CONFIG_FILE, "w").write("\n".join(out) + "\n")
	except Exception:
		pass
	try:
		import chat
		chat.AppendChat(chat.CHAT_TYPE_INFO, "Çözünürlük oyun yeniden açılınca uygulanır.")
	except Exception:
		pass


def GraphicsRows():
	rows = []
	rows.append(("toggle", "Otomatik Ayar (cihaza göre)", IsAuto, SetAuto))
	rows.append(("step", "Grafik Kalitesi", GetQuality, SetQuality, 0.0, 4.0, 1.0, lambda v: QUALITY_NAMES[max(0, min(4, int(round(v))))]))
	if Has("SetFpsLimit"):
		rows.append(("choice", "FPS", lambda: int(Call("GetFpsLimit", 60)), lambda v: Call("SetFpsLimit", None, v), (("30", 30), ("60", 60))))
	if Has("GetDisplaySize"):
		rows.append(("choice", "Çözünürlük", GetResolution, SetResolution, (("%75", 75), ("Tam", 100))))
	if Has("SetTextureQuality"):
		rows.append(("choice", "Doku Kalitesi", lambda: int(Call("GetTextureQuality", 2)), lambda v: Call("SetTextureQuality", None, v), (("Düşük", 0), ("Orta", 1), ("Yüksek", 2))))
	if Has("SetShadowLevel"):
		rows.append(("choice", "Gölge Kalitesi", lambda: int(Call("GetShadowLevel", 3)), lambda v: Call("SetShadowLevel", None, v), (("Düşük", 1), ("Orta", 3), ("Yüksek", 5))))
	if Has("SetPostFilter"):
		rows.append(("choice", "Görüntü Filtresi", lambda: int(Call("GetPostFilter", 1)), lambda v: Call("SetPostFilter", None, v), (("Kapalı", 0), ("Doğal", 1), ("Canlı", 2))))
	if Has("SetViewRange"):
		rows.append(("choice", "Görüş Mesafesi", lambda: int(Call("GetViewRange", 0)), lambda v: Call("SetViewRange", None, v), (("Kısa", 2), ("Orta", 1), ("Uzak", 0))))
	if Has("SetEffectScale"):
		rows.append(("step", "Efekt Yoğunluğu", lambda: float(Call("GetEffectScale", 3500.0)) / 3500.0, lambda v: Call("SetEffectScale", None, v * 3500.0), 0.0, 1.0, 0.1, Percent))
	return rows


def GameRows():
	rows = []
	rows.append(("step", "Kamera Mesafesi", GetCameraLevel, SetCameraLevel, 0.0, 4.0, 1.0, lambda v: CAMERA_NAMES[max(0, min(4, int(round(v))))]))
	rows.append(("toggle", "İsimleri Her Zaman Göster", lambda: bool(Call("IsAlwaysShowName", False)), lambda v: Call("SetAlwaysShowNameFlag", None, bool(v))))
	rows.append(("toggle", "Hasarı Göster", lambda: bool(Call("IsShowDamage", True)), lambda v: Call("SetShowDamageFlag", None, bool(v))))
	rows.append(("toggle", "Pazar Yazıları", lambda: bool(Call("IsShowSalesText", True)), lambda v: Call("SetShowSalesTextFlag", None, bool(v))))
	if Has("SetHidePets"):
		rows.append(("toggle", "Evcil Hayvanları Gizle", lambda: bool(Call("IsHidePets", False)), lambda v: Call("SetHidePets", None, bool(v))))
		rows.append(("toggle", "Binekleri Gizle", lambda: bool(Call("IsHideMounts", False)), lambda v: Call("SetHideMounts", None, bool(v))))
	return rows


def SoundRows():
	return [
		("step", "Müzik", GetMusic, SetMusic, 0.0, 1.0, 0.1, Percent),
		("step", "Efekt Sesi", GetSound, SetSound, 0.0, 1.0, 0.1, Percent),
	]


def OpenHudEditor():
	# AKRAVON: joystick, saldiri halkasi ve yardimci dugmeler icin HUD duzenleme modu
	import uiMobileHudEdit
	Hide()
	uiMobileHudEdit.Open()


def ControlRows():
	return [
		("step", "Kamera Eğimi", lambda: float(Call("GetCameraReturnPitch", 15.0)), SetSystemFloat("SetCameraReturnPitch", False), 0.0, 45.0, 1.0, lambda v: "%d" % int(round(v))),
		("step", "Kamera Hızı", lambda: float(Call("GetCameraDragSens", 1.0)), SetSystemFloat("SetCameraDragSens", False), 0.25, 3.0, 0.25, lambda v: "x%.2f" % v),
		("step", "Yakınlaştırma Hızı", lambda: float(Call("GetPinchZoomSens", 1.0)), SetSystemFloat("SetPinchZoomSens", False), 0.25, 3.0, 0.25, lambda v: "x%.2f" % v),
		("step", "Joystick Boyu", lambda: float(Call("GetJoystickSize", 200.0)), SetSystemFloat("SetJoystickSize", True), 120.0, 320.0, 20.0, lambda v: "%d" % int(round(v))),
		("step", "Saldırı Düğmesi", lambda: float(Call("GetAttackButtonSize", 140.0)), SetSystemFloat("SetAttackButtonSize", True), 100.0, 320.0, 20.0, lambda v: "%d" % int(round(v))),
		("step", "Joystick Şeffaflığı", lambda: float(uiAkravonHudAlpha.GetPercent("joy")), SetHudAlpha("joy"), 20.0, 100.0, 10.0, lambda v: "%%%d" % int(round(v))),
		("step", "Saldırı Şeffaflığı", lambda: float(uiAkravonHudAlpha.GetPercent("atk")), SetHudAlpha("atk"), 20.0, 100.0, 10.0, lambda v: "%%%d" % int(round(v))),
		("toggle", "Kamera Eğime Dönsün", lambda: bool(Call("IsCameraReturn", 1)), lambda v: Call("SetCameraReturn", None, 1 if v else 0)),
		("toggle", "Joystick Dokunulan Yere Gelsin", uiMobileHud.IsDynamicJoystick, SetDynamicJoystick),
		("action", "HUD Düzenle (taşı / boyutlandır)", None, OpenHudEditor, "Düzenle"),
	]


TABS = (("Grafik", GraphicsRows), ("Oyun", GameRows), ("Ses", SoundRows), ("Kontroller", ControlRows))


# ---------------------------------------------------------------- bilesenler

BUTTON_IMAGE = "d:/ymir work/ui/akravon_legacy/sb_btn230_%d.png"
BUTTON_SIZE = (230.0, 44.0)
BUTTON_TEXT = uiMobileStyle.Color(250, 236, 210)
BUTTON_TEXT_ACTIVE = uiMobileStyle.Color(255, 214, 120)


class SkinButton(uiMobileStyle.Pressable):
	# AKRAVON: diger pencerelerle ayni yatay depo dugmesi + Marcellus buyuk harf
	# (iki gorsel onceden yuklenir, basis sirasinda gorsel yuklenmez: dokunma kaybolmasin)
	def __init__(self, width, height, text, font):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.skins = []
		for index in (1, 2):
			skin = ui.ExpandedImageBox()
			skin.SetParent(self)
			skin.AddFlag("not_pick")
			skin.LoadImage(BUTTON_IMAGE % index)
			skin.SetScale(width / BUTTON_SIZE[0], height / BUTTON_SIZE[1])
			skin.SetPosition(0, 0)
			self.skins.append(skin)
		self.skins[0].Show()
		size = int(font.split(":")[1])
		self.label = uiMobileStyle.CreateText(self, uiMobileStyle.TrUpper(text), width // 2, max(0, (height - size) // 2 - 1), BUTTON_TEXT, font, "center")
		self.selected = False

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.skins = []
		self.label = None

	def SetSelected(self, selected):
		self.selected = selected
		self.RefreshVisual()

	def RefreshVisual(self):
		if not getattr(self, "skins", None):
			return
		if self.selected:
			self.skins[0].Hide()
			self.skins[1].Show()
		else:
			self.skins[1].Hide()
			self.skins[0].Show()
		self.label.SetPackedFontColor(BUTTON_TEXT_ACTIVE if self.selected else BUTTON_TEXT)


class SmallBox(uiMobileStyle.Pressable):
	# kucuk kutu dugme (+ / - / secenek): koyu zemin + ince bronz cizgi, secili / basili halde parlar
	def __init__(self, width, height, text, font):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.box = uiAkravon.Box()
		self.box.SetParent(self)
		self.box.SetSize(width, height)
		self.box.Show()
		self.label = uiMobileStyle.CreateText(self, text, width // 2, height // 2 - int(font.split(":")[1]) // 2 - 1, uiAkravon.TEXT_COLOR, font, "center")
		self.selected = False
		self.RefreshVisual()

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.box = None
		self.label = None

	def SetSelected(self, selected):
		self.selected = selected
		self.RefreshVisual()

	def RefreshVisual(self):
		if not getattr(self, "box", None):
			return
		self.box.SetLineColor(uiAkravon.BOX_FOCUS if (self.selected or self.pressed) else uiAkravon.BOX_LINE)
		if self.selected:
			self.box.SetFillColor(uiMobileStyle.Color(176, 136, 70, 255))
			self.label.SetPackedFontColor(uiAkravon.DARK_TEXT)
		else:
			self.box.SetFillColor(uiAkravon.BOX_HOVER_FILL if self.pressed else uiAkravon.BOX_FILL)
			self.label.SetPackedFontColor(uiAkravon.TEXT_COLOR)


class RoundBox(uiMobileStyle.Pressable):
	# AKRAVON tema: + / - (karakter penceresindeki artiyla ayni); basinca altin. Gorseller onceden yuklenir.
	def __init__(self, size, text, font = None):
		uiMobileStyle.Pressable.__init__(self, size, size)
		kind = "minus" if text.strip() == "-" else "plus"
		self.images = []
		for state in ("normal", "gold"):
			image = ui.ExpandedImageBox()
			image.SetParent(self)
			image.AddFlag("not_pick")
			image.LoadImage("d:/ymir work/ui/akravon_legacy/akr_%s_%s.png" % (kind, state))
			image.SetScale(size / 64.0, size / 64.0)
			image.Hide()
			self.images.append(image)
		self.images[0].Show()

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.images = []

	def Run(self):
		# dokunmatikte "basili" durumu gelmeyebilir: her calismada 0.15 sn altin goster
		import app
		self.goldUntil = app.GetTime() + 0.15
		self.RefreshVisual()
		uiMobileStyle.Pressable.Run(self)

	def OnUpdate(self):
		until = getattr(self, "goldUntil", 0)
		if until:
			import app
			if app.GetTime() >= until:
				self.goldUntil = 0
				self.RefreshVisual()

	def RefreshVisual(self):
		if getattr(self, "images", None):
			gold = self.pressed or bool(getattr(self, "goldUntil", 0))
			if gold:
				self.images[0].Hide()
				self.images[1].Show()
			else:
				self.images[1].Hide()
				self.images[0].Show()


class OptionRow(ui.Window):
	def __init__(self, parent, width, spec):
		ui.Window.__init__(self)
		self.SetParent(parent)
		self.SetSize(width, ROW_HEIGHT)
		self.kind = spec[0]
		self.getter = spec[2]
		self.setter = spec[3]
		self.panel = uiAkravon.Box()
		self.panel.SetParent(self)
		self.panel.SetSize(width, ROW_HEIGHT)
		self.panel.Show()
		self.panel.SetFillColor(uiMobileStyle.Color(6, 9, 20, 190))
		self.panel.SetLineColor(uiMobileStyle.Color(64, 58, 44, 255))
		self.label = uiMobileStyle.CreateText(self, spec[1], 16, ROW_HEIGHT // 2 - 10, uiMobileStyle.BRIGHT_COLOR, FONT_ROW)
		self.widgets = []
		right = width - CONTROL_RIGHT
		top = (ROW_HEIGHT - CHOICE_HEIGHT) // 2
		if self.kind == "toggle":
			self.check = uiAkravon.CheckBox("", False)
			self.check.SetParent(self)
			self.check.SetPosition(right - self.check.GetWidth() + 8, (ROW_HEIGHT - 34) // 2)
			self.check.SetChangeEvent(ui.__mem_func__(self.__OnToggle))
			self.check.Show()
			self.widgets.append(self.check)
		elif self.kind == "choice":
			self.choices = spec[4]
			x = right
			self.choiceBoxes = []
			for (text, value) in reversed(self.choices):
				width = uiMobileStyle.TextWidth(text, FONT_CHOICE) + 28
				x -= width
				box = SmallBox(width, CHOICE_HEIGHT, text, FONT_CHOICE)
				box.SetParent(self)
				box.SetPosition(x, top)
				box.SetEvent(ui.__mem_func__(self.__OnChoice), value)
				box.Show()
				self.choiceBoxes.insert(0, (value, box))
				self.widgets.append(box)
				x -= CHOICE_GAP
		elif self.kind == "step":
			(self.low, self.high, self.step, self.format) = spec[4:8]
			self.plus = RoundBox(CHOICE_HEIGHT, "+", FONT_SIGN)
			self.plus.SetParent(self)
			self.plus.SetPosition(right - CHOICE_HEIGHT, top)
			self.plus.SetEvent(ui.__mem_func__(self.__OnStep), 1)
			self.plus.Show()
			self.value = uiMobileStyle.CreateText(self, "", right - CHOICE_HEIGHT - VALUE_WIDTH // 2, ROW_HEIGHT // 2 - 9, uiMobileStyle.GOLD_COLOR, FONT_VALUE, "center")
			self.minus = RoundBox(CHOICE_HEIGHT, "-", FONT_SIGN)
			self.minus.SetParent(self)
			self.minus.SetPosition(right - CHOICE_HEIGHT * 2 - VALUE_WIDTH, top)
			self.minus.SetEvent(ui.__mem_func__(self.__OnStep), -1)
			self.minus.Show()
			self.widgets += [self.plus, self.minus]
		elif self.kind == "action":
			text = spec[4] if len(spec) > 4 else "Aç"
			actionWidth = uiMobileStyle.TextWidth(text, FONT_CHOICE) + 40
			self.actionBox = SmallBox(actionWidth, CHOICE_HEIGHT, text, FONT_CHOICE)
			self.actionBox.SetParent(self)
			self.actionBox.SetPosition(right - actionWidth, top)
			self.actionBox.SetEvent(ui.__mem_func__(self.__OnAction))
			self.actionBox.Show()
			self.widgets.append(self.actionBox)
		self.Refresh()

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		for widget in self.widgets:
			widget.Destroy()
		self.widgets = []
		self.getter = None
		self.setter = None
		self.panel = None
		self.label = None
		self.Hide()

	def __OnAction(self):
		self.setter()

	def __OnToggle(self, value):
		self.setter(value)
		Call("SaveConfig", None)
		self.Refresh()

	def __OnChoice(self, value):
		ManualChange()
		self.setter(value)
		Call("SaveConfig", None)	# AKRAVON: her degisiklikte kaydet (pencere kapanmadan oyun kapansa da sifirlanmasin)
		self.Refresh()

	def __OnStep(self, direction):
		current = self.getter()
		current = max(self.low, min(self.high, round((current + self.step * direction) / self.step) * self.step))
		if self.label.GetText() != "Grafik Kalitesi":
			ManualChange()
		self.setter(current)
		Call("SaveConfig", None)
		self.Refresh()

	def Refresh(self):
		try:
			current = self.getter()
		except Exception:
			return
		if self.kind == "toggle":
			self.check.SetValue(current)
		elif self.kind == "choice":
			best = min(self.choiceBoxes, key = lambda item: abs(item[0] - current))[0]
			for (value, box) in self.choiceBoxes:
				box.SetSelected(value == best)
		elif self.kind == "step":
			self.value.SetText(self.format(current))


def TabButton(width, text):
	# AKRAVON tema: ortak dokunmatik sekme (lacivert, secili / basili altin)
	import uiAkravonDugme
	tab = uiAkravonDugme.YaziDugme(width, TAB_HEIGHT, text, uiAkravonDugme.YAZI_SEKME)
	tab.SetSelected = tab.SeciliAyarla
	tab.SetEvent = tab.OlayAyarla
	tab.Destroy = tab.Yok
	return tab


class OptionsWindow(uiMobileStyle.Pressable):
	def __init__(self):
		import uiMobileCreate
		(screenWidth, screenHeight) = (wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())
		uiMobileStyle.Pressable.__init__(self, screenWidth, screenHeight)
		self.AddFlag("float")
		self.dim = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(0, 0, 0, DIM_ALPHA), 0, 0, screenWidth, screenHeight)
		self.panel = uiAkravon.Frame(True, True)
		self.panel.SetParent(self)
		self.panel.Show()
		uiAkravon.SkinFrame(self.panel, "panel_thin")
		self.title = uiMobileStyle.CreateText(self.panel, "Oyun Seçenekleri", PANEL_WIDTH // 2, TOP, uiAkravon.HEADER_COLOR, FONT_TITLE, "center")

		contentX = SIDE_WIDTH + 16
		innerWidth = PANEL_WIDTH - contentX - PADDING_X
		tabWidth = SIDE_TAB_WIDTH
		self.sideLine = uiMobileStyle.CreateBar(self.panel, uiMobileStyle.Color(96, 74, 42, 255), SIDE_WIDTH, CONTENT_TOP, 1, 10)
		self.tabs = []
		self.pages = []
		# AKRAVON: satirlar ekrana sigacak sekilde (en fazla 54 px) otomatik kisilir
		global ROW_HEIGHT, CHOICE_HEIGHT, STEP_SIZE
		mostRows = max([len(factory()) for (name, factory) in TABS] + [1])
		available = screenHeight - 16 - CONTENT_TOP - CLOSE_GAP - 64 - BOTTOM
		stepHeight = max(40, min(54, available // mostRows))
		ROW_HEIGHT = stepHeight - ROW_GAP
		CHOICE_HEIGHT = min(40, ROW_HEIGHT - 6)
		STEP_SIZE = CHOICE_HEIGHT
		maxRows = 0
		for (index, (name, factory)) in enumerate(TABS):
			tab = TabButton(tabWidth, name)
			tab.SetParent(self.panel)
			tab.SetPosition((SIDE_WIDTH - SIDE_TAB_WIDTH) // 2, CONTENT_TOP + index * SIDE_TAB_STEP)
			tab.SetEvent(ui.__mem_func__(self.SelectTab), index)
			tab.Show()
			self.tabs.append(tab)
			rows = []
			for (rowIndex, spec) in enumerate(factory()):
				row = OptionRow(self.panel, innerWidth, spec)
				row.SetPosition(contentX, CONTENT_TOP + rowIndex * (ROW_HEIGHT + ROW_GAP))
				rows.append(row)
			maxRows = max(maxRows, len(rows))
			self.pages.append(rows)

		y = CONTENT_TOP + maxRows * (ROW_HEIGHT + ROW_GAP) - ROW_GAP + CLOSE_GAP
		self.closeButton = uiMobileCreate.PlateButton("create_button", "KAPAT")
		self.closeButton.SetParent(self.panel)
		self.closeButton.SetPosition((PANEL_WIDTH - self.closeButton.GetWidth()) // 2, y)
		self.closeButton.SetEvent(ui.__mem_func__(self.Close))
		self.closeButton.Show()
		height = y + self.closeButton.GetHeight() + BOTTOM
		self.sideLine.SetSize(1, max(10, y - CONTENT_TOP - CLOSE_GAP))
		self.panel.SetSize(PANEL_WIDTH, height)
		self.panel.SetPosition((screenWidth - PANEL_WIDTH) // 2, max(8, (screenHeight - height) // 2))

		self.selected = -1
		self.SetEvent(ui.__mem_func__(self.Close))
		self.SelectTab(0)

	def Destroy(self):
		for rows in self.pages:
			for row in rows:
				row.Destroy()
		for tab in self.tabs:
			tab.Destroy()
		self.pages = []
		self.tabs = []
		self.closeButton.Destroy()
		self.closeButton = None
		uiMobileStyle.Pressable.Destroy(self)
		self.dim = None
		self.panel = None
		self.title = None

	def SelectTab(self, index):
		self.selected = index
		for (tabIndex, tab) in enumerate(self.tabs):
			tab.SetSelected(tabIndex == index)
		for (pageIndex, rows) in enumerate(self.pages):
			for row in rows:
				if pageIndex == index:
					row.Refresh()
					row.Show()
				else:
					row.Hide()

	def Open(self, tab = 0):
		import uiAkravonPencere
		uiAkravonPencere.Acildi("secenekler", self)
		self.SelectTab(tab)
		self.Show()
		self.SetTop()

	def Close(self):
		import uiAkravonPencere
		uiAkravonPencere.Kapandi("secenekler")
		Call("SaveConfig", None)
		self.Hide()

	def OnPressEscapeKey(self):
		self.Close()
		return True


window = None


def Open(tab = 0):
	global window
	try:
		if window is None:
			window = OptionsWindow()
		window.Open(tab)
	except Exception:
		import dbg, traceback
		dbg.TraceError("uiMobileOptions.Open: " + traceback.format_exc())


def Hide():
	if window is not None:
		window.Close()


def Destroy():
	global window
	if window is not None:
		window.Destroy()
	window = None
