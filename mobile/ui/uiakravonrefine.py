# -*- coding: utf-8 -*-
# AKRAVON: Yukseltme penceresi (sifirdan). Uc sutun: solda silah (slot), ortada esyanin BILGI KUTUSU
# (ad, seviye, ozellikler, [ Giyilebilir ] siniflar, tas yuvalari), sagda gerekli malzemeler / basari sansi / bedel;
# altta TAMAM - VAZGEC. Oyunun RefineDialogNew sinifi ayni metot adlariyla calisir; bu modul yalniz gorunumu cizer.
# Ozellik satirlari gizli ItemToolTip'in urettigi satirlardan (metin + renk) alinir.
# Cokme guvenligi: tum parcalar Attach'ta BIR KEZ olusturulur (havuz); acildiktan sonra pencere nesnesi olusturulmaz / silinmez.
from __future__ import division
import re
import ui
import item
import player
import wndMgr
import uiMobileStyle
import uiAkravon
import uiAkravonQuest
import uiAkravonInventory
import uiAkravonOffer

W = 640
H_MIN = 440
PAD = 24
TITLE_H = 56
SLOT_W = 70
SLOT_H = 108
BOX_X = 108
BOX_W = 282
SPLIT = 406
RIGHT_X = 424
RIGHT_R = W - PAD
ROW_STEP = 24
ROW_POOL = 14
STONE_POOL = 3
STONE_STEP = 44
MAT_POOL = 4
MAT_STEP = 56

GOLD = uiMobileStyle.Color(222, 184, 110)
MUTED = uiMobileStyle.Color(160, 148, 120)
LINE_COLOR = uiMobileStyle.Color(150, 118, 58)
GREEN = 0xff6EE87F
YELLOW = 0xffFFD84D
RED = 0xffFF6B66
ROW_NAME_COLOR = 0xffA89C78

VALUE_PATTERN = re.compile(r"^(.*?)\s*:?\s*([+\-]?\s?%?\d[\d%\s\-\+\.,]*)$")


