from __future__ import division
import app
import chr
import dbg
import net
import wndMgr
import localeInfo
import uiScriptLocale
import ui
import introSelect
import uiMobileCompat
import uiMobileStyle
import uiAkravon
import uiMobileCreate

from _weakref import proxy
from uiMobileCreate import HDImage, HDSize, CenterText, LeftText, PlateButton, CREAM, VALUE_COLOR, PIP_ON, PIP_OFF

# AKRAVON: karakter olusturma ekraniyla ayni dil - ayni cerceve, kart, panel ve alt serit (1280x720 mantiksal)
SIDE_MARGIN = uiMobileCreate.SIDE_MARGIN
FOOTER_HEIGHT = uiMobileCreate.FOOTER_HEIGHT
TITLE_POS = (86, 53)
CARD_X = 82
CARD_TOP = 150
CARD_STEP = 78
PORTRAIT_X = 18
CARD_TEXT_X = 84
FLAG_WIDTH = 28
PANEL_TOP = 150
PANEL_PAD = 32
PANEL_ROWS_Y = (124, 154, 184)
PANEL_DIVIDERS = (104, 212)
STAT_HEADING_Y = 226
STAT_ROWS_Y = (262, 292, 322, 352)
STAT_TRACK_X = 116
STAT_TRACK_SIZE = (120, 8)
DELETE_Y = 374
DOUBLE_TAP_TIME = 0.35
ROTATION_SPEED = 0.6
STAT_REFERENCE = 10.0

FONT_NAME = "Marcellus:24"
FONT_DETAIL = "Lato:17"
FONT_PANEL_NAME = "Marcellus:32"
FONT_ROW = "Marcellus:20"
FONT_HEADING = "Marcellus:22"
FONT_STAT = "Marcellus:20"
FONT_HINT = "Lato:17"
MUTED = uiMobileStyle.Color(168, 156, 132)
DIVIDER = uiMobileStyle.Color(176, 140, 74, 150)
DANGER = uiMobileStyle.Color(214, 112, 92)

JOB_NAMES = (localeInfo.JOB_WARRIOR, localeInfo.JOB_ASSASSIN, localeInfo.JOB_SURA, localeInfo.JOB_SHAMAN, getattr(localeInfo, "JOB_WOLFMAN", ""))
EMPIRE_NAMES = {
	net.EMPIRE_A : localeInfo.EMPIRE_A,
	net.EMPIRE_B : getattr(localeInfo, "EMPIRE_B", ""),
	net.EMPIRE_C : localeInfo.EMPIRE_C,
}
STAT_LABELS = ("MOBILE_STAT_HP", "MOBILE_STAT_INT", "MOBILE_STAT_STR", "MOBILE_STAT_DEX")
STAT_SLOTS = ("ACCOUNT_CHARACTER_SLOT_HTH", "ACCOUNT_CHARACTER_SLOT_INT", "ACCOUNT_CHARACTER_SLOT_STR", "ACCOUNT_CHARACTER_SLOT_DEX")
HIDDEN_NAMES = ("character_board", "left_button", "right_button", "name_warrior", "name_assassin", "name_sura", "name_shaman", "name_wolfman")


def Locale(name):
	return getattr(uiScriptLocale, name, name)


def FormatPlayTime(minutes):
	hours = minutes // 60
	if hours > 0:
		return Locale("MOBILE_HOURS") % hours
	return "%d %s" % (minutes, localeInfo.MINUTE)


def GetSlotInteger(slot, name):
	return net.GetAccountCharacterSlotDataInteger(slot, getattr(net, name))


def IsSlotUsed(slot):
	return slot >= 0 and bool(GetSlotInteger(slot, "ACCOUNT_CHARACTER_SLOT_ID"))


def PortraitName(race):
	try:
		import uiCharacter
		path = uiCharacter.FACE_IMAGE_DICT.get(race)
	except Exception:
		path = None
	if not path:
		return None
	return "portrait_%s" % path.replace("\\", "/").split("/")[-1].split(".")[0].lower()


