# -*- coding: utf-8 -*-
# AKRAVON: Derece Detaylari - klasik 9 derece, bonus yok (motor: 12000+ = Kahraman)
import player
import wndMgr
import ui
import localeInfo
import uiMobileStyle

IMG = "d:/ymir work/ui/akravon_legacy/"
W, H = 380, 520
GOLD = uiMobileStyle.Color(222, 184, 110)
TEXT = uiMobileStyle.Color(232, 226, 210)
DIM = uiMobileStyle.Color(150, 140, 120)
# (motor derece no, renk, puan araligi)
GRADES = (
	(27, uiMobileStyle.Color(120, 200, 255), "12.000 ve üzeri"),
	(28, uiMobileStyle.Color(140, 215, 255), "8.000 - 11.999"),
	(29, uiMobileStyle.Color(160, 230, 200), "4.000 - 7.999"),
	(30, uiMobileStyle.Color(190, 240, 190), "1.000 - 3.999"),
	(31, uiMobileStyle.Color(232, 226, 210), "0 - 999"),
	(32, uiMobileStyle.Color(240, 190, 120), "-1 - -3.999"),
	(33, uiMobileStyle.Color(240, 150, 90), "-4.000 - -7.999"),
	(34, uiMobileStyle.Color(235, 100, 70), "-8.000 - -11.999"),
	(35, uiMobileStyle.Color(220, 50, 50), "-12.000 ve altı"),
)
ROW_H = 44
ROW_GAP = 4
TOP = 66

panel = None


def Open():
	global panel
	if panel is None:
		panel = AlignPanel()
	panel.Open()


class AlignPanel(ui.Window):
	def __init__(self):
		ui.Window.__init__(self)
		self.AddFlag("float")
		self.AddFlag("movable")
		self.SetSize(W, H)
		self.keep = []
		board = ui.ExpandedImageBox()
		board.SetParent(self)
		board.AddFlag("not_pick")
		board.LoadImage(IMG + "align_board.png")
		board.Show()
		self.keep.append(board)
		self.keep.append(uiMobileStyle.CreateText(self, "Derece", W // 2, 13, GOLD, "Marcellus:22", "center"))
		self.keep.append(uiMobileStyle.CreateBar(self, uiMobileStyle.Color(96, 74, 42), 16, 50, W - 32, 1))
		close = ui.Button()
		close.SetParent(self)
		close.SetUpVisual(IMG + "close_1.png")
		close.SetOverVisual(IMG + "close_2.png")
		close.SetDownVisual(IMG + "close_3.png")
		close.SetPosition(W - 40, 10)
		close.SAFE_SetEvent(self.Close)
		close.Show()
		self.keep.append(close)
		self.rows = []
		for (index, (grade, color, span)) in enumerate(GRADES):
			y = TOP + index * (ROW_H + ROW_GAP)
			row = ui.ExpandedImageBox()
			row.SetParent(self)
			row.AddFlag("not_pick")
			row.LoadImage(IMG + "align_row.png")
			row.SetPosition(18, y)
			row.Show()
			dot = uiMobileStyle.CreateBar(row, color, 14, 17, 10, 10)
			name = uiMobileStyle.CreateText(row, localeInfo.TITLE_NAME_LIST[grade], 36, 11, color, "LatoBd:17")
			value = uiMobileStyle.CreateText(row, span, W - 36 - 14, 12, DIM, "Lato:15", "right")
			self.rows.append((grade, row, name, value))
			self.keep += [dot, name, value]
		self.note = uiMobileStyle.CreateText(self, "", W // 2, H - 34, DIM, "Lato:15", "center")
		self.keep.append(self.note)
		self.SetPosition((wndMgr.GetScreenWidth() - W) // 2, (wndMgr.GetScreenHeight() - H) // 2)

	def Refresh(self):
		(point, grade) = player.GetAlignmentData()
		for (g, row, name, value) in self.rows:
			row.LoadImage(IMG + ("align_row_on.png" if g == grade else "align_row.png"))
			value.SetPackedFontColor(GOLD if g == grade else DIM)
		self.note.SetText("Derece puanın: %s" % localeInfo.NumberToString(point) if hasattr(localeInfo, "NumberToString") else "Derece puanın: %d" % point)

	def Open(self):
		self.Refresh()
		self.Show()
		self.SetTop()

	def Close(self):
		self.Hide()

	def OnPressEscapeKey(self):
		self.Close()
		return True
