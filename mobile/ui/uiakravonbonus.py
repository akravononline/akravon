# -*- coding: utf-8 -*-
# AKRAVON: karakter penceresi "Bonuslar" bolumu - kategori plakalari + iki sutun bonus kartlari
# veri: uiCharacterDetails.bonus_list (oyunun kendi listesi), deger: player.GetStatus
import ui
import player
import grp
import wndMgr

IMG = "d:/ymir work/ui/akravon_legacy/"
C_HEAD = grp.GenerateColor(0.89, 0.76, 0.50, 1.0)
C_LABEL = grp.GenerateColor(0.70, 0.66, 0.58, 1.0)
C_DARK = grp.GenerateColor(0.16, 0.09, 0.03, 1.0)
C_TEXT = grp.GenerateColor(0.91, 0.89, 0.82, 1.0)

TAB_W, TAB_H, TAB_GAP = 66, 36, 4	# sagdaki kapat (X) dugmesine degmesin
ROW_H, ROW_GAP = 30, 2	# AKRAVON: telefonda okunakli; sigmayan satirlar parmakla kaydirilir
NAME_MAX = 250
FONT_ROW = "Lato:19"
FONT_TAB = "Lato:17"


def _BonusName(applyType):
	# uiCharacterDetails ile ayni: etki metninden sayi / % / + temizlenir
	try:
		from uiToolTip import ItemToolTip
		text = ItemToolTip.AFFECT_DICT[applyType](0)
	except Exception:
		return "?"
	out = ""
	for ch in text:
		if ch in "%+" or ch.isdigit():
			continue
		out += ch
	out = out.strip().rstrip(":").rstrip(".").strip()
	# AKRAVON: cumle bicimli etkiler (orn. "hasar, HP ile absorbe edilecek") kisa ada cevrilir
	if "absorbe" in out:
		return "HP Emme" if "HP" in out else "SP Emme"
	if out[:1].islower():
		out = out[:1].upper() + out[1:]
	return out


def _Text(parent, x, y, text, font, color, align = "left"):
	t = ui.TextLine()
	t.SetParent(parent)
	t.AddFlag("not_pick")
	t.SetFontName(font)
	t.SetPackedFontColor(color)
	if align == "right":
		t.SetHorizontalAlignRight()
	elif align == "center":
		t.SetHorizontalAlignCenter()
	t.SetPosition(x, y)
	t.SetText(text)
	t.Show()
	return t


def _Image(parent, x, y, name):
	img = ui.ExpandedImageBox()
	img.SetParent(parent)
	img.AddFlag("not_pick")
	img.LoadImage(IMG + name)
	img.SetPosition(x, y)
	img.Show()
	return img


class BonusPanel(ui.Window):
	def __init__(self, width, height):
		ui.Window.__init__(self)
		self.SetSize(width, height)
		self.width = width
		self.scroll = 0
		self.dragging = False
		self.dragY = 0
		self.dragScroll = 0
		self.category = 0
		self.tabs = []
		self.rows = []
		try:
			import uiCharacterDetails
			self.data = uiCharacterDetails.bonus_list
		except Exception:
			self.data = []


		# kategori plakalari (PvM / PvP / Saldiri / Savunma)
		names = ["PvM", "PvP", "Sald\xfdr\xfd", "Savunma"]
		for i in xrange(len(self.data)):
			btn = ui.Button()
			btn.SetParent(self)
			btn.SetUpVisual(IMG + "cw_tab_normal.png")
			btn.SetOverVisual(IMG + "cw_tab_over.png")
			btn.SetDownVisual(IMG + "cw_tab_down.png")
			btn.SetPosition(i * (TAB_W + TAB_GAP), 0)
			btn.SetEvent(ui.__mem_func__(self.SelectCategory), i)
			btn.Show()
			label = _Text(btn, TAB_W // 2, 7, names[i] if i < len(names) else str(i + 1), FONT_TAB, C_TEXT, "center")
			self.tabs.append((btn, label))

		# tek sutun satir: solda bonus adi, sagda deger (tam genislik, tasma yok)
		self.top = TAB_H + 10
		self.visible = max(1, (height - self.top) // (ROW_H + ROW_GAP))
		for k in xrange(self.visible):
			card = _Image(self, 0, self.top + k * (ROW_H + ROW_GAP), "cw_bonusrow.png")
			if card.GetHeight() > 0:
				card.SetScale(1.0, float(ROW_H) / card.GetHeight())
			name = _Text(card, 10, (ROW_H - 19) // 2 - 1, "", FONT_ROW, C_TEXT)
			value = _Text(card, width - 12, (ROW_H - 19) // 2 - 1, "", FONT_ROW, C_HEAD, "right")
			self.rows.append((card, name, value))
		# kaydirma cubugu (ince altin serit)
		self.track = ui.Bar()
		self.track.SetParent(self)
		self.track.AddFlag("not_pick")
		self.track.SetColor(grp.GenerateColor(0.89, 0.76, 0.50, 0.55))
		self.track.Hide()

		self.SelectCategory(0)

	def __del__(self):
		ui.Window.__del__(self)

	def OnMouseLeftButtonDown(self):
		self.dragging = True
		self.dragY = wndMgr.GetMousePosition()[1]
		self.dragScroll = self.scroll
		return True

	def OnMouseLeftButtonUp(self):
		self.dragging = False
		return True

	def OnUpdate(self):
		if not self.dragging:
			return
		moved = int((self.dragY - wndMgr.GetMousePosition()[1]) / float(ROW_H + ROW_GAP))
		scroll = max(0, min(self.__MaxScroll(), self.dragScroll + moved))
		if scroll != self.scroll:
			self.scroll = scroll
			self.Refresh()

	def __Bonuses(self):
		return self.data[self.category][1] if self.category < len(self.data) else []

	def __MaxScroll(self):
		return max(0, len(self.__Bonuses()) - self.visible)

	def SelectCategory(self, index):
		self.category = index
		self.scroll = 0
		for (i, (btn, label)) in enumerate(self.tabs):
			if i == index:
				btn.SetUpVisual(IMG + "cw_tab_down.png")
				label.SetPackedFontColor(C_DARK)
			else:
				btn.SetUpVisual(IMG + "cw_tab_normal.png")
				label.SetPackedFontColor(C_TEXT)
		self.Refresh()

	def Refresh(self):
		bonuses = self.__Bonuses()
		maxScroll = self.__MaxScroll()
		if maxScroll > 0:
			span = self.visible * (ROW_H + ROW_GAP)
			barH = max(24, span * self.visible // len(bonuses))
			self.track.SetPosition(self.width - 3, self.top + (span - barH) * self.scroll // maxScroll)
			self.track.SetSize(3, barH)
			self.track.Show()
		else:
			self.track.Hide()
		for (k, (card, name, value)) in enumerate(self.rows):
			index = k + self.scroll
			if index >= len(bonuses):
				card.Hide()
				continue
			(applyType, pointType) = bonuses[index]
			text = _BonusName(applyType)
			name.SetText(text)
			while name.GetTextSize()[0] > NAME_MAX and len(text) > 4:
				text = text[:-1]
				name.SetText(text + "...")
			try:
				value.SetText(str(player.GetStatus(pointType)))
			except Exception:
				value.SetText("0")
			card.Show()
