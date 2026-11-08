# AKRAVON: klasik (2008-2009) arayuz dili - koyu lacivert zemin, ince eskitilmis altin cizgiler, keskin koseler
# Paneller resimsiz (ui.Bar) cizilir; dugme/simge resimleri mobile_ui/akravon/ altindadir.
import ui
import uiMobileStyle

ASSET_PATH = "mobile_ui/akravon/%s.png"

FRAME_FILL = uiMobileStyle.Color(8, 11, 24, 232)
FRAME_LINE = uiMobileStyle.Color(176, 140, 74)
FRAME_INNER = uiMobileStyle.Color(98, 76, 42, 210)
CORNER_COLOR = uiMobileStyle.Color(222, 184, 108)
STATE_OPEN_COLOR = uiMobileStyle.Color(128, 206, 120)
DARK_TEXT = uiMobileStyle.Color(40, 24, 8)
NAME_FONT = "Cinzel:24"
BOX_FILL = uiMobileStyle.Color(4, 6, 14, 215)
BOX_LINE = uiMobileStyle.Color(132, 104, 58)
BOX_FOCUS = uiMobileStyle.Color(226, 188, 112)
BOX_HOVER_FILL = uiMobileStyle.Color(22, 24, 38, 230)
HEADER_COLOR = uiMobileStyle.Color(214, 182, 118)
TEXT_COLOR = uiMobileStyle.Color(228, 222, 206)
HINT_COLOR = uiMobileStyle.Color(140, 132, 116)

FONT_HEADER = "Marcellus:20"
FONT_TITLE = "Marcellus:30"
FONT_VALUE = "Lato:19"
FONT_HINT = "Lato:19"


def Asset(name):
	return ASSET_PATH % name


def CreateImage(parent, name, x = 0, y = 0):
	image = ui.ExpandedImageBox()
	image.SetParent(parent)
	image.AddFlag("not_pick")
	image.LoadImage(Asset(name))
	image.SetPosition(x, y)
	image.Show()
	return image


def LoadImage(image, name):
	image.LoadImage(Asset(name))


def CreateBar(parent, color):
	bar = ui.Bar()
	bar.SetParent(parent)
	bar.AddFlag("not_pick")
	bar.SetColor(color)
	bar.Show()
	return bar


def PlaceBar(bar, x, y, width, height):
	bar.SetPosition(x, y)
	bar.SetSize(max(0, width), max(0, height))


# AKRAVON: kutu/pencereye uretilmis HD gorsel giydirme (2 kat cozunurluklu resim, tam boyuta olceklenir)
def _LoadSkin(skin, name):
	# resmin kendi boyu olcekten once saklanir (GetWidth olcekli boyu dondurebilir)
	LoadImage(skin, name)
	skin.SetScale(1.0, 1.0)
	skin.baseSize = (max(1, skin.GetWidth()), max(1, skin.GetHeight()))


def _FitSkin(owner):
	skin = getattr(owner, "skin", None)
	if not skin:
		return
	(width, height) = (owner.GetWidth(), owner.GetHeight())
	(baseW, baseH) = getattr(skin, "baseSize", (1, 1))
	skin.SetScale(float(width) / baseW, float(height) / baseH)


def SkinBox(box, name, focusName = None):
	box.fill.Hide()
	for line in box.lines:
		line.Hide()
	for bar in getattr(box, "extra", []):
		bar.Hide()
	box.skin = CreateImage(box, name)
	_LoadSkin(box.skin, name)
	box.skinNames = (name, focusName or name)
	box.skinState = 0
	_FitSkin(box)
	return box.skin


def SetSkinState(box, active):
	skin = getattr(box, "skin", None)
	state = 1 if active else 0
	if not skin or state == box.skinState:
		return
	box.skinState = state
	_LoadSkin(skin, box.skinNames[state])
	_FitSkin(box)


def SkinFrame(frame, name):
	for bar in [frame.fill] + frame.outer + frame.inner + frame.corners + frame.diamonds:
		bar.Hide()
	frame.skin = CreateImage(frame, name)
	_LoadSkin(frame.skin, name)
	_FitSkin(frame)
	return frame.skin


