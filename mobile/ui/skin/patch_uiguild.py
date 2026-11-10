# -*- coding: utf-8 -*-
# AKRAVON: orijinal uiguild.py'yi (10_root_1.zip) kendi tasarimimiza uyarlar -> ui/orijinalden_degisen/uiguild.py
# Tahta / Uyeler / Rutbeler: buyuk satirlar (parmak), yalniz dolu satirlar, parmakla surukleyerek kaydirma, dokununca islem.
# Betik cp1254 (latin1 ile tasinir); eklenen kodda yalnizca ASCII.
import ast
import os
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT_ZIP = 'C:/AkravonTest/ios_dosyalar/mobile/10_root_1.zip'
DST = os.path.join(HERE, '..', 'orijinalden_degisen', 'uiguild.py')

text = zipfile.ZipFile(ROOT_ZIP).read('uiguild.py').decode('latin1').replace('\r\n', '\n')


def Rep(a, b, n = 1):
	global text
	assert text.count(a) == n, (text.count(a), a[:70])
	text = text.replace(a, b)


def Region(start, end, new):
	global text
	i = text.index(start)
	j = text.index(end, i)
	text = text[:i] + new + text[j:]


Rep('DISABLE_GUILD_SKILL = False', '''import uiMobileStyle
GD_IMG = "d:/ymir work/ui/akravon_legacy/"
C_TEXT = 0xfff5efe6
C_DIM = 0xffc7bda8
C_GOLD = 0xffe3c280

DISABLE_GUILD_SKILL = False''')

Rep('''		if self.isDown:
			grp.SetColor(ui.WHITE_COLOR)
		else:
			grp.SetColor(ui.HALF_WHITE_COLOR)''', '''		if self.isDown:
			grp.SetColor(0x66e3c280)
		else:
			grp.SetColor(0x33e3c280)''')

