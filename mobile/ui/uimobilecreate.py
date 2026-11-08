from __future__ import division
import chr
import dbg
import event
import net
import wndMgr
import localeInfo
import uiScriptLocale
import ui
import introCreate
import uiMobileCompat
import uiMobileStyle
import uiAkravon

from _weakref import proxy

# AKRAVON: konumlar onayli referansin (skin/akravon_referans_olustur.png) 1280x720 olcegindeki yerleridir
CARD_X = 82
CARD_Y = (150, 214, 278, 342)
CARD_ICON = (50, 28)
CARD_TEXT_X = 92
GENDER_LABEL = (98, 419)
GENDER_X = (88, 216)
GENDER_Y = 441
CHIP_CENTER = (60, 20)
SHAPE_LABEL = (98, 498)
SHAPE_POS = (89, 521)
SHAPE_ARROW_WIDTH = 46
TITLE_POS = (86, 53)
PANEL_POS = (898, 150)
PANEL_ICON = (56, 44)
PANEL_TITLE = (84, 28)
DESCRIPTION_POS = (26, 86)
DESCRIPTION_SIZE = (252, 190)
DESCRIPTION_LINE = 22
FONT_DESC = "Lato:17"
STAT_LABEL_X = 26
STAT_ROW_Y = (284, 316, 348, 380)
STAT_PIP_X = 112
STAT_PIP_STEP = 22
STAT_PIP_SIZE = (19, 10)
STAT_PIP_COUNT = 6
STAT_VALUE_X = 274
FOOTER_HEIGHT = 99
SIDE_MARGIN = 82
BUTTON_GAP = 14
FRAME_INSET = 8
ROTATION_SPEED = 0.6
HD_SCALE = 0.5	# AKRAVON: parcalar 2 kat cozunurlukte, yarim olcekle cizilir
PANEL_DIVIDERS = (72, 266)
GOLD_LINE = uiMobileStyle.Color(176, 140, 74, 190)
MUTED_LABEL = uiMobileStyle.Color(168, 156, 132)
GOLD_CORNER = uiMobileStyle.Color(214, 176, 104)
FOOTER_FILL = uiMobileStyle.Color(8, 11, 22, 236)

FONT_CARD = "Marcellus:26"
FONT_LABEL = "Marcellus:22"
FONT_PANEL_TITLE = "Marcellus:30"
FONT_STAT = "Marcellus:20"
FONT_BUTTON = "Marcellus:24"
CREAM = uiMobileStyle.Color(232, 214, 176)
VALUE_COLOR = uiMobileStyle.Color(226, 218, 200)
PIP_ON = uiMobileStyle.Color(222, 183, 114)
PIP_OFF = uiMobileStyle.Color(49, 49, 49)

EMPIRE_NAMES = {
	net.EMPIRE_A : localeInfo.EMPIRE_A,
	net.EMPIRE_B : getattr(localeInfo, "EMPIRE_B", ""),
	net.EMPIRE_C : localeInfo.EMPIRE_C,
}
RACE_NAMES = (localeInfo.JOB_WARRIOR, localeInfo.JOB_ASSASSIN, localeInfo.JOB_SURA, localeInfo.JOB_SHAMAN)
STAT_LABELS = ("MOBILE_STAT_HP", "MOBILE_STAT_INT", "MOBILE_STAT_STR", "MOBILE_STAT_DEX")
HIDDEN_NAMES = ("character_board", "left_button", "right_button", "name_warrior", "name_assassin", "name_sura", "name_shaman", "name_wolfman", "right_line", "bottom_line", "left_line", "top_line")


def Locale(name):
	return getattr(uiScriptLocale, name, name)


def CenterText(parent, text, centerX, centerY, color, font):
	return uiMobileStyle.CreateText(parent, text, centerX, centerY - FontHalf(font), color, font, "center")


def LeftText(parent, text, x, centerY, color, font, align = "left"):
	return uiMobileStyle.CreateText(parent, text, x, centerY - FontHalf(font), color, font, align)


