from __future__ import division
import grp
import net
import ui
import uiMobileCompat
import uiMobileAssets
import wndMgr

from _weakref import proxy

ASSET_PATH = "mobile_ui/cynex/%s.png"
ASSET_SCALE = 1.0
ORNATE_INSET = 3
ORNATE_BASE_COLOR = (17, 12, 12, 235)


def Color(red, green, blue, alpha = 255):
	return grp.GenerateColor(red / 255.0, green / 255.0, blue / 255.0, alpha / 255.0)


# Cynex: renk ve yazi tipleri YeniKlasor4 mobil setinden (Marcellus basliklar, Alegreya Sans govde)
TEXT_COLOR = Color(243, 230, 204)
CREAM_COLOR = Color(246, 223, 166)
BRIGHT_COLOR = Color(243, 230, 204)
SOFT_COLOR = Color(201, 173, 122)
MUTED_COLOR = Color(156, 136, 102)
LABEL_COLOR = Color(240, 207, 135)
GOLD_COLOR = Color(246, 223, 166)
DANGER_COLOR = Color(226, 110, 92)
DESCRIPTION_COLOR = Color(226, 210, 178)
DARK_TEXT_COLOR = Color(42, 22, 8)
DIVIDER_COLOR = Color(110, 86, 44)
TRACK_COLOR = Color(30, 22, 16)
FILL_COLOR = Color(217, 169, 78)
PIP_ON_COLOR = Color(217, 169, 78)
DIM_COLOR = Color(0, 0, 0, 140)
LIST_HOVER_COLOR = Color(240, 207, 135, 20)
LIST_SELECT_COLOR = Color(217, 169, 78, 46)
LIST_ACCENT_COLOR = Color(240, 207, 135)

# AKRAVON: okunakli serbest lisansli yazilar - Lato (metin), Cinzel (baslik); OFL lisanslari fonts/ altinda
FONT_SMALL = "Lato:15"
FONT_NORMAL = "Lato:17"
FONT_MEDIUM = "Lato:19"
FONT_LARGE = "Lato:21"
FONT_TITLE = "Cinzel:26"

PANEL_STYLES = uiMobileAssets.PANEL_STYLES

RACE_KEYS = ("warrior", "assassin", "sura", "shaman")
EMPIRE_KEYS = {
	net.EMPIRE_A : "a",
	net.EMPIRE_B : "b",
	net.EMPIRE_C : "c",
}
EMPIRE_FLAG_IMAGES = {
	net.EMPIRE_A : "d:/ymir work/ui/intro/empire/empireflag_a.sub",
	net.EMPIRE_B : "d:/ymir work/ui/intro/empire/empireflag_b.sub",
	net.EMPIRE_C : "d:/ymir work/ui/intro/empire/empireflag_c.sub",
}
EMPIRE_COLORS = {
	net.EMPIRE_A : Color(196, 70, 56),
	net.EMPIRE_B : Color(222, 180, 66),
	net.EMPIRE_C : Color(80, 132, 214),
}

PRESS_RELEASE_STATES = ("normal", "over", "down")


def Asset(name):
	return ASSET_PATH % name


def InputTop(height):
	# giris pencereleri (depo sifresi, miktar, rutbe adi ...): klavye ekranin alt %48'ini kaplar; pencerenin alt kenari klavyenin 10 px ustunde
	keyboardTop = int(wndMgr.GetScreenHeight() * 0.52)
	return max(8, keyboardTop - height - 10)


def TrUpper(text):
	# AKRAVON: dugme yazilari her yerde BUYUK harf; Turkce i -> I (noktali), dotless -> I kuralina gore
	try:
		converted = text.decode("cp1254")
	except Exception:
		return text
	converted = converted.replace(u"i", u"\u0130").replace(u"\u0131", u"I").upper()
	try:
		return converted.encode("cp1254")
	except Exception:
		return text


def RaceKey(job):
	if 0 <= job < len(RACE_KEYS):
		return RACE_KEYS[job]
	return RACE_KEYS[0]


def CreateImage(parent, name, x = 0, y = 0):
	image = ui.ExpandedImageBox()
	image.SetParent(parent)
	image.AddFlag("not_pick")
	LoadAsset(image, name)
	image.SetPosition(x, y)
	image.Show()
	return image


def LoadAsset(image, name):
	image.assetName = name
	image.LoadImage(Asset(name))
	image.SetScale(ASSET_SCALE, ASSET_SCALE)


def HasAsset(name):
	return name in uiMobileAssets.ASSET_SIZES


def AssetSize(name):
	return uiMobileAssets.ASSET_SIZES.get(name, (1, 1))


