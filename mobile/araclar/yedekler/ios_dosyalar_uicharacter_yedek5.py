import ui
import uiScriptLocale
import app
import net
import dbg
import snd
import player
import mouseModule
import wndMgr
import skill
import playerSettingModule
import quest
import localeInfo
import uiToolTip
import constInfo
import emotion
import chr
import uiAlignmentPanel

if app.ENABLE_DETAILS_INTERFACE:
	import uiCharacterDetails

SHOW_ONLY_ACTIVE_SKILL = False
HIDE_SUPPORT_SKILL_POINT = True

if app.ENABLE_PASIF_SKILL:
	SHOW_LIMIT_SUPPORT_SKILL_LIST = [121, 122, 123, 124, 126, 127, 129, 128, 131, 137, 138, 139, 140]
else:
	SHOW_LIMIT_SUPPORT_SKILL_LIST = [121, 122, 123, 124, 126, 127, 129, 128, 131, 137, 138, 139, 140]

FACE_IMAGE_DICT = {
	playerSettingModule.RACE_WARRIOR_M	: "icon/face/warrior_m.tga",
	playerSettingModule.RACE_WARRIOR_W	: "icon/face/warrior_w.tga",
	playerSettingModule.RACE_ASSASSIN_M	: "icon/face/assassin_m.tga",
	playerSettingModule.RACE_ASSASSIN_W	: "icon/face/assassin_w.tga",
	playerSettingModule.RACE_SURA_M		: "icon/face/sura_m.tga",
	playerSettingModule.RACE_SURA_W		: "icon/face/sura_w.tga",
	playerSettingModule.RACE_SHAMAN_M	: "icon/face/shaman_m.tga",
	playerSettingModule.RACE_SHAMAN_W	: "icon/face/shaman_w.tga",
}

if app.ENABLE_WOLFMAN_CHARACTER:
	FACE_IMAGE_DICT.update({playerSettingModule.RACE_WOLFMAN_M  : "icon/face/wolfman_m.tga",})

def unsigned32(n):
	return n & 0xFFFFFFFFL

ALIGNMENT_LIST = [
	# AttrName0 - AttrValue0 - AttrName1 - AttrValue1
	[[localeInfo.ALIGN_REWARD_BONUS_A, 2, localeInfo.ALIGN_REWARD_BONUS_B, 2, localeInfo.ALIGN_REWARD_BONUS_C, 2]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 4, localeInfo.ALIGN_REWARD_BONUS_B, 4, localeInfo.ALIGN_REWARD_BONUS_C, 4]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 6, localeInfo.ALIGN_REWARD_BONUS_B, 6, localeInfo.ALIGN_REWARD_BONUS_C, 6]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 8, localeInfo.ALIGN_REWARD_BONUS_B, 8, localeInfo.ALIGN_REWARD_BONUS_C, 8]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 10, localeInfo.ALIGN_REWARD_BONUS_B, 10, localeInfo.ALIGN_REWARD_BONUS_C, 10]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 12, localeInfo.ALIGN_REWARD_BONUS_B, 12, localeInfo.ALIGN_REWARD_BONUS_C, 12]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 14, localeInfo.ALIGN_REWARD_BONUS_B, 14, localeInfo.ALIGN_REWARD_BONUS_C, 14]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 16, localeInfo.ALIGN_REWARD_BONUS_B, 16, localeInfo.ALIGN_REWARD_BONUS_C, 16]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 18, localeInfo.ALIGN_REWARD_BONUS_B, 18, localeInfo.ALIGN_REWARD_BONUS_C, 18]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 20, localeInfo.ALIGN_REWARD_BONUS_B, 20, localeInfo.ALIGN_REWARD_BONUS_C, 20]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 22, localeInfo.ALIGN_REWARD_BONUS_B, 22, localeInfo.ALIGN_REWARD_BONUS_C, 22]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 24, localeInfo.ALIGN_REWARD_BONUS_B, 24, localeInfo.ALIGN_REWARD_BONUS_C, 24]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 26, localeInfo.ALIGN_REWARD_BONUS_B, 26, localeInfo.ALIGN_REWARD_BONUS_C, 26]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 28, localeInfo.ALIGN_REWARD_BONUS_B, 28, localeInfo.ALIGN_REWARD_BONUS_C, 28]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 30, localeInfo.ALIGN_REWARD_BONUS_B, 30, localeInfo.ALIGN_REWARD_BONUS_C, 30]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 32, localeInfo.ALIGN_REWARD_BONUS_B, 32, localeInfo.ALIGN_REWARD_BONUS_C, 32]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 34, localeInfo.ALIGN_REWARD_BONUS_B, 34, localeInfo.ALIGN_REWARD_BONUS_C, 34]],	
	[[localeInfo.ALIGN_REWARD_BONUS_A, 36, localeInfo.ALIGN_REWARD_BONUS_B, 36, localeInfo.ALIGN_REWARD_BONUS_C, 36]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 38, localeInfo.ALIGN_REWARD_BONUS_B, 38, localeInfo.ALIGN_REWARD_BONUS_C, 38]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 40, localeInfo.ALIGN_REWARD_BONUS_B, 40, localeInfo.ALIGN_REWARD_BONUS_C, 40]],
	[[localeInfo.ALIGN_REWARD_BONUS_A, 50, localeInfo.ALIGN_REWARD_BONUS_B, 50, localeInfo.ALIGN_REWARD_BONUS_C, 50]],

]

