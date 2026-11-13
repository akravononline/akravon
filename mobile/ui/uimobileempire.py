# -*- coding: utf-8 -*-
# AKRAVON: bolge (krallik) secme ekrani - bizim tasarim dili (skin/build_akravon_empire.py)
from __future__ import division
import dbg
import net
import wndMgr
import ui
import introEmpire
import uiMobileStyle
import uiAkravon

from _weakref import proxy

IMG = "d:/ymir work/ui/akravon_legacy/"
CARD_W = 380
CARD_H = 420
CARD_GAP = 30
GOLD = uiMobileStyle.Color(222, 184, 110)
TEXT = uiMobileStyle.Color(250, 236, 210)
SOFT = uiMobileStyle.Color(190, 178, 152)
BODY_FONT = "Lato:17"
HIDDEN_NAMES = ("Atlas", "left_button", "right_button", "empire_board", "Title", "desc_phase", "BackGround", "Alpha", "Top_Line", "Bottom_Line", "text_board")

# bolge: (anahtar, bolge adi, vurgu rengi, aciklama, koyler)
EMPIRES = {
	net.EMPIRE_A : ("a", "KIRMIZI BÖLGE", uiMobileStyle.Color(228, 96, 80), "Kızıl sancak altında toplanan savaşçıların bölgesi. Yolculuğun Yongan köyünde başlar.", "Yongan  ·  Joan"),
	net.EMPIRE_B : ("b", "SARI BÖLGE", uiMobileStyle.Color(238, 200, 84), "Sarı sancaklı bölge. Yolculuğun Jayang köyünde başlar.", "Jayang  ·  Bokjung"),
	net.EMPIRE_C : ("c", "MAVİ BÖLGE", uiMobileStyle.Color(110, 160, 240), "Mavi sancağın diyarı. Yolculuğun Pyungmoo köyünde başlar.", "Pyungmoo  ·  Bakra"),
}