def StretchAsset(image, name, width, height):
	(assetWidth, assetHeight) = AssetSize(name)
	image.SetScale(float(width) * ASSET_SCALE / max(1, assetWidth), float(height) * ASSET_SCALE / max(1, assetHeight))


def ImageSize(image):
	size = uiMobileAssets.ASSET_SIZES.get(getattr(image, "assetName", None))
	if size:
		return size
	return (image.GetWidth(), image.GetHeight())


def CreateCenteredImage(parent, name, centerX, centerY):
	image = CreateImage(parent, name)
	(width, height) = ImageSize(image)
	image.SetPosition(int(centerX - width / 2), int(centerY - height / 2))
	return image


def SetCenteredAsset(image, name, centerX, centerY):
	LoadAsset(image, name)
	(width, height) = ImageSize(image)
	image.SetPosition(int(centerX - width / 2), int(centerY - height / 2))


def SetFlagImage(flag, empireId, width):
	flag.SetScale(1.0, 1.0)
	flag.LoadImage(EMPIRE_FLAG_IMAGES.get(empireId, EMPIRE_FLAG_IMAGES[net.EMPIRE_A]))
	scale = float(width) / max(1, flag.GetWidth())
	flag.SetScale(scale, scale)


def CreateFlag(parent, empireId, x, y, width):
	flag = ui.ExpandedImageBox()
	flag.SetParent(parent)
	flag.AddFlag("not_pick")
	SetFlagImage(flag, empireId, width)
	flag.SetPosition(x, y)
	flag.Show()
	return flag


def FlagHeight(flag, width):
	return int(flag.GetHeight() * float(width) / max(1, flag.GetWidth()))


def CreateText(parent, text, x, y, color = TEXT_COLOR, font = FONT_NORMAL, align = "left", outline = False):
	line = ui.TextLine()
	line.SetParent(parent)
	line.AddFlag("not_pick")
	line.SetFontName(font)
	if outline:
		line.SetOutline()
	if align == "center":
		line.SetHorizontalAlignCenter()
	elif align == "right":
		line.SetHorizontalAlignRight()
	line.SetPackedFontColor(color)
	line.SetPosition(x, y)
	line.SetText(text)
	line.Show()
	return line


def CreateBar(parent, color, x, y, width, height):
	bar = ui.Bar()
	bar.SetParent(parent)
	bar.AddFlag("not_pick")
	bar.SetColor(color)
	bar.SetPosition(x, y)
	bar.SetSize(max(0, width), max(0, height))
	bar.Show()
	return bar


def TextWidth(text, font):
	probe = ui.TextLine()
	probe.SetFontName(font)
	probe.SetText(text)
	return probe.GetTextSize()[0]


def WrapLines(text, width, font):
	lines = []
	for paragraph in text.split("\n"):
		current = ""
		for word in paragraph.split(" "):
			candidate = word if not current else current + " " + word
			if current and TextWidth(candidate, font) > width:
				lines.append(current)
				current = word
			else:
				current = candidate
		lines.append(current)
	return lines


def CreateWrappedText(parent, text, x, y, width, color = TEXT_COLOR, font = FONT_NORMAL, lineHeight = 20, align = "left"):
	lines = []
	for (index, content) in enumerate(WrapLines(text, width, font)):
		lineX = x + width // 2 if align == "center" else x
		lines.append(CreateText(parent, content, lineX, y + index * lineHeight, color, font, align))
	return lines


def HideThinBoardFrame(board):
	for name in ("Corners", "Lines"):
		for piece in getattr(board, name, []):
			piece.Hide()
	base = getattr(board, "Base", None)
	if base:
		base.Hide()


def RestyleThinBoard(board, style, width, height):
	HideThinBoardFrame(board)
	frame = Panel(style)
	frame.SetParent(board)
	frame.AddFlag("not_pick")
	frame.SetPosition(0, 0)
	frame.SetSize(width, height)
	frame.SetCenterVisible(False)
	frame.Show()
	base = getattr(board, "Base", None)
	if base:
		(radius, border) = PANEL_STYLES[style][:2]
		base.SetColor(frame.GetFillColor())
		base.SetPosition(border, radius)
		base.SetSize(width - border * 2, height - radius * 2)
		base.Show()
	return frame


def ParkWindow(window):
	window.SetPosition(-4000, -4000)