HELPERS = '''class DragList(ui.Window):
	# liste alani: parmagi surukle = kaydir (satir adimi), kisa dokunus = onTap(gorunenSatir, mutlakSatir, x)
	def __init__(self, parent, x, y, width, height, rowStep, onScroll, onTap):
		ui.Window.__init__(self)
		self.SetParent(parent)
		self.SetPosition(x, y)
		self.SetSize(width, height)
		self.rowStep = rowStep
		self.onScroll = onScroll
		self.onTap = onTap
		self.offset = 0
		self.maxOffset = 0
		self.dragging = False
		self.moved = False
		self.dragY = 0
		self.dragOffset = 0
		self.Show()

	def __del__(self):
		ui.Window.__del__(self)

	def SetRange(self, maxOffset):
		self.maxOffset = max(0, maxOffset)
		if self.offset > self.maxOffset:
			self.offset = self.maxOffset

	def OnMouseLeftButtonDown(self):
		self.dragging = True
		self.moved = False
		self.dragY = wndMgr.GetMousePosition()[1]
		self.dragOffset = self.offset
		return True

	def OnUpdate(self):
		if not self.dragging:
			return
		delta = self.dragY - wndMgr.GetMousePosition()[1]
		if abs(delta) > 12:
			self.moved = True
		offset = max(0, min(self.maxOffset, self.dragOffset + int(round(delta / float(self.rowStep)))))
		if offset != self.offset:
			self.offset = offset
			self.onScroll(offset)

	def OnMouseLeftButtonUp(self):
		if self.dragging and not self.moved:
			(mouseX, mouseY) = wndMgr.GetMousePosition()
			(left, top) = self.GetGlobalPosition()
			row = int((mouseY - top) // self.rowStep)
			if row >= 0:
				self.onTap(row, self.offset + row, mouseX - left)
		self.dragging = False
		return True

class EditableTextSlot(ui.ImageBox):
	def __init__(self, parent, x, y, image = GD_IMG + "gd_slot_200x48.png"):
		ui.ImageBox.__init__(self)
		self.SetParent(parent)
		self.SetPosition(x, y)
		self.LoadImage(image)

		self.mouseReflector = MouseReflector(self)
		self.mouseReflector.SetSize(self.GetWidth(), self.GetHeight())

		self.Enable = True
		self.textLine = uiMobileStyle.CreateText(self, "", self.GetWidth() / 2, (self.GetHeight() - 17) / 2 - 1, C_TEXT, "Lato:17", "center")
		self.event = lambda *arg: None
		self.arg = 0
		self.Show()

		self.mouseReflector.UpdateRect()

	def __del__(self):
		ui.ImageBox.__del__(self)

	def SetText(self, text):
		self.textLine.SetText(text)

	def SetEvent(self, event, arg):
		self.event = event
		self.arg = arg

	def Disable(self):
		self.Enable = False

	def OnMouseOverIn(self):
		if not self.Enable:
			return
		self.mouseReflector.Show()

	def OnMouseOverOut(self):
		if not self.Enable:
			return
		self.mouseReflector.Hide()

	def OnMouseLeftButtonDown(self):
		if not self.Enable:
			return
		self.mouseReflector.Down()

	def OnMouseLeftButtonUp(self):
		if not self.Enable:
			return
		self.mouseReflector.Up()
		self.event(self.arg)

class CheckBox(ui.ImageBox):
	def __init__(self, parent, x, y, event, filename = GD_IMG + "gd_slot_52x48.png"):
		ui.ImageBox.__init__(self)
		self.SetParent(parent)
		self.SetPosition(x, y)
		self.LoadImage(filename)

		self.mouseReflector = MouseReflector(self)
		self.mouseReflector.SetSize(self.GetWidth(), self.GetHeight())

		image = ui.MakeImageBox(self, GD_IMG + "gd_check.png", 0, 0)
		image.AddFlag("not_pick")
		image.SetWindowHorizontalAlignCenter()
		image.SetWindowVerticalAlignCenter()
		image.Hide()
		self.Enable = True
		self.image = image
		self.event = event
		self.Show()

		self.mouseReflector.UpdateRect()

	def __del__(self):
		ui.ImageBox.__del__(self)

	def SetCheck(self, flag):
		if flag:
			self.image.Show()
		else:
			self.image.Hide()

	def Disable(self):
		self.Enable = False

	def OnMouseOverIn(self):
		if not self.Enable:
			return
		self.mouseReflector.Show()

	def OnMouseOverOut(self):
		if not self.Enable:
			return
		self.mouseReflector.Hide()

	def OnMouseLeftButtonDown(self):
		if not self.Enable:
			return
		self.mouseReflector.Down()

	def OnMouseLeftButtonUp(self):
		if not self.Enable:
			return
		self.mouseReflector.Up()
		self.event()

class AkrComboBox(ui.ComboBox):
	def __init__(self):
		ui.ComboBox.__init__(self)
		self.textLine.SetPackedFontColor(C_TEXT)
		self.textLine.SetFontName("Lato:15")
		listBox = self.listBox

		def ListRender():
			x, y = listBox.GetGlobalPosition()
			y -= listBox.TEMPORARY_PLACE
			h = listBox.height + listBox.TEMPORARY_PLACE * 2
			grp.SetColor(0xf2101830)
			grp.RenderBar(x, y, listBox.width, h)
			grp.SetColor(0xffd6ac60)
			grp.RenderBox(x, y, listBox.width, h)
			ui.ListBox.OnRender(listBox)
		listBox.OnRender = ListRender

	def OnRender(self):
		x, y = self.GetGlobalPosition()
		grp.SetColor(0xf00a1022)
		grp.RenderBar(x, y, self.width, self.height)
		if self.isOver and self.enable:
			grp.SetColor(0xffd6ac60)
		else:
			grp.SetColor(0xff70562e)
		grp.RenderBox(x, y, self.width, self.height)

'''
Region('class EditableTextSlot(ui.ImageBox):', 'class ChangeGradeNameDialog(ui.ScriptWindow):', HELPERS)
Rep('self.SetPosition(xMouse - self.GetWidth()/2, yMouse + 50)', 'self.SetPosition((wndMgr.GetScreenWidth() - self.GetWidth()) / 2, (wndMgr.GetScreenHeight() - self.GetHeight()) / 2)')

