import ui


def GetGradeText(grade, level):
	if grade == 3:
		return "P", 0xFFFF7770
	if level <= 0:
		return "", 0xFFE8E2D2
	# GetSkillLevel already returns the level within the M/G grade.
	if grade == 2:
		return "G%d" % level, 0xFFFFD16A
	if grade == 1:
		return "M%d" % level, 0xFF85D8FF
	return str(level), 0xFFE8E2D2


class SkillGradeBadge(ui.Window):
	def __init__(self, parent, circleSize):
		ui.Window.__init__(self)
		self.SetParent(parent)
		self.AddFlag("not_pick")
		width = int(round(circleSize * 0.60))
		height = int(round(width * 112.0 / 256.0))
		self.SetSize(width, height)
		self.SetPosition((circleSize - width) // 2, int(round(circleSize * 0.84)))

		self.background = ui.ExpandedImageBox()
		self.background.SetParent(self)
		self.background.AddFlag("not_pick")
		self.background.LoadImage("mobile_ui/akravon/hud/hud_skill_grade.png")
		self.background.SetScale(width / 256.0, height / 112.0)
		self.background.Show()

		self.text = ui.TextLine()
		self.text.SetParent(self)
		self.text.AddFlag("not_pick")
		fontSize = max(12, int(round(circleSize * 0.27)))
		self.fontSize = fontSize
		self.labelWidth = width - 6
		self.text.SetFontName("LatoBd:%d" % fontSize)
		self.text.SetOutline()
		self.text.SetHorizontalAlignCenter()
		self.text.SetPosition(width // 2, (height - fontSize) // 2 - 1)
		self.text.Show()
		self.Hide()

	def Refresh(self, grade, level):
		text = GetGradeText(grade, level)[0]
		self.SetLabel(text)

	def RefreshCount(self, count):
		self.SetLabel(str(count) if count > 0 else "")

	def SetLabel(self, text):
		fontSize = min(self.fontSize, max(9, int(self.labelWidth / max(1.0, len(text) * 0.65))))
		self.text.SetText("")
		self.text.SetFontName("LatoBd:%d" % fontSize)
		self.text.SetText(text)
		self.text.SetPackedFontColor(0xFFFFE6AC)
		if text:
			self.Show()
		else:
			self.Hide()