class ListSelectionRenderer(object):
	def __init__(self, listBox):
		self.listBox = proxy(listBox)

	def __call__(self):
		try:
			box = self.listBox
			(x, y) = box.GetGlobalPosition()
			y -= box.TEMPORARY_PLACE
			if box.overLine != -1:
				grp.SetColor(LIST_HOVER_COLOR)
				grp.RenderBar(x + 2, y + box.overLine * box.stepSize + 4, box.width - 3, box.stepSize)
			if box.selectedLine != -1 and box.basePos <= box.selectedLine < box.basePos + box.showLineCount:
				rowY = y + (box.selectedLine - box.basePos) * box.stepSize + 4
				grp.SetColor(LIST_SELECT_COLOR)
				grp.RenderBar(x + 2, rowY, box.width - 3, box.stepSize)
				grp.SetColor(LIST_ACCENT_COLOR)
				grp.RenderBar(x + 2, rowY, 2, box.stepSize)
		except ReferenceError:
			pass


def RunButtonEvent(button, *extra):
	function = getattr(button, "eventFunc", None)
	if not function:
		return False
	arguments = getattr(button, "eventArgs", ()) or ()
	function(*(tuple(arguments) + extra))
	return True


class Panel(ui.Window):
	def __init__(self, style = "panel"):
		ui.Window.__init__(self)
		self.style = None
		self.radius = 0
		self.border = 0
		self.width = 0
		self.height = 0

		self.corners = [ui.ExpandedImageBox() for index in range(4)]
		for corner in self.corners:
			corner.SetParent(self)
			corner.AddFlag("not_pick")
			corner.Show()

		self.borders = [self.__CreatePiece() for index in range(4)]
		self.fills = [self.__CreatePiece() for index in range(3)]
		self.SetStyle(style)

	def __del__(self):
		ui.Window.__del__(self)

	def __CreatePiece(self):
		bar = ui.Bar()
		bar.SetParent(self)
		bar.AddFlag("not_pick")
		bar.Show()
		return bar

	def SetStyle(self, style):
		if style == self.style or style not in PANEL_STYLES:
			return
		self.style = style
		(radius, border, fill, borderColor) = PANEL_STYLES[style]
		self.radius = radius
		self.border = border
		for (corner, suffix) in zip(self.corners, ("tl", "tr", "bl", "br")):
			LoadAsset(corner, "%s_%s" % (style, suffix))
		for bar in self.borders:
			bar.SetColor(Color(*borderColor))
		for bar in self.fills:
			bar.SetColor(Color(*fill))
		self.__Layout()

	def GetStyle(self):
		return self.style

	def GetFillColor(self):
		return Color(*PANEL_STYLES[self.style][2])

	def SetCenterVisible(self, visible):
		if visible:
			self.fills[2].Show()
		else:
			self.fills[2].Hide()

	def SetSize(self, width, height):
		self.width = width
		self.height = height
		ui.Window.SetSize(self, width, height)
		self.__Layout()

	def __Place(self, window, x, y, width, height):
		window.SetPosition(x, y)
		window.SetSize(max(0, width), max(0, height))

	def __Layout(self):
		width = max(self.width, self.radius * 2)
		height = max(self.height, self.radius * 2)
		radius = self.radius
		border = self.border

		self.corners[0].SetPosition(0, 0)
		self.corners[1].SetPosition(width - radius, 0)
		self.corners[2].SetPosition(0, height - radius)
		self.corners[3].SetPosition(width - radius, height - radius)

		(top, bottom, left, right) = self.borders
		self.__Place(top, radius, 0, width - radius * 2, border)
		self.__Place(bottom, radius, height - border, width - radius * 2, border)
		self.__Place(left, 0, radius, border, height - radius * 2)
		self.__Place(right, width - border, radius, border, height - radius * 2)

		(topFill, bottomFill, centerFill) = self.fills
		self.__Place(topFill, radius, border, width - radius * 2, radius - border)
		self.__Place(bottomFill, radius, height - radius, width - radius * 2, radius - border)
		self.__Place(centerFill, border, radius, width - border * 2, height - radius * 2)


class OrnatePanel(ui.Window):
	EDGE_NAMES = ("ornate_t", "ornate_b", "ornate_l", "ornate_r")
	CORNER_NAMES = ("ornate_tl", "ornate_tr", "ornate_bl", "ornate_br")

	def __init__(self, withFill = True):
		ui.Window.__init__(self)
		self.fill = CreateImage(self, "ornate_fill") if withFill else None
		self.edges = [CreateImage(self, name) for name in self.EDGE_NAMES]
		self.corners = [CreateImage(self, name) for name in self.CORNER_NAMES]

	def __del__(self):
		ui.Window.__del__(self)

	def SetSize(self, width, height):
		(cornerWidth, cornerHeight) = AssetSize("ornate_tl")
		width = max(width, cornerWidth * 2 + 2)
		height = max(height, cornerHeight * 2 + 2)
		ui.Window.SetSize(self, width, height)

		if self.fill:
			self.fill.SetPosition(ORNATE_INSET, ORNATE_INSET)
			StretchAsset(self.fill, "ornate_fill", width - ORNATE_INSET * 2, height - ORNATE_INSET * 2)

		(top, bottom, left, right) = self.edges
		middleWidth = width - cornerWidth * 2
		middleHeight = height - cornerHeight * 2
		top.SetPosition(cornerWidth, 0)
		StretchAsset(top, "ornate_t", middleWidth, cornerHeight)
		bottom.SetPosition(cornerWidth, height - cornerHeight)
		StretchAsset(bottom, "ornate_b", middleWidth, cornerHeight)
		left.SetPosition(0, cornerHeight)
		StretchAsset(left, "ornate_l", cornerWidth, middleHeight)
		right.SetPosition(width - cornerWidth, cornerHeight)
		StretchAsset(right, "ornate_r", cornerWidth, middleHeight)

		(topLeft, topRight, bottomLeft, bottomRight) = self.corners
		topLeft.SetPosition(0, 0)
		topRight.SetPosition(width - cornerWidth, 0)
		bottomLeft.SetPosition(0, height - cornerHeight)
		bottomRight.SetPosition(width - cornerWidth, height - cornerHeight)