COMMENT = '''class CommentSlot(ui.Window):

	TEXT_LIMIT = 40

	def __init__(self):
		ui.Window.__init__(self)

		self.slotImage = ui.MakeImageBox(self, GD_IMG + "gd_slot_300x44.png", 0, 0)
		self.slotImage.AddFlag("not_pick")

		self.slotSimpleText = uiMobileStyle.CreateText(self, "", 12, 13, C_TEXT, "Lato:16", "left")

		self.SetSize(self.slotImage.GetWidth(), self.slotImage.GetHeight())
		self.len = 0

	def SetText(self, text):
		self.len = len(text)
		if len(text) > self.TEXT_LIMIT:
			limitText = grpText.GetSplitingTextLine(text, self.TEXT_LIMIT-3, 0)
			self.slotSimpleText.SetText(limitText + "...")
		else:
			self.slotSimpleText.SetText(text)

'''
Region('class CommentSlot(ui.Window):', 'class GuildWindow(ui.ScriptWindow):', COMMENT)
Rep('MEMBER_LINE_COUNT = 13', 'MEMBER_LINE_COUNT = 7')

BOARD = '''	def __MakeBoardPage(self):

		page = self.pageWindow["BOARD"]

		page.boardDict = {}
		page.boardRows = {}
		page.boardColX = [4, 22, 148, 456]
		page.boardNotice = {}
		page.boardCount = 0

		for i in xrange(12):

			## NoticeMark
			noticeMarkImage = ui.MakeImageBox(page, "d:/ymir work/ui/game/guild/notice_mark.sub", 4, 0)
			noticeMarkImage.Hide()
			page.Children.append(noticeMarkImage)

			## Name
			nameSlotImage = ui.MakeImageBox(page, GD_IMG + "gd_slot_120x44.png", 22, 0)
			nameSlot = uiMobileStyle.CreateText(nameSlotImage, "", 60, 13, C_GOLD, "Lato:16", "center")
			page.Children.append(nameSlotImage)
			page.Children.append(nameSlot)

			## Delete Button
			deleteButton = ui.MakeButton(page, 456, 0, localeInfo.GUILD_DELETE, GD_IMG, "gd_x_up.png", "gd_x_over.png", "gd_x_down.png")
			page.Children.append(deleteButton)

			## Comment
			commentSlot = CommentSlot()
			commentSlot.SetParent(page)
			commentSlot.SetPosition(148, 0)
			commentSlot.Show()
			page.Children.append(commentSlot)

			boardSlotList = []
			boardSlotList.append(noticeMarkImage)
			boardSlotList.append(nameSlot)
			boardSlotList.append(commentSlot)
			page.boardDict[i] = boardSlotList
			page.boardRows[i] = [noticeMarkImage, nameSlotImage, commentSlot, deleteButton]

		## PostComment - Have to make this here for that fit tooltip's position.
		postCommentButton = ui.MakeButton(page, 306, 386, localeInfo.GUILD_COMMENT, GD_IMG, "gd_send_up.png", "gd_send_over.png", "gd_send_down.png")
		postCommentButton.SetEvent(ui.__mem_func__(self.OnPostComment))
		page.Children.append(postCommentButton)

		page.dragList = DragList(page, 0, 0, 512, 376, 50, ui.__mem_func__(self.__OnBoardScroll), ui.__mem_func__(self.__OnBoardTap))
		page.Children.append(page.dragList)
		self.__LayoutBoardRows()

	def __LayoutBoardRows(self):
		page = self.pageWindow["BOARD"]
		count = page.boardCount
		page.dragList.SetRange(count - 7)
		offset = page.dragList.offset
		for i, widgets in page.boardRows.items():
			r = i - offset
			visible = (i < count) and (0 <= r < 7)
			for k in xrange(len(widgets)):
				widget = widgets[k]
				if not visible:
					widget.Hide()
					continue
				if k == 0 and not page.boardNotice.get(i, False):
					widget.Hide()
					continue
				widget.SetPosition(page.boardColX[k], r * 50 + (k == 0 and 14 or 0))
				widget.Show()
		page.dragList.SetTop()

	def __OnBoardScroll(self, offset):
		self.__LayoutBoardRows()

	def __OnBoardTap(self, row, index, x):
		page = self.pageWindow["BOARD"]
		if index < page.boardCount and x >= 456:
			self.OnDeleteComment(index)

'''
Region('	def __MakeBoardPage(self):', '	def __MakeMemberPage(self):', BOARD)