class Skin(object):
	def __init__(self, dialog):
		self.dialog = dialog
		board = dialog.board
		self.board = board
		for part in list(getattr(board, "Corners", [])) + list(getattr(board, "Lines", [])) + [getattr(board, "Base", None), dialog.titleBar]:
			if part:
				part.Hide()
		for name in ("AcceptButton", "CancelButton", "SuccessPercentage", "Cost"):
			try:
				dialog.GetChild(name).Hide()
			except Exception:
				pass
		self.frame = uiAkravonQuest.Board9()
		self.frame.SetParent(board)
		self.frame.AddFlag("not_pick")
		self.frame.Show()
		self.title = uiMobileStyle.CreateText(board, "Yükseltme", W // 2, 13, GOLD, "Marcellus:24", "center")
		self.divider = uiMobileStyle.CreateBar(board, LINE_COLOR, 18, TITLE_H, W - 36, 1)
		self.close = uiAkravonInventory.CloseButton(board, W - 50, 8, ui.__mem_func__(dialog.CancelRefine))	# ortak kural: dokunma alani 40 px, gorsel (W-46, 12)
		# 1) silah slotu
		self.slot = uiAkravon.Box()
		self.slot.SetParent(board)
		self.slot.SetPosition(PAD, 72)
		self.slot.SetSize(SLOT_W, SLOT_H)
		self.slot.SetFillColor(uiMobileStyle.Color(8, 12, 26, 220))
		self.slot.SetLineColor(LINE_COLOR)
		self.slot.Show()
		self.icon = ui.ExpandedImageBox()
		self.icon.SetParent(board)
		self.icon.AddFlag("not_pick")
		self.icon.Show()
		# 2) bilgi kutusu
		self.box = uiAkravon.Box()
		self.box.SetParent(board)
		self.box.SetPosition(BOX_X, 72)
		self.box.SetSize(BOX_W, 300)
		self.box.SetFillColor(uiMobileStyle.Color(8, 12, 26, 220))
		self.box.SetLineColor(LINE_COLOR)
		self.box.Show()
		self.hint = uiMobileStyle.CreateText(board, "Yükseltme sonucu", BOX_X + 14, 78, MUTED, "Lato:15")
		self.name = uiMobileStyle.CreateText(board, "", BOX_X + 14, 94, GOLD, "Marcellus:22")
		self.level = uiMobileStyle.CreateText(board, "", BOX_X + 14, 126, uiMobileStyle.CREAM_COLOR, "Lato:16")
		self.boxLine = uiMobileStyle.CreateBar(board, LINE_COLOR, BOX_X + 10, 152, BOX_W - 20, 1)
		self.rowName = []
		self.rowValue = []
		for i in range(ROW_POOL):
			self.rowName.append(uiMobileStyle.CreateText(board, "", BOX_X + 14, 0, MUTED, "Lato:16"))
			self.rowValue.append(uiMobileStyle.CreateText(board, "", BOX_X + BOX_W - 14, 0, uiMobileStyle.CREAM_COLOR, "LatoBd:16", "right"))
		self.stoneSlot = []
		self.stoneIcon = []
		self.stoneName = []
		self.stoneAffect = []
		self.stoneLine = uiMobileStyle.CreateBar(board, LINE_COLOR, BOX_X + 10, 0, BOX_W - 20, 1)
		for i in range(STONE_POOL):
			slot = uiAkravon.Box()
			slot.SetParent(board)
			slot.SetSize(36, 36)
			slot.SetFillColor(uiMobileStyle.Color(14, 24, 48, 255))
			slot.SetLineColor(LINE_COLOR)
			slot.Hide()
			self.stoneSlot.append(slot)
			icon = ui.ExpandedImageBox()
			icon.SetParent(board)
			icon.AddFlag("not_pick")
			icon.Hide()
			self.stoneIcon.append(icon)
			self.stoneName.append(uiMobileStyle.CreateText(board, "", 0, 0, uiMobileStyle.CREAM_COLOR, "Lato:15"))
			self.stoneAffect.append(uiMobileStyle.CreateText(board, "", 0, 0, GREEN, "Lato:15"))
		# 3) sag panel
		self.vline = uiMobileStyle.CreateBar(board, LINE_COLOR, SPLIT, 72, 1, 100)
		self.matHeader = uiMobileStyle.CreateText(board, "Gerekli malzeme", RIGHT_X, 72, GOLD, "Marcellus:18")
		self.matNone = uiMobileStyle.CreateText(board, "Malzeme gerekmiyor", RIGHT_X, 104, MUTED, "Lato:16")
		self.matSlot = []
		self.matIcon = []
		self.matName = []
		self.matCount = []
		for i in range(MAT_POOL):
			slot = uiAkravon.Box()
			slot.SetParent(board)
			slot.SetSize(44, 44)
			slot.SetFillColor(uiMobileStyle.Color(14, 24, 48, 255))
			slot.SetLineColor(LINE_COLOR)
			slot.Hide()
			self.matSlot.append(slot)
			icon = ui.ExpandedImageBox()
			icon.SetParent(board)
			icon.AddFlag("not_pick")
			icon.Hide()
			self.matIcon.append(icon)
			self.matName.append(uiMobileStyle.CreateText(board, "", RIGHT_X + 54, 0, uiMobileStyle.CREAM_COLOR, "Lato:16"))
			self.matCount.append(uiMobileStyle.CreateText(board, "", RIGHT_X + 54, 0, uiMobileStyle.CREAM_COLOR, "LatoBd:15"))
		self.successLine = uiMobileStyle.CreateBar(board, LINE_COLOR, RIGHT_X, 0, RIGHT_R - RIGHT_X, 1)
		self.successLabel = uiMobileStyle.CreateText(board, "Başarı şansı", RIGHT_X, 0, MUTED, "Lato:17")
		self.successValue = uiMobileStyle.CreateText(board, "", RIGHT_R, 0, uiMobileStyle.CREAM_COLOR, "LatoBd:21", "right")
		self.costLine = uiMobileStyle.CreateBar(board, LINE_COLOR, RIGHT_X, 0, RIGHT_R - RIGHT_X, 1)
		self.costLabel = uiMobileStyle.CreateText(board, "Bedel", RIGHT_X, 0, MUTED, "Lato:17")
		self.costValue = uiMobileStyle.CreateText(board, "", RIGHT_R, 0, GOLD, "LatoBd:18", "right")
		self.coin = ui.ExpandedImageBox()
		self.coin.SetParent(board)
		self.coin.AddFlag("not_pick")
		self.coin.LoadImage("d:/ymir work/ui/akravon_legacy/inv_coin.png")
		self.coin.Show()
		# 4) alt dugmeler (standart)
		self.yes = uiAkravonOffer.SkinButton("Tamam")
		self.yes.SetParent(board)
		self.yes.SetEvent(ui.__mem_func__(dialog.OpenQuestionDialog))
		self.yes.Show()
		self.no = uiAkravonOffer.SkinButton("Vazgeç")
		self.no.SetParent(board)
		self.no.SetEvent(ui.__mem_func__(dialog.CancelRefine))
		self.no.Show()
		self.entries = []
		self.stones = []
		self.materials = 0
		self.percentage = 100
		self.costText = ""

	# ---------------------------------------------------------------- veri
	def Fill(self, vnum, cost, percentage, lines, metinSlot, tip):
		self.percentage = percentage
		self.materials = 0
		item.SelectItem(vnum)
		try:
			self.icon.LoadImage(item.GetIconImageFileName())
		except Exception:
			pass
		self.FitIcon(self.icon, PAD, 72, SLOT_W, SLOT_H)
		self.name.SetText(lines[0][0] if lines else item.GetItemName())
		level = ""
		entries = []
		for (text, color) in lines[1:]:
			clean = text.strip()
			if not clean:
				continue
			if clean.lower().startswith("seviye"):
				level = clean
				continue
			match = VALUE_PATTERN.match(clean)
			if match and match.group(1) and len(match.group(1)) >= 3:
				entries.append([match.group(1).strip(), match.group(2).strip(), color])
			else:
				entries.append([clean, None, color])
		# "[ Giyilebilir ]" ile sinif listesi TEK satir
		merged = []
		for entry in entries:
			if merged and merged[-1][1] is None and merged[-1][0].startswith("[") and entry[1] is None and not entry[0].startswith("["):
				merged[-1][0] = merged[-1][0] + "   " + entry[0]
			else:
				merged.append(entry)
		self.level.SetText(level)
		self.entries = merged[:ROW_POOL]
		# tas yuvalari
		stones = []
		try:
			for index in range(player.METIN_SOCKET_MAX_NUM):
				data = metinSlot[index]
				slotType = tip.GetMetinSocketType(data)
				itemIndex = tip.GetMetinItemIndex(data)
				if 0 == slotType:
					continue
				if itemIndex:
					item.SelectItem(itemIndex)
					name = item.GetItemName()
					icon = item.GetIconImageFileName()
					affect = ""
					try:
						(affectType, affectValue) = item.GetAffect(0)
						affect = tip._ItemToolTip__GetAffectString(affectType, affectValue) or ""
					except Exception:
						pass
					stones.append((name, icon, affect))
				else:
					stones.append(("Boş", None, ""))
		except Exception:
			pass
		self.stones = stones[:STONE_POOL]
		self.costText = self.Money(cost)
		for index in range(MAT_POOL):
			self.matSlot[index].Hide()
			self.matIcon[index].Hide()
			self.matName[index].SetText("")
			self.matCount[index].SetText("")
		self.Layout()

	def FitIcon(self, icon, x, y, boxW, boxH):
		# yuksek cozunurluklu ikonlar kutuya sigacak sekilde kuculur, ortalanir
		icon.SetScale(1.0, 1.0)
		(width, height) = (max(1, icon.GetWidth()), max(1, icon.GetHeight()))
		scale = min(1.0, (boxW - 10) / float(width), (boxH - 10) / float(height))
		icon.SetScale(scale, scale)
		icon.SetPosition(x + int((boxW - width * scale) // 2), y + int((boxH - height * scale) // 2))

	def Money(self, value):
		text = str(int(value))
		groups = []
		while text:
			groups.insert(0, text[-3:])
			text = text[:-3]
		return ".".join(groups) + " Yang"

	def AddMaterial(self, vnum, count):
		index = self.materials
		if index >= MAT_POOL:
			return
		self.materials += 1
		item.SelectItem(vnum)
		self.matName[index].SetText(item.GetItemName())
		try:
			self.matIcon[index].LoadImage(item.GetIconImageFileName())
		except Exception:
			pass
		have = player.GetItemCountByVnum(vnum)
		enough = have >= count
		self.matCount[index].SetText("%d / %d" % (have, count))
		self.matCount[index].SetPackedFontColor(GREEN if enough else RED)
		self.matSlot[index].SetLineColor(LINE_COLOR if enough else uiMobileStyle.Color(255, 107, 102))
		self.Layout()

	# ---------------------------------------------------------------- yerlesim
	def Layout(self):
		dialog = self.dialog
		y = 164
		for index in range(ROW_POOL):
			if index < len(self.entries):
				(left, right, color) = self.entries[index]
				self.rowName[index].SetText(left)
				self.rowName[index].SetPosition(BOX_X + 14, y)
				if right is None:
					self.rowName[index].SetPackedFontColor(color)
					self.rowValue[index].SetText("")
				else:
					self.rowName[index].SetPackedFontColor(ROW_NAME_COLOR)
					self.rowValue[index].SetText(right)
					self.rowValue[index].SetPackedFontColor(color)
					self.rowValue[index].SetPosition(BOX_X + BOX_W - 14, y)
				self.rowName[index].Show()
				self.rowValue[index].Show()
				y += ROW_STEP
			else:
				self.rowName[index].Hide()
				self.rowValue[index].Hide()
		if self.stones:
			y += 6
			self.stoneLine.SetPosition(BOX_X + 10, y)
			self.stoneLine.Show()
			y += 10
		else:
			self.stoneLine.Hide()
		for index in range(STONE_POOL):
			if index < len(self.stones):
				(name, icon, affect) = self.stones[index]
				self.stoneSlot[index].SetPosition(BOX_X + 14, y)
				self.stoneSlot[index].Show()
				if icon:
					try:
						self.stoneIcon[index].LoadImage(icon)
					except Exception:
						pass
					self.FitIcon(self.stoneIcon[index], BOX_X + 14, y, 36, 36)
					self.stoneIcon[index].Show()
				else:
					self.stoneIcon[index].Hide()
				self.stoneName[index].SetText(name)
				self.stoneName[index].SetPosition(BOX_X + 60, y + 2)
				self.stoneAffect[index].SetText(affect)
				self.stoneAffect[index].SetPosition(BOX_X + 60, y + 20)
				self.stoneName[index].Show()
				self.stoneAffect[index].Show()
				y += STONE_STEP
			else:
				self.stoneSlot[index].Hide()
				self.stoneIcon[index].Hide()
				self.stoneName[index].Hide()
				self.stoneAffect[index].Hide()
		boxBottom = y + 8
		rightNeed = 72 + 34 + max(1, self.materials) * MAT_STEP + 130
		height = max(H_MIN, boxBottom + 98, rightNeed + 70)
		self.box.SetSize(BOX_W, boxBottom - 72)
		dialog.SetSize(W, height)
		self.board.SetSize(W, height)
		self.frame.SetSize(W, height)
		self.vline.SetSize(1, height - 72 - 92)
		if self.materials:
			self.matHeader.SetText("Gerekli malzemeler")
			self.matNone.Hide()
		else:
			self.matHeader.SetText("Gerekli malzeme")
			self.matNone.SetPosition(RIGHT_X, 108)
			self.matNone.Show()
		for index in range(MAT_POOL):
			if index < self.materials:
				top = 106 + index * MAT_STEP
				self.matSlot[index].SetPosition(RIGHT_X, top)
				self.matSlot[index].Show()
				self.FitIcon(self.matIcon[index], RIGHT_X, top, 44, 44)
				self.matIcon[index].Show()
				self.matName[index].SetPosition(RIGHT_X + 54, top + 3)
				self.matCount[index].SetPosition(RIGHT_X + 54, top + 24)
				self.matName[index].Show()
				self.matCount[index].Show()
			else:
				self.matSlot[index].Hide()
				self.matIcon[index].Hide()
				self.matName[index].Hide()
				self.matCount[index].Hide()
		costY = height - 160
		successY = height - 198
		if self.percentage < 100:
			color = GREEN if self.percentage >= 80 else (YELLOW if self.percentage >= 50 else RED)
			self.successValue.SetText("%%%d" % self.percentage)
			self.successValue.SetPackedFontColor(color)
			self.successLine.SetPosition(RIGHT_X, successY - 10)
			self.successLabel.SetPosition(RIGHT_X, successY)
			self.successValue.SetPosition(RIGHT_R, successY - 2)
			self.successLine.Show()
			self.successLabel.Show()
			self.successValue.Show()
		else:
			self.successLine.Hide()
			self.successLabel.Hide()
			self.successValue.Hide()
		self.costLine.SetPosition(RIGHT_X, costY - 10)
		self.costLabel.SetPosition(RIGHT_X, costY)
		self.costValue.SetText(self.costText)
		self.costValue.SetPosition(RIGHT_R, costY - 2)
		self.coin.SetPosition(RIGHT_R - uiMobileStyle.TextWidth(self.costText, "LatoBd:18") - 30, costY - 2)
		self.yes.SetPosition(W // 2 - 190 - 10, height - 66)
		self.no.SetPosition(W // 2 + 10, height - 66)
		# "pencere kapanmasin" kutusu yok
		try:
			box = dialog.checkBoxRefine
			box.Hide()
			box.Show = lambda *args: None
		except Exception:
			pass
		# envanter / ekipman aciksa onlarin SOLUNDA (ustune binmez), degilse ekranin ortasinda
		x = (wndMgr.GetScreenWidth() - W) // 2
		try:
			import uiToolTip
			for anchor in uiToolTip.ANCHOR_WINDOWS:
				if not anchor.IsShow():
					continue
				left = anchor.GetGlobalPosition()[0]
				try:
					equip = anchor.GetChild("EquipBoard")
					if equip.IsShow():
						left = min(left, equip.GetGlobalPosition()[0])
				except Exception:
					pass
				if left - W - 12 >= 4:
					x = left - W - 12
				break
		except Exception:
			pass
		dialog.SetPosition(x, max(4, (wndMgr.GetScreenHeight() - height) // 2))


def Attach(dialog):
	try:
		dialog.akr = Skin(dialog)
		toolTip = dialog.toolTip
		toolTip.akrLines = []
		original = toolTip.AppendTextLine

		def Record(text, color = None, centerAlign = True):
			if color is None:
				color = toolTip.FONT_COLOR
			toolTip.akrLines.append((text, color))
			return original(text, color, centerAlign)
		toolTip.AppendTextLine = Record

		def NoShow(*args):
			return	# eski ipucu kutusu hic gosterilmez (yalniz veri uretir)
		toolTip.Show = NoShow
		toolTip.ShowToolTip = NoShow
		toolTip.SetPosition(-4000, -4000)
		toolTip.Hide()
	except Exception:
		import dbg
		import traceback
		dbg.TraceError("uiAkravonRefine.Attach: " + traceback.format_exc())
		dialog.akr = None


def BeginData(dialog):
	dialog.toolTip.akrLines = []


def Fill(dialog, vnum, cost, percentage, metinSlot):
	if not getattr(dialog, "akr", None):
		return False
	try:
		lines = list(dialog.toolTip.akrLines)
		dialog.toolTip.Hide()
		try:
			dialog.itemImage.Hide()
			for slot in dialog.slotList:
				slot.Hide()
		except Exception:
			pass
		dialog.akr.Fill(vnum, cost, percentage, lines, metinSlot, dialog.toolTip)
		return True
	except Exception:
		import dbg
		import traceback
		dbg.TraceError("uiAkravonRefine.Fill: " + traceback.format_exc())
		return False
