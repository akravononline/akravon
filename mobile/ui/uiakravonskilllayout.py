import ui
import wndMgr
import app
import uiMobileCompat
import player
import skill
import uiAkravonSkillGrade
import uiAkravonSkillIcon
import item
import dbg
import emotion
import uiAkravonActionIcon

ART = "d:/ymir work/ui/akravon_legacy/"


def Texture(parent, filename, x, y, width, height):
	image = ui.ExpandedImageBox()
	image.SetParent(parent)
	image.AddFlag("not_pick")
	image.LoadImage(ART + filename + ".png")
	image.SetScale(float(width) / image.GetWidth(), float(height) / image.GetHeight())
	image.SetPosition(x, y)
	image.Show()
	return image


def GetQuickSlotChoices(category):
	choices = []
	if category in ("skills", "support"):
		for source in range(1, 255):
			skillIndex = player.GetSkillIndex(source)
			if not skillIndex:
				continue
			if (category == "skills" and skill.GetSkillType(skillIndex) == skill.SKILL_TYPE_ACTIVE
					and player.GetSkillLevel(source) > 0) or (
					category == "support" and skillIndex in uiAkravonActionIcon.SUPPORT_IDS):
				grade = player.GetSkillGrade(source)
				label = skill.GetSkillName(skillIndex)
				choices.append((source, skillIndex, label, uiAkravonSkillIcon.GetIcon(skillIndex, grade)))
	elif category == "emotions":
		for index in sorted(emotion.EMOTION_DICT):
			icon = uiAkravonActionIcon.GetEmotionIcon(index)
			if icon:
				choices.append((index, index, emotion.EMOTION_DICT[index]["name"], icon))
	else:
		for source in range(player.INVENTORY_SLOT_COUNT):
			itemIndex = player.GetItemIndex(source)
			count = player.GetItemCount(source)
			if itemIndex and count > 0 and item.CanAddToQuickSlotItem(itemIndex):
				item.SelectItem(itemIndex)
				choices.append((source, itemIndex, item.GetItemName(),
					item.GetIconImageFileName()))
	return choices


def ClampPosition(x, y, size, width, height):
	return (max(0, min(int(round(x)), width - size)),
		max(0, min(int(round(y)), height - int(size * 1.11 + 1))))