MEMBER = '''	def __MakeMemberPage(self):

		page = self.pageWindow["MEMBER"]

		lineStep = 52
		page.memberDict = {}
		page.memberRows = {}

		for i in xrange(self.MEMBER_LINE_COUNT):

			inverseLineIndex = self.MEMBER_LINE_COUNT - i - 1
			yPos = 30 + inverseLineIndex*lineStep
			## Name
			nameSlotImage = ui.MakeImageBox(page, GD_IMG + "gd_slot_130x46.png", 0, yPos)
			nameSlot = uiMobileStyle.CreateText(nameSlotImage, "", 65, 13, C_TEXT, "Lato:16", "center")
			page.Children.append(nameSlotImage)
			page.Children.append(nameSlot)

			## Grade
			gradeSlot = AkrComboBox()
			gradeSlot.SetParent(page)
			gradeSlot.SetPosition(136, yPos)
			gradeSlot.SetSize(104, 46)
			gradeSlot.SetEvent(lambda gradeNumber, lineIndex=inverseLineIndex, argSelf=proxy(self): argSelf.OnChangeMemberGrade(lineIndex, gradeNumber))
			gradeSlot.Show()
			page.Children.append(gradeSlot)

			## Job
			jobSlotImage = ui.MakeImageBox(page, GD_IMG + "gd_slot_76x46.png", 246, yPos)
			jobSlot = uiMobileStyle.CreateText(jobSlotImage, "", 38, 14, C_TEXT, "Lato:15", "center")
			page.Children.append(jobSlotImage)
			page.Children.append(jobSlot)

			## Level
			levelSlotImage = ui.MakeImageBox(page, GD_IMG + "gd_slot_50x46.png", 328, yPos)
			levelSlot = uiMobileStyle.CreateText(levelSlotImage, "", 25, 13, C_TEXT, "Lato:16", "center")
			page.Children.append(levelSlotImage)
			page.Children.append(levelSlot)

			## Offer
			offerSlotImage = ui.MakeImageBox(page, GD_IMG + "gd_slot_56x46.png", 384, yPos)
			offerSlot = uiMobileStyle.CreateText(offerSlotImage, "", 28, 14, C_TEXT, "Lato:15", "center")
			page.Children.append(offerSlotImage)
			page.Children.append(offerSlot)

			## General Enable
			event = lambda argSelf=proxy(self), argIndex=inverseLineIndex: apply(argSelf.OnEnableGeneral, (argIndex,))
			generalEnableCheckBox = CheckBox(page, 444, yPos, event, GD_IMG + "gd_slot_44x46.png")
			page.Children.append(generalEnableCheckBox)

			memberSlotList = []
			memberSlotList.append(nameSlot)
			memberSlotList.append(gradeSlot)
			memberSlotList.append(jobSlot)
			memberSlotList.append(levelSlot)
			memberSlotList.append(offerSlot)
			memberSlotList.append(generalEnableCheckBox)
			page.memberDict[inverseLineIndex] = memberSlotList
			page.memberRows[inverseLineIndex] = [nameSlotImage, gradeSlot, jobSlotImage, levelSlotImage, offerSlotImage, generalEnableCheckBox]

		page.dragList = DragList(page, 0, 30, 490, self.MEMBER_LINE_COUNT * lineStep, lineStep, ui.__mem_func__(self.__OnMemberScroll), ui.__mem_func__(self.__OnMemberTap))
		page.Children.append(page.dragList)

	def __LayoutMemberRows(self):
		page = self.pageWindow["MEMBER"]
		for line, widgets in page.memberRows.items():
			isMember = guild.IsMember(line)
			for widget in widgets:
				if isMember:
					widget.Show()
				else:
					widget.Hide()
		page.dragList.SetTop()

	def __OnMemberScroll(self, offset):
		page = self.pageWindow["MEMBER"]
		count = guild.GetMemberCount()
		span = max(1, count - self.MEMBER_LINE_COUNT)
		page.scrollBar.SetPos(float(offset) / float(span))
		self.OnScrollMemberLine()

	def __OnMemberTap(self, row, index, x):
		page = self.pageWindow["MEMBER"]
		if not page.memberDict.has_key(row):
			return
		if not guild.IsMember(row):
			return
		slotList = page.memberDict[row]
		if 136 <= x < 240:
			slotList[1].OnMouseLeftButtonUp()
		elif 444 <= x < 488 and slotList[5].Enable:
			slotList[5].event()

'''
Region('	def __MakeMemberPage(self):', '	def __MakeBaseInfoPage(self):', MEMBER)