def DescribeCharacter(slot):
	race = GetSlotInteger(slot, "ACCOUNT_CHARACTER_SLOT_RACE")
	level = GetSlotInteger(slot, "ACCOUNT_CHARACTER_SLOT_LEVEL")
	job = chr.RaceToJob(race)
	jobName = JOB_NAMES[job] if 0 <= job < len(JOB_NAMES) else ""
	return (job, Locale("MOBILE_CHARACTER_DETAIL") % (level, jobName))


class CharacterCard(uiMobileStyle.Pressable):
	def __init__(self, owner, slotIndex):
		(width, height) = HDSize("select_card_normal")
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.owner = proxy(owner)
		self.slotIndex = slotIndex
		self.state = None
		self.empireId = None
		self.image = HDImage(self, "select_card_empty")
		centerY = height // 2
		self.emblem = uiMobileStyle.CreateImage(self, "emblem_empty", PORTRAIT_X, centerY - 26)
		self.plus = uiMobileStyle.CreateCenteredImage(self, "icon_plus", PORTRAIT_X + 26, centerY)
		self.name = LeftText(self, "", CARD_TEXT_X, centerY - 11, CREAM, FONT_NAME)
		self.detail = LeftText(self, "", CARD_TEXT_X, centerY + 14, MUTED, FONT_DETAIL)
		self.flag = None
		self.SetEvent(ui.__mem_func__(self.__Notify))

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.owner = None
		self.image = None
		self.emblem = None
		self.plus = None
		self.name = None
		self.detail = None
		self.flag = None

	def __Notify(self):
		if self.owner:
			self.owner.OnSelectCard(self.slotIndex)

	def __SetBack(self, state):
		if state == self.state:
			return
		self.state = state
		uiAkravon.LoadImage(self.image, "select_card_%s" % state)
		self.image.SetScale(uiMobileCreate.HD_SCALE, uiMobileCreate.HD_SCALE)

	def __SetFlag(self, empireId):
		if not self.flag:
			self.flag = uiMobileStyle.CreateFlag(self, empireId, 0, 0, FLAG_WIDTH)
		elif empireId != self.empireId:
			uiMobileStyle.SetFlagImage(self.flag, empireId, FLAG_WIDTH)
		self.empireId = empireId
		flagHeight = uiMobileStyle.FlagHeight(self.flag, FLAG_WIDTH)
		self.flag.SetPosition(self.GetWidth() - 22 - FLAG_WIDTH, (self.GetHeight() - flagHeight) // 2)
		self.flag.Show()

	def Refresh(self, selected):
		slot = self.slotIndex
		centerY = self.GetHeight() // 2
		if not IsSlotUsed(slot):
			self.__SetBack("active" if selected else "empty")
			uiMobileStyle.LoadAsset(self.emblem, "emblem_empty")
			self.plus.Show()
			self.name.SetText(Locale("MOBILE_NEW_CHARACTER"))
			self.name.SetPackedFontColor(MUTED)
			self.name.SetPosition(CARD_TEXT_X, centerY - uiMobileCreate.FontHalf(FONT_NAME))
			self.detail.SetText("")
			if self.flag:
				self.flag.Hide()
			return

		(job, detail) = DescribeCharacter(slot)
		self.__SetBack("active" if selected else "normal")
		portrait = PortraitName(GetSlotInteger(slot, "ACCOUNT_CHARACTER_SLOT_RACE"))
		uiMobileStyle.LoadAsset(self.emblem, portrait if portrait else "emblem_%s" % uiMobileStyle.RaceKey(job))
		self.plus.Hide()
		self.name.SetText(net.GetAccountCharacterSlotDataString(slot, net.ACCOUNT_CHARACTER_SLOT_NAME))
		self.name.SetPackedFontColor(CREAM)
		self.name.SetPosition(CARD_TEXT_X, centerY - 11 - uiMobileCreate.FontHalf(FONT_NAME))
		self.detail.SetText(detail)
		self.__SetFlag(net.GetEmpireID())


class StatBar(object):
	def __init__(self, parent, label, y, width):
		centerY = y + STAT_TRACK_SIZE[1] // 2
		self.label = LeftText(parent, label, PANEL_PAD, centerY, CREAM, FONT_STAT)
		self.track = uiMobileStyle.CreateBar(parent, PIP_OFF, STAT_TRACK_X, y, STAT_TRACK_SIZE[0], STAT_TRACK_SIZE[1])
		self.fill = uiMobileStyle.CreateBar(parent, PIP_ON, STAT_TRACK_X, y, 0, STAT_TRACK_SIZE[1])
		self.value = LeftText(parent, "", width - PANEL_PAD, centerY, VALUE_COLOR, FONT_STAT, "right")

	def SetValue(self, value, reference):
		ratio = max(0.0, min(1.0, float(value) / reference)) if reference > 0 else 0.0
		self.fill.SetSize(int(STAT_TRACK_SIZE[0] * ratio), STAT_TRACK_SIZE[1])
		self.value.SetText(str(value))


class InfoPanel(object):
	def __init__(self, parent, x, y, deleteEvent):
		(width, height) = HDSize("select_panel")
		self.width = width
		panel = ui.Window()
		panel.SetParent(parent)
		panel.AddFlag("not_pick")
		panel.SetPosition(x, y)
		panel.SetSize(width, height)
		panel.Show()
		self.panel = panel
		self.items = [HDImage(panel, "select_panel")]
		for dividerY in PANEL_DIVIDERS:
			self.items.append(uiMobileStyle.CreateBar(panel, DIVIDER, 22, dividerY, width - 44, 1))

		self.name = uiMobileStyle.CreateText(panel, "", PANEL_PAD + 6, 30, CREAM, FONT_PANEL_NAME)
		self.detail = uiMobileStyle.CreateText(panel, "", PANEL_PAD + 6, 70, MUTED, FONT_DETAIL)
		self.values = {}
		for ((key, labelName), rowY) in zip((("empire", "MOBILE_EMPIRE"), ("guild", "MOBILE_GUILD"), ("playTime", "MOBILE_PLAYTIME")), PANEL_ROWS_Y):
			self.items.append(LeftText(panel, Locale(labelName), PANEL_PAD, rowY, CREAM, FONT_ROW))
			self.values[key] = LeftText(panel, "", width - PANEL_PAD, rowY, VALUE_COLOR, FONT_ROW, "right")
		self.items.append(uiMobileStyle.CreateText(panel, Locale("MOBILE_STATS_TITLE"), PANEL_PAD, STAT_HEADING_Y, CREAM, FONT_HEADING))
		self.stats = [StatBar(panel, Locale(labelName), rowY, width) for (labelName, rowY) in zip(STAT_LABELS, STAT_ROWS_Y)]

		self.deleteButton = uiMobileStyle.TextButton(Locale("MOBILE_DELETE_CHARACTER"), "icon_trash", DANGER, FONT_ROW, 30)
		self.deleteButton.SetParent(panel)
		self.deleteButton.SetPosition((width - self.deleteButton.GetWidth()) // 2, DELETE_Y)
		self.deleteButton.SetEvent(deleteEvent)
		self.deleteButton.Show()
		self.flag = None
		self.flagEmpire = None

	def Destroy(self):
		self.deleteButton.Destroy()
		self.deleteButton = None
		self.items = []
		self.stats = []
		self.panel.Hide()
		self.panel = None
		self.flag = None

	def Show(self):
		self.panel.Show()

	def Hide(self):
		self.panel.Hide()

	def __SetEmpire(self, empireId):
		name = EMPIRE_NAMES.get(empireId, "")
		value = self.values["empire"]
		value.SetText(name)
		if not self.flag:
			self.flag = uiMobileStyle.CreateFlag(self.panel, empireId, 0, 0, 22)
		elif empireId != self.flagEmpire:
			uiMobileStyle.SetFlagImage(self.flag, empireId, 22)
		self.flagEmpire = empireId
		flagHeight = uiMobileStyle.FlagHeight(self.flag, 22)
		textWidth = uiMobileStyle.TextWidth(name, FONT_ROW)
		self.flag.SetPosition(self.width - PANEL_PAD - textWidth - 8 - 22, PANEL_ROWS_Y[0] - flagHeight // 2)

	def Refresh(self, slot):
		(job, detail) = DescribeCharacter(slot)
		self.name.SetText(net.GetAccountCharacterSlotDataString(slot, net.ACCOUNT_CHARACTER_SLOT_NAME))
		self.detail.SetText(detail)
		self.__SetEmpire(net.GetEmpireID())
		guildName = net.GetAccountCharacterSlotDataString(slot, net.ACCOUNT_CHARACTER_SLOT_GUILD_NAME)
		self.values["guild"].SetText(guildName if guildName else Locale("MOBILE_NO_GUILD"))
		self.values["playTime"].SetText(FormatPlayTime(GetSlotInteger(slot, "ACCOUNT_CHARACTER_SLOT_PLAYTIME")))
		values = [GetSlotInteger(slot, name) for name in STAT_SLOTS]
		reference = max(STAT_REFERENCE, max(values) * 1.25)
		for (row, value) in zip(self.stats, values):
			row.SetValue(value, reference)


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


class MobileSelectCharacterWindow(introSelect.SelectCharacterWindow):
	def __init__(self, stream):
		self.mobileReady = False
		self.__ResetMobileState()
		introSelect.SelectCharacterWindow.__init__(self, stream)

	def __ResetMobileState(self):
		self.cards = []
		self.widgets = []
		self.decor = []
		self.infoPanel = None
		self.startButton = None
		self.startMode = None
		self.shownSlot = None
		self.rotationArea = None
		self.rotations = {}
		self.lastTapSlot = -1
		self.lastTapTime = 0.0

	def Open(self):
		self.CharacterRenderer = uiMobileCreate.MobileCharacterRenderer
		introSelect.SelectCharacterWindow.Open(self)
		if not self.dlgBoard:
			return
		try:
			if self.backGround:
				self.backGround.LoadImage(uiAkravon.Asset("create_background"))
				self.backGround.SetScale(float(wndMgr.GetScreenWidth()) / 1280.0, float(wndMgr.GetScreenHeight()) / 720.0)
			self.__ApplyMobileLayout()
			self.mobileReady = True
		except Exception as error:
			dbg.TraceError("MobileSelectCharacterWindow.Open - layout error: %s" % error)
		self.__SafeRefresh()

	def Close(self):
		for widget in self.cards + self.widgets:
			widget.Destroy()
		if self.rotationArea:
			self.rotationArea.Destroy()
		if self.infoPanel:
			self.infoPanel.Destroy()
		for item in self.decor:
			item.Hide()
		self.mobileReady = False
		self.__ResetMobileState()
		introSelect.SelectCharacterWindow.Close(self)

	def SelectSlot(self, index):
		introSelect.SelectCharacterWindow.SelectSlot(self, index)
		self.__SafeRefresh()

	def DeleteCharacter(self, index):
		introSelect.SelectCharacterWindow.DeleteCharacter(self, index)
		self.__SafeRefresh()

	def __SafeRefresh(self):
		if not self.mobileReady:
			return
		try:
			self.__RefreshMobile()
		except Exception as error:
			self.mobileReady = False
			dbg.TraceError("MobileSelectCharacterWindow - refresh error: %s" % error)

	def __Keep(self, widget):
		self.widgets.append(widget)
		return widget

	def __ApplyMobileLayout(self):
		board = self.dlgBoard
		screenWidth = wndMgr.GetScreenWidth()
		screenHeight = wndMgr.GetScreenHeight()
		footerY = screenHeight - FOOTER_HEIGHT
		(panelW, panelH) = HDSize("select_panel")
		panelX = screenWidth - SIDE_MARGIN - panelW
		(cardW, cardH) = HDSize("select_card_normal")

		for button in (self.btnStart, self.btnCreate, self.btnExit, self.btnDelete):
			if button:
				uiMobileStyle.ParkWindow(button)
		for name in HIDDEN_NAMES:
			child = board.GetChild2(name)
			if child:
				child.Hide()

		self.rotationArea = RotationArea(self)
		self.rotationArea.SetParent(board)
		self.rotationArea.SetPosition(CARD_X + cardW + 10, 60)
		self.rotationArea.SetSize(max(1, panelX - CARD_X - cardW - 20), footerY - 60)
		self.rotationArea.Show()

		self.decor.extend(uiMobileCreate.BuildScreenFrame(board, screenWidth, screenHeight, footerY))
		self.decor.append(HDImage(board, "select_title", TITLE_POS[0], TITLE_POS[1]))

		self.decor.extend(uiMobileCreate.BuildServerBadge(board, screenWidth - SIDE_MARGIN, 46))

		for index in range(self.SLOT_COUNT):
			card = CharacterCard(self, index)
			card.SetParent(board)
			card.SetPosition(CARD_X, CARD_TOP + index * CARD_STEP)
			card.Show()
			self.cards.append(card)

		self.infoPanel = InfoPanel(board, panelX, PANEL_TOP, ui.__mem_func__(self.__OnClickDelete))
		self.__BuildFooter(board, screenWidth, footerY)

	def __BuildFooter(self, board, screenWidth, footerY):
		back = self.__Keep(PlateButton("create_back", Locale("MOBILE_EXIT_GAME_UPPER"), None))
		buttonY = footerY + (FOOTER_HEIGHT - back.GetHeight()) // 2
		back.text.SetPosition(back.GetWidth() // 2 + 12, back.GetHeight() // 2 - uiMobileCreate.FontHalf(uiMobileCreate.FONT_BUTTON))
		back.SetParent(board)
		back.SetPosition(SIDE_MARGIN, buttonY)
		back.SetEvent(ui.__mem_func__(self.ExitSelect))
		back.Show()

		self.startButton = self.__Keep(PlateButton("create_button", Locale("MOBILE_ENTER_GAME_BUTTON")))
		self.startButton.SetParent(board)
		self.startButton.SetPosition(screenWidth - SIDE_MARGIN - self.startButton.GetWidth(), buttonY)
		self.startButton.SetEvent(ui.__mem_func__(self.__OnClickStart))
		self.startButton.Show()

	def __ShowOnlySelectedModel(self):
		# AKRAVON: yalniz secili karakterin 3D modeli gorunur
		for index in range(self.SLOT_COUNT):
			if chr.HasInstance(index):
				chr.SelectInstance(index)
				if index == self.slot:
					chr.Show()
				else:
					chr.Hide()

	def __RefreshMobile(self):
		self.shownSlot = self.slot
		self.__ShowOnlySelectedModel()
		for card in self.cards:
			card.Refresh(card.slotIndex == self.slot)

		used = IsSlotUsed(self.slot)
		mode = "start" if used else "create"
		if mode != self.startMode:
			self.startMode = mode
			self.startButton.text.SetText(Locale("MOBILE_ENTER_GAME_BUTTON" if used else "MOBILE_CREATE_BUTTON"))

		if not used:
			self.infoPanel.Hide()
			return
		self.infoPanel.Show()
		self.infoPanel.Refresh(self.slot)

	def __OnClickStart(self):
		if IsSlotUsed(self.slot):
			self.StartGame()
		else:
			self.CreateCharacter()

	def __OnClickDelete(self):
		if self.btnDelete:
			uiMobileStyle.RunButtonEvent(self.btnDelete)

	def OnSelectCard(self, slotIndex):
		now = app.GetTime()
		isDoubleTap = slotIndex == self.lastTapSlot and now - self.lastTapTime <= DOUBLE_TAP_TIME
		self.lastTapSlot = slotIndex
		self.lastTapTime = now

		if slotIndex != self.slot:
			self.SelectSlot(slotIndex)

		if not IsSlotUsed(slotIndex):
			self.CreateCharacter()
		elif isDoubleTap:
			self.StartGame()

	def OnUpdate(self):
		introSelect.SelectCharacterWindow.OnUpdate(self)
		if self.mobileReady and self.slot != self.shownSlot:
			self.__SafeRefresh()

	def RotateSelected(self, delta):
		if self.slot < 0 or not chr.HasInstance(self.slot):
			return
		rotation = (self.rotations.get(self.slot, 0.0) + delta) % 360.0
		self.rotations[self.slot] = rotation
		chr.SelectInstance(self.slot)
		chr.SetRotation(rotation)