class Box(ui.Window):
	# alan / secici kutusu: koyu zemin + tek ince cizgi
	def __init__(self, fill = None, line = None):
		ui.Window.__init__(self)
		self.AddFlag("not_pick")
		self.lineColor = line or BOX_LINE
		self.fillColor = fill or BOX_FILL
		self.fill = CreateBar(self, self.fillColor)
		self.lines = [CreateBar(self, self.lineColor) for index in range(4)]

	def __del__(self):
		ui.Window.__del__(self)

	def SetSize(self, width, height):
		ui.Window.SetSize(self, width, height)
		PlaceBar(self.fill, 1, 1, width - 2, height - 2)
		(top, bottom, left, right) = self.lines
		PlaceBar(top, 0, 0, width, 1)
		PlaceBar(bottom, 0, height - 1, width, 1)
		PlaceBar(left, 0, 0, 1, height)
		PlaceBar(right, width - 1, 0, 1, height)
		_FitSkin(self)

	def SetLineColor(self, color):
		for line in self.lines:
			line.SetColor(color)

	def SetFillColor(self, color):
		self.fill.SetColor(color)


class Frame(ui.Window):
	# pencere: lacivert zemin, 2px eskitilmis altin dis cizgi + ince ic cizgi, kose susleri, ust/alt elmas
	CORNER = 10

	def __init__(self, diamonds = True, pickable = False):
		ui.Window.__init__(self)
		if not pickable:
			self.AddFlag("not_pick")
		self.fill = CreateBar(self, FRAME_FILL)
		self.outer = [CreateBar(self, FRAME_LINE) for index in range(4)]
		self.inner = [CreateBar(self, FRAME_INNER) for index in range(4)]
		self.corners = [CreateBar(self, CORNER_COLOR) for index in range(8)]
		self.diamonds = []
		if diamonds:
			self.diamonds = [CreateImage(self, "diamond"), CreateImage(self, "diamond")]

	def __del__(self):
		ui.Window.__del__(self)

	def SetSize(self, width, height):
		ui.Window.SetSize(self, width, height)
		PlaceBar(self.fill, 2, 2, width - 4, height - 4)
		(top, bottom, left, right) = self.outer
		PlaceBar(top, 0, 0, width, 2)
		PlaceBar(bottom, 0, height - 2, width, 2)
		PlaceBar(left, 0, 0, 2, height)
		PlaceBar(right, width - 2, 0, 2, height)
		(top, bottom, left, right) = self.inner
		PlaceBar(top, 5, 5, width - 10, 1)
		PlaceBar(bottom, 5, height - 6, width - 10, 1)
		PlaceBar(left, 5, 5, 1, height - 10)
		PlaceBar(right, width - 6, 5, 1, height - 10)
		# kose susleri: ic cizgi uzerinde kalin L parcalari
		c = self.CORNER
		pieces = ((4, 4, c, 3), (4, 4, 3, c), (width - 4 - c, 4, c, 3), (width - 7, 4, 3, c),
			(4, height - 7, c, 3), (4, height - 4 - c, 3, c), (width - 4 - c, height - 7, c, 3), (width - 7, height - 4 - c, 3, c))
		for (bar, (x, y, w, h)) in zip(self.corners, pieces):
			PlaceBar(bar, x, y, w, h)
		if self.diamonds:
			(top, bottom) = self.diamonds
			top.SetPosition((width - top.GetWidth()) // 2, -top.GetHeight() // 2)
			bottom.SetPosition((width - bottom.GetWidth()) // 2, height - bottom.GetHeight() // 2)
		_FitSkin(self)


class ImageButton(uiMobileStyle.Pressable):
	# resimli dugme: <ad>_normal / _over / _down (istege bagli genislige olceklenir)
	def __init__(self, name, width = None):
		self.name = name
		probe = ui.ExpandedImageBox()
		probe.LoadImage(Asset(name + "_normal"))
		(baseW, baseH) = (probe.GetWidth(), probe.GetHeight())
		probe = None
		self.scale = float(width) / baseW if width else 1.0
		uiMobileStyle.Pressable.__init__(self, int(baseW * self.scale), int(baseH * self.scale))
		self.image = CreateImage(self, name + "_normal")
		self.image.SetScale(self.scale, self.scale)

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.image = None

	def RefreshVisual(self):
		if not getattr(self, "image", None):
			return
		state = self.GetVisualState() if self.enabled else "down"
		LoadImage(self.image, "%s_%s" % (self.name, state))
		self.image.SetScale(self.scale, self.scale)


class CheckBox(uiMobileStyle.Pressable):
	# uiMobileStyle.Toggle ile ayni arayuz (SetChangeEvent / SetValue / GetValue)
	def __init__(self, text, value = False):
		textWidth = uiMobileStyle.TextWidth(text, FONT_VALUE)
		uiMobileStyle.Pressable.__init__(self, 26 + 12 + textWidth + 8, 34)
		self.value = bool(value)
		self.image = CreateImage(self, "check_off", 0, 4)
		self.text = uiMobileStyle.CreateText(self, text, 38, 7, TEXT_COLOR, FONT_VALUE)
		self.onChange = None
		self.SetEvent(ui.__mem_func__(self.__Flip))
		self.RefreshVisual()

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.onChange = None
		self.image = None
		self.text = None

	def __Flip(self):
		self.SetValue(not self.value)
		if self.onChange:
			self.onChange(self.value)

	def SetChangeEvent(self, function):
		self.onChange = function

	def SetValue(self, value):
		self.value = bool(value)
		self.RefreshVisual()

	def GetValue(self):
		return self.value

	def RefreshVisual(self):
		if getattr(self, "image", None):
			LoadImage(self.image, "check_on" if self.value else "check_off")


class Selector(uiMobileStyle.Pressable):
	# kanal / sunucu secici: kutu + durum noktasi + deger + ok
	def __init__(self, width, height):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.box = Box()
		self.box.SetParent(self)
		self.box.SetSize(width, height)
		self.box.Show()
		self.dot = uiMobileStyle.CreateImage(self, "dot_closed", 14, height // 2 - 3)
		self.dot.Hide()
		self.value = uiMobileStyle.CreateText(self, "", 14, height // 2 - 11, TEXT_COLOR, FONT_VALUE)
		self.chevron = CreateImage(self, "chevron")
		self.chevron.SetPosition(width - 22, (height - 16) // 2)

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.box = None
		self.dot = None
		self.value = None
		self.chevron = None

	def SetValue(self, text, dotName = None):
		if dotName:
			uiMobileStyle.LoadAsset(self.dot, dotName)
			self.dot.Show()
			self.value.SetPosition(28, self.GetHeight() // 2 - 11)
		else:
			self.dot.Hide()
			self.value.SetPosition(14, self.GetHeight() // 2 - 11)
		self.value.SetText(text)

	def RefreshVisual(self):
		if getattr(self, "box", None):
			active = self.pressed or self.hovered
			self.box.SetLineColor(BOX_FOCUS if active else BOX_LINE)
			self.box.SetFillColor(BOX_HOVER_FILL if active else BOX_FILL)


class Field(ui.Window):
	# simgeli giris alani: bosken soluk yer tutucu yazi, odakta parlak cizgi
	TEXT_X = 46

	def __init__(self, width, height, hint, iconName = None):
		ui.Window.__init__(self)
		self.SetSize(width, height)
		self.box = Box()
		self.box.SetParent(self)
		self.box.SetSize(width, height)
		self.box.Show()
		self.icon = CreateImage(self, iconName) if iconName else None
		self.textX = self.TEXT_X if iconName else 14
		if self.icon:
			self.icon.SetPosition(14 + (24 - self.icon.GetWidth()) // 2, (height - self.icon.GetHeight()) // 2)
		self.hint = uiMobileStyle.CreateText(self, hint, self.textX, height // 2 - 11, HINT_COLOR, FONT_HINT) if hint else None
		self.editLine = None

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		self.box = None
		self.icon = None
		self.hint = None
		self.editLine = None

	def Attach(self, editLine, rightPadding = 14):
		self.editLine = editLine
		editLine.SetParent(self)
		editLine.SetPosition(self.textX, self.GetHeight() // 2 - 9)
		editLine.SetSize(self.GetWidth() - self.textX - rightPadding, 18)
		editLine.SetPackedFontColor(TEXT_COLOR)
		try:
			editLine.SetFontName(FONT_VALUE)	# yazilan metin de okunakli yazi tipinde
		except Exception:
			pass
		editLine.Show()

	def OnUpdate(self):
		if not self.editLine:
			return
		try:
			focused = self.editLine.IsFocus()
			empty = not self.editLine.GetText()
		except Exception:
			focused = False
			empty = True
		self.box.SetLineColor(BOX_FOCUS if focused else BOX_LINE)
		SetSkinState(self.box, focused)
		if self.hint and empty and not focused:
			self.hint.Show()
		elif self.hint:
			self.hint.Hide()

	def OnMouseLeftButtonDown(self):
		if self.editLine:
			ui.Window.SetFocus(self.editLine)
		return True


class StyledBox(Box):
	# uiMobileStyle.Panel ile uyumlu: SetStyle("field" | "chip_active" | ...)
	ACTIVE_STYLES = ("chip_active", "chip_soft_active", "field_focus", "card_active")

	def __init__(self):
		Box.__init__(self)
		self.style = "field"

	def SetStyle(self, style):
		self.style = style
		if style == "card_active" or style == "chip_soft_active":
			self.SetLineColor(uiMobileStyle.Color(188, 151, 92, 230))
			self.SetFillColor(uiMobileStyle.Color(22, 27, 40, 242))
		else:
			active = style in self.ACTIVE_STYLES
			self.SetLineColor(BOX_FOCUS if active else BOX_LINE)
			self.SetFillColor(BOX_HOVER_FILL if active else BOX_FILL)

	def GetStyle(self):
		return self.style


def CreateBox(parent, width, height):
	box = StyledBox()
	box.SetParent(parent)
	box.SetSize(width, height)
	box.Show()
	return box


PLATE_SIZES = {"chip": (104, 56), "close": (180, 48), "wide": (240, 56), "back": (130, 44), "gold": (300, 58)}


class Plate(ui.Window):
	# lacivert plaka (resim); uiMobileStyle.Panel ile uyumlu SetStyle
	def __init__(self, kind, width, height):
		ui.Window.__init__(self)
		self.AddFlag("not_pick")
		self.kind = kind
		self.image = CreateImage(self, "plate_%s_normal" % kind)
		(baseWidth, baseHeight) = PLATE_SIZES[kind]
		self.image.SetScale(float(width) / baseWidth, float(height) / baseHeight)
		ui.Window.SetSize(self, width, height)
		self.style = "field"

	def __del__(self):
		ui.Window.__del__(self)

	def SetStyle(self, style):
		self.style = style
		LoadImage(self.image, "plate_%s_%s" % (self.kind, "active" if style in StyledBox.ACTIVE_STYLES else "normal"))

	def GetStyle(self):
		return self.style


def CreatePlate(parent, kind, width, height):
	plate = Plate(kind, width, height)
	plate.SetParent(parent)
	plate.Show()
	return plate


class TextButton(uiMobileStyle.Pressable):
	# plakali metin dugmesi (Kapat vb.)
	def __init__(self, width, height, text, kind = "close", font = None):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.box = CreatePlate(self, kind, width, height)
		self.dark = kind == "gold"
		self.textColor = DARK_TEXT if self.dark else HEADER_COLOR if kind == "back" else TEXT_COLOR
		self.text = uiMobileStyle.CreateText(self, text, width // 2, height // 2 - 12, self.textColor, font or FONT_VALUE, "center")

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.box = None
		self.text = None

	def RefreshVisual(self):
		if getattr(self, "box", None):
			active = self.pressed or self.hovered
			self.box.SetStyle("chip_active" if active else "field")
			if not self.dark:
				normalColor = HEADER_COLOR if self.box.kind == "back" else TEXT_COLOR
				self.text.SetPackedFontColor(uiMobileStyle.GOLD_COLOR if active else normalColor)

	def SetText(self, text):
		self.text.SetText(text)


class PrimaryTextButton(uiMobileStyle.PrimaryButton):
	def __init__(self, sizeKey, text):
		uiMobileStyle.PrimaryButton.__init__(self, sizeKey, "label_start")
		self.label.Hide()
		self.text = uiMobileStyle.CreateText(self, text, self.GetWidth() // 2, self.GetHeight() // 2 - 11, DARK_TEXT, FONT_VALUE, "center")

	def Destroy(self):
		uiMobileStyle.PrimaryButton.Destroy(self)
		self.text = None

	def SetText(self, text):
		self.text.SetText(text)


class PopupContent(ui.Window):
	# AKRAVON uyari icerigi: ekran karartma + klasik pencere + mesaj + plaka dugme
	WIDTH = 440
	PAD = 28
	BUTTON_NAMES = {"OK": "Tamam", "Ok": "Tamam", "Cancel": "İptal", "iptal": "İptal"}

	def __init__(self, screenWidth, screenHeight, buttonEvent):
		ui.Window.__init__(self)
		self.SetSize(screenWidth, screenHeight)
		self.dim = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(0, 0, 0, 150), 0, 0, screenWidth, screenHeight)
		self.frame = Frame(True, True)
		self.frame.SetParent(self)
		self.frame.Show()
		self.title = uiMobileStyle.CreateText(self.frame, "AKRAVON", self.WIDTH // 2, 22, HEADER_COLOR, FONT_TITLE, "center")
		self.titleLine = uiMobileStyle.CreateBar(self.frame, FRAME_INNER, self.PAD, 62, self.WIDTH - self.PAD * 2, 1)
		self.lines = []
		self.button = TextButton(180, 48, "")
		self.button.SetParent(self.frame)
		self.button.SetEvent(buttonEvent)
		self.button.Show()

	def __del__(self):
		ui.Window.__del__(self)

	def SetContent(self, message, buttonName):
		for line in self.lines:
			line.Hide()
		self.lines = uiMobileStyle.CreateWrappedText(self.frame, message, self.PAD, 80, self.WIDTH - self.PAD * 2, TEXT_COLOR, FONT_VALUE, 26, "center")
		y = 80 + len(self.lines) * 26 + 22
		self.button.text.SetText(self.BUTTON_NAMES.get(buttonName, buttonName))
		self.button.SetPosition((self.WIDTH - self.button.GetWidth()) // 2, y)
		y += self.button.GetHeight() + 26
		self.frame.SetSize(self.WIDTH, y)
		self.frame.SetPosition((self.GetWidth() - self.WIDTH) // 2, (self.GetHeight() - y) // 2)


def RestylePopupDialog(dialog):
	# networkModule.PopupDialog: davranis ayni (Open/Close/olay), yalniz gorunum AKRAVON
	import wndMgr
	screenWidth = wndMgr.GetScreenWidth()
	screenHeight = wndMgr.GetScreenHeight()
	board = dialog.GetChild("board")
	board.Hide()
	dialog.SetPosition(0, 0)
	dialog.SetSize(screenWidth, screenHeight)
	content = PopupContent(screenWidth, screenHeight, ui.__mem_func__(dialog.Close))
	content.SetParent(dialog)
	content.SetPosition(0, 0)
	content.Show()
	dialog.akravonPopup = content


class CardBox(Box):
	# karakter karti: bos (soluk), normal, secili (altin cift kenar) - uiMobileStyle "card*" stilleriyle uyumlu
	LOOKS = {
		"card_empty": (uiMobileStyle.Color(17, 12, 12, 130), uiMobileStyle.Color(110, 86, 44, 150)),
		"card": (uiMobileStyle.Color(17, 12, 12, 205), uiMobileStyle.Color(110, 86, 44, 255)),
		"card_active": (uiMobileStyle.Color(35, 27, 19, 225), uiMobileStyle.Color(236, 196, 116)),
	}

	def __init__(self):
		Box.__init__(self)
		self.style = None
		self.extra = [CreateBar(self, BOX_FOCUS) for index in range(4)]

	def SetSize(self, width, height):
		Box.SetSize(self, width, height)
		(top, bottom, left, right) = self.extra
		PlaceBar(top, 1, 1, width - 2, 1)
		PlaceBar(bottom, 1, height - 2, width - 2, 1)
		PlaceBar(left, 1, 1, 1, height - 2)
		PlaceBar(right, width - 2, 1, 1, height - 2)

	def SetStyle(self, style):
		self.style = style
		(fill, line) = self.LOOKS.get(style, self.LOOKS["card"])
		self.SetFillColor(fill)
		self.SetLineColor(line)
		for bar in self.extra:
			if style == "card_active":
				bar.Show()
			else:
				bar.Hide()

	def GetStyle(self):
		return self.style


def CreateCardBox(parent, width, height, style):
	box = CardBox()
	box.SetParent(parent)
	box.SetSize(width, height)
	box.SetStyle(style)
	box.Show()
	return box


class TopBar(object):
	# ekran ust cubugu: solda geri plakasi, ortada Cinzel baslik, sagda sunucu bilgisi
	def __init__(self, parent, screenWidth, backText, titleText, metaText, backEvent, backWidth = 130):
		self.backButton = TextButton(backWidth, 44, backText, "back")
		self.backButton.SetParent(parent)
		self.backButton.SetPosition(24, 18)
		self.backButton.SetEvent(backEvent)
		self.backButton.Show()
		self.title = uiMobileStyle.CreateText(parent, titleText, screenWidth // 2, 20, HEADER_COLOR, "Cinzel:30", "center", True)
		self.line = CreateBar(parent, FRAME_LINE)
		PlaceBar(self.line, screenWidth // 2 - 160, 62, 320, 1)
		self.meta = uiMobileStyle.CreateText(parent, metaText, screenWidth - 24, 30, HEADER_COLOR, FONT_VALUE, "right", True)

	def Destroy(self):
		if self.backButton:
			self.backButton.Destroy()
		self.backButton = None
		self.title = None
		self.line = None
		self.meta = None

	def SetMeta(self, text):
		if self.meta:
			self.meta.SetText(text)