GRADE = '''	def __MakeGradePage(self):

		lineStep = 56
		page = self.pageWindow["GRADE"]

		page.gradeDict = {}
		page.gradeRows = {}
		page.gradeColX = [0, 52, 262, 326, 390, 454]

		for i in xrange(15):

			index = i+1
			## GradeNumber
			gradeNumberSlotImage = ui.MakeImageBox(page, GD_IMG + "gd_slot_44x48.png", 0, 0)
			gradeNumberSlot = uiMobileStyle.CreateText(gradeNumberSlotImage, str(i+1), 22, 14, C_GOLD, "Lato:17", "center")
			page.Children.append(gradeNumberSlotImage)
			page.Children.append(gradeNumberSlot)

			## GradeName
			gradeNameSlot = EditableTextSlot(page, 52, 0, GD_IMG + "gd_slot_200x48.png")
			gradeNameSlot.SetEvent(ui.__mem_func__(self.OnOpenChangeGradeName), index)
			page.Children.append(gradeNameSlot)

			## Invite Authority
			event = lambda argSelf=proxy(self), argIndex=index, argAuthority=1<<0: apply(argSelf.OnCheckAuthority, (argIndex,argAuthority))
			inviteAuthorityCheckBox = CheckBox(page, 262, 0, event)
			page.Children.append(inviteAuthorityCheckBox)

			## DriveOut Authority
			event = lambda argSelf=proxy(self), argIndex=index, argAuthority=1<<1: apply(argSelf.OnCheckAuthority, (argIndex,argAuthority))
			driveoutAuthorityCheckBox = CheckBox(page, 326, 0, event)
			page.Children.append(driveoutAuthorityCheckBox)

			## Notice Authority
			event = lambda argSelf=proxy(self), argIndex=index, argAuthority=1<<2: apply(argSelf.OnCheckAuthority, (argIndex,argAuthority))
			noticeAuthorityCheckBox = CheckBox(page, 390, 0, event)
			page.Children.append(noticeAuthorityCheckBox)

			## Skill Authority
			event = lambda argSelf=proxy(self), argIndex=index, argAuthority=1<<3: apply(argSelf.OnCheckAuthority, (argIndex,argAuthority))
			skillAuthorityCheckBox = CheckBox(page, 454, 0, event)
			page.Children.append(skillAuthorityCheckBox)

			gradeSlotList = []
			gradeSlotList.append(gradeNameSlot)
			gradeSlotList.append(inviteAuthorityCheckBox)
			gradeSlotList.append(driveoutAuthorityCheckBox)
			gradeSlotList.append(noticeAuthorityCheckBox)
			gradeSlotList.append(skillAuthorityCheckBox)
			page.gradeDict[index] = gradeSlotList
			page.gradeRows[index] = [gradeNumberSlotImage, gradeNameSlot, inviteAuthorityCheckBox, driveoutAuthorityCheckBox, noticeAuthorityCheckBox, skillAuthorityCheckBox]

		masterSlotList = page.gradeDict[1]
		for slot in masterSlotList:
			slot.Disable()

		page.dragList = DragList(page, 0, 30, 512, 7 * lineStep, lineStep, ui.__mem_func__(self.__OnGradeScroll), ui.__mem_func__(self.__OnGradeTap))
		page.dragList.SetRange(15 - 7)
		page.Children.append(page.dragList)
		self.__LayoutGradeRows()

	def __LayoutGradeRows(self):
		page = self.pageWindow["GRADE"]
		offset = page.dragList.offset
		for index, widgets in page.gradeRows.items():
			r = index - 1 - offset
			for k in xrange(len(widgets)):
				widget = widgets[k]
				if 0 <= r < 7:
					widget.SetPosition(page.gradeColX[k], 30 + r * 56)
					widget.Show()
				else:
					widget.Hide()
		page.dragList.SetTop()

	def __OnGradeScroll(self, offset):
		self.__LayoutGradeRows()

	def __OnGradeTap(self, row, index, x):
		page = self.pageWindow["GRADE"]
		grade = index + 1
		if not page.gradeDict.has_key(grade):
			return
		slotList = page.gradeDict[grade]
		if 52 <= x < 258:
			if slotList[0].Enable:
				self.OnOpenChangeGradeName(grade)
		elif x >= 262:
			column = min(3, int((x - 262) // 64))
			checkBox = slotList[1 + column]
			if checkBox.Enable:
				checkBox.event()

'''
Region('	def __MakeGradePage(self):', '	def CanOpen(self):', GRADE)