def Overlaps(a, b, size):
	radius = size / 2.0 + 2
	if (a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2 < (radius * 2) ** 2:
		return True
	for circle, plaque in ((a, b), (b, a)):
		cx, cy = circle[0] + size / 2.0, circle[1] + size / 2.0
		left, right = plaque[0] + size * 0.20, plaque[0] + size * 0.80
		top, bottom = plaque[1] + size * 0.84, plaque[1] + size * 1.11
		nearX, nearY = max(left, min(cx, right)), max(top, min(cy, bottom))
		if (cx - nearX) ** 2 + (cy - nearY) ** 2 < radius ** 2:
			return True
	return abs(a[0] - b[0]) < size * 0.60 + 3 and abs(a[1] - b[1]) < size * 0.27 + 3


class EditButton(ui.Window):
	def __init__(self, parent, x, text, event):
		ui.Window.__init__(self, "UI")
		if parent:
			self.SetParent(parent)
		self.SetPosition(x, 0)
		self.SetSize(104, 32)
		self.event = event
		self.background = Texture(self, "cw_skillcard", 0, 0, 104, 32)
		self.text = ui.TextLine()
		self.text.SetParent(self)
		self.text.AddFlag("not_pick")
		self.text.SetFontName("Lato:18")
		self.text.SetPackedFontColor(0xFFFFE6AC)
		self.text.SetHorizontalAlignCenter()
		self.text.SetPosition(52, 5)
		self.text.SetText(text)
		self.text.Show()
		ui.EnableSingleTap(self)
		self.SetTouchTapEvent(ui.__mem_func__(self.OnTap))
		self.Show()

	def SetButtonSize(self, width, height):
		self.SetSize(width, height)
		self.background.SetScale(float(width) / 167.0, float(height) / 56.0)
		self.text.SetPosition(width // 2, (height - 16) // 2)

	def SetSelected(self, selected):
		width, height = self.GetWidth(), self.GetHeight()
		self.background.LoadImage(ART + ("cw_group_down.png" if selected else "cw_skillcard.png"))
		self.background.SetScale(float(width) / self.background.GetWidth(), float(height) / self.background.GetHeight())
		self.text.SetPackedFontColor(0xFF2A1608 if selected else 0xFFE8E2D2)
		self.text.SetOutline(False)

	def OnTap(self, x, y):
		self.event()

	def OnMouseLeftButtonDown(self):
		self.event()
		return True


class HudIconButton(ui.Window):
	def __init__(self, size, texture, event):
		ui.Window.__init__(self, "UI")
		self.AddFlag("float")
		self.SetSize(size, size)
		self.event = event
		self.face = ui.ExpandedImageBox()
		self.face.SetParent(self)
		self.face.AddFlag("not_pick")
		self.face.LoadImage(texture if texture.startswith("d:") else "mobile_ui/akravon/hud/%s.png" % texture)
		self.face.SetScale(float(size) / self.face.GetWidth(), float(size) / self.face.GetHeight())
		self.face.SetPosition(0, 0)
		self.face.Show()
		ui.EnableSingleTap(self)
		self.SetTouchTapEvent(ui.__mem_func__(self.OnTap))

	def OnTap(self, x, y):
		self.event()

	def OnMouseLeftButtonDown(self):
		self.event()
		return True

	def Destroy(self):
		self.ClearTouchEvents()
		self.event = None
		self.Hide()


class DragHandle(ui.Window):
	def __init__(self, owner, index, size):
		ui.Window.__init__(self, "UI")
		self.owner = owner
		self.index = index
		self.touchId = None
		self.touchOffset = (0, 0)
		self.touchStart = (0, 0)
		self.touchMoved = False
		self.mouseStart = None
		self.mouseMoved = False
		uiMobileCompat.SetTouchNative(self)
		self.SetSize(size, int(size * 1.11 + 1))
		self.AddFlag("float")
		self.AddFlag("movable")
		self.border = ui.Box()
		self.border.SetParent(self)
		self.border.AddFlag("not_pick")
		self.border.SetSize(size, int(size * 1.11 + 1))
		self.border.SetPosition(0, 0)
		self.border.SetColor(0xFFFFD16A)
		self.border.Show()

	def OnMouseLeftButtonDown(self):
		self.mouseStart = wndMgr.GetMousePosition()
		self.mouseMoved = False
		return True

	def OnMoveWindow(self, x, y):
		if self.mouseStart is not None:
			mx, my = wndMgr.GetMousePosition()
			if abs(mx - self.mouseStart[0]) + abs(my - self.mouseStart[1]) > 8:
				self.mouseMoved = True
		self.owner.Move(self.index, x, y)

	def OnMouseLeftButtonUp(self):
		self.SetPosition(*self.owner.arc.slots[self.index].GetLocalPosition())
		if self.mouseStart is not None and not self.mouseMoved:
			self.owner.arc.SelectSkill(self.index)
		self.mouseStart = None
		return True

	def OnUpdate(self):
		if not self.owner or not self.owner.editing or not self.IsShow():
			return
		if self.touchId is not None:
			for i in range(app.GetTouchCount()):
				point = app.GetTouchPoint(i)
				if point and point[0] == self.touchId:
					if abs(point[1] - self.touchStart[0]) + abs(point[2] - self.touchStart[1]) > 8:
						self.touchMoved = True
					self.owner.Move(self.index, point[1] - self.touchOffset[0],
						point[2] - self.touchOffset[1])
					return
			self.touchId = None
			self.owner.activeHandle = None
			self.SetPosition(*self.owner.arc.slots[self.index].GetLocalPosition())
			if not self.touchMoved:
				self.owner.arc.SelectSkill(self.index)
		elif self.owner.activeHandle is None:
			x, y = self.GetGlobalPosition()
			for i in range(app.GetTouchCount()):
				point = app.GetTouchPointEx(i)
				if point and point[3] == 2 and x <= point[1] < x + self.GetWidth() \
						and y <= point[2] < y + self.GetHeight():
					self.touchId = point[0]
					self.touchOffset = (point[1] - x, point[2] - y)
					self.touchStart = (point[1], point[2])
					self.touchMoved = False
					self.owner.activeHandle = self.index
					return


class LayoutEditor(object):
	def __init__(self, arc, size, buttonPosition = None, buttonSize = 36):
		self.arc = arc
		self.size = size
		self.editing = False
		self.activeHandle = None
		self.handles = [DragHandle(self, i, size) for i in range(len(arc.slots))]
		self.toggle = HudIconButton(buttonSize, "hud_skill_edit", self.OpenPanel)
		if buttonPosition is None:
			buttonPosition = (wndMgr.GetScreenWidth() - buttonSize - 12, wndMgr.GetScreenHeight() - 230)
		self.toggle.SetPosition(*buttonPosition)

	def OpenPanel(self):
		self.arc.CancelPageTransition()
		index = self.arc.picker.target
		self.arc.picker.Open(min(index if index is not None else 0, self.arc.count - 1))

	def Lock(self):
		if self.editing:
			self.arc.SavePositions()
		self.editing = False
		self.Refresh()

	def StartMoving(self):
		self.arc.CancelPageTransition()
		self.arc.picker.Hide()
		import mouseModule
		if mouseModule.mouseController.isAttached():
			mouseModule.mouseController.DeattachObject()
		self.editing = True
		self.Refresh()

	def Toggle(self):
		if self.editing:
			self.Lock()
		else:
			self.StartMoving()

	def Refresh(self):
		panelOpen = self.arc.picker.IsShow()
		if self.arc.shown:
			self.toggle.Show()
			self.toggle.SetTop()
		else:
			self.toggle.Hide()
		for i, handle in enumerate(self.handles):
			handle.SetPosition(*self.arc.slots[i].GetLocalPosition())
			if self.arc.shown and self.editing and not panelOpen and i < self.arc.count:
				handle.Show()
				handle.SetTop()
			else:
				handle.touchId = None
				handle.Hide()
		if not self.editing or not self.arc.shown or panelOpen:
			self.activeHandle = None
		if panelOpen:
			self.arc.picker.SetTop()

	def Move(self, index, x, y):
		position = ClampPosition(x, y, self.size, wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())
		for i, slot in enumerate(self.arc.slots[:self.arc.count]):
			if i != index and Overlaps(position, slot.GetLocalPosition(), self.size):
				self.handles[index].SetPosition(*position)
				return
		self.arc.slots[index].SetPosition(*position)
		self.handles[index].SetPosition(*position)
		self.arc.ArrangePlus()

	def Reset(self):
		self.arc.ResetPositions()
		self.Refresh()

	def Destroy(self):
		for handle in self.handles:
			handle.owner = None
			handle.Hide()
		self.handles = []
		self.toggle.Destroy()
		self.arc = None


class QuickSlotPicker(ui.Window):
	PAGE_SIZE = 12

	def __init__(self, arc):
		# Window initialization calls Hide before the owner can bind this picker.
		self.arc = None
		ui.Window.__init__(self, "UI")
		self.arc = arc
		self.target = None
		self.buttons = []
		self.choices = []
		self.category = "skills"
		self.page = 0
		self.AddFlag("float")
		self.SetSize(540, 540)
		self.SetPosition((wndMgr.GetScreenWidth() - 540) // 2,
			max(0, (wndMgr.GetScreenHeight() - 540) // 2))
		self.background = Texture(self, "cw_board", 0, 0, 540, 540)
		self.title = ui.TextLine()
		self.title.SetParent(self)
		self.title.AddFlag("not_pick")
		self.title.SetFontName("Lato:22")
		self.title.SetPackedFontColor(0xFFFFE6AC)
		self.title.SetPosition(20, 14)
		self.title.Show()
		self.closeButton = HudIconButton(28, ART + "close_1.png", self.Hide)
		self.closeButton.SetParent(self)
		self.closeButton.SetPosition(494, 14)
		self.closeButton.Show()
		self.slotButtons = []
		for index in range(8):
			button = EditButton(self, 0, "%d" % (index + 1),
				lambda index=index: self.SelectSlot(index))
			button.SetPosition(20 + index * 62, 52)
			button.SetButtonSize(58, 30)
			self.slotButtons.append(button)
		self.skillsTab = EditButton(self, 20, "Beceriler", lambda: self.SelectCategory("skills"))
		self.skillsTab.SetPosition(20, 94)
		self.itemsTab = EditButton(self, 132, "E\xfeyalar", lambda: self.SelectCategory("items"))
		self.skillsTab.SetButtonSize(116, 34)
		self.itemsTab.SetPosition(148, 94)
		self.itemsTab.SetButtonSize(116, 34)
		self.emotionsTab = EditButton(self, 276, "Hareketler", lambda: self.SelectCategory("emotions"))
		self.emotionsTab.SetPosition(276, 94)
		self.emotionsTab.SetButtonSize(116, 34)
		self.supportTab = EditButton(self, 404, "Destek", lambda: self.SelectCategory("support"))
		self.supportTab.SetPosition(404, 94)
		self.supportTab.SetButtonSize(116, 34)
		self.hint = ui.TextLine()
		self.hint.SetParent(self)
		self.hint.AddFlag("not_pick")
		self.hint.SetFontName("Lato:16")
		self.hint.SetPackedFontColor(0xFFBFAE8A)
		self.hint.SetPosition(282, 103)
		self.hint.SetText("Se\xe7ili yuvaya ata")
		self.hint.Hide()
		self.previous = EditButton(self, 20, "< Geri", lambda: self.ChangePage(-1))
		self.previous.SetPosition(20, 444)
		self.next = EditButton(self, 414, "\xddleri >", lambda: self.ChangePage(1))
		self.next.SetPosition(414, 444)
		self.status = ui.TextLine()
		self.status.SetParent(self)
		self.status.AddFlag("not_pick")
		self.status.SetFontName("Lato:16")
		self.status.SetPackedFontColor(0xFFE8E2D2)
		self.status.SetHorizontalAlignCenter()
		self.status.SetPosition(270, 451)
		self.status.Show()
		self.actions = []
		for index, (text, event) in enumerate((
				("+ Slot", self.AddSlot), ("- Slot", self.RemoveSlot),
				("S\xfdf\xfdrla", self.Reset),
				("Ta\xfe\xfd", self.StartMoving), ("Kilitle", self.Lock))):
			button = EditButton(self, 0, text, event)
			button.SetPosition(20 + index * 102, 488)
			button.SetButtonSize(94, 34)
			self.actions.append(button)
		ui.Window.Hide(self)

	def Open(self, index):
		self.target = index
		self.SelectCategory("skills")
		self.Show()
		if self.arc:
			self.arc.editor.Refresh()
		self.SetTop()

	def Hide(self):
		ui.Window.Hide(self)
		if self.arc:
			self.arc.editor.Refresh()

	def SelectSlot(self, index):
		self.target = index
		self.RenderPage()

	def AddSlot(self):
		if self.arc.count == len(self.arc.slots):
			self.status.SetText("En fazla 8 yuva kullan\xfdlabilir")
			return
		self.arc.AddSlot()
		self.RenderPage()

	def RemoveSlot(self):
		if self.arc.count <= 5:
			self.status.SetText("En az 5 yuva kalmal\xfd")
			return
		self.arc.RemoveSlot()
		self.target = min(self.target, self.arc.count - 1)
		self.RenderPage()

	def Reset(self):
		self.arc.editor.Reset()
		self.target = min(self.target, self.arc.count - 1)
		self.RenderPage()

	def StartMoving(self):
		self.arc.editor.StartMoving()

	def Lock(self):
		self.arc.editor.Lock()
		self.Hide()

	def SelectCategory(self, category):
		self.category = category
		self.page = 0
		self.choices = GetQuickSlotChoices(category)
		self.RenderPage()

	def ChangePage(self, step):
		self.page = max(0, min((max(1, len(self.choices)) - 1) // self.PAGE_SIZE, self.page + step))
		self.RenderPage()

	def RenderPage(self):
		self.title.SetText("H\xfdzl\xfd Eri\xfeim  /  Yuva %d" % (self.target + 1))
		for index, button in enumerate(self.slotButtons):
			button.SetSelected(index == self.target)
			if not self.arc or index < self.arc.count:
				button.Show()
			else:
				button.Hide()
		for button in self.buttons:
			button.event = None
			button.ClearTouchEvents()
			button.Hide()
		self.buttons = []
		start = self.page * self.PAGE_SIZE
		self.skillsTab.SetSelected(self.category == "skills")
		self.itemsTab.SetSelected(self.category == "items")
		self.emotionsTab.SetSelected(self.category == "emotions")
		self.supportTab.SetSelected(self.category == "support")
		for row, (source, vnum, label, icon) in enumerate(self.choices[start:start + self.PAGE_SIZE]):
			button = EditButton(self, 0, "",
				lambda source=source, vnum=vnum: self.Choose(source, vnum))
			button.SetPosition(20 + (row % 2) * 256, 140 + (row // 2) * 50)
			button.SetButtonSize(248, 48)
			# The client does not invalidate glyph UVs on a font change.
			button.text.SetText("")
			button.text.SetFontName("Lato:18")
			button.text.SetHorizontalAlignLeft()
			button.text.SetPosition(52, 4)
			button.text.SetText(label)
			# Measure instead of clipping glyphs, and keep the font fixed.
			if button.text.GetTextSize()[0] > 184:
				while label and button.text.GetTextSize()[0] > 184:
					label = label.rsplit(" ", 1)[0] if " " in label else ""
					button.text.SetText(label + "...")
			button.detail = ui.TextLine()
			button.detail.SetParent(button)
			button.detail.AddFlag("not_pick")
			button.detail.SetFontName("Lato:16")
			button.detail.SetPackedFontColor(0xFFE2C280)
			button.detail.SetPosition(52, 26)
			if self.category == "items":
				detail = "Adet: %d" % player.GetItemCount(source)
			elif self.category == "emotions":
				detail = "Hedef gerekli" if source > 50 else "Tekli hareket"
			elif self.category == "support" and not skill.CanUseSkill(vnum):
				detail = "Pasif - yuvaya atanamaz"
			elif player.GetSkillLevel(source) <= 0:
				detail = "Hen\xfcz \xf6\xf0renilmedi"
			else:
				detail = "Seviye: %s" % uiAkravonSkillGrade.GetGradeText(
					player.GetSkillGrade(source), player.GetSkillLevel(source))[0]
			button.detail.SetText(detail)
			button.detail.Show()
			if icon:
				button.icon = ui.ExpandedImageBox()
				button.icon.SetParent(button)
				button.icon.AddFlag("not_pick")
				button.icon.LoadImage(icon)
				button.icon.SetScale(38.0 / button.icon.GetWidth(), 38.0 / button.icon.GetHeight())
				button.icon.SetPosition(6, 5)
				button.icon.Show()
			self.buttons.append(button)
		pages = max(1, (len(self.choices) + self.PAGE_SIZE - 1) // self.PAGE_SIZE)
		if self.choices:
			self.status.SetText("%d / %d" % (self.page + 1, pages))
		else:
			self.status.SetText("Uygun esya yok" if self.category == "items" else "Uygun kayit yok")
		if self.page > 0:
			self.previous.Show()
		else:
			self.previous.Hide()
		if self.page + 1 < pages:
			self.next.Show()
		else:
			self.next.Hide()

	def Choose(self, source, expected):
		if self.category == "emotions":
			if source != expected or source not in emotion.EMOTION_DICT or not uiAkravonActionIcon.GetEmotionIcon(source):
				dbg.TraceError("uiMobileHud: selected quickslot emotion changed")
				self.SelectCategory("emotions")
				self.status.SetText("Hareket degisti; yeniden sec")
				return
			slotType = player.SLOT_TYPE_EMOTION
		elif self.category == "items":
			if player.GetItemIndex(source) != expected or player.GetItemCount(source) <= 0 \
					or not item.CanAddToQuickSlotItem(expected):
				dbg.TraceError("uiMobileHud: selected inventory quickslot item changed")
				self.SelectCategory("items")
				self.status.SetText("Envanter degisti; yeniden sec")
				return
			slotType = player.SLOT_TYPE_INVENTORY
		else:
			if player.GetSkillIndex(source) != expected \
					or (self.category == "skills" and skill.GetSkillType(expected) != skill.SKILL_TYPE_ACTIVE) \
					or (self.category == "support" and expected not in uiAkravonActionIcon.SUPPORT_IDS):
				dbg.TraceError("uiMobileHud: selected quickslot skill changed")
				self.SelectCategory(self.category)
				self.status.SetText("Beceri degisti; yeniden sec")
				return
			if self.category == "support" and not skill.CanUseSkill(expected):
				dbg.TraceError("uiMobileHud: passive support skill cannot be used from quickslot")
				self.status.SetText("Pasif yetenek yuvadan kullan\xfdlamaz")
				return
			if player.GetSkillLevel(source) <= 0:
				dbg.TraceError("uiMobileHud: selected quickslot skill is not learned")
				self.status.SetText("Bu yetenek hen\xfcz \xf6\xf0renilmedi")
				return
			slotType = player.SLOT_TYPE_SKILL
		player.RequestAddLocalQuickSlot(self.target, slotType, source)
		self.Hide()

	def OnPressEscapeKey(self):
		self.Hide()
		return True

	def Destroy(self):
		for button in self.buttons + self.slotButtons + self.actions + [
				self.closeButton, self.skillsTab, self.itemsTab, self.emotionsTab,
				self.supportTab, self.previous, self.next]:
			button.ClearTouchEvents()
			button.event = None
			button.Hide()
		self.buttons = []
		self.arc = None
		self.Hide()
