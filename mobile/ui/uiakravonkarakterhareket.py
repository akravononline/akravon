# -*- coding: utf-8 -*-
# AKRAVON Karakter penceresi - HAREKETLER sayfasi (tekli / karsilikli). Pencere: uiakravonkarakter.py
# Dokun: hareketi yap. Basili tut: hizli yuvaya surukle.
import net
import player
import chr
import chat
import emotion
import wndMgr
import mouseModule
import localeInfo
import ui
import uiToolTip
import uiAkravonParca as P

KARSILIKLI = 50	# 50 ustu yuvalar karsilikli hareket


class HareketSayfasi(object):
	def __init__(self, sayfa, pencere):
		self.sayfa = sayfa
		self.pencere = pencere
		self.ipucu = uiToolTip.ToolTip()
		P.Yazi(sayfa, 0, 22, "Tekli Hareketler", P.ALTIN, P.GOVDE)
		self.tekli = self.__Izgara(46, 1)
		P.Yazi(sayfa, 0, 202, "Karşılıklı Hareketler", P.ALTIN, P.GOVDE)
		self.karsilikli = self.__Izgara(226, KARSILIKLI + 1)
		import uiAkravonActionIcon
		for (no, veri) in emotion.EMOTION_DICT.items():
			izgara = self.karsilikli if no > KARSILIKLI else self.tekli
			izgara.SetEmotionSlot(no, no)
			ikon = uiAkravonActionIcon.GetCharacterEmotionIcon(no)
			if ikon:
				izgara.SetCoverButton(no, ikon, ikon, ikon, ikon)
			else:
				izgara.SetCoverButton(no)

	def __Izgara(self, y, ilk):
		izgara = ui.GridSlotWindow()
		izgara.SetParent(self.sayfa)
		izgara.SetPosition(0, y)
		izgara.ArrangeSlot(ilk, 6, 3, 40, 40, 37, 8)
		izgara.SetSlotBaseImage(P.GORSEL + "cw_slot40.png", 1.0, 1.0, 1.0, 1.0)
		izgara.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
		izgara.SetSelectItemSlotEvent(ui.__mem_func__(self.__Tut))
		izgara.SetUnselectItemSlotEvent(ui.__mem_func__(self.Yap))
		izgara.SetUseSlotEvent(ui.__mem_func__(self.Yap))
		izgara.SetOverInItemEvent(ui.__mem_func__(self.__IpucuGoster))
		izgara.SetOverOutItemEvent(ui.__mem_func__(self.IpucuGizle))
		try:
			if hasattr(wndMgr, "SetSlotUnitSize"):
				wndMgr.SetSlotUnitSize(izgara.hWnd, 40, 40)
		except Exception:
			pass
		izgara.Show()
		return izgara

	def __Tut(self, no):
		if no in emotion.EMOTION_DICT:
			mouseModule.mouseController.AttachObject(self.pencere, player.SLOT_TYPE_EMOTION, no, no)

	def Yap(self, no):
		if no not in emotion.EMOTION_DICT or player.IsActingEmotion():
			return
		komut = emotion.EMOTION_DICT[no]["command"]
		if no > KARSILIKLI:
			hedef = player.GetTargetVID()
			if 0 == hedef or hedef == player.GetMainCharacterIndex() or chr.IsNPC(hedef) or chr.IsEnemy(hedef):
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.EMOTION_CHOOSE_ONE)
				return
			komut += " " + chr.GetNameByVID(hedef)
		net.SendChatPacket(komut)

	def __IpucuGoster(self, no):
		if no not in emotion.EMOTION_DICT:
			return
		self.ipucu.ClearToolTip()
		self.ipucu.SetTitle(emotion.EMOTION_DICT[no]["name"])
		self.ipucu.AlignHorizonalCenter()
		self.ipucu.ShowToolTip()

	def IpucuGizle(self):
		if self.ipucu:
			self.ipucu.HideToolTip()

	def Yenile(self):
		pass

	def Yok(self):
		self.IpucuGizle()
		self.ipucu = None
		self.pencere = None