# beceri sayfasi: buyuk kutular, buyuk + dugmeleri
Rep('page.activeSlot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)\n', 'page.activeSlot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)\n\t\twndMgr.SetSlotUnitSize(page.activeSlot.hWnd, 64, 64, 4)\n\t\twndMgr.SetSlotRoundIcons(page.activeSlot.hWnd, 1)\n')
Rep('page.affectSlot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)\n', 'page.affectSlot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)\n\t\twndMgr.SetSlotUnitSize(page.affectSlot.hWnd, 50, 50, 4)\n')
for kind in ('up', 'over', 'down'):
	Rep('"d:/ymir work/ui/game/windows/btn_plus_%s.sub"' % kind, 'GD_IMG + "gd_plus_%s.png"' % kind, 2)

# beceri: motorun kose kapak karesi yok; becerinin ALTINDA seviye kutusu + "+" (puan varsa)
Rep("		page.activeSlot.SetCoverButton(slotIndex)\n			page.activeSlot.RefreshSlot()\n			guild.SetSkillIndex(slotIndex, len(playerSettingModule.PASSIVE_GUILD_SKILL_INDEX_LIST)+i)\n",
	"		page.activeSlot.SetCoverButton(slotIndex, GD_IMG + 'gd_clear.png', GD_IMG + 'gd_clear.png', GD_IMG + 'gd_clear.png', GD_IMG + 'gd_dim64.png')\n			page.activeSlot.RefreshSlot()\n			guild.SetSkillIndex(slotIndex, len(playerSettingModule.PASSIVE_GUILD_SKILL_INDEX_LIST)+i)\n\n		page.plusButtons = []\n		page.levelTexts = []\n		for i in xrange(len(playerSettingModule.ACTIVE_GUILD_SKILL_INDEX_LIST)):\n			levelBox = ui.MakeImageBox(page, GD_IMG + 'gd_lvbox.png', 16 + i * 80 + 12, 114)\n			page.Children.append(levelBox)\n			levelText = uiMobileStyle.CreateText(levelBox, '0', 20, 5, C_GOLD, 'LatoBd:16', 'center')\n			page.Children.append(levelText)\n			page.levelTexts.append(levelText)\n			plusButton = ui.MakeButton(page, 16 + i * 80 + 54, 113, '', GD_IMG, 'gd_plus_up.png', 'gd_plus_over.png', 'gd_plus_down.png')\n			plusButton.SetEvent(ui.__mem_func__(self.OnUpGuildSkill), page.activeSlot.GetStartIndex() + i, self.GUILD_SKILL_ACTIVE_SLOT)\n			plusButton.Hide()\n			page.Children.append(plusButton)\n			page.plusButtons.append(plusButton)\n")