class NineSlicePanel(ui.Window):
	PIECE_KEYS = ("c", "t", "b", "l", "r", "tl", "tr", "bl", "br")

	def __init__(self, prefix):
		ui.Window.__init__(self)
		self.prefix = prefix
		self.width = 0
		self.height = 0
		self.pieces = {}
		for key in self.PIECE_KEYS:
			self.pieces[key] = CreateImage(self, "%s_%s" % (prefix, key))

	def __del__(self):
		ui.Window.__del__(self)

	def SetPrefix(self, prefix):
		if prefix == self.prefix:
			return
		self.prefix = prefix
		for (key, image) in self.pieces.items():
			LoadAsset(image, "%s_%s" % (prefix, key))
		self.__Layout()

	def SetSize(self, width, height):
		self.width = width
		self.height = height
		ui.Window.SetSize(self, width, height)
		self.__Layout()

	def __Name(self, key):
		return "%s_%s" % (self.prefix, key)

	def __Layout(self):
		(cornerWidth, cornerHeight) = AssetSize(self.__Name("tl"))
		width = max(self.width, cornerWidth * 2 + 1)
		height = max(self.height, cornerHeight * 2 + 1)
		middleWidth = width - cornerWidth * 2
		middleHeight = height - cornerHeight * 2
		pieces = self.pieces

		pieces["c"].SetPosition(cornerWidth, cornerHeight)
		StretchAsset(pieces["c"], self.__Name("c"), middleWidth, middleHeight)
		pieces["t"].SetPosition(cornerWidth, 0)
		StretchAsset(pieces["t"], self.__Name("t"), middleWidth, cornerHeight)
		pieces["b"].SetPosition(cornerWidth, height - cornerHeight)
		StretchAsset(pieces["b"], self.__Name("b"), middleWidth, cornerHeight)
		pieces["l"].SetPosition(0, cornerHeight)
		StretchAsset(pieces["l"], self.__Name("l"), cornerWidth, middleHeight)
		pieces["r"].SetPosition(width - cornerWidth, cornerHeight)
		StretchAsset(pieces["r"], self.__Name("r"), cornerWidth, middleHeight)
		pieces["tl"].SetPosition(0, 0)
		pieces["tr"].SetPosition(width - cornerWidth, 0)
		pieces["bl"].SetPosition(0, height - cornerHeight)
		pieces["br"].SetPosition(width - cornerWidth, height - cornerHeight)