class ImageButton(uiMobileStyle.Pressable):
	def __init__(self, width, height, prefix, label, font):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.prefix = prefix
		self.image = ui.ExpandedImageBox()
		self.image.SetParent(self)
		self.image.AddFlag("not_pick")
		self.image.LoadImage(IMG + prefix % 1)
		self.image.Show()
		self.label = uiMobileStyle.CreateText(self, label, width // 2, height // 2 - 12, TEXT, font, "center")

	def RefreshVisual(self):
		if getattr(self, "image", None):
			state = {"normal" : 1, "over" : 2, "down" : 3}.get(self.GetVisualState(), 1)
			self.image.LoadImage(IMG + self.prefix % state)

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.image = None
		self.label = None


class EmpireCard(uiMobileStyle.Pressable):
	def __init__(self, owner, empireId, scale = 1.0):
		# AKRAVON: kart olcegi ekrana gore (genislik ve yukseklik) hesaplanir; tum olculer ayni oranla kucultulur
		self.scale = scale
		self.cw = int(CARD_W * scale)
		self.ch = int(CARD_H * scale)
		cw = self.cw
		ch = self.ch
		uiMobileStyle.Pressable.__init__(self, cw, ch)
		self.owner = proxy(owner)
		self.empireId = empireId
		(key, name, accent, description, villages) = EMPIRES.get(empireId, EMPIRES[net.EMPIRE_A])
		self.keep = []
		self.selected = None

		self.glow = ui.ExpandedImageBox()
		self.glow.SetParent(self)
		self.glow.AddFlag("not_pick")
		self.glow.LoadImage(IMG + "emp_glow.png")
		self.glow.SetScale(scale, scale)
		self.glow.SetPosition(int(-20 * scale), int(-20 * scale))
		self.glow.Hide()
		self.bg = ui.ExpandedImageBox()
		self.bg.SetParent(self)
		self.bg.AddFlag("not_pick")
		self.bg.LoadImage(IMG + "emp_card_off.png")
		self.bg.SetScale(scale, scale)
		self.bg.Show()

		emblem = ui.ExpandedImageBox()
		emblem.SetParent(self)
		emblem.AddFlag("not_pick")
		emblem.LoadImage(IMG + "emp_emblem_%s.png" % key)
		emblem.SetScale(scale, scale)
		emblem.SetPosition((cw - int(120 * scale)) // 2, int(30 * scale))
		emblem.Show()
		self.keep.append(emblem)
		bodyFont = "Lato:%d" % max(13, int(17 * scale))
		y = int((30 + 150 + 22) * scale)
		self.keep.append(uiMobileStyle.CreateText(self, name, cw // 2, y, accent, "Marcellus:%d" % max(20, int(32 * scale)), "center"))
		y += int(54 * scale)
		self.keep.append(uiMobileStyle.CreateBar(self, uiMobileStyle.Color(96, 74, 42), int(40 * scale), y, cw - int(80 * scale), 1))
		y += int(18 * scale)
		for line in uiMobileStyle.WrapLines(description, cw - int(64 * scale), bodyFont):
			self.keep.append(uiMobileStyle.CreateText(self, line, cw // 2, y, TEXT, bodyFont, "center"))
			y += max(18, int(26 * scale))
		self.keep.append(uiMobileStyle.CreateText(self, villages, cw // 2, ch - int(46 * scale), SOFT, "Marcellus:%d" % max(14, int(19 * scale)), "center"))
		self.SetEvent(ui.__mem_func__(self.__Notify))

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.owner = None
		self.glow = None
		self.bg = None
		self.keep = []

	def __Notify(self):
		if self.owner:
			self.owner.OnSelectEmpire(self.empireId)

	def SetSelected(self, selected):
		if selected == self.selected:
			return
		self.selected = selected
		self.bg.LoadImage(IMG + ("emp_card_on.png" if selected else "emp_card_off.png"))
		self.bg.SetScale(self.scale, self.scale)	# LoadImage olcegi sifirlar: tekrar uygula
		if selected:
			self.glow.Show()
		else:
			self.glow.Hide()


class MobileEmpireLayout(object):
	def Open(self):
		self.mobileCards = []
		self.mobileKeep = []
		self.mobileSelectButton = None
		self.mobileBackButton = None
		self.mobileReady = False
		self.shownEmpire = None
		super(MobileEmpireLayout, self).Open()
		if not getattr(self, "selectButton", None):
			return
		# sari bolge (Chunjo): eski betik yalniz kirmizi ve mavi tanimliyordu - uc bolge birlikte gosterilir
		for table in (self.empireAreaDestAlpha, self.empireAreaFlagDestAlpha, self.empireFlagDestAlpha, self.empireAreaCurAlpha, self.empireAreaFlagCurAlpha, self.empireFlagCurAlpha):
			table.setdefault(net.EMPIRE_B, 0.0)
		try:
			self.__ApplyMobileLayout()
			self.mobileReady = True
		except Exception:
			import traceback
			dbg.TraceError("MobileEmpireLayout.Open - layout error: %s" % traceback.format_exc())

	def Close(self):
		for card in getattr(self, "mobileCards", []):
			card.Destroy()
		for button in (getattr(self, "mobileSelectButton", None), getattr(self, "mobileBackButton", None)):
			if button:
				button.Destroy()
		self.mobileCards = []
		self.mobileKeep = []
		self.mobileSelectButton = None
		self.mobileBackButton = None
		self.mobileReady = False
		super(MobileEmpireLayout, self).Close()

	def __ApplyMobileLayout(self):
		screenWidth = wndMgr.GetScreenWidth()
		screenHeight = wndMgr.GetScreenHeight()
		for name in HIDDEN_NAMES:
			child = self.GetChild2(name)
			if child:
				child.Hide()
		for button in list(self.empireAreaButton.values()):
			button.Hide()
		for button in (self.selectButton, self.exitButton):
			uiMobileStyle.ParkWindow(button)
		descriptionBox = getattr(self, "descriptionBox", None)
		if descriptionBox:
			descriptionBox.Hide()

		# arka plan: karakter olusturma manzarasi + koyu ortu
		background = ui.ExpandedImageBox()
		background.SetParent(self)
		background.AddFlag("not_pick")
		background.LoadImage(uiAkravon.Asset("create_background"))
		background.SetScale(screenWidth / 1280.0, screenHeight / 720.0)
		background.SetPosition(0, 0)
		background.Show()
		self.mobileKeep.append(background)
		self.mobileKeep.append(uiMobileStyle.CreateBar(self, uiMobileStyle.Color(170, 4, 8, 18), 0, 0, screenWidth, screenHeight))

		self.mobileKeep.append(uiMobileStyle.CreateText(self, "BÖLGENİ SEÇ", screenWidth // 2, 46, GOLD, "Marcellus:38", "center"))
		self.mobileKeep.append(uiMobileStyle.CreateBar(self, uiMobileStyle.Color(120, 92, 50), screenWidth // 2 - 170, 100, 340, 1))
		self.mobileKeep.append(uiMobileStyle.CreateText(self, "Bu seçim hesabındaki tüm karakterler için geçerlidir.", screenWidth // 2, 116, SOFT, "Lato:18", "center"))

		self.mobileBackButton = ImageButton(130, 46, "emp_back_%d.png", "GERİ", "Marcellus:19")
		self.mobileBackButton.SetParent(self)
		self.mobileBackButton.SetPosition(28, 24)
		self.mobileBackButton.SetEvent(ui.__mem_func__(self.__OnClickExit))
		self.mobileBackButton.Show()

		empireIds = [net.EMPIRE_A, net.EMPIRE_B, net.EMPIRE_C]
		# AKRAVON: ekran hangi boyutta olursa olsun kartlar ve alttaki dugme sigsin (genislik ve yukseklige gore olcek)
		count = len(empireIds)
		scaleW = float(screenWidth - 48 - (count - 1) * CARD_GAP) / (count * CARD_W)
		scaleH = float(screenHeight - 146 - 120) / CARD_H
		scale = max(0.5, min(1.0, scaleW, scaleH))
		gap = int(CARD_GAP * min(1.0, scale + 0.2))
		cardW = int(CARD_W * scale)
		cardH = int(CARD_H * scale)
		rowWidth = count * cardW + (count - 1) * gap
		startX = (screenWidth - rowWidth) // 2
		top = max(146, (screenHeight - cardH - 120) // 2 + 30)
		top = min(top, screenHeight - cardH - 34 - 56 - 12)
		top = max(110, top)
		for (index, empireId) in enumerate(empireIds):
			card = EmpireCard(self, empireId, scale)
			card.SetParent(self)
			card.SetPosition(startX + index * (cardW + gap), top)
			card.Show()
			self.mobileCards.append(card)

		self.mobileSelectButton = ImageButton(300, 56, "emp_btn300_%d.png", "BÖLGEYİ SEÇ", "Marcellus:23")
		self.mobileSelectButton.SetParent(self)
		self.mobileSelectButton.SetPosition((screenWidth - 300) // 2, top + cardH + 24)
		self.mobileSelectButton.SetEvent(ui.__mem_func__(self.__OnClickSelect))
		self.mobileSelectButton.Show()
		self.__RefreshCards()

	def __OnClickSelect(self):
		uiMobileStyle.RunButtonEvent(self.selectButton)

	def __OnClickExit(self):
		uiMobileStyle.RunButtonEvent(self.exitButton)

	def __RefreshCards(self):
		self.shownEmpire = self.empireID
		for card in self.mobileCards:
			card.SetSelected(card.empireId == self.empireID)

	def OnUpdate(self):
		super(MobileEmpireLayout, self).OnUpdate()
		if getattr(self, "mobileReady", False) and self.empireID != self.shownEmpire:
			self.__RefreshCards()


class MobileSelectEmpireWindow(MobileEmpireLayout, introEmpire.SelectEmpireWindow):
	pass


class MobileReselectEmpireWindow(MobileEmpireLayout, introEmpire.ReselectEmpireWindow):
	pass