class CharacterWindow(ui.ScriptWindow):

	ACTIVE_PAGE_SLOT_COUNT = 8
	SUPPORT_PAGE_SLOT_COUNT = 12

	PAGE_SLOT_COUNT = 12
	PAGE_HORSE = 2

	chDetailsWnd = None
	isOpenedDetailsWnd = False
	SKILL_GROUP_NAME_DICT = {
		playerSettingModule.JOB_WARRIOR	: { 1 : localeInfo.SKILL_GROUP_WARRIOR_1,	2 : localeInfo.SKILL_GROUP_WARRIOR_2, },
		playerSettingModule.JOB_ASSASSIN	: { 1 : localeInfo.SKILL_GROUP_ASSASSIN_1,	2 : localeInfo.SKILL_GROUP_ASSASSIN_2, },
		playerSettingModule.JOB_SURA		: { 1 : localeInfo.SKILL_GROUP_SURA_1,		2 : localeInfo.SKILL_GROUP_SURA_2, },
		playerSettingModule.JOB_SHAMAN		: { 1 : localeInfo.SKILL_GROUP_SHAMAN_1,	2 : localeInfo.SKILL_GROUP_SHAMAN_2, },
	}
	if app.ENABLE_WOLFMAN_CHARACTER:
		SKILL_GROUP_NAME_DICT.update({playerSettingModule.JOB_WOLFMAN		: { 1 : localeInfo.JOB_WOLFMAN1,	2 : localeInfo.JOB_WOLFMAN2, },})

	STAT_DESCRIPTION =	{
		"HTH" : localeInfo.STAT_TOOLTIP_CON,
		"INT" : localeInfo.STAT_TOOLTIP_INT,
		"STR" : localeInfo.STAT_TOOLTIP_STR,
		"DEX" : localeInfo.STAT_TOOLTIP_DEX,
	}


	STAT_MINUS_DESCRIPTION = localeInfo.STAT_MINUS_DESCRIPTION

	def __init__(self):
		if app.ENABLE_DETAILS_INTERFACE:
			self.chDetailsWnd = None
			self.isOpenedDetailsWnd = False
			self.isOpenedDetailsWnd_X = 0
			self.isOpenedDetailsWnd_Y = 0

		ui.ScriptWindow.__init__(self)
		self.state = "STATUS"
		self.isLoaded = 0

		self.toolTipSkill = 0

		self.alignmentPanelDlg = None

		self.__Initialize()
		self.__LoadWindow()

		self.statusPlusCommandDict={
			"HTH" : "/stat ht",
			"INT" : "/stat iq",
			"STR" : "/stat st",
			"DEX" : "/stat dx",
		}

		if app.ENABLE_STATUS_UP_RENEWAL:
			self.faststatusPlusCommandDict={
				"HTH" : "/stat ht 10",
				"INT" : "/stat iq 10",
				"STR" : "/stat st 10",
				"DEX" : "/stat dx 10",
			}

		self.statusMinusCommandDict={
			"HTH-" : "/stat- ht",
			"INT-" : "/stat- iq",
			"STR-" : "/stat- st",
			"DEX-" : "/stat- dx",
		}

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def __Initialize(self):
		self.refreshToolTip = 0
		self.curSelectedSkillGroup = 0
		self.canUseHorseSkill = -1
		self.ExpandBtn = None
		self.MinimizeBtn = None
		self.chDetailsWnd = None
		self.isOpenedDetailsWnd = False
		self.toolTip = None
		self.toolTipJob = None
		self.toolTipAlignment = None
		self.toolTipSkill = None

		self.faceImage = None
		self.statusPlusLabel = None
		self.statusPlusValue = None
		self.activeSlot = None
		self.tabDict = None
		self.tabButtonDict = None
		self.pageDict = None
		self.titleBarDict = None
		self.statusPlusButtonDict = None
		self.statusMinusButtonDict = None

		self.skillPageDict = None
		self.questShowingStartIndex = 0
		self.questScrollBar = None
		self.questSlot = None
		self.questNameList = None
		self.questLastTimeList = None
		self.questLastCountList = None
		self.skillGroupButton = ()

		self.activeSlot = None
		self.activeSkillPointValue = None
		self.supportSkillPointValue = None
		self.skillGroupButton1 = None
		self.skillGroupButton2 = None
		self.activeSkillGroupName = None

		self.guildNameSlot = None
		self.guildNameValue = None
		self.characterNameSlot = None
		self.characterNameValue = None

		self.emotionToolTip = None
		self.soloEmotionSlot = None
		self.dualEmotionSlot = None

	if app.ENABLE_DETAILS_INTERFACE:
		def OnTop(self):
			if self.chDetailsWnd:
				self.chDetailsWnd.SetTop()

		def Hide(self):
			if self.chDetailsWnd:
				self.isOpenedDetailsWnd = False	# AKRAVON: bonuslar gomulu, tekrar acilista Durum
				self.chDetailsWnd.Close()
			wndMgr.Hide(self.hWnd)

	def Show(self):
		self.__LoadWindow()
		self.__InitCharacterDetailsUIButton()
		if self.chDetailsWnd and self.isOpenedDetailsWnd:
			self.chDetailsWnd.Show()
			self.SetTop()
		ui.ScriptWindow.Show(self)
		# AKRAVON: soldaki karakter portresiyle ayni hizada, durum cubugunun altinda acilir
		try:
			import wndMgr
			import uiMobileHud
			self.SetPosition(uiMobileHud.SCREEN_EDGE + 8, max(8, min(96, wndMgr.GetScreenHeight() - self.GetHeight() - 8)))
		except Exception:
			pass

	def __LoadScript(self, fileName):
		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, fileName)

	def __BindObject(self):
		self.toolTip = uiToolTip.ToolTip()
		self.toolTipJob = uiToolTip.ToolTip()
		self.toolTipAlignment = uiToolTip.ToolTip(130)

		self.faceImage = self.GetChild("Face_Image")
		GetObject = self.GetChild

		faceSlot=self.GetChild("Face_Slot")
		if 949 == app.GetDefaultCodePage():
			faceSlot.SAFE_SetStringEvent("MOUSE_OVER_IN", self.__ShowJobToolTip)
			faceSlot.SAFE_SetStringEvent("MOUSE_OVER_OUT", self.__HideJobToolTip)

		self.statusPlusLabel = self.GetChild("Status_Plus_Label")
		self.statusPlusValue = self.GetChild("Status_Plus_Value")

		self.characterNameSlot = self.GetChild("Character_Name_Slot")
		self.characterNameValue = self.GetChild("Character_Name")
		self.guildNameSlot = self.GetChild("Guild_Name_Slot")
		self.guildNameValue = self.GetChild("Guild_Name")
		self.characterNameSlot.SAFE_SetStringEvent("MOUSE_OVER_IN", self.__ShowAlignmentToolTip)
		self.characterNameSlot.SAFE_SetStringEvent("MOUSE_OVER_OUT", self.__HideAlignmentToolTip)
		self.characterNameSlot.SetEvent(ui.__mem_func__(self.__UseAlignmentToolTip), "mouse_click")
		# AKRAVON: Derece kartina dokununca derece detaylari acilir
		try:
			self.GetChild("Align_Card").SetEvent(ui.__mem_func__(self.__UseAlignmentToolTip), "mouse_click")
		except Exception:
			pass

		self.activeSlot = self.GetChild("Skill_Active_Slot")
		self.activeSkillPointValue = self.GetChild("Active_Skill_Point_Value")
		self.supportSkillPointValue = self.GetChild("Support_Skill_Point_Value")
		self.skillGroupButton1 = self.GetChild("Skill_Group_Button_1")
		self.skillGroupButton2 = self.GetChild("Skill_Group_Button_2")
		self.activeSkillGroupName = self.GetChild("Active_Skill_Group_Name")

		self.tabDict = {
			"STATUS"	: self.GetChild("Tab_01"),
			"SKILL"		: self.GetChild("Tab_02"),
			"EMOTICON"	: self.GetChild("Tab_03"),
			"QUEST"		: self.GetChild("Tab_04"),
		}

		self.tabButtonDict = {
			"STATUS"	: self.GetChild("Tab_Button_01"),
			"SKILL"		: self.GetChild("Tab_Button_02"),
			"EMOTICON"	: self.GetChild("Tab_Button_03"),
			"QUEST"		: self.GetChild("Tab_Button_04")
		}

		self.pageDict = {
			"STATUS"	: self.GetChild("Character_Page"),
			"SKILL"		: self.GetChild("Skill_Page"),
			"EMOTICON"	: self.GetChild("Emoticon_Page"),
			"QUEST"		: self.GetChild("Quest_Page")
		}

		self.titleBarDict = {
			"STATUS"	: self.GetChild("Character_TitleBar"),
			"SKILL"		: self.GetChild("Skill_TitleBar"),
			"EMOTICON"	: self.GetChild("Emoticon_TitleBar"),
			"QUEST"		: self.GetChild("Quest_TitleBar")
		}

		self.statusPlusButtonDict = {
			"HTH"		: self.GetChild("HTH_Plus"),
			"INT"		: self.GetChild("INT_Plus"),
			"STR"		: self.GetChild("STR_Plus"),
			"DEX"		: self.GetChild("DEX_Plus"),
		}

		self.statusMinusButtonDict = {
			"HTH-"		: self.GetChild("HTH_Minus"),
			"INT-"		: self.GetChild("INT_Minus"),
			"STR-"		: self.GetChild("STR_Minus"),
			"DEX-"		: self.GetChild("DEX_Minus"),
		}

		self.skillPageDict = {
			"ACTIVE" : self.GetChild("Skill_Active_Slot"),
			"SUPPORT" : self.GetChild("Skill_ETC_Slot"),
			"HORSE" : self.GetChild("Skill_Active_Slot"),
		}

		self.skillPageStatDict = {
			"SUPPORT"	: player.SKILL_SUPPORT,
			"ACTIVE"	: player.SKILL_ACTIVE,
			"HORSE"		: player.SKILL_HORSE,
		}

		self.skillGroupButton = (
			self.GetChild("Skill_Group_Button_1"),
			self.GetChild("Skill_Group_Button_2"),
		)


		global SHOW_ONLY_ACTIVE_SKILL
		global HIDE_SUPPORT_SKILL_POINT
		if SHOW_ONLY_ACTIVE_SKILL or HIDE_SUPPORT_SKILL_POINT:
			self.GetChild("Support_Skill_Point_Label").Hide()

		self.soloEmotionSlot = self.GetChild("SoloEmotionSlot")
		self.dualEmotionSlot = self.GetChild("DualEmotionSlot")
		self.__SetEmotionSlot()

		self.questShowingStartIndex = 0
		self.questScrollBar = self.GetChild("Quest_ScrollBar")
		self.questScrollBar.SetScrollEvent(ui.__mem_func__(self.OnQuestScroll))
		self.questSlot = self.GetChild("Quest_Slot")
		for i in xrange(quest.QUEST_MAX_NUM):
			self.questSlot.HideSlotBaseImage(i)
			self.questSlot.SetCoverButton(i,\
											"d:/ymir work/ui/game/quest/slot_button_01.sub",\
											"d:/ymir work/ui/game/quest/slot_button_02.sub",\
											"d:/ymir work/ui/game/quest/slot_button_03.sub",\
											"d:/ymir work/ui/game/quest/slot_button_03.sub", True)

		self.questNameList = []
		self.questLastTimeList = []
		self.questLastCountList = []
		for i in xrange(quest.QUEST_MAX_NUM):
			self.questNameList.append(self.GetChild("Quest_Name_0" + str(i)))
			self.questLastTimeList.append(self.GetChild("Quest_LastTime_0" + str(i)))
			self.questLastCountList.append(self.GetChild("Quest_LastCount_0" + str(i)))

		if app.ENABLE_DETAILS_INTERFACE:
			MainBoard = self.GetChild("board")
			self.ExpandBtn = ui.MakeButton(MainBoard, 520, 16, "", "d:/ymir work/ui/game/belt_inventory/", "btn_minimize_normal.tga", "btn_minimize_over.tga", "btn_minimize_down.tga")
			self.ExpandBtn.SetEvent(ui.__mem_func__(self.__ClickExpandButton))
			self.MinimizeBtn = ui.MakeButton(MainBoard, 520, 16, "", "d:/ymir work/ui/game/belt_inventory/", "btn_expand_normal.tga", "btn_expand_over.tga", "btn_expand_down.tga")
			self.MinimizeBtn.SetEvent(ui.__mem_func__(self.__ClickMinimizeButton))

	if app.ENABLE_DETAILS_INTERFACE:
		def __InitCharacterDetailsUIButton(self):
			self.ExpandBtn.Hide()	# AKRAVON: yerine kenar cubugunda "Bonuslar"
			self.MinimizeBtn.Hide()

		def __ClickExpandButton(self):
			if not self.chDetailsWnd:
				self.chDetailsWnd = uiCharacterDetails.CharacterDetailsUI()
				self.chDetailsWnd.Show()

				if (self.isOpenedDetailsWnd_X > 0) or (self.isOpenedDetailsWnd_Y > 0):
					self.chDetailsWnd.AdjustPosition(self.isOpenedDetailsWnd_X, self.isOpenedDetailsWnd_Y)
			else:
				self.chDetailsWnd.Show()

			self.SetTop()
			self.ExpandBtn.Hide()
			self.MinimizeBtn.Show()

		def __ClickMinimizeButton(self):
			self.chDetailsWnd.Hide()
			self.MinimizeBtn.Hide()
			self.ExpandBtn.Show()

		def OnMoveWindow(self, x, y):
			if self.chDetailsWnd:
				self.chDetailsWnd.AdjustPosition(x, y)
			else:
				self.isOpenedDetailsWnd_X = x
				self.isOpenedDetailsWnd_Y = y

	def __SetSkillSlotEvent(self):
		for skillPageValue in self.skillPageDict.itervalues():
			skillPageValue.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
			skillPageValue.SetSelectItemSlotEvent(ui.__mem_func__(self.SelectSkill))
			skillPageValue.SetSelectEmptySlotEvent(ui.__mem_func__(self.SelectEmptySlot))
			skillPageValue.SetUnselectItemSlotEvent(ui.__mem_func__(self.ClickSkillSlot))
			skillPageValue.SetUseSlotEvent(ui.__mem_func__(self.ClickSkillSlot))
			skillPageValue.SetOverInItemEvent(ui.__mem_func__(self.OverInItem))
			skillPageValue.SetOverOutItemEvent(ui.__mem_func__(self.OverOutItem))
			skillPageValue.SetPressedSlotButtonEvent(ui.__mem_func__(self.OnPressedSlotButton))
			skillPageValue.AppendSlotButton("d:/ymir work/ui/game/windows/btn_plus_up.sub",\
											"d:/ymir work/ui/game/windows/btn_plus_over.sub",\
											"d:/ymir work/ui/game/windows/btn_plus_down.sub")

	def __SetEmotionSlot(self):

		self.emotionToolTip = uiToolTip.ToolTip()

		for slot in (self.soloEmotionSlot, self.dualEmotionSlot):
			slot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
			slot.SetSelectItemSlotEvent(ui.__mem_func__(self.__SelectEmotion))
			slot.SetUnselectItemSlotEvent(ui.__mem_func__(self.__ClickEmotionSlot))
			slot.SetUseSlotEvent(ui.__mem_func__(self.__ClickEmotionSlot))
			slot.SetOverInItemEvent(ui.__mem_func__(self.__OverInEmotion))
			slot.SetOverOutItemEvent(ui.__mem_func__(self.__OverOutEmotion))
			slot.AppendSlotButton("d:/ymir work/ui/game/windows/btn_plus_up.sub",\
											"d:/ymir work/ui/game/windows/btn_plus_over.sub",\
											"d:/ymir work/ui/game/windows/btn_plus_down.sub")

		for slotIdx, datadict in emotion.EMOTION_DICT.items():
			emotionIdx = slotIdx

			slot = self.soloEmotionSlot
			if slotIdx > 50:
				slot = self.dualEmotionSlot

			slot.SetEmotionSlot(slotIdx, emotionIdx)
			import uiAkravonActionIcon
			icon = uiAkravonActionIcon.GetCharacterEmotionIcon(emotionIdx)
			if icon:
				slot.SetCoverButton(slotIdx, icon, icon, icon, icon)
			else:
				slot.SetCoverButton(slotIdx)

	def __SelectEmotion(self, slotIndex):
		if not slotIndex in emotion.EMOTION_DICT:
			return

		if app.IsPressed(app.DIK_LCONTROL):
			player.RequestAddToEmptyLocalQuickSlot(player.SLOT_TYPE_EMOTION, slotIndex)
			return

		mouseModule.mouseController.AttachObject(self, player.SLOT_TYPE_EMOTION, slotIndex, slotIndex)

	def __ClickEmotionSlot(self, slotIndex):
		print "click emotion"
		if not slotIndex in emotion.EMOTION_DICT:
			return

		print "check acting"
		if player.IsActingEmotion():
			return

		command = emotion.EMOTION_DICT[slotIndex]["command"]
		print "command", command

		if slotIndex > 50:
			vid = player.GetTargetVID()

			if 0 == vid or vid == player.GetMainCharacterIndex() or chr.IsNPC(vid) or chr.IsEnemy(vid):
				import chat
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.EMOTION_CHOOSE_ONE)
				return

			command += " " + chr.GetNameByVID(vid)

		print "send_command", command
		net.SendChatPacket(command)

	def ActEmotion(self, emotionIndex):
		self.__ClickEmotionSlot(emotionIndex)

	def __OverInEmotion(self, slotIndex):
		if self.emotionToolTip:

			if not slotIndex in emotion.EMOTION_DICT:
				return

			self.emotionToolTip.ClearToolTip()
			self.emotionToolTip.SetTitle(emotion.EMOTION_DICT[slotIndex]["name"])
			self.emotionToolTip.AlignHorizonalCenter()
			self.emotionToolTip.ShowToolTip()

	def __OverOutEmotion(self):
		if self.emotionToolTip:
			self.emotionToolTip.HideToolTip()

	def __BindEvent(self):
		for i in xrange(len(self.skillGroupButton)):
			self.skillGroupButton[i].SetEvent(lambda arg=i: self.__SelectSkillGroup(arg))

		self.RefreshQuest()
		self.__HideJobToolTip()

		for (tabKey, tabButton) in self.tabButtonDict.items():
			tabButton.SetEvent(ui.__mem_func__(self.__OnClickTabButton), tabKey)

		for (statusPlusKey, statusPlusButton) in self.statusPlusButtonDict.items():
			statusPlusButton.SAFE_SetEvent(self.__OnClickStatusPlusButton, statusPlusKey)
			statusPlusButton.ShowToolTip = lambda arg=statusPlusKey: self.__OverInStatButton(arg)
			statusPlusButton.HideToolTip = lambda arg=statusPlusKey: self.__OverOutStatButton()

		for (statusMinusKey, statusMinusButton) in self.statusMinusButtonDict.items():
			statusMinusButton.SAFE_SetEvent(self.__OnClickStatusMinusButton, statusMinusKey)
			statusMinusButton.ShowToolTip = lambda arg=statusMinusKey: self.__OverInStatMinusButton(arg)
			statusMinusButton.HideToolTip = lambda arg=statusMinusKey: self.__OverOutStatMinusButton()

		for titleBarValue in self.titleBarDict.itervalues():
			if app.ENABLE_DETAILS_INTERFACE:
				titleBarValue.SetCloseEvent(ui.__mem_func__(self.Close))
			else:
				titleBarValue.SetCloseEvent(ui.__mem_func__(self.Hide))

		self.questSlot.SetSelectItemSlotEvent(ui.__mem_func__(self.__SelectQuest))

	def __LoadWindow(self):
		if self.isLoaded == 1:
			return

		self.isLoaded = 1

		try:
			if localeInfo.IsARABIC() or localeInfo.IsVIETNAM() or localeInfo.IsJAPAN():
				self.__LoadScript(uiScriptLocale.LOCALE_UISCRIPT_PATH + "CharacterWindow.py")
			else:
				self.__LoadScript("UIScript/CharacterWindow.py")

			self.__BindObject()
			self.__BindEvent()
			# AKRAVON: yetenek yuvalari 40 px (mobil dokunma)
			try:
				import wndMgr
				if hasattr(wndMgr, "SetSlotUnitSize"):
					for akrName in ("SoloEmotionSlot", "DualEmotionSlot"):
						wndMgr.SetSlotUnitSize(self.GetChild(akrName).hWnd, 40, 40)
					wndMgr.SetSlotUnitSize(self.GetChild("Skill_ETC_Slot").hWnd, 30, 30)
					# AKRAVON: motorun dolu yuvaya cizdigi koyu kare zemin beceri yuvalarinda cizilmesin
					for akrName in ("Skill_ETC_Slot", "Skill_Active_Slot"):
						wndMgr.SetSlotRoundIcons(self.GetChild(akrName).hWnd, 1)
			except Exception:
				pass
			# AKRAVON: kenar cubugu "Bonuslar" bolumu
			try:
				self.GetChild("Akravon_Nav_Bonus").Hide()
				self.GetChild("Tab_Button_05").SetEvent(ui.__mem_func__(self.AkravonShowBonus))
			except Exception:
				pass
			# AKRAVON: kapat dugmesi kare (tum pencerelerle ayni)
			for akrName in ("Character_TitleBar", "Skill_TitleBar", "Emoticon_TitleBar", "Quest_TitleBar"):
				try:
					akrButton = self.GetChild(akrName).btnClose
					akrButton.SetUpVisual("d:/ymir work/ui/akravon_legacy/inv_close_1.png")
					akrButton.SetOverVisual("d:/ymir work/ui/akravon_legacy/inv_close_1.png")
					akrButton.SetDownVisual("d:/ymir work/ui/akravon_legacy/inv_close_3.png")
					akrButton.SetToolTipText("")
					akrButton.SetPosition(30, 2)
				except Exception:
					pass
		except:
			import exception
			exception.Abort("CharacterWindow.__LoadWindow")

		#self.tabButtonDict["EMOTICON"].Disable()
		self.SetState("STATUS")

	def Destroy(self):
		self.ClearDictionary()
		if app.ENABLE_DETAILS_INTERFACE:
			if self.chDetailsWnd:
				self.chDetailsWnd.Destroy()
				self.chDetailsWnd=None
		self.__Initialize()

	def Close(self):
		if 0 != self.toolTipSkill:
			self.toolTipSkill.Hide()

		if app.ENABLE_DETAILS_INTERFACE:
			if self.chDetailsWnd:
				self.isOpenedDetailsWnd = self.chDetailsWnd.IsShow()
				self.chDetailsWnd.Close()
		self.Hide()

	def SetSkillToolTip(self, toolTipSkill):
		self.toolTipSkill = toolTipSkill

	def __OnClickStatusPlusButton(self, statusKey):
		if player.GetStatus(player.STAT) <= 0:
			import chat
			chat.AppendChat(chat.CHAT_TYPE_INFO, "Da\xf0\xfdt\xfdlacak stat\xfc puan\xfd yok.")
			return
		try:
			if app.ENABLE_STATUS_UP_RENEWAL and app.IsPressed(app.DIK_LCONTROL):
				statusPlusCommand=self.faststatusPlusCommandDict[statusKey]
			else:
				statusPlusCommand=self.statusPlusCommandDict[statusKey]
			net.SendChatPacket(statusPlusCommand)
		except KeyError, msg:
			dbg.TraceError("CharacterWindow.__OnClickStatusPlusButton KeyError: %s", msg)

	def __OnClickStatusMinusButton(self, statusKey):
		if self.__GetStatMinusPoint() <= 0:
			import chat
			chat.AppendChat(chat.CHAT_TYPE_INFO, "Stat s\xfdf\xfdrlama puan\xfd yok.")
			return
		try:
			statusMinusCommand=self.statusMinusCommandDict[statusKey]
			net.SendChatPacket(statusMinusCommand)
		except KeyError, msg:
			dbg.TraceError("CharacterWindow.__OnClickStatusMinusButton KeyError: %s", msg)


	def AkravonSkillCard(self, slotIndex, skillIndex, skillGrade, skillLevel):
		# AKRAVON: tek yuva = mevcut kademe ikonu + HD yuvarlak ikon + ad + rozet (Lv / M1 / G1 / P)
		try:
			k = (slotIndex - 1) % self.ACTIVE_PAGE_SLOT_COUNT
			hd = self.GetChild("Skill_HD_%d" % k)
			nameText = self.GetChild("Skill_Name_%d" % k)
			gradeText = self.GetChild("Skill_Grade_%d" % k)
		except Exception:
			return
		import uiAkravonSkillGrade
		if not hasattr(hd, "skillGradeBadge"):
			hd.skillGradeBadge = uiAkravonSkillGrade.SkillGradeBadge(hd, 48)
		if 0 == skillIndex:
			hd.Hide()
			hd.skillGradeBadge.Hide()
			nameText.SetText("")
			gradeText.SetText("")
			return
		grade = min(skillGrade, skill.SKILL_GRADE_COUNT - 1)
		page = self.skillPageDict["ACTIVE"]
		page.SetSkillSlotNew(slotIndex, skillIndex, grade, skillLevel)
		page.SetSlotCount(slotIndex, 0)
		page.SetCoverButton(slotIndex, "d:/ymir work/ui/akravon_legacy/cw_clear32.png", "d:/ymir work/ui/akravon_legacy/cw_clear32.png", "d:/ymir work/ui/akravon_legacy/cw_clear32.png", "d:/ymir work/ui/akravon_legacy/cw_clear32.png")
		if not self.__CanUseSkillNow():
			page.DisableCoverButton(slotIndex)
		try:
			nameText.SetText(skill.GetSkillName(skillIndex, grade))
		except Exception:
			nameText.SetText(skill.GetSkillName(skillIndex))
		# AKRAVON: kademe yazisi kartin icinde (rozet yok): Perfect / Grand Master N / Master N / Level N
		hd.skillGradeBadge.Hide()
		if skillGrade >= 3:
			(akrLabel, akrColor) = ("Perfect", 0xFFFF7770)
		elif skillGrade == 2:
			(akrLabel, akrColor) = ("Grand Master %d" % skillLevel, 0xFFFFD16A)
		elif skillGrade == 1:
			(akrLabel, akrColor) = ("Master %d" % skillLevel, 0xFF85D8FF)
		elif skillLevel > 0:
			(akrLabel, akrColor) = ("Level %d" % skillLevel, 0xFFE8E2D2)
		else:
			(akrLabel, akrColor) = ("", 0xFFE8E2D2)
		gradeText.SetText(akrLabel)
		gradeText.SetPackedFontColor(akrColor)
		try:
			import uiAkravonSkillIcon
			icon = uiAkravonSkillIcon.GetIcon(skillIndex, skillGrade)	# P = kademe 3 (_04)
		except Exception:
			icon = None
		if icon:
			try:
				hd.LoadImage(icon.replace("akravon_skill/", "akravon_skill_oct/"))	# orijinal gibi sekizgen
				hd.SetScale(48.0 / 96.0, 48.0 / 96.0)
			except Exception:
				hd.LoadImage(icon)
				hd.SetScale(48.0 / 64.0, 48.0 / 64.0)
			hd.Show()
		else:
			hd.Hide()

	def AkravonShowBonus(self):
		# AKRAVON: karakter bonuslari pencerenin sag alanina gomulu acilir
		for tabButton in self.tabButtonDict.itervalues():
			tabButton.SetUp()
		for akrDict in (self.tabDict, self.pageDict, self.titleBarDict):
			for akrValue in akrDict.itervalues():
				akrValue.Hide()
		self.titleBarDict["STATUS"].Show()
		try:
			self.GetChild("Akravon_Nav_Bonus").Show()
			self.GetChild("Tab_Button_05").Down()
		except Exception:
			pass
		if not getattr(self, "akravonBonus", None):
			import uiAkravonBonus
			self.akravonBonus = uiAkravonBonus.BonusPanel(426, 424)
			self.akravonBonus.SetParent(self.GetChild("board"))
			self.akravonBonus.SetPosition(170, 66)
		self.akravonBonus.Refresh()
		self.akravonBonus.Show()

	def __OnClickTabButton(self, stateKey):
		self.SetState(stateKey)

	def SetState(self, stateKey):

		self.state = stateKey
		try:
			self.GetChild("Akravon_Nav_Bonus").Hide()
			self.GetChild("Tab_Button_05").SetUp()
			if self.chDetailsWnd:
				self.chDetailsWnd.Hide()
			if getattr(self, "akravonBonus", None):
				self.akravonBonus.Hide()
		except Exception:
			pass

		for (tabKey, tabButton) in self.tabButtonDict.items():
			if stateKey!=tabKey:
				tabButton.SetUp()

		for tabValue in self.tabDict.itervalues():
			tabValue.Hide()

		for pageValue in self.pageDict.itervalues():
			pageValue.Hide()

		for titleBarValue in self.titleBarDict.itervalues():
			titleBarValue.Hide()

		self.titleBarDict[stateKey].Show()
		self.tabDict[stateKey].Show()
		self.pageDict[stateKey].Show()


	def GetState(self):
		return self.state

	def __GetTotalAtkText(self):
		minAtk=player.GetStatus(player.ATT_MIN)
		maxAtk=player.GetStatus(player.ATT_MAX)
		atkBonus=player.GetStatus(player.ATT_BONUS)
		attackerBonus=player.GetStatus(player.ATTACKER_BONUS)

		if minAtk==maxAtk:
			return "%d" % (minAtk+atkBonus+attackerBonus)
		else:
			return "%d-%d" % (minAtk+atkBonus+attackerBonus, maxAtk+atkBonus+attackerBonus)

	def __GetTotalMagAtkText(self):
		minMagAtk=player.GetStatus(player.MAG_ATT)+player.GetStatus(player.MIN_MAGIC_WEP)
		maxMagAtk=player.GetStatus(player.MAG_ATT)+player.GetStatus(player.MAX_MAGIC_WEP)

		if minMagAtk==maxMagAtk:
			return "%d" % (minMagAtk)
		else:
			return "%d-%d" % (minMagAtk, maxMagAtk)

	def __GetTotalDefText(self):
		defValue=player.GetStatus(player.DEF_GRADE)
		if constInfo.ADD_DEF_BONUS_ENABLE:
			defValue+=player.GetStatus(player.DEF_BONUS)
		return "%d" % (defValue)

	def RefreshStatus(self):
		if self.isLoaded==0:
			return

		try:
			self.GetChild("Level_Value").SetText(str(player.GetStatus(player.LEVEL)))
			self.GetChild("Exp_Value").SetText(str(unsigned32(player.GetEXP())))
			self.GetChild("RestExp_Value").SetText(str(unsigned32(player.GetStatus(player.NEXT_EXP)) - unsigned32(player.GetStatus(player.EXP))))
			self.GetChild("HP_Value").SetText(str(player.GetStatus(player.HP)) + '/' + str(player.GetStatus(player.MAX_HP)))
			self.GetChild("SP_Value").SetText(str(player.GetStatus(player.SP)) + '/' + str(player.GetStatus(player.MAX_SP)))
			# AKRAVON: EXP / HP / SP cubuk doluluklari
			for (akrName, akrCur, akrMax) in (("Exp_Fill", unsigned32(player.GetEXP()), unsigned32(player.GetStatus(player.NEXT_EXP))),
					("HP_Fill", player.GetStatus(player.HP), player.GetStatus(player.MAX_HP)),
					("SP_Fill", player.GetStatus(player.SP), player.GetStatus(player.MAX_SP))):
				try:
					akrRate = min(1.0, max(0.0, float(akrCur) / float(akrMax))) if akrMax > 0 else 0.0
					self.GetChild(akrName).SetRenderingRect(0.0, 0.0, akrRate - 1.0, 0.0)
				except Exception:
					pass

			self.GetChild("STR_Value").SetText(str(player.GetStatus(player.ST)))
			self.GetChild("DEX_Value").SetText(str(player.GetStatus(player.DX)))
			self.GetChild("HTH_Value").SetText(str(player.GetStatus(player.HT)))
			self.GetChild("INT_Value").SetText(str(player.GetStatus(player.IQ)))

			self.GetChild("ATT_Value").SetText(self.__GetTotalAtkText())
			self.GetChild("DEF_Value").SetText(self.__GetTotalDefText())

			self.GetChild("MATT_Value").SetText(self.__GetTotalMagAtkText())
			#self.GetChild("MATT_Value").SetText(str(player.GetStatus(player.MAG_ATT)))

			self.GetChild("MDEF_Value").SetText(str(player.GetStatus(player.MAG_DEF)))
			self.GetChild("ASPD_Value").SetText(str(player.GetStatus(player.ATT_SPEED)))
			self.GetChild("MSPD_Value").SetText(str(player.GetStatus(player.MOVING_SPEED)))
			self.GetChild("CSPD_Value").SetText(str(player.GetStatus(player.CASTING_SPEED)))
			self.GetChild("ER_Value").SetText(str(player.GetStatus(player.EVADE_RATE)))

		except:
			#import exception
			#exception.Abort("CharacterWindow.RefreshStatus.BindObject")
			pass

		self.__RefreshStatusPlusButtonList()
		self.__RefreshStatusMinusButtonList()
		self.RefreshAlignment()

		if self.refreshToolTip:
			self.refreshToolTip()

		if app.ENABLE_DETAILS_INTERFACE:
			if self.chDetailsWnd:
				if self.chDetailsWnd.IsShow():
					self.chDetailsWnd.Refresh()
	def __RefreshStatusPlusButtonList(self):
		if self.isLoaded==0:
			return

		statusPlusPoint=player.GetStatus(player.STAT)

		# AKRAVON: artir (+) butonlari hep gorunur; puan yoksa soluk
		self.ShowStatusPlusButtonList()
		self.AkravonDim(self.statusPlusButtonDict, statusPlusPoint > 0)
		if statusPlusPoint>0:
			self.statusPlusValue.SetText(str(statusPlusPoint))
			self.statusPlusLabel.Show()
		else:
			self.statusPlusValue.SetText(str(0))
			self.statusPlusLabel.Hide()

	def __RefreshStatusMinusButtonList(self):
		if self.isLoaded==0:
			return

		statusMinusPoint=self.__GetStatMinusPoint()

		self.__HideStatusMinusButtonList()	# AKRAVON: orijinal Metin2'de statu dusurme yok

	def AkravonDim(self, buttonDict, usable):
		# AKRAVON: dugmeler hep gorunur (SetDiffuseColor ui.Button uzerinde cokertiyor: soluklastirma yok)
		return

	def RefreshAlignment(self):
		point, grade = player.GetAlignmentData()

		import colorInfo
		COLOR_DICT = {	0 : colorInfo.TITLE_RGB_GOOD_31,
						1 : colorInfo.TITLE_RGB_GOOD_30,
						2 : colorInfo.TITLE_RGB_GOOD_29,
						3 : colorInfo.TITLE_RGB_GOOD_28,
						4 : colorInfo.TITLE_RGB_GOOD_27,
						5 : colorInfo.TITLE_RGB_GOOD_26,
						6 : colorInfo.TITLE_RGB_GOOD_25,
						7 : colorInfo.TITLE_RGB_GOOD_24,
						8 : colorInfo.TITLE_RGB_GOOD_23,
						9 : colorInfo.TITLE_RGB_GOOD_22,
						10 : colorInfo.TITLE_RGB_GOOD_21,
						11 : colorInfo.TITLE_RGB_GOOD_20,
						12 : colorInfo.TITLE_RGB_GOOD_19,
						13 : colorInfo.TITLE_RGB_GOOD_18,
						14 : colorInfo.TITLE_RGB_GOOD_17,
						15 : colorInfo.TITLE_RGB_GOOD_16,
						16 : colorInfo.TITLE_RGB_GOOD_15,
						17 : colorInfo.TITLE_RGB_GOOD_14,
						18 : colorInfo.TITLE_RGB_GOOD_13,
						19 : colorInfo.TITLE_RGB_GOOD_12,
						20 : colorInfo.TITLE_RGB_GOOD_11,
						21 : colorInfo.TITLE_RGB_GOOD_10,
						22 : colorInfo.TITLE_RGB_GOOD_9,
						23 : colorInfo.TITLE_RGB_GOOD_8,
						24 : colorInfo.TITLE_RGB_GOOD_7,
						25 : colorInfo.TITLE_RGB_GOOD_6,
						26 : colorInfo.TITLE_RGB_GOOD_5,
						27 : colorInfo.TITLE_RGB_GOOD_4,
						28 : colorInfo.TITLE_RGB_GOOD_3,
						29 : colorInfo.TITLE_RGB_GOOD_2,
						30 : colorInfo.TITLE_RGB_GOOD_1,
						31 : colorInfo.TITLE_RGB_NORMAL,
						32 : colorInfo.TITLE_RGB_EVIL_1,
						33 : colorInfo.TITLE_RGB_EVIL_2,
						34 : colorInfo.TITLE_RGB_EVIL_3,
						35 : colorInfo.TITLE_RGB_EVIL_4, }

		colorList = COLOR_DICT.get(grade, colorInfo.TITLE_RGB_NORMAL)
		gradeColor = ui.GenerateColor(colorList[0], colorList[1], colorList[2])
		# AKRAVON: derece karti (ad + puan)
		try:
			self.GetChild("Align_Name").SetText(localeInfo.TITLE_NAME_LIST[grade])
			self.GetChild("Align_Name").SetPackedFontColor(gradeColor)
			self.GetChild("Align_Value").SetText("Puan : " + (localeInfo.NumberToMoneyString(point).replace(" Yang", "") if hasattr(localeInfo, "NumberToMoneyString") else str(point)))
		except Exception:
			pass

		self.toolTipAlignment.ClearToolTip()
		self.toolTipAlignment.AutoAppendTextLine(localeInfo.TITLE_NAME_LIST[grade], gradeColor)
		self.toolTipAlignment.AutoAppendTextLine(localeInfo.ALIGNMENT_NAME + str(point))
		self.toolTipAlignment.AppendSpace(2)

		if (grade < 27):
			global ALIGNMENT_LIST
			iNum = int(26-grade)
			isArry = ALIGNMENT_LIST[iNum]

			self.toolTipAlignment.AutoAppendTextLine(str(isArry[0][0] % isArry[0][1]), self.toolTipAlignment.POSITIVE_COLOR)
			self.toolTipAlignment.AutoAppendTextLine(str(isArry[0][2] % isArry[0][3]), self.toolTipAlignment.POSITIVE_COLOR)
			self.toolTipAlignment.AutoAppendTextLine(str(isArry[0][4] % isArry[0][5]), self.toolTipAlignment.POSITIVE_COLOR)
			self.toolTipAlignment.AppendSpace(3)

		self.toolTipAlignment.AutoAppendTextLine(localeInfo.ALIGN_REWARD_DESCRIPTION)
		self.toolTipAlignment.AppendSpace(3)

		self.toolTipAlignment.AlignHorizonalCenter()

	def __ShowStatusMinusButtonList(self):
		for (stateMinusKey, statusMinusButton) in self.statusMinusButtonDict.items():
			statusMinusButton.Show()

	def __HideStatusMinusButtonList(self):
		for (stateMinusKey, statusMinusButton) in self.statusMinusButtonDict.items():
			statusMinusButton.Hide()

	def ShowStatusPlusButtonList(self):
		for (statePlusKey, statusPlusButton) in self.statusPlusButtonDict.items():
			statusPlusButton.Show()

	def HideStatusPlusButtonList(self):
		for (statePlusKey, statusPlusButton) in self.statusPlusButtonDict.items():
			statusPlusButton.Hide()

	def SelectSkill(self, skillSlotIndex):

		mouseController = mouseModule.mouseController

		if False == mouseController.isAttached():

			srcSlotIndex = self.__RealSkillSlotToSourceSlot(skillSlotIndex)
			selectedSkillIndex = player.GetSkillIndex(srcSlotIndex)

			if skill.CanUseSkill(selectedSkillIndex):

				if app.IsPressed(app.DIK_LCONTROL):

					player.RequestAddToEmptyLocalQuickSlot(player.SLOT_TYPE_SKILL, srcSlotIndex)
					return

				mouseController.AttachObject(self, player.SLOT_TYPE_SKILL, srcSlotIndex, selectedSkillIndex)

		else:

			mouseController.DeattachObject()

	def SelectEmptySlot(self, SlotIndex):
		mouseModule.mouseController.DeattachObject()

	## ToolTip
	def OverInItem(self, slotNumber):

		if mouseModule.mouseController.isAttached():
			return

		if 0 == self.toolTipSkill:
			return

		srcSlotIndex = self.__RealSkillSlotToSourceSlot(slotNumber)
		skillIndex = player.GetSkillIndex(srcSlotIndex)
		skillLevel = player.GetSkillLevel(srcSlotIndex)
		skillGrade = player.GetSkillGrade(srcSlotIndex)
		skillType = skill.GetSkillType(skillIndex)

		## ACTIVE
		if skill.SKILL_TYPE_ACTIVE == skillType:
			overInSkillGrade = self.__GetSkillGradeFromSlot(slotNumber)

			if overInSkillGrade == skill.SKILL_GRADE_COUNT-1 and skillGrade == skill.SKILL_GRADE_COUNT:
				self.toolTipSkill.SetSkillNew(srcSlotIndex, skillIndex, skillGrade, skillLevel)
			elif overInSkillGrade == skillGrade:
				self.toolTipSkill.SetSkillNew(srcSlotIndex, skillIndex, overInSkillGrade, skillLevel)
			else:
				self.toolTipSkill.SetSkillOnlyName(srcSlotIndex, skillIndex, overInSkillGrade)


		else:
			self.toolTipSkill.SetSkillNew(srcSlotIndex, skillIndex, skillGrade, skillLevel)

	def OverOutItem(self):
		if 0 != self.toolTipSkill:
			self.toolTipSkill.HideToolTip()

	## Quest
	def __SelectQuest(self, slotIndex):
		questIndex = quest.GetQuestIndex(self.questShowingStartIndex+slotIndex)

		import event
		event.QuestButtonClick(-2147483648 + questIndex)

	def RefreshQuest(self):

		if self.isLoaded==0:
			return

		questCount = quest.GetQuestCount()
		questRange = range(quest.QUEST_MAX_NUM)

		if questCount > quest.QUEST_MAX_NUM:
			self.questScrollBar.Show()
		else:
			self.questScrollBar.Hide()

		for i in questRange[:questCount]:
			(questName, questIcon, questCounterName, questCounterValue) = quest.GetQuestData(self.questShowingStartIndex+i)
			self.questNameList[i].SetText(questName)
			self.questNameList[i].Show()
			self.questLastCountList[i].Show()
			self.questLastTimeList[i].Show()

			if len(questCounterName) > 0:
				self.questLastCountList[i].SetText("%s : %d" % (questCounterName, questCounterValue))
			else:
				self.questLastCountList[i].SetText("")

			## Icon
			self.questSlot.SetSlot(i, i, 1, 1, questIcon)

		for i in questRange[questCount:]:
			self.questNameList[i].Hide()
			self.questLastTimeList[i].Hide()
			self.questLastCountList[i].Hide()
			self.questSlot.ClearSlot(i)
			self.questSlot.HideSlotBaseImage(i)

		self.__UpdateQuestClock()

	def __UpdateQuestClock(self):
		if "QUEST" == self.state:
			# QUEST_LIMIT_COUNT_BUG_FIX
			for i in xrange(min(quest.GetQuestCount(), quest.QUEST_MAX_NUM)):
			# END_OF_QUEST_LIMIT_COUNT_BUG_FIX
				(lastName, lastTime) = quest.GetQuestLastTime(i)

				clockText = localeInfo.QUEST_UNLIMITED_TIME
				if len(lastName) > 0:

					if lastTime <= 0:
						clockText = localeInfo.QUEST_TIMEOVER

					else:
						questLastMinute = lastTime / 60
						questLastSecond = lastTime % 60

						clockText = lastName + " : "

						if questLastMinute > 0:
							clockText += str(questLastMinute) + localeInfo.QUEST_MIN
							if questLastSecond > 0:
								clockText += " "

						if questLastSecond > 0:
							clockText += str(questLastSecond) + localeInfo.QUEST_SEC

				self.questLastTimeList[i].SetText(clockText)

	def __GetStatMinusPoint(self):
		POINT_STAT_RESET_COUNT = 112
		return player.GetStatus(POINT_STAT_RESET_COUNT)

	def __OverInStatMinusButton(self, stat):
		try:
			self.__ShowStatToolTip(self.STAT_MINUS_DESCRIPTION[stat] % self.__GetStatMinusPoint())
		except KeyError:
			pass

		self.refreshToolTip = lambda arg=stat: self.__OverInStatMinusButton(arg)

	def __OverOutStatMinusButton(self):
		self.__HideStatToolTip()
		self.refreshToolTip = 0

	def __OverInStatButton(self, stat):
		return	# AKRAVON: mobilde ipucu takili kaliyordu
		try:
			# AKRAVON: mobilde Ctrl yok - "Ctrl + tik +10 statu" ipucu gosterilmez
			self.__ShowStatToolTip(self.STAT_DESCRIPTION[stat])
		except KeyError:
			pass

	def __OverOutStatButton(self):
		self.__HideStatToolTip()

	def __ShowStatToolTip(self, statDesc):
		self.toolTip.ClearToolTip()
		self.toolTip.AppendTextLine(statDesc)
		self.toolTip.Show()

	def __HideStatToolTip(self):
		self.toolTip.Hide()

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def OnUpdate(self):
		self.__UpdateQuestClock()

	## Skill Process
	def __RefreshSkillPage(self, name, slotCount):
		global SHOW_LIMIT_SUPPORT_SKILL_LIST

		skillPage = self.skillPageDict[name]

		startSlotIndex = skillPage.GetStartIndex()
		if "ACTIVE" == name:
			if self.PAGE_HORSE == self.curSelectedSkillGroup:
				startSlotIndex += slotCount

		getSkillType=skill.GetSkillType
		getSkillIndex=player.GetSkillIndex
		getSkillGrade=player.GetSkillGrade
		getSkillLevel=player.GetSkillLevel
		getSkillLevelUpPoint=skill.GetSkillLevelUpPoint
		getSkillMaxLevel=skill.GetSkillMaxLevel

		for i in xrange(slotCount+1):
			slotIndex = i + startSlotIndex
			skillIndex = getSkillIndex(slotIndex)
			if app.ENABLE_SLOT_WINDOW_EX:
				activeList = {}
				cooltmList = {}

			for j in xrange(skill.SKILL_GRADE_COUNT):
				realSlotIndex = self.__GetRealSkillSlot(j, i)
				if app.ENABLE_SLOT_WINDOW_EX:
					activeList[realSlotIndex] = skillPage.IsActivatedSlot(realSlotIndex)
					cooltmList[realSlotIndex] = skillPage.GetSlotCoolTime(realSlotIndex)
				skillPage.ClearSlot(realSlotIndex)

			if "ACTIVE" == name:
				self.AkravonSkillCard(slotIndex, 0, 0, 0)	# AKRAVON: grup degisince eski kart artigi kalmasin
			if 0 == skillIndex:
				continue

			skillGrade = getSkillGrade(slotIndex)
			skillLevel = getSkillLevel(slotIndex)
			skillType = getSkillType(skillIndex)

			if player.SKILL_INDEX_RIDING == skillIndex:
				if 1 == skillGrade:
					skillLevel += 19
				elif 2 == skillGrade:
					skillLevel += 29
				elif 3 == skillGrade:
					skillLevel = 40

				skillPage.SetSkillSlotNew(slotIndex, skillIndex, max(skillLevel-1, 0), skillLevel)
				skillPage.SetSlotCount(slotIndex, skillLevel)
				if "ACTIVE" == name:
					self.AkravonSkillCard(slotIndex, skillIndex, 0, skillLevel)	# AKRAVON: binicilik de kart olarak gorunur

			## ACTIVE
			elif skill.SKILL_TYPE_ACTIVE == skillType:
				for j in xrange(skill.SKILL_GRADE_COUNT):
					realSlotIndex = self.__GetRealSkillSlot(j, slotIndex)
					skillPage.SetSkillSlotNew(realSlotIndex, skillIndex, j, skillLevel)
					skillPage.SetCoverButton(realSlotIndex, "d:/ymir work/ui/akravon_legacy/cw_clear32.png", "d:/ymir work/ui/akravon_legacy/cw_clear32.png", "d:/ymir work/ui/akravon_legacy/cw_clear32.png", "d:/ymir work/ui/akravon_legacy/cw_clear32.png")

					if (skillGrade == skill.SKILL_GRADE_COUNT) and j == (skill.SKILL_GRADE_COUNT-1):
						skillPage.SetSlotCountNew(realSlotIndex, skillGrade, skillLevel)
					elif (not self.__CanUseSkillNow()) or (skillGrade != j):
						skillPage.SetSlotCount(realSlotIndex, 0)
						skillPage.DisableCoverButton(realSlotIndex)
					else:
						skillPage.SetSlotCountNew(realSlotIndex, skillGrade, skillLevel)

					if app.ENABLE_SLOT_WINDOW_EX:
						isActive = activeList.get(realSlotIndex, False)
						coolTime,elapsedTime = cooltmList.get(realSlotIndex, (0,0))
						if isActive:
							skillPage.ActivateSlot(realSlotIndex)
						if coolTime:
							skillPage.SetSlotCoolTime(realSlotIndex, coolTime, elapsedTime)
				self.AkravonSkillCard(slotIndex, skillIndex, skillGrade, skillLevel)
			else:
				if not SHOW_LIMIT_SUPPORT_SKILL_LIST or skillIndex in SHOW_LIMIT_SUPPORT_SKILL_LIST:
					realSlotIndex = self.__GetETCSkillRealSlotIndex(slotIndex)
					skillPage.SetSkillSlot(realSlotIndex, skillIndex, skillLevel)
					try:
						skillPage.HideSlotBaseImage(realSlotIndex)	# AKRAVON: arkadaki koyu kare zemin yok
					except Exception:
						pass
					try:
						import uiAkravonSkillIcon
						akrHD = self.GetChild("Support_HD_%d" % (realSlotIndex - 101))
						akrIcon = uiAkravonSkillIcon.GetIcon(skillIndex, 0)
						akrHD.Hide()	# AKRAVON: destek becerisi = orijinal ikon + sekizgen cerceve (cw_supportframe)
					except Exception:
						pass
					skillPage.SetSlotCountNew(realSlotIndex, skillGrade, skillLevel)

					if skill.CanUseSkill(skillIndex):
						skillPage.SetCoverButton(realSlotIndex, "d:/ymir work/ui/akravon_legacy/cw_clear32.png", "d:/ymir work/ui/akravon_legacy/cw_clear32.png", "d:/ymir work/ui/akravon_legacy/cw_clear32.png", "d:/ymir work/ui/akravon_legacy/cw_clear32.png")

			skillPage.RefreshSlot()

	def RefreshSkill(self):

		if self.isLoaded==0:
			return

		if self.__IsChangedHorseRidingSkillLevel():
			self.RefreshCharacter()
			return


		global SHOW_ONLY_ACTIVE_SKILL
		if SHOW_ONLY_ACTIVE_SKILL:
			self.__RefreshSkillPage("ACTIVE", self.ACTIVE_PAGE_SLOT_COUNT)
		else:
			self.__RefreshSkillPage("ACTIVE", self.ACTIVE_PAGE_SLOT_COUNT)
			self.__RefreshSkillPage("SUPPORT", self.SUPPORT_PAGE_SLOT_COUNT)

		self.RefreshSkillPlusButtonList()

	def CanShowPlusButton(self, skillIndex, skillLevel, curStatPoint):

		if 0 == skillIndex:
			return False

		if not skill.CanLevelUpSkill(skillIndex, skillLevel):
			return False

		return True

	def __RefreshSkillPlusButton(self, name):
		global HIDE_SUPPORT_SKILL_POINT
		if HIDE_SUPPORT_SKILL_POINT and "SUPPORT" == name:
			return

		slotWindow = self.skillPageDict[name]
		slotWindow.HideAllSlotButton()

		slotStatType = self.skillPageStatDict[name]
		if 0 == slotStatType:
			return

		statPoint = player.GetStatus(slotStatType)
		startSlotIndex = slotWindow.GetStartIndex()
		if "HORSE" == name:
			startSlotIndex += self.ACTIVE_PAGE_SLOT_COUNT

		if statPoint > 0:
			for i in xrange(self.PAGE_SLOT_COUNT):
				slotIndex = i + startSlotIndex
				skillIndex = player.GetSkillIndex(slotIndex)
				skillGrade = player.GetSkillGrade(slotIndex)
				skillLevel = player.GetSkillLevel(slotIndex)

				if skillIndex == 0:
					continue
				if skillGrade != 0:
					continue

				if name == "HORSE":
					if player.GetStatus(player.LEVEL) >= skill.GetSkillLevelLimit(skillIndex):
						if skillLevel < 20:
							slotWindow.ShowSlotButton(self.__GetETCSkillRealSlotIndex(slotIndex))

				else:
					if "SUPPORT" == name:
						if not SHOW_LIMIT_SUPPORT_SKILL_LIST or skillIndex in SHOW_LIMIT_SUPPORT_SKILL_LIST:
							if self.CanShowPlusButton(skillIndex, skillLevel, statPoint):
								slotWindow.ShowSlotButton(slotIndex)
					else:
						if self.CanShowPlusButton(skillIndex, skillLevel, statPoint):
							slotWindow.ShowSlotButton(slotIndex)


	def RefreshSkillPlusButtonList(self):

		if self.isLoaded==0:
			return

		self.RefreshSkillPlusPointLabel()

		if not self.__CanUseSkillNow():
			return

		try:
			if self.PAGE_HORSE == self.curSelectedSkillGroup:
				self.__RefreshSkillPlusButton("HORSE")
			else:
				self.__RefreshSkillPlusButton("ACTIVE")

			self.__RefreshSkillPlusButton("SUPPORT")

		except:
			import exception
			exception.Abort("CharacterWindow.RefreshSkillPlusButtonList.BindObject")

	def RefreshSkillPlusPointLabel(self):
		if self.isLoaded==0:
			return

		if self.PAGE_HORSE == self.curSelectedSkillGroup:
			activeStatPoint = player.GetStatus(player.SKILL_HORSE)
			self.activeSkillPointValue.SetText(str(activeStatPoint))

		else:
			activeStatPoint = player.GetStatus(player.SKILL_ACTIVE)
			self.activeSkillPointValue.SetText(str(activeStatPoint))

		supportStatPoint = max(0, player.GetStatus(player.SKILL_SUPPORT))
		self.supportSkillPointValue.SetText(str(supportStatPoint))

	## Skill Level Up Button
	def OnPressedSlotButton(self, slotNumber):
		srcSlotIndex = self.__RealSkillSlotToSourceSlot(slotNumber)

		skillIndex = player.GetSkillIndex(srcSlotIndex)
		curLevel = player.GetSkillLevel(srcSlotIndex)
		maxLevel = skill.GetSkillMaxLevel(skillIndex)

		net.SendChatPacket("/skillup " + str(skillIndex))

	## Use Skill
	def ClickSkillSlot(self, slotIndex):

		srcSlotIndex = self.__RealSkillSlotToSourceSlot(slotIndex)
		skillIndex = player.GetSkillIndex(srcSlotIndex)
		skillType = skill.GetSkillType(skillIndex)

		if not self.__CanUseSkillNow():
			if skill.SKILL_TYPE_ACTIVE == skillType:
				return

		for slotWindow in self.skillPageDict.values():
			if slotWindow.HasSlot(slotIndex):
				if skill.CanUseSkill(skillIndex):
					player.ClickSkillSlot(srcSlotIndex)
					return

		mouseModule.mouseController.DeattachObject()

	def OnUseSkill(self, slotIndex, coolTime):

		skillIndex = player.GetSkillIndex(slotIndex)
		skillType = skill.GetSkillType(skillIndex)

		## ACTIVE
		if skill.SKILL_TYPE_ACTIVE == skillType:
			skillGrade = player.GetSkillGrade(slotIndex)
			slotIndex = self.__GetRealSkillSlot(skillGrade, slotIndex)
		## ETC
		else:
			slotIndex = self.__GetETCSkillRealSlotIndex(slotIndex)

		for slotWindow in self.skillPageDict.values():
			if slotWindow.HasSlot(slotIndex):
				slotWindow.SetSlotCoolTime(slotIndex, coolTime)
				return

	def OnActivateSkill(self, slotIndex):
		skillGrade = player.GetSkillGrade(slotIndex)
		slotIndex = self.__GetRealSkillSlot(skillGrade, slotIndex)
		for slotWindow in self.skillPageDict.values():
			if slotWindow.HasSlot(slotIndex):
				slotWindow.ActivateSlot(slotIndex)
				return

	def OnDeactivateSkill(self, slotIndex):
		skillGrade = player.GetSkillGrade(slotIndex)
		slotIndex = self.__GetRealSkillSlot(skillGrade, slotIndex)
		for slotWindow in self.skillPageDict.values():
			if slotWindow.HasSlot(slotIndex):
				slotWindow.DeactivateSlot(slotIndex)
				return

	def __ShowJobToolTip(self):
		self.toolTipJob.ShowToolTip()

	def __HideJobToolTip(self):
		self.toolTipJob.HideToolTip()

	def __SetJobText(self, mainJob, subJob):
		if player.GetStatus(player.LEVEL)<5:
			subJob=0

		if 949 == app.GetDefaultCodePage():
			self.toolTipJob.ClearToolTip()

			try:
				jobInfoTitle=localeInfo.JOBINFO_TITLE[mainJob][subJob]
				jobInfoData=localeInfo.JOBINFO_DATA_LIST[mainJob][subJob]
			except IndexError:
				print "uiCharacter.CharacterWindow.__SetJobText(mainJob=%d, subJob=%d)" % (mainJob, subJob)
				return

			self.toolTipJob.AutoAppendTextLine(jobInfoTitle)
			self.toolTipJob.AppendSpace(5)

			for jobInfoDataLine in jobInfoData:
				self.toolTipJob.AutoAppendTextLine(jobInfoDataLine)

			self.toolTipJob.AlignHorizonalCenter()

	def __UseAlignmentToolTip(self):
		try:
			self.toolTipAlignment.HideToolTip()
		except Exception:
			pass
		# AKRAVON: klasik 9 derece, bonussuz liste (uiakravonalign.py)
		import uiAkravonAlign
		uiAkravonAlign.Open(self)

	def __ShowAlignmentToolTip(self):
		return	# AKRAVON: dokunmatikte ipucu takili kaliyordu; derece paneli kartin kendisinde aciliyor

	def __HideAlignmentToolTip(self):
		self.toolTipAlignment.HideToolTip()

	def RefreshCharacter(self):

		if self.isLoaded==0:
			return

		## Name
		try:
			characterName = player.GetName()
			guildName = player.GetGuildName()
			self.characterNameValue.SetText(characterName)
			self.guildNameValue.SetText(guildName)
			if not guildName:
				if localeInfo.IsARABIC():
					self.characterNameSlot.SetPosition(190, 34)
				else:
					self.characterNameSlot.SetPosition(14, 178)

				self.guildNameSlot.Hide()
			else:
				if localeInfo.IsJAPAN():
					self.characterNameSlot.SetPosition(143, 34)
				else:
					self.characterNameSlot.SetPosition(14, 178)
				self.guildNameSlot.Show()
		except:
			import exception
			exception.Abort("CharacterWindow.RefreshCharacter.BindObject")

		race = net.GetMainActorRace()
		group = net.GetMainActorSkillGroup()
		empire = net.GetMainActorEmpire()

		## Job Text
		job = chr.RaceToJob(race)
		self.__SetJobText(job, group)

		## FaceImage
		try:
			faceImageName = FACE_IMAGE_DICT[race]

			try:
				self.faceImage.LoadImage(faceImageName)
				try:
					self.faceImage.SetScale(2.0, 2.0)	# AKRAVON: buyuk portre
				except Exception:
					pass
			except:
				print "CharacterWindow.RefreshCharacter(race=%d, faceImageName=%s)" % (race, faceImageName)
				self.faceImage.Hide()

		except KeyError:
			self.faceImage.Hide()

		## GroupName
		self.__SetSkillGroupName(race, group)

		## Skill
		if 0 == group:
			self.__SelectSkillGroup(0)

		else:
			self.__SetSkillSlotData(race, group, empire)

			if self.__CanUseHorseSkill():
				self.__SelectSkillGroup(0)

	def __SetSkillGroupName(self, race, group):

		job = chr.RaceToJob(race)

		if not self.SKILL_GROUP_NAME_DICT.has_key(job):
			return

		nameList = self.SKILL_GROUP_NAME_DICT[job]

		if 0 == group:
			self.skillGroupButton1.SetText(nameList[1])
			self.skillGroupButton2.SetText(nameList[2])
			self.skillGroupButton1.Show()
			self.skillGroupButton2.Show()
			self.activeSkillGroupName.Hide()

			if app.ENABLE_WOLFMAN_CHARACTER and playerSettingModule.RACE_WOLFMAN_M == race:
				self.skillGroupButton2.Hide()
		else:

			if self.__CanUseHorseSkill():
				self.activeSkillGroupName.Hide()
				self.skillGroupButton1.SetText(nameList.get(group, "Noname"))
				self.skillGroupButton2.SetText(localeInfo.SKILL_GROUP_HORSE)
				self.skillGroupButton1.Show()
				self.skillGroupButton2.Show()

			else:
				self.activeSkillGroupName.SetText(nameList.get(group, "Noname"))
				self.activeSkillGroupName.Show()
				self.skillGroupButton1.Hide()
				self.skillGroupButton2.Hide()

	def __SetSkillSlotData(self, race, group, empire=0):

		## SkillIndex
		playerSettingModule.RegisterSkill(race, group, empire)

		## Event
		self.__SetSkillSlotEvent()

		## Refresh
		self.RefreshSkill()

	def __SelectSkillGroup(self, index):
		for btn in self.skillGroupButton:
			btn.SetUp()
		self.skillGroupButton[index].Down()

		if self.__CanUseHorseSkill():
			if 0 == index:
				index = net.GetMainActorSkillGroup()-1
			elif 1 == index:
				index = self.PAGE_HORSE

		self.curSelectedSkillGroup = index
		self.__SetSkillSlotData(net.GetMainActorRace(), index+1, net.GetMainActorEmpire())

	def __CanUseSkillNow(self):
		if 0 == net.GetMainActorSkillGroup():
			return False

		return True

	def __CanUseHorseSkill(self):
		return True#Mk

		slotIndex = player.GetSkillSlotIndex(player.SKILL_INDEX_RIDING)

		if not slotIndex:
			return False

		grade = player.GetSkillGrade(slotIndex)
		level = player.GetSkillLevel(slotIndex)
		if level < 0:
			level *= -1
		if grade >= 1 and level >= 1:
			return True

		return False

	def __IsChangedHorseRidingSkillLevel(self):
		ret = False

		if -1 == self.canUseHorseSkill:
			self.canUseHorseSkill = self.__CanUseHorseSkill()

		if self.canUseHorseSkill != self.__CanUseHorseSkill():
			ret = True

		self.canUseHorseSkill = self.__CanUseHorseSkill()
		return ret

	def __GetRealSkillSlot(self, skillGrade, skillSlot):
		return skillSlot + min(skill.SKILL_GRADE_COUNT-1, skillGrade)*skill.SKILL_GRADE_STEP_COUNT

	def __GetETCSkillRealSlotIndex(self, skillSlot):
		if skillSlot > 100:
			return skillSlot
		return skillSlot % self.ACTIVE_PAGE_SLOT_COUNT

	def __RealSkillSlotToSourceSlot(self, realSkillSlot):
		if realSkillSlot > 100:
			return realSkillSlot
		if self.PAGE_HORSE == self.curSelectedSkillGroup:
			return realSkillSlot + self.ACTIVE_PAGE_SLOT_COUNT
		return realSkillSlot % skill.SKILL_GRADE_STEP_COUNT

	def __GetSkillGradeFromSlot(self, skillSlot):
		return int(skillSlot / skill.SKILL_GRADE_STEP_COUNT)

	def SelectSkillGroup(self, index):
		self.__SelectSkillGroup(index)

	def OnQuestScroll(self):
		questCount = quest.GetQuestCount()
		scrollLineCount = max(0, questCount - quest.QUEST_MAX_NUM)
		startIndex = int(scrollLineCount * self.questScrollBar.GetPos())

		if startIndex != self.questShowingStartIndex:
			self.questShowingStartIndex = startIndex
			self.RefreshQuest()