def FontHalf(font):
	return int(font.split(":")[1]) // 2 + 1


def HDImage(parent, name, x = 0, y = 0):
	image = uiAkravon.CreateImage(parent, name, x, y)
	image.SetScale(HD_SCALE, HD_SCALE)
	return image


def HDSize(name):
	probe = ui.ExpandedImageBox()
	probe.LoadImage(uiAkravon.Asset(name))
	return (int(probe.GetWidth() * HD_SCALE), int(probe.GetHeight() * HD_SCALE))


def CenteredIcon(parent, name, centerX, centerY):
	(width, height) = HDSize(name)
	return HDImage(parent, name, centerX - width // 2, centerY - height // 2)


def BuildServerBadge(board, rightX, centerY):
	# AKRAVON: sag ust sunucu rozeti - "Sunucu  (yesil nokta) Turkuaz  |  CH-1"
	server = ""
	channel = ""
	try:
		parts = [part.strip() for part in net.GetServerInfo().split(",") if part.strip()]
		if parts:
			server = parts[0]
		if len(parts) > 1:
			digits = "".join([c for c in parts[-1] if c.isdigit()])
			channel = "CH-%s" % digits if digits else parts[-1]
	except Exception:
		pass
	font = "Marcellus:20"
	label = Locale("MOBILE_SERVER_LABEL")
	pieces = [(label, MUTED_LABEL), (None, None), (server, CREAM), ("|", MUTED_LABEL), (channel, CREAM)]
	gap = 12
	dot = 10
	widths = [dot if text is None else uiMobileStyle.TextWidth(text, font) for (text, color) in pieces]
	pad = 18
	height = 38
	width = pad * 2 + sum(widths) + gap * (len(pieces) - 1)
	x = rightX - width
	items = []
	box = uiAkravon.Box(uiMobileStyle.Color(8, 11, 22, 225), GOLD_LINE)
	box.SetParent(board)
	box.SetPosition(x, centerY - height // 2)
	box.SetSize(width, height)
	box.Show()
	items.append(box)
	cx = x + pad
	for ((text, color), w) in zip(pieces, widths):
		if text is None:
			image = HDImage(board, "status_dot", cx + dot // 2 - 8, centerY - 8)
			items.append(image)
		else:
			items.append(LeftText(board, text, cx, centerY, color, font))
		cx += w + gap
	return items


def BuildScreenFrame(board, width, height, footerY):
	# AKRAVON: kesintisiz ince altin cerceve + kose susleri + ust/alt elmas; alt serit koyu zemin (secme ekrani da kullanir)
	items = [uiMobileStyle.CreateBar(board, FOOTER_FILL, 0, footerY, width, height - footerY)]
	i = FRAME_INSET
	for (x, y, w, h) in ((i, i, width - i * 2, 1), (i, height - i - 1, width - i * 2, 1),
			(i, i, 1, height - i * 2), (width - i - 1, i, 1, height - i * 2), (i, footerY, width - i * 2, 1)):
		items.append(uiMobileStyle.CreateBar(board, GOLD_LINE, x, y, w, h))
	c = 22
	for (cx, cy, dx, dy) in ((i, i, 1, 1), (width - i, i, -1, 1), (i, height - i, 1, -1), (width - i, height - i, -1, -1)):
		items.append(uiMobileStyle.CreateBar(board, GOLD_CORNER, cx if dx > 0 else cx - c, cy if dy > 0 else cy - 3, c, 3))
		items.append(uiMobileStyle.CreateBar(board, GOLD_CORNER, cx if dx > 0 else cx - 3, cy if dy > 0 else cy - c, 3, c))
	for y in (i, height - i - 1):
		diamond = uiAkravon.CreateImage(board, "diamond")
		diamond.SetPosition(width // 2 - diamond.GetWidth() // 2, y - diamond.GetHeight() // 2)
		items.append(diamond)
	return items


class ImageToggle(uiMobileStyle.Pressable):
	# referanstan kesilmis normal / secili zeminli dokunmatik kutu
	def __init__(self, prefix):
		self.prefix = prefix
		self.selected = False
		(width, height) = HDSize(prefix + "_normal")
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.image = HDImage(self, prefix + "_normal")

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.image = None

	def SetSelected(self, selected):
		if selected == self.selected:
			return
		self.selected = selected
		self.RefreshVisual()

	def RefreshVisual(self):
		if getattr(self, "image", None):
			uiAkravon.LoadImage(self.image, "%s_%s" % (self.prefix, "active" if self.selected else "normal"))
			self.image.SetScale(HD_SCALE, HD_SCALE)


class RaceTile(ImageToggle):
	def __init__(self, owner, slotIndex):
		ImageToggle.__init__(self, "create_card")
		self.owner = proxy(owner)
		self.slotIndex = slotIndex
		self.icon = CenteredIcon(self, "create_icon_%s" % uiMobileStyle.RACE_KEYS[slotIndex], CARD_ICON[0], CARD_ICON[1])
		self.name = LeftText(self, RACE_NAMES[slotIndex], CARD_TEXT_X, CARD_ICON[1], CREAM, FONT_CARD)
		self.SetEvent(ui.__mem_func__(self.__Notify))

	def Destroy(self):
		ImageToggle.Destroy(self)
		self.owner = None
		self.icon = None
		self.name = None

	def __Notify(self):
		if self.owner:
			self.owner.OnSelectRace(self.slotIndex)


class GenderChip(ImageToggle):
	def __init__(self, label, event, index):
		ImageToggle.__init__(self, "create_chip")
		self.label = CenterText(self, label, CHIP_CENTER[0], CHIP_CENTER[1], CREAM, FONT_LABEL)
		self.SetEvent(event, index)

	def Destroy(self):
		ImageToggle.Destroy(self)
		self.label = None


class PlateButton(uiMobileStyle.Pressable):
	# referanstan kesilmis dugme: normal / basili
	def __init__(self, prefix, text, textCenterX = None):
		self.prefix = prefix
		(width, height) = HDSize(prefix + "_normal")
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.image = HDImage(self, prefix + "_normal")
		if textCenterX is None:
			textCenterX = self.GetWidth() // 2
		self.text = CenterText(self, text, textCenterX, self.GetHeight() // 2, CREAM, FONT_BUTTON)

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.image = None
		self.text = None

	def RefreshVisual(self):
		if getattr(self, "image", None):
			uiAkravon.LoadImage(self.image, "%s_%s" % (self.prefix, "down" if self.pressed else "normal"))
			self.image.SetScale(HD_SCALE, HD_SCALE)


class TapArea(uiMobileStyle.Pressable):
	def __init__(self, width, height):
		uiMobileStyle.Pressable.__init__(self, width, height)


class ShapeSelector(ui.Window):
	def __init__(self):
		ui.Window.__init__(self)
		self.selected = 0
		self.event = None
		self.image = HDImage(self, "create_shape")
		(width, height) = HDSize("create_shape")
		self.SetSize(width, height)
		self.arrows = []
		for (x, delta) in ((0, -1), (width - SHAPE_ARROW_WIDTH, 1)):
			arrow = TapArea(SHAPE_ARROW_WIDTH, height)
			arrow.SetParent(self)
			arrow.SetPosition(x, 0)
			arrow.SetEvent(ui.__mem_func__(self.__Step), delta)
			arrow.Show()
			self.arrows.append(arrow)
		self.value = CenterText(self, "1", width // 2, height // 2, CREAM, FONT_LABEL)

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		for arrow in self.arrows:
			arrow.Destroy()
		self.arrows = []
		self.image = None
		self.value = None
		self.event = None

	def SetEvent(self, function):
		self.event = function

	def SetSelected(self, index):
		self.selected = index
		self.value.SetText(str(index + 1))

	def __Step(self, delta):
		self.selected = (self.selected + delta) % 2
		self.value.SetText(str(self.selected + 1))
		if self.event:
			self.event(self.selected)


class StatRow(object):
	def __init__(self, parent, label, y):
		centerY = y + STAT_PIP_SIZE[1] // 2
		self.label = LeftText(parent, label, STAT_LABEL_X, centerY, CREAM, FONT_STAT)
		self.pips = [uiMobileStyle.CreateBar(parent, PIP_OFF, STAT_PIP_X + index * STAT_PIP_STEP, y, STAT_PIP_SIZE[0], STAT_PIP_SIZE[1]) for index in range(STAT_PIP_COUNT)]
		self.value = LeftText(parent, "", STAT_VALUE_X, centerY, VALUE_COLOR, FONT_STAT, "right")

	def SetValue(self, value):
		for (index, pip) in enumerate(self.pips):
			pip.SetColor(PIP_ON if index < value else PIP_OFF)
		self.value.SetText(str(value))


class RotationArea(ui.Window):
	def __init__(self, owner):
		ui.Window.__init__(self)
		self.owner = proxy(owner)
		self.dragging = False
		self.lastX = 0

	def __del__(self):
		ui.Window.__del__(self)

	def Destroy(self):
		self.owner = None

	def OnMouseLeftButtonDown(self):
		self.dragging = True
		self.lastX = wndMgr.GetMousePosition()[0]
		return True

	def OnMouseLeftButtonUp(self):
		self.dragging = False
		return True

	def OnUpdate(self):
		if not self.dragging or not self.owner:
			return
		x = wndMgr.GetMousePosition()[0]
		delta = x - self.lastX
		self.lastX = x
		if delta:
			self.owner.RotateSelected(delta * ROTATION_SPEED)


class MobileCharacterRenderer(introCreate.CreateCharacterWindow.CharacterRenderer):
	def OnRender(self):
		screenWidth = wndMgr.GetScreenWidth()
		screenHeight = wndMgr.GetScreenHeight()
		viewportLeft = screenWidth * 0.094
		renderWidth = screenWidth - viewportLeft * 2

		introCreate.grp.ClearDepthBuffer()
		introCreate.grp.SetGameRenderState()
		introCreate.grp.PushState()
		introCreate.grp.SetOmniLight()
		introCreate.grp.SetViewport(viewportLeft / screenWidth, 0.0, renderWidth / screenWidth, 1.0)
		introCreate.app.SetCenterPosition(0.0, 0.0, 0.0)
		introCreate.app.SetCamera(1550.0, 15.0, 180.0, 95.0)
		introCreate.grp.SetPerspective(10.0, renderWidth / screenHeight, 1000.0, 3000.0)
		(x, y) = introCreate.app.GetCursorPosition()
		introCreate.grp.SetCursorPosition(x, y)
		chr.Deform()
		chr.Render()
		introCreate.grp.RestoreViewport()
		introCreate.grp.PopState()
		introCreate.grp.SetInterfaceRenderState()


class MobileCreateCharacterWindow(introCreate.CreateCharacterWindow):
	def __init__(self, stream):
		self.mobileReady = False
		self.__ResetMobileState()
		introCreate.CreateCharacterWindow.__init__(self, stream)

	def __ResetMobileState(self):
		self.tiles = []
		self.widgets = []
		self.decor = []
		self.statRows = []
		self.panel = None
		self.raceIcon = None
		self.raceName = None
		self.descriptionLines = []
		self.descriptionSlot = -1
		self.genderChips = []
		self.shapeControl = None
		self.nameField = None
		self.rotationArea = None
		self.rotations = {}
		self.shownState = None
		self.shownDescription = None
		self.descriptionWidth = 0

	def Open(self):
		self.CharacterRenderer = MobileCharacterRenderer
		introCreate.CreateCharacterWindow.Open(self)
		try:
			if self.backGround:
				self.backGround.LoadImage(uiAkravon.Asset("create_background"))
				self.backGround.SetScale(float(wndMgr.GetScreenWidth()) / 1280.0, float(wndMgr.GetScreenHeight()) / 720.0)
			self.__ApplyMobileLayout()
			self.mobileReady = True
		except Exception as error:
			dbg.TraceError("MobileCreateCharacterWindow.Open - layout error: %s" % error)
		self.__SafeRefresh()

	def __SafeRefresh(self):
		if not self.mobileReady:
			return
		try:
			self.__RefreshMobile()
			self.__ShowOnlySelectedModel()
			self.__RefreshDescriptionWidth()
		except Exception as error:
			self.mobileReady = False
			dbg.TraceError("MobileCreateCharacterWindow - refresh error: %s" % error)

	def Close(self):
		for widget in self.tiles + self.genderChips + self.widgets:
			widget.Destroy()
		if self.rotationArea:
			self.rotationArea.Destroy()
		for item in self.decor:
			item.Hide()
		if self.panel:
			self.panel.Hide()
		self.mobileReady = False
		self.__ResetMobileState()
		introCreate.CreateCharacterWindow.Close(self)

	def __Keep(self, widget):
		self.widgets.append(widget)
		return widget

	def __Decor(self, item):
		self.decor.append(item)
		return item

	def __ApplyMobileLayout(self):
		board = self.dlgBoard
		screenWidth = wndMgr.GetScreenWidth()
		screenHeight = wndMgr.GetScreenHeight()
		rightShift = screenWidth - 1280
		footerY = screenHeight - FOOTER_HEIGHT

		parked = [self.btnCancel, self.btnCreate, self.btnPrev, self.btnNext, self.btnLeft, self.btnRight] + list(self.genderButtonList) + list(self.shapeButtonList)
		for button in parked:
			if button:
				uiMobileStyle.ParkWindow(button)
		for name in HIDDEN_NAMES:
			child = board.GetChild2(name)
			if child:
				child.Hide()

		self.rotationArea = RotationArea(self)
		self.rotationArea.SetParent(board)
		self.rotationArea.SetPosition(CARD_X + 270, 60)
		self.rotationArea.SetSize(max(1, PANEL_POS[0] + rightShift - CARD_X - 280), footerY - 60)
		self.rotationArea.Show()

		self.__BuildFrame(board, screenWidth, screenHeight, footerY)
		self.__Decor(uiAkravon.CreateImage(board, "create_title", TITLE_POS[0], TITLE_POS[1]))

		self.decor.extend(BuildServerBadge(board, screenWidth - SIDE_MARGIN, 46))

		for (index, y) in enumerate(CARD_Y):
			tile = RaceTile(self, index)
			tile.SetParent(board)
			tile.SetPosition(CARD_X, y)
			tile.Show()
			self.tiles.append(tile)

		self.__Decor(uiMobileStyle.CreateText(board, Locale("MOBILE_GENDER"), GENDER_LABEL[0], GENDER_LABEL[1], CREAM, FONT_LABEL))
		for (index, labelName) in enumerate(("MOBILE_MALE", "MOBILE_FEMALE")):
			chip = GenderChip(Locale(labelName), ui.__mem_func__(self.__OnSelectGender), index)
			chip.SetParent(board)
			chip.SetPosition(GENDER_X[index], GENDER_Y)
			chip.Show()
			self.genderChips.append(chip)

		self.__Decor(uiMobileStyle.CreateText(board, Locale("MOBILE_SHAPE"), SHAPE_LABEL[0], SHAPE_LABEL[1], CREAM, FONT_LABEL))
		self.shapeControl = self.__Keep(ShapeSelector())
		self.shapeControl.SetParent(board)
		self.shapeControl.SetPosition(SHAPE_POS[0], SHAPE_POS[1])
		self.shapeControl.SetEvent(ui.__mem_func__(self.__OnSelectShape))
		self.shapeControl.Show()

		self.__BuildPanel(board, screenWidth - SIDE_MARGIN - HDSize("create_panel")[0], PANEL_POS[1])
		self.__BuildFooter(board, screenWidth, footerY)

	def __BuildFrame(self, board, width, height, footerY):
		self.decor.extend(BuildScreenFrame(board, width, height, footerY))

	def __BuildPanel(self, board, x, y):
		panel = ui.Window()
		panel.SetParent(board)
		panel.AddFlag("not_pick")
		panel.SetPosition(x, y)
		panel.SetSize(*HDSize("create_panel"))
		panel.Show()
		self.panel = panel
		self.__Decor(HDImage(panel, "create_panel"))
		for dividerY in PANEL_DIVIDERS:
			self.__Decor(uiMobileStyle.CreateBar(panel, uiMobileStyle.Color(176, 140, 74, 150), 22, dividerY, panel.GetWidth() - 44, 1))
		self.raceIcon = CenteredIcon(panel, "create_icon_warrior", PANEL_ICON[0], PANEL_ICON[1])
		self.raceName = uiMobileStyle.CreateText(panel, "", PANEL_TITLE[0], PANEL_TITLE[1], CREAM, FONT_PANEL_TITLE)

		# AKRAVON: aciklama oyunun 5 satirlik sayfali kutusu yerine panele sarilarak tam yazilir
		self.textBoard.Hide()
		if getattr(self, "descriptionBox", None):
			self.descriptionBox.Hide()
		self.descriptionLines = []
		self.descriptionSlot = -1

		for (labelName, rowY) in zip(STAT_LABELS, STAT_ROW_Y):
			self.statRows.append(StatRow(panel, Locale(labelName), rowY))

	def __RefreshDescriptionText(self, slot):
		if slot == self.descriptionSlot or not self.panel:
			return
		self.descriptionSlot = slot
		for line in self.descriptionLines:
			line.Hide()
		self.descriptionLines = uiMobileStyle.CreateWrappedText(self.panel, Locale("MOBILE_JOBDESC_%d" % slot),
			DESCRIPTION_POS[0], DESCRIPTION_POS[1], DESCRIPTION_SIZE[0], VALUE_COLOR, FONT_DESC, DESCRIPTION_LINE)

	def __BuildFooter(self, board, screenWidth, footerY):
		# AKRAVON: dugmeler ayni boyda, iki kenara esit uzakta; isim etiketi + alan ortada
		back = self.__Keep(PlateButton("create_back", Locale("MOBILE_CANCEL_CREATE_UPPER"), None))
		buttonY = footerY + (FOOTER_HEIGHT - back.GetHeight()) // 2
		centerY = footerY + FOOTER_HEIGHT // 2
		back.text.SetPosition(back.GetWidth() // 2 + 12, back.GetHeight() // 2 - FontHalf(FONT_BUTTON))
		back.SetParent(board)
		back.SetPosition(SIDE_MARGIN, buttonY)
		back.SetEvent(ui.__mem_func__(self.CancelCreate))
		back.Show()

		create = self.__Keep(PlateButton("create_button", Locale("MOBILE_CREATE_BUTTON")))
		create.SetParent(board)
		create.SetPosition(screenWidth - SIDE_MARGIN - create.GetWidth(), buttonY)
		create.SetEvent(ui.__mem_func__(self.CreateCharacter))
		create.Show()

		label = Locale("MOBILE_CHARACTER_NAME")
		(fieldW, fieldH) = HDSize("create_field")
		labelW = uiMobileStyle.TextWidth(label, FONT_LABEL)
		groupX = (screenWidth - (labelW + BUTTON_GAP + fieldW)) // 2
		self.__Decor(LeftText(board, label, groupX, centerY, CREAM, FONT_LABEL))
		self.nameField = self.__Keep(uiAkravon.Field(fieldW, fieldH, ""))
		self.nameField.box.Hide()
		self.__Decor(HDImage(self.nameField, "create_field"))
		self.nameField.SetParent(board)
		self.nameField.SetPosition(groupX + labelW + BUTTON_GAP, centerY - fieldH // 2)
		self.nameField.Attach(self.editCharacterName)
		self.nameField.Show()
		self.editCharacterName.SetTop()
		self.__InstallNameFocus()

	def __InstallNameFocus(self):
		editLine = self.editCharacterName
		field = proxy(self.nameField)

		def FocusWhenTouched():
			try:
				touched = editLine.IsIn() or field.IsIn()
			except ReferenceError:
				touched = False
			if editLine.IsFocus() or touched:
				ui.Window.SetFocus(editLine)

		editLine.SetFocus = FocusWhenTouched
		editLine.KillFocus()

	def __CurrentShape(self):
		try:
			return self.shapeList[self.gender][self.slot]
		except Exception:
			return 0

	def __RefreshMobile(self):
		slot = self.slot
		gender = self.gender
		shape = self.__CurrentShape()
		self.shownState = (slot, gender, shape)

		for tile in self.tiles:
			tile.SetSelected(tile.slotIndex == slot)
		for (index, chip) in enumerate(self.genderChips):
			chip.SetSelected(index == gender)
		self.shapeControl.SetSelected(shape)

		if not 0 <= slot < len(uiMobileStyle.RACE_KEYS):
			return

		iconName = "create_icon_%s" % uiMobileStyle.RACE_KEYS[slot]
		uiAkravon.LoadImage(self.raceIcon, iconName)
		self.raceIcon.SetScale(HD_SCALE, HD_SCALE)
		(iconW, iconH) = HDSize(iconName)
		self.raceIcon.SetPosition(PANEL_ICON[0] - iconW // 2, PANEL_ICON[1] - iconH // 2)
		self.raceName.SetText(RACE_NAMES[slot])
		self.__RefreshDescriptionText(slot)
		for (row, value) in zip(self.statRows, self.stat):
			row.SetValue(value)

	def __RefreshDescriptionWidth(self):
		if self.descIndex == self.shownDescription:
			return
		self.shownDescription = self.descIndex
		if hasattr(event, "SetEventSetWidth"):
			event.SetEventSetWidth(self.descIndex, self.descriptionWidth)

	def __ShowOnlySelectedModel(self):
		if not 0 <= self.slot < len(uiMobileStyle.RACE_KEYS):
			return
		for gender in range(len(self.shapeList)):
			for race in range(len(uiMobileStyle.RACE_KEYS)):
				characterId = introCreate.BASE_CHR_ID + introCreate.GetRealIDnSlot(gender, race)
				if not chr.HasInstance(characterId):
					continue
				chr.SelectInstance(characterId)
				if gender == self.gender and race == self.slot:
					chr.Show()
				else:
					chr.Hide()
		selectedId = introCreate.BASE_CHR_ID + introCreate.GetRealIDnSlot(self.gender, self.slot)
		if chr.HasInstance(selectedId):
			chr.SelectInstance(selectedId)

	def OnSelectRace(self, slotIndex):
		self._CreateCharacterWindow__SelectSlot(slotIndex)
		self.__SafeRefresh()

	def __OnSelectGender(self, index):
		if index != self.gender:
			self._CreateCharacterWindow__SelectGender(index)
		self.__SafeRefresh()

	def __OnSelectShape(self, index):
		self._CreateCharacterWindow__SelectShape(index)
		self.__SafeRefresh()

	def RotateSelected(self, delta):
		chrId = introCreate.BASE_CHR_ID + introCreate.GetRealIDnSlot(self.gender, self.slot)
		if not chr.HasInstance(chrId):
			return
		rotation = (self.rotations.get(chrId, 0.0) + delta) % 360.0
		self.rotations[chrId] = rotation
		chr.SelectInstance(chrId)
		chr.SetRotation(rotation)

	def OnUpdate(self):
		introCreate.CreateCharacterWindow.OnUpdate(self)
		if not self.mobileReady:
			return
		if (self.slot, self.gender, self.__CurrentShape()) != self.shownState or self.descIndex != self.shownDescription:
			self.__SafeRefresh()
