# -*- coding: utf-8 -*-
# AKRAVON: lonca YATIRIM (tecrube bagisi) penceresi - uiPickMoney.PickMoneyDialog yerine; ayni arayuz, bizim tasarim dilimiz.
#  ustte ortali baslik + kapat, "Tecrube puanin" satiri, buyuk sayi kutusu, hizli +100 / +1000 / TUMU, ONAYLA / VAZGEC.
#  Klavye acilinca pencere ekranin ustune dogru yerlesir, kutu klavyenin ustunde kalir.
from __future__ import division
import app
import ui
import wndMgr
import localeInfo
import uiMobileStyle
import uiMobileChat
import uiAkravon
import uiAkravonQuest

W = 430
H = 272
PAD = 16
TITLE_H = 54
GOLD = uiMobileStyle.Color(222, 184, 110)
CREAM = uiMobileStyle.Color(250, 236, 210)
FIELD_H = 50
EDGE = uiMobileStyle.Color(150, 118, 58)


class FlatButton(uiMobileStyle.Pressable):
	# hizli degerler (+100 / +1000 / TUMU): duz altin kenarli lacivert dugme
	def __init__(self, width, height, text, primary, font):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.edge = uiMobileStyle.CreateBar(self, EDGE, 0, 0, width, height)
		self.fillColor = uiMobileStyle.Color(14, 24, 48)
		self.fill = uiMobileStyle.CreateBar(self, self.fillColor, 2, 2, width - 4, height - 4)
		self.label = uiMobileStyle.CreateText(self, uiMobileStyle.TrUpper(text), width // 2, height // 2 - 11, CREAM, font, "center")

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.edge = None
		self.fill = None
		self.label = None

	def RefreshVisual(self):
		if getattr(self, "fill", None):
			self.fill.SetColor(uiMobileStyle.Color(40, 64, 120) if self.pressed else self.fillColor)


class SkinButton(uiMobileStyle.Pressable):
	# ONAYLA / VAZGEC: her yerde kullandigimiz standart dugme (lacivert, altin pahli cerceve), yazi krem Marcellus.
	# 190 genislikte hazir gorsel (sb_btn190_*: sb_btn230'dan uretildi; kenarlar sikismaz)
	IMG = "d:/ymir work/ui/akravon_legacy/"

	def __init__(self, text, width = 190):
		uiMobileStyle.Pressable.__init__(self, 190, 44)
		self.skin = ui.ExpandedImageBox()
		self.skin.SetParent(self)
		self.skin.AddFlag("not_pick")
		self.skin.LoadImage(self.IMG + "sb_btn190_1.png")
		self.skin.Show()
		self.label = uiMobileStyle.CreateText(self, uiMobileStyle.TrUpper(text), 95, 44 // 2 - 11, CREAM, "Marcellus:19", "center")

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.skin = None
		self.label = None

	def RefreshVisual(self):
		if getattr(self, "skin", None):
			self.skin.LoadImage(self.IMG + ("sb_btn190_3.png" if self.pressed else "sb_btn190_1.png"))


class PickMoneyDialog(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.AddFlag("float")	# lonca penceresinin onunde acilir (SetTop calisir)
		self.eventAccept = None
		self.maxValue = 0
		self.unitValue = 100
		self.built = False
		self.titleName = "Yatırım"
		self.maxLength = 9

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	# --- eski arayuz ---
	def LoadDialog(self):
		pass

	def SetMax(self, length):
		self.maxLength = length
		if self.built:
			self.input.SetMax(length)

	def SetTitleName(self, text):
		self.titleName = text
		if self.built:
			self.title.SetText(text)

	def SetAcceptEvent(self, event):
		self.eventAccept = event

	def Destroy(self):
		self.eventAccept = None
		if self.built:
			for widget in (self.close, self.accept, self.cancel, self.quick1, self.quick2, self.quick3):
				widget.Destroy()
			self.close = None
			self.accept = None
			self.cancel = None
			self.quick1 = None
			self.quick2 = None
			self.quick3 = None
			self.input = None
		self.built = False
		self.Hide()

	# --- kurulum ---
	def __Build(self):
		self.built = True
		self.SetSize(W, H)
		self.board = uiAkravonQuest.Board9()
		self.board.SetParent(self)
		self.board.AddFlag("not_pick")
		self.board.SetSize(W, H)
		self.board.Show()
		self.title = uiMobileStyle.CreateText(self, self.titleName, W // 2, 15, GOLD, "Marcellus:21", "center")
		self.divider = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(150, 118, 58), PAD, TITLE_H, W - PAD * 2, 1)
		import uiAkravonInventory
		self.close = uiAkravonInventory.CloseButton(self, W - PAD - 34, 15, ui.__mem_func__(self.Close))

		self.info = uiMobileStyle.CreateText(self, "", PAD + 2, TITLE_H + 12, CREAM, "Lato:17", "left")
		self.maxText = uiMobileStyle.CreateText(self, "", W - PAD - 2, TITLE_H + 10, GOLD, "LatoBd:20", "right")

		fieldY = TITLE_H + 40
		self.fieldPanel = uiMobileChat.InsetField(W - PAD * 2, FIELD_H)
		self.fieldPanel.SetParent(self)
		self.fieldPanel.SetPosition(PAD, fieldY)
		self.fieldPanel.Show()
		self.input = ui.EditLine()
		self.input.SetParent(self)
		self.input.SetPosition(PAD + 14, fieldY + FIELD_H // 2 - 12)
		self.input.SetSize(W - PAD * 2 - 32, 30)
		self.input.SetMax(self.maxLength)
		self.input.SetNumberMode()
		self.input.SetFontName("Lato:24")
		self.input.SetPackedFontColor(0xFFFFFFFF)
		self.input.SetReturnEvent(ui.__mem_func__(self.OnAccept))
		self.input.SetEscapeEvent(ui.__mem_func__(self.Close))
		self.input.Show()

		quickY = fieldY + FIELD_H + 10
		width = (W - PAD * 2 - 16) // 3
		self.quick1 = FlatButton(width, 40, "+100", False, "Marcellus:18")
		self.quick2 = FlatButton(width, 40, "+1000", False, "Marcellus:18")
		self.quick3 = FlatButton(width, 40, "Tümü", False, "Marcellus:18")
		for (index, (button, event)) in enumerate(((self.quick1, self.__Add100), (self.quick2, self.__Add1000), (self.quick3, self.__All))):
			button.SetParent(self)
			button.SetPosition(PAD + index * (width + 8), quickY)
			button.SetEvent(ui.__mem_func__(event))
			button.Show()

		buttonY = H - PAD - 44
		pairX = (W - 190 * 2 - 12) // 2
		self.accept = SkinButton("Onayla")
		self.accept.SetParent(self)
		self.accept.SetPosition(pairX, buttonY)
		self.accept.SetEvent(ui.__mem_func__(self.OnAccept))
		self.accept.Show()
		self.cancel = SkinButton("Vazgeç")
		self.cancel.SetParent(self)
		self.cancel.SetPosition(pairX + 190 + 12, buttonY)
		self.cancel.SetEvent(ui.__mem_func__(self.Close))
		self.cancel.Show()

	# --- ac / kapat ---
	def Open(self, maxValue, unitValue = 100):
		if not self.built:
			self.__Build()
		self.maxValue = long(maxValue)
		self.unitValue = unitValue if unitValue else 100
		self.info.SetText("Tecrübe puanın")
		self.maxText.SetText("%s" % localeInfo.NumberToString(self.maxValue) if hasattr(localeInfo, "NumberToString") else str(self.maxValue))
		self.input.SetText(str(self.unitValue))
		# ust yarida: klavye acilinca kutu klavyenin ustunde kalsin
		top = max(10, int(wndMgr.GetScreenHeight() * 0.04))
		self.SetPosition((wndMgr.GetScreenWidth() - W) // 2, top)
		self.Show()
		self.SetTop()
		self.input.SetFocus()

	def Close(self):
		if self.built:
			self.input.KillFocus()
		self.Hide()

	def OnPressEscapeKey(self):
		self.Close()
		return True

	# --- hizli degerler ---
	def __Value(self):
		text = self.input.GetText()
		return long(text) if text and text.isdigit() else 0

	def __Set(self, value):
		value = max(0, min(self.maxValue, long(value)))
		self.input.SetText(str(value))
		self.input.SetFocus()

	def __Add100(self):
		self.__Set(self.__Value() + 100)

	def __Add1000(self):
		self.__Set(self.__Value() + 1000)

	def __All(self):
		self.__Set(self.maxValue)

	def OnAccept(self):
		money = min(self.__Value(), self.maxValue)
		if money > 0 and self.eventAccept:
			self.eventAccept(money)
		self.Close()
