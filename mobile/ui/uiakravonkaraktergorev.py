# -*- coding: utf-8 -*-
# AKRAVON Karakter penceresi - GOREVLER sayfasi. Pencere: uiakravonkarakter.py
#   Sekmeler: Devam Edenler / Gecmis (biten gorevler: uiAkravonQuest.GetHistory)
#   Sayfa basina 5 gorev; fazlasi oklarla (ince kaydirma cubugu yok)
#   Devam eden goreve dokun: gorev penceresi acilir
import quest
import event
import localeInfo
import ui
import uiAkravonParca as P
import uiAkravonDugme

SATIR = 5	# quest.QUEST_MAX_NUM ile ayni
SATIR_Y = 54
SATIR_ADIM = 70


class GorevSayfasi(object):
	def __init__(self, sayfa):
		self.sayfa = sayfa
		self.gecmis = False
		self.ilk = 0
		self.gorunur = False
		P.Yazi(sayfa, 0, 22, "Görevler", P.ALTIN, P.BOLUM)
		self.sekmeler = []
		for (i, (yazi, x, genislik)) in enumerate((("Devam Edenler", 130, 150), ("Geçmiş", 288, 110))):
			dugme = uiAkravonDugme.YaziDugme(genislik, 36, yazi)
			dugme.SetParent(sayfa)
			dugme.SetPosition(x, 13)
			dugme.OlayAyarla(self.Mod, i == 1)
			dugme.Show()
			self.sekmeler.append(dugme)
		for i in range(SATIR):
			P.Resim(sayfa, 0, SATIR_Y + i * SATIR_ADIM, "cw_questcard.png")
		self.ikonlar = ui.GridSlotWindow()
		self.ikonlar.SetParent(sayfa)
		self.ikonlar.SetPosition(12, SATIR_Y + 15)
		self.ikonlar.ArrangeSlot(0, 1, SATIR, 32, 32, 0, SATIR_ADIM - 32)
		self.ikonlar.SetSlotBaseImage("d:/ymir work/ui/game/quest/slot_base.sub", 1.0, 1.0, 1.0, 1.0)
		for i in range(SATIR):
			self.ikonlar.HideSlotBaseImage(i)
		self.ikonlar.Show()
		self.adlar = []
		self.sureler = []
		self.sayaclar = []
		self.dokunmalar = []
		for i in range(SATIR):
			y = SATIR_Y + i * SATIR_ADIM
			self.adlar.append(P.Yazi(sayfa, 56, y + 9, "", P.PARLAK, P.GOVDE))
			self.sureler.append(P.Yazi(sayfa, 56, y + 30, "", P.SOLUK, P.NOT))
			self.sayaclar.append(P.Yazi(sayfa, 56, y + 46, "", P.SOLUK, P.NOT))
			dokunma = uiAkravonDugme.Dokunma(406, 62)
			dokunma.SetParent(sayfa)
			dokunma.SetPosition(0, y)
			dokunma.OlayAyarla(self.__GorevAc, i)
			dokunma.Show()
			self.dokunmalar.append(dokunma)
		self.solOk = uiAkravonDugme.IkonDugme("arrow_l", 34)
		self.solOk.SetParent(sayfa)
		self.solOk.GorselKonumu(290, 412)
		self.solOk.OlayAyarla(self.Kaydir, -1)
		self.sayfaYazi = P.Yazi(sayfa, 354, 419, "", P.ALTIN, P.GOVDE, "center")
		self.sagOk = uiAkravonDugme.IkonDugme("arrow_r", 34)
		self.sagOk.SetParent(sayfa)
		self.sagOk.GorselKonumu(386, 412)
		self.sagOk.OlayAyarla(self.Kaydir, 1)
		self.__SekmeGorunumu()

	def __SekmeGorunumu(self):
		for (i, dugme) in enumerate(self.sekmeler):
			dugme.SeciliAyarla((i == 1) == self.gecmis)

	def Mod(self, gecmis):
		self.gecmis = bool(gecmis)
		self.ilk = 0
		self.__SekmeGorunumu()
		self.Yenile()

	def __Sayi(self):
		if self.gecmis:
			import uiAkravonQuest
			return len(uiAkravonQuest.GetHistory())
		return quest.GetQuestCount()

	def Kaydir(self, yon):
		sayi = self.__Sayi()
		self.ilk = max(0, min(self.ilk + yon * SATIR, max(0, (sayi - 1) // SATIR * SATIR)))
		self.Yenile()

	def __GorevAc(self, i):
		if self.gecmis:
			return
		sira = self.ilk + i
		if sira >= quest.GetQuestCount():
			return
		event.QuestButtonClick(-2147483648 + quest.GetQuestIndex(sira))

	def Yenile(self):
		sayi = self.__Sayi()
		if self.ilk >= sayi:
			self.ilk = max(0, (sayi - 1) // SATIR * SATIR) if sayi else 0
		gecmisListe = None
		if self.gecmis:
			import uiAkravonQuest
			gecmisListe = uiAkravonQuest.GetHistory()
		for i in range(SATIR):
			sira = self.ilk + i
			self.ikonlar.ClearSlot(i)
			self.ikonlar.HideSlotBaseImage(i)
			if sira >= sayi:
				for yazi in (self.adlar[i], self.sureler[i], self.sayaclar[i]):
					yazi.SetText("")
				continue
			if self.gecmis:
				(ad, zaman) = gecmisListe[sira]
				self.adlar[i].SetText(ad)
				self.sureler[i].SetText(zaman)
				self.sayaclar[i].SetText("Tamamlandı")
			else:
				(ad, ikon, sayacAdi, sayacDegeri) = quest.GetQuestData(sira)
				self.adlar[i].SetText(ad)
				self.sayaclar[i].SetText(("%s : %d" % (sayacAdi, sayacDegeri)) if sayacAdi else "")
				self.ikonlar.SetSlot(i, i, 1, 1, ikon)
		sayfaSayisi = max(1, (sayi + SATIR - 1) // SATIR)
		self.sayfaYazi.SetText("%d/%d" % (self.ilk // SATIR + 1, sayfaSayisi))
		for parca in (self.solOk, self.sagOk, self.sayfaYazi):
			if sayfaSayisi > 1:
				parca.Show()
			else:
				parca.Hide()
		self.SureleriGuncelle()

	def SureleriGuncelle(self):
		if self.gecmis:
			return
		for i in range(SATIR):
			sira = self.ilk + i
			if sira >= quest.GetQuestCount():
				continue
			(ad, kalan) = quest.GetQuestLastTime(sira)
			yazi = localeInfo.QUEST_UNLIMITED_TIME
			if ad:
				if kalan <= 0:
					yazi = localeInfo.QUEST_TIMEOVER
				else:
					dakika = kalan / 60
					saniye = kalan % 60
					yazi = ad + " : "
					if dakika > 0:
						yazi += str(dakika) + localeInfo.QUEST_MIN + (" " if saniye > 0 else "")
					if saniye > 0:
						yazi += str(saniye) + localeInfo.QUEST_SEC
			self.sureler[i].SetText(yazi)

	def Yok(self):
		for dugme in self.sekmeler + self.dokunmalar + [self.solOk, self.sagOk]:
			dugme.Yok()
		self.sekmeler = []
		self.dokunmalar = []