Rep("		page.passiveSlot.HideAllSlotButton()\n		page.activeSlot.HideAllSlotButton()\n", "		page.passiveSlot.HideAllSlotButton()\n		page.activeSlot.HideAllSlotButton()\n		for plusButton in page.plusButtons:\n			plusButton.Hide()\n")
Rep("			page.activeSlot.SetSlotCount(slotIndex, skillLevel)\n", "			page.activeSlot.SetSlotCount(slotIndex, 0)\n			page.levelTexts[i].SetText(str(skillLevel))\n")
Rep("					page.activeSlot.ShowSlotButton(slotIndex)", "					page.plusButtons[i].Show()")

# yorum tahtasi yenileme: yalniz dolu satirlar
BOARD_REFRESH = '''	def RefreshGuildBoardPage(self):

		if self.isLoaded==0:
			return

		page = self.pageWindow["BOARD"]

		self.BOARD_LINE_MAX_NUM = 12
		lineIndex = 0
		page.boardNotice = {}

		commentCount = guild.GetGuildBoardCommentCount()
		for i in xrange(commentCount):

			commentID, chrName, comment = self.__GetGuildBoardCommentData(i)

			if not comment:
				continue

			if lineIndex >= self.BOARD_LINE_MAX_NUM:
				break

			slotList = page.boardDict[lineIndex]

			if "!" == comment[0]:
				page.boardNotice[lineIndex] = True
				slotList[1].SetText(chrName)
				slotList[2].SetText(comment[1:])

			else:
				slotList[1].SetText(chrName)
				slotList[2].SetText(comment)

			lineIndex += 1

		for i in xrange(self.BOARD_LINE_MAX_NUM - lineIndex):
			slotList = page.boardDict[lineIndex+i]
			slotList[1].SetText("")
			slotList[2].SetText("")

		page.boardCount = lineIndex
		self.__LayoutBoardRows()

'''
Region('	def RefreshGuildBoardPage(self):', '	def RefreshGuildMemberPage(self):', BOARD_REFRESH)

# uye listesi: kaydirma araligi + dolu satirlar
Rep('''		else:
			page.scrollBar.Hide()
		self.RefreshGuildMemberPageGradeComboBox()
		self.RefreshGuildMemberPageMemberList()
''', '''		else:
			page.scrollBar.Hide()
		page.dragList.SetRange(count - self.MEMBER_LINE_COUNT)
		self.RefreshGuildMemberPageGradeComboBox()
		self.RefreshGuildMemberPageMemberList()
''')
Rep('''			gradeComboBox.SetCurrentItem(guild.GetGradeName(grade))
			if 1 != grade:
				gradeComboBox.Enable()

	def RefreshGuildMemberPageGradeComboBox(self):''', '''			gradeComboBox.SetCurrentItem(guild.GetGradeName(grade))
			if 1 != grade:
				gradeComboBox.Enable()

		self.__LayoutMemberRows()

	def RefreshGuildMemberPageGradeComboBox(self):''')

for (name, block) in (('HELPERS', HELPERS), ('COMMENT', COMMENT)):
	ast.parse(block)
for (name, block) in (('BOARD', BOARD), ('MEMBER', MEMBER), ('GRADE', GRADE), ('BOARD_REFRESH', BOARD_REFRESH)):
	ast.parse('class T(object):\n' + block)
open(DST, 'wb').write(text.replace('\n', '\r\n').encode('latin1'))
print('uiguild.py uretildi:', len(text), 'karakter')