class ThreeSlicePanel(ui.Window):
	def __init__(self, prefix):
		ui.Window.__init__(self)
		self.prefix = prefix
		self.width = 0
		self.height = 0
		self.middle = CreateImage(self, prefix + "_m")
		self.left = CreateImage(self, prefix + "_l")
		self.right = CreateImage(self, prefix + "_r")

	def __del__(self):
		ui.Window.__del__(self)

	def SetPrefix(self, prefix):
		if prefix == self.prefix:
			return
		self.prefix = prefix
		LoadAsset(self.middle, prefix + "_m")
		LoadAsset(self.left, prefix + "_l")
		LoadAsset(self.right, prefix + "_r")
		self.__Layout()

	def SetSize(self, width, height):
		self.width = width
		self.height = height
		ui.Window.SetSize(self, width, height)
		self.__Layout()

	def __Layout(self):
		(capWidth, capHeight) = AssetSize(self.prefix + "_l")
		capWidth = int(round(capWidth * float(self.height) / max(1, capHeight)))
		capWidth = min(capWidth, self.width // 2)
		self.left.SetPosition(0, 0)
		StretchAsset(self.left, self.prefix + "_l", capWidth, self.height)
		self.middle.SetPosition(capWidth, 0)
		StretchAsset(self.middle, self.prefix + "_m", max(0, self.width - capWidth * 2), self.height)
		self.right.SetPosition(self.width - capWidth, 0)
		StretchAsset(self.right, self.prefix + "_r", capWidth, self.height)


class SlotPanel(NineSlicePanel):
	FOCUS_STYLES = ("field_focus", "chip_active", "card_active", "race_warrior", "race_assassin", "race_sura", "race_shaman")
	DIM_STYLES = ("card_empty",)

	def __init__(self, style = "field"):
		self.style = style
		NineSlicePanel.__init__(self, self.__Prefix(style))

	def __Prefix(self, style):
		if style in self.FOCUS_STYLES:
			return "slot_focus"
		if style in self.DIM_STYLES:
			return "slot_dim"
		return "slot"

	def SetStyle(self, style):
		self.style = style
		self.SetPrefix(self.__Prefix(style))

	def GetStyle(self):
		return self.style


class WindowPanel(ui.Window):
	def __init__(self):
		ui.Window.__init__(self)
		self.fill = CreateImage(self, "ornate_fill")
		self.frame = NineSlicePanel("window")
		self.frame.SetParent(self)
		self.frame.AddFlag("not_pick")
		self.frame.Show()

	def __del__(self):
		ui.Window.__del__(self)

	def SetSize(self, width, height):
		ui.Window.SetSize(self, width, height)
		self.fill.SetPosition(1, 1)
		StretchAsset(self.fill, "ornate_fill", max(1, width - 2), max(1, height - 2))
		self.frame.SetSize(width, height)


def RestyleBoard(board, width, height):
	for name in ("Corners", "Lines"):
		for piece in getattr(board, name, []):
			piece.Hide()
	base = getattr(board, "Base", None)
	if base:
		LoadAsset(base, "ornate_fill")
		if hasattr(base, "SetRenderingRect"):
			base.SetRenderingRect(0.0, 0.0, 0.0, 0.0)
		base.SetPosition(1, 1)
		StretchAsset(base, "ornate_fill", max(1, width - 2), max(1, height - 2))
		base.Show()
	frame = NineSlicePanel("window")
	frame.SetParent(board)
	frame.AddFlag("not_pick")
	frame.SetPosition(0, 0)
	frame.SetSize(width, height)
	frame.Show()
	return frame


TITLE_TEXT_Y = 7


def RestyleTitleBar(titleBar, titleText = None):
	for name in ("imgLeft", "imgCenter", "imgRight"):
		image = getattr(titleBar, name, None)
		if image:
			image.Hide()
	button = getattr(titleBar, "btnClose", None)
	if button:
		button.SetUpVisual(Asset("close_30_normal"))
		button.SetOverVisual(Asset("close_30_over"))
		button.SetDownVisual(Asset("close_30_down"))
		if hasattr(button, "SetButtonScale"):
			button.SetButtonScale(1.0, 1.0)
		button.SetSize(30, 30)
	if titleText:
		titleText.SetPackedFontColor(GOLD_COLOR)
		titleText.SetPosition(titleText.GetLocalPosition()[0], TITLE_TEXT_Y)


def CreateOrnatePanel(parent, x, y, width, height, pickable = True):
	panel = OrnatePanel()
	panel.SetParent(parent)
	if not pickable:
		panel.AddFlag("not_pick")
	panel.SetPosition(x, y)
	panel.SetSize(width, height)
	panel.Show()
	return panel


def RestyleThinBoardOrnate(board, width, height):
	HideThinBoardFrame(board)
	frame = OrnatePanel(False)
	frame.SetParent(board)
	frame.AddFlag("not_pick")
	frame.SetPosition(0, 0)
	frame.SetSize(width, height)
	frame.Show()
	base = getattr(board, "Base", None)
	if base:
		base.SetColor(Color(*ORNATE_BASE_COLOR))
		base.SetPosition(ORNATE_INSET, ORNATE_INSET)
		base.SetSize(width - ORNATE_INSET * 2, height - ORNATE_INSET * 2)
		base.Show()
	return frame


SLOT_STYLES = ("field", "field_focus", "chip_active", "card", "card_active", "card_empty")


def CreatePanel(parent, style, x, y, width, height, pickable = True):
	panel = SlotPanel(style) if style in SLOT_STYLES else Panel(style)
	panel.SetParent(parent)
	if not pickable:
		panel.AddFlag("not_pick")
	panel.SetPosition(x, y)
	panel.SetSize(width, height)
	panel.Show()
	return panel


class Pressable(ui.Window):
	def __init__(self, width, height):
		ui.Window.__init__(self)
		self.SetSize(width, height)
		ui.EnableSingleTap(self)
		self.eventFunction = None
		self.eventArguments = ()
		self.pressed = False
		self.hovered = False
		self.enabled = True
		self.SetTouchTapEvent(ui.__mem_func__(self.__OnTap))

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		self.eventFunction = None
		self.eventArguments = ()
		self.eventTouchTap = None
		self.Hide()

	def SetEvent(self, function, *arguments):
		self.eventFunction = function
		self.eventArguments = arguments

	def SetEnabled(self, enabled):
		self.enabled = enabled
		self.RefreshVisual()

	def Run(self):
		import app
		now = app.GetTime()	# AKRAVON: cift tetiklenme (dokunma + fare) yutulur
		if now - getattr(self, "lastRunTime", -10.0) < getattr(self, "minGap", 0.35):
			return
		self.lastRunTime = now
		if self.enabled and self.eventFunction:
			self.eventFunction(*self.eventArguments)

	def RefreshVisual(self):
		pass

	def GetVisualState(self):
		if self.pressed:
			return "down"
		if self.hovered:
			return "over"
		return "normal"

	def __OnTap(self, x, y):
		self.Run()

	def OnMouseOverIn(self):
		self.hovered = True
		self.RefreshVisual()

	def OnMouseOverOut(self):
		self.hovered = False
		self.pressed = False
		self.RefreshVisual()

	def OnMouseLeftButtonDown(self):
		self.pressed = True
		self.RefreshVisual()
		return True

	def OnMouseLeftButtonUp(self):
		wasPressed = self.pressed
		self.pressed = False
		self.RefreshVisual()
		if wasPressed and self.IsIn():
			self.Run()
		return True


ROUND_BUTTON_DISC = "hud_disc"
# disk + ikon tonu: normal, ustunde, basili, devre disi
ROUND_BUTTON_TINTS = {
	"normal" : (0.92, 0.92, 0.92, 1.0),
	"over" : (1.0, 1.0, 1.0, 1.0),
	"down" : (0.66, 0.6, 0.5, 1.0),
	"disabled" : (0.45, 0.45, 0.45, 1.0),
}


class RoundIconButton(Pressable):
	# yuvarlak dokunma dugmesi: HUD diski (olcekli) + ortada ikon ya da cocuk (eksi cubugu gibi)
	def __init__(self, size, iconName = None):
		Pressable.__init__(self, size, size)
		self.disc = CreateImage(self, ROUND_BUTTON_DISC)
		(discWidth, discHeight) = AssetSize(ROUND_BUTTON_DISC)
		self.disc.SetScale(float(size) / discWidth, float(size) / discHeight)
		self.icon = CreateCenteredImage(self, iconName, size // 2, size // 2) if iconName else None
		self.decor = []
		self.RefreshVisual()

	def Destroy(self):
		Pressable.Destroy(self)
		self.disc = None
		self.icon = None
		self.decor = []

	def AddBar(self, width, height, color = None):
		size = self.GetWidth()
		bar = CreateBar(self, color or GOLD_COLOR, (size - width) // 2, (size - height) // 2, width, height)
		self.decor.append(bar)
		return bar

	def RefreshVisual(self):
		disc = getattr(self, "disc", None)
		if not disc:
			return
		(red, green, blue, alpha) = ROUND_BUTTON_TINTS[self.GetVisualState() if self.enabled else "disabled"]
		wndMgr.SetDiffuseColor(disc.hWnd, red, green, blue, alpha)
		if self.icon:
			wndMgr.SetDiffuseColor(self.icon.hWnd, red, green, blue, alpha)


class StateImageButton(Pressable):
	# hazir durum gorselli dugme (<ad>_normal / _over / _down [/ _disable]: close_28, messenger_add ...), istenirse
	# genislige olcekli
	def __init__(self, baseName, width = None):
		(assetWidth, assetHeight) = AssetSize(baseName + "_normal")
		self.scale = float(width) / max(1, assetWidth) if width else 1.0
		Pressable.__init__(self, int(round(assetWidth * self.scale)), int(round(assetHeight * self.scale)))
		self.baseName = baseName
		self.image = CreateImage(self, baseName + "_normal")
		self.RefreshVisual()

	def Destroy(self):
		Pressable.Destroy(self)
		self.image = None

	def RefreshVisual(self):
		image = getattr(self, "image", None)
		if not image:
			return
		state = self.GetVisualState()
		if not self.enabled:
			state = "disable" if HasAsset(self.baseName + "_disable") else "down"
		LoadAsset(image, "%s_%s" % (self.baseName, state))
		image.SetScale(self.scale * ASSET_SCALE, self.scale * ASSET_SCALE)


class PrimaryButton(Pressable):
	def __init__(self, sizeKey, labelName):
		(width, height) = [int(value) for value in sizeKey.split("_")[1].split("x")]
		Pressable.__init__(self, width, height)
		self.sizeKey = sizeKey
		self.background = CreateImage(self, "%s_normal" % sizeKey)
		self.label = CreateImage(self, labelName)
		self.SetLabel(labelName)

	def Destroy(self):
		Pressable.Destroy(self)
		self.background = None
		self.label = None

	def SetLabel(self, labelName):
		LoadAsset(self.label, labelName)
		(width, height) = ImageSize(self.label)
		self.label.SetPosition((self.GetWidth() - width) // 2, (self.GetHeight() - height) // 2)

	def RefreshVisual(self):
		state = self.GetVisualState() if self.enabled else "down"
		LoadAsset(self.background, "%s_%s" % (self.sizeKey, state))


class PillButton(Pressable):
	def __init__(self, width, height, text, iconName = None, style = "pill", textColor = None, font = FONT_MEDIUM):
		Pressable.__init__(self, width, height)
		if style == "pill":
			self.panel = ThreeSlicePanel("secondary")
			self.panel.SetParent(self)
			self.panel.AddFlag("not_pick")
			self.panel.SetSize(width, height)
			self.panel.Show()
			textColor = textColor or BRIGHT_COLOR
		else:
			self.panel = CreatePanel(self, style, 0, 0, width, height, False)
		self.icon = None
		contentX = width // 2
		if iconName:
			self.icon = CreateImage(self, iconName)
			(iconWidth, iconHeight) = ImageSize(self.icon)
			textWidth = TextWidth(text, font)
			startX = (width - iconWidth - 6 - textWidth) // 2
			self.icon.SetPosition(startX, (height - iconHeight) // 2)
			contentX = startX + iconWidth + 6 + textWidth // 2
		self.textColor = textColor or SOFT_COLOR
		self.text = CreateText(self, text, contentX, height // 2 - 9, self.textColor, font, "center")

	def Destroy(self):
		Pressable.Destroy(self)
		self.panel = None
		self.icon = None
		self.text = None

	def SetText(self, text):
		self.text.SetText(text)

	def RefreshVisual(self):
		if getattr(self, "text", None):
			self.text.SetPackedFontColor(GOLD_COLOR if self.pressed or self.hovered else self.textColor)
		panel = getattr(self, "panel", None)
		if isinstance(panel, ThreeSlicePanel):
			state = self.GetVisualState()
			panel.SetPrefix("secondary" if state == "normal" else "secondary_%s" % state)


class TextButton(Pressable):
	def __init__(self, text, iconName = None, color = SOFT_COLOR, font = FONT_NORMAL, height = 36):
		textWidth = TextWidth(text, font)
		iconWidth = 0
		if iconName:
			iconWidth = 22
		Pressable.__init__(self, textWidth + iconWidth + 4, height)
		self.icon = None
		x = 0
		if iconName:
			self.icon = CreateImage(self, iconName)
			(width, iconHeight) = ImageSize(self.icon)
			self.icon.SetPosition(0, (height - iconHeight) // 2)
			x = iconWidth
		self.text = CreateText(self, text, x, height // 2 - 8, color, font)

	def Destroy(self):
		Pressable.Destroy(self)
		self.icon = None
		self.text = None


class Toggle(Pressable):
	def __init__(self, text, value = False):
		textWidth = TextWidth(text, FONT_MEDIUM)
		Pressable.__init__(self, 44 + 10 + textWidth, 30)
		self.value = bool(value)
		self.image = CreateImage(self, "toggle_off", 0, 2)
		self.text = CreateText(self, text, 54, 6, SOFT_COLOR, FONT_MEDIUM)
		self.onChange = None
		self.SetEvent(ui.__mem_func__(self.__Flip))
		self.RefreshVisual()

	def Destroy(self):
		Pressable.Destroy(self)
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
			LoadAsset(self.image, "toggle_on" if self.value else "toggle_off")


class SegmentedControl(ui.Window):
	def __init__(self, width, height, labels):
		ui.Window.__init__(self)
		self.SetSize(width, height)
		self.panel = CreatePanel(self, "field", 0, 0, width, height, False)
		self.event = None
		self.selected = -1
		self.items = []
		count = len(labels)
		gap = 4
		itemWidth = (width - gap * (count + 1)) // count
		for (index, label) in enumerate(labels):
			item = Pressable(itemWidth, height - gap * 2)
			item.SetParent(self)
			item.SetPosition(gap + index * (itemWidth + gap), gap)
			item.highlight = CreatePanel(item, "field_focus", 0, 0, itemWidth, height - gap * 2, False)
			item.highlight.Hide()
			item.label = CreateText(item, label, itemWidth // 2, (height - gap * 2) // 2 - 9, MUTED_COLOR, FONT_MEDIUM, "center")
			item.SetEvent(ui.__mem_func__(self.__OnSelect), index)
			item.Show()
			self.items.append(item)

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		for item in self.items:
			item.highlight = None
			item.label = None
			item.Destroy()
		self.items = []
		self.event = None
		self.panel = None

	def SetEvent(self, function):
		self.event = function

	def __OnSelect(self, index):
		if self.event:
			self.event(index)

	def SetSelected(self, index):
		if index == self.selected:
			return
		self.selected = index
		for (position, item) in enumerate(self.items):
			if position == index:
				item.highlight.Show()
				item.label.SetPackedFontColor(GOLD_COLOR)
			else:
				item.highlight.Hide()
				item.label.SetPackedFontColor(MUTED_COLOR)


class TopBar(object):
	def __init__(self, parent, screenWidth, backText, titleName, metaText, backEvent):
		self.backButton = PillButton(TextWidth(backText, FONT_MEDIUM) + 56, 48, backText, "icon_back")
		self.backButton.SetParent(parent)
		self.backButton.SetPosition(24, 18)
		self.backButton.SetEvent(backEvent)
		self.backButton.Show()
		self.title = CreateCenteredImage(parent, titleName, screenWidth // 2, 18 + 24)
		self.meta = CreateText(parent, metaText, screenWidth - 24, 33, LABEL_COLOR, FONT_NORMAL, "right", True)

	def Destroy(self):
		if self.backButton:
			self.backButton.Destroy()
		self.backButton = None
		self.title = None
		self.meta = None

	def SetMeta(self, text):
		if self.meta:
			self.meta.SetText(text)


class StatRow(object):
	def __init__(self, parent, label, x, y, width):
		self.trackX = x + 76
		self.trackWidth = width - 76 - 40
		self.label = CreateText(parent, label, x, y, SOFT_COLOR, FONT_NORMAL)
		self.track = CreateBar(parent, TRACK_COLOR, self.trackX, y + 5, self.trackWidth, 8)
		self.fill = CreateBar(parent, FILL_COLOR, self.trackX, y + 5, 0, 8)
		self.value = CreateText(parent, "", x + width, y, BRIGHT_COLOR, FONT_NORMAL, "right")

	def SetValue(self, value, reference):
		ratio = 0.0
		if reference > 0:
			ratio = max(0.0, min(1.0, float(value) / reference))
		self.fill.SetSize(int(self.trackWidth * ratio), 8)
		self.value.SetText(str(value))


class PipRow(object):
	def __init__(self, parent, label, x, y, width, count = 6):
		self.label = CreateText(parent, label, x, y, SOFT_COLOR, FONT_NORMAL)
		pipX = x + 76
		available = width - 76 - 40
		gap = 3
		pipWidth = (available - gap * (count - 1)) // count
		self.pips = [CreateBar(parent, TRACK_COLOR, pipX + index * (pipWidth + gap), y + 4, pipWidth, 9) for index in range(count)]
		self.value = CreateText(parent, "", x + width, y, BRIGHT_COLOR, FONT_NORMAL, "right")

	def SetValue(self, value):
		for (index, pip) in enumerate(self.pips):
			pip.SetColor(PIP_ON_COLOR if index < value else TRACK_COLOR)
		self.value.SetText(str(value))


class Field(ui.Window):
	def __init__(self, width, height, label):
		ui.Window.__init__(self)
		self.SetSize(width, height)
		self.panel = CreatePanel(self, "field", 0, 0, width, height, False)
		self.label = CreateText(self, label, 14, 7, MUTED_COLOR, FONT_SMALL)
		self.editLine = None

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		self.panel = None
		self.label = None
		self.editLine = None

	def Attach(self, editLine, rightPadding = 14):
		self.editLine = editLine
		editLine.SetParent(self)
		editLine.SetPosition(14, self.GetHeight() - 26)
		editLine.SetSize(self.GetWidth() - 14 - rightPadding, 18)
		editLine.SetPackedFontColor(BRIGHT_COLOR)
		editLine.Show()

	def OnUpdate(self):
		if not self.editLine:
			return
		try:
			focused = self.editLine.IsFocus()
		except Exception:
			focused = False
		self.panel.SetStyle("field_focus" if focused else "field")

	def OnMouseLeftButtonDown(self):
		if self.editLine:
			ui.Window.SetFocus(self.editLine)
		return True

# AKRAVON: eski pencereleri yuklenirken bizim tasarimla giydir (uiAkravonKit)
try:
	import uiAkravonKit
	uiAkravonKit.Install()
except Exception:
	import dbg
	import traceback
	dbg.TraceError("uiAkravonKit.Install: " + traceback.format_exc())
