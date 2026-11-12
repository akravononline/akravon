# -*- coding: utf-8 -*-
# AKRAVON Karakter penceresi - YETENEKLER sayfasi. Pencere: uiakravonkarakter.py
#   Ust: kalan puan + grup sekmeleri (grup secilmediyse iki grup onizleme; secildiyse grup + Binicilik)
#   Kartlar: 6 aktif beceri (ikon, ad, kademe, arti). Kart k  <->  motor yuvasi k+1  <->  oyuncu beceri yuvasi TABAN+k+1
#   Destek: sayfa basina 6 destek becerisi (oklarla sayfa), motor yuvasi 101+j
# Ikonlar motorun yuva penceresinde (surukle -> hizli yuva, bekleme suresi, ipucu); ustunde HD ikon katmani.
import app
import net
import player
import skill
import chr
import wndMgr
import mouseModule
import localeInfo
import playerSettingModule
import ui
import uiAkravonParca as P
import uiAkravonDugme

AKTIF_YUVA = 8	# motor: aktif beceri sayfasi yuva sayisi (binicilik = 8 sonrasi)
KART = 6
SAYFA_BINICILIK = 2
DESTEK_SAYFA = 6
DESTEK_TABAN = 101
DESTEK_GIZLI = (137, 138, 139, 140)
DESTEK_LISTE = (121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 137, 138, 139, 140)
DESTEK_IKON = {121 : "tongsol", 122 : "combo", 123 : "fishing", 124 : "mining", 125 : "making", 126 : "language1", 127 : "language2",
	128 : "language3", 129 : "polymorph", 130 : "riding_01", 131 : "summon"}
BOS = P.GORSEL + "cw_clear32.png"

KART_X = (0, 219)
KART_Y = 66
KART_ADIM = 54
DESTEK_Y = 262
DESTEK_ADIM = 58

GRUP_ADLARI = {
	playerSettingModule.JOB_WARRIOR : {1 : localeInfo.SKILL_GROUP_WARRIOR_1, 2 : localeInfo.SKILL_GROUP_WARRIOR_2},
	playerSettingModule.JOB_ASSASSIN : {1 : localeInfo.SKILL_GROUP_ASSASSIN_1, 2 : localeInfo.SKILL_GROUP_ASSASSIN_2},
	playerSettingModule.JOB_SURA : {1 : localeInfo.SKILL_GROUP_SURA_1, 2 : localeInfo.SKILL_GROUP_SURA_2},
	playerSettingModule.JOB_SHAMAN : {1 : localeInfo.SKILL_GROUP_SHAMAN_1, 2 : localeInfo.SKILL_GROUP_SHAMAN_2},
}


def _KartKonumu(k):
	return (KART_X[k % 2], (k // 2) * KART_ADIM)


def _DestekKonumu(j):
	return (KART_X[j % 2], (j // 2) * DESTEK_ADIM)


class BeceriSayfasi(object):
	def __init__(self, sayfa, pencere):
		self.sayfa = sayfa
		self.pencere = pencere	# surukle-birak sahibi (mouseModule)
		self.ipucu = None
		self.secilenGrup = 0	# motor grup sayfasi (0 / 1 grup, 2 binicilik)
		self.sekme = 0
		self.destekSayfa = 0
		self.destekHucre = {}	# kaynak yuva -> motor yuvasi (101..106)

		P.Yazi(sayfa, 0, 28, "Kalan Puan:", P.SOLUK, P.GOVDE)
		self.kalan = P.Yazi(sayfa, 92, 28, "0", P.ALTIN, P.GOVDE_BUYUK)
		self.grupDugmeleri = []
		for i in range(2):
			dugme = uiAkravonDugme.YaziDugme(84, 36, "", uiAkravonDugme.YAZI_DAR)
			dugme.SetParent(sayfa)
			dugme.SetPosition(252 + i * 90, 19)
			dugme.OlayAyarla(self.GrupSec, i)
			self.grupDugmeleri.append(dugme)
		self.grupAdi = P.Yazi(sayfa, 252, 28, "", P.ALTIN, P.GOVDE)

		# aktif beceri kartlari
		kartlar = P.Pencere(sayfa, 0, KART_Y, 426, 162)
		self.adlar = []
		self.kademeler = []
		self.hdIkonlar = []
		for k in range(KART):
			(x, y) = _KartKonumu(k)
			kart = P.Resim(kartlar, x, y, "cw_skillcard2.png")
			self.adlar.append(P.Yazi(kart, 58, 6, "", P.PARLAK, P.GOVDE_KUCUK))
			self.kademeler.append(P.Yazi(kart, 58, 27, "", P.ALTIN, P.GOVDE))
		self.aktifYuva = self.__YuvaPenceresi(0, KART_Y, 162, [(k + 1, _KartKonumu(k)[0] + 4, _KartKonumu(k)[1] + 3) for k in range(KART)])
		hdKatman = P.Pencere(sayfa, 0, KART_Y, 426, 162)
		for k in range(KART):
			(x, y) = _KartKonumu(k)
			ikon = P.Resim(hdKatman, x + 4, y + 3, BOS, True)
			ikon.Hide()
			self.hdIkonlar.append(ikon)
		self.artilar = []
		for k in range(KART):
			(x, y) = _KartKonumu(k)
			arti = uiAkravonDugme.IkonDugme("plus", 32)
			arti.SetParent(sayfa)
			arti.GorselKonumu(x + 207 - 39, KART_Y + y + 9)
			arti.OlayAyarla(self.__BeceriArttir, k)
			arti.Hide()
			self.artilar.append(arti)

		# destek becerileri
		P.Yazi(sayfa, 0, 234, "Destek Yetenekleri", P.SOLUK, P.GOVDE_KUCUK)
		self.destekAdlar = []
		self.destekKademeler = []
		self.destekIkonlar = []
		destek = P.Pencere(sayfa, 0, DESTEK_Y, 426, 174)
		for j in range(DESTEK_SAYFA):
			(x, y) = _DestekKonumu(j)
			self.destekAdlar.append(P.Yazi(destek, x + 58, y + 6, "", P.PARLAK, P.GOVDE))
			self.destekKademeler.append(P.Yazi(destek, x + 58, y + 28, "", P.ALTIN, P.GOVDE))
		self.destekYuva = self.__YuvaPenceresi(0, DESTEK_Y, 174, [(DESTEK_TABAN + j, _DestekKonumu(j)[0] + 4, _DestekKonumu(j)[1] + 5) for j in range(DESTEK_SAYFA)])
		destekHd = P.Pencere(sayfa, 0, DESTEK_Y, 426, 174)
		for j in range(DESTEK_SAYFA):
			(x, y) = _DestekKonumu(j)
			ikon = P.Resim(destekHd, x + 2, y + 3, BOS, True)
			ikon.Hide()
			self.destekIkonlar.append(ikon)
		self.solOk = uiAkravonDugme.IkonDugme("arrow_l", 34)
		self.solOk.SetParent(sayfa)
		self.solOk.GorselKonumu(290, 228)
		self.solOk.OlayAyarla(self.DestekSayfa, -1)
		self.sayfaYazi = P.Yazi(sayfa, 354, 235, "1/1", P.ALTIN, P.GOVDE, "center")
		self.sagOk = uiAkravonDugme.IkonDugme("arrow_r", 34)
		self.sagOk.SetParent(sayfa)
		self.sagOk.GorselKonumu(386, 228)
		self.sagOk.OlayAyarla(self.DestekSayfa, 1)

	def __YuvaPenceresi(self, x, y, yukseklik, yuvalar):
		yuva = ui.SlotWindow()
		yuva.SetParent(self.sayfa)
		yuva.SetPosition(x, y)
		yuva.SetSize(426, yukseklik)
		for (no, yx, yy) in yuvalar:
			yuva.AppendSlot(no, yx, yy, 44, 44)
		yuva.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
		yuva.SetSelectItemSlotEvent(ui.__mem_func__(self.__Tut))
		yuva.SetSelectEmptySlotEvent(ui.__mem_func__(self.__BosYuva))
		yuva.SetUnselectItemSlotEvent(ui.__mem_func__(self.__Kullan))
		yuva.SetUseSlotEvent(ui.__mem_func__(self.__Kullan))
		yuva.SetOverInItemEvent(ui.__mem_func__(self.__IpucuGoster))
		yuva.SetOverOutItemEvent(ui.__mem_func__(self.IpucuGizle))
		try:
			if hasattr(wndMgr, "SetSlotUnitSize"):
				wndMgr.SetSlotUnitSize(yuva.hWnd, 44, 44)
			if hasattr(wndMgr, "SetSlotRoundIcons"):
				wndMgr.SetSlotRoundIcons(yuva.hWnd, 1)
		except Exception:
			pass
		yuva.Show()
		return yuva

	# ---------------------------------------------------------------- yuva eslemesi
	def __Taban(self):
		return AKTIF_YUVA if self.secilenGrup == SAYFA_BINICILIK else 0

	def __Kaynak(self, hucre):
		# motor yuvasi -> oyuncu beceri yuvasi
		if hucre >= DESTEK_TABAN:
			for (kaynak, h) in self.destekHucre.items():
				if h == hucre:
					return kaynak
			return 0
		return self.__Taban() + hucre

	def __Hucre(self, kaynak):
		# oyuncu beceri yuvasi -> motor yuvasi (bu sayfada yoksa 0)
		if kaynak in self.destekHucre:
			return self.destekHucre[kaynak]
		hucre = kaynak - self.__Taban()
		if 1 <= hucre <= KART:
			return hucre
		return 0

	def __Yuva(self, hucre):
		return self.destekYuva if hucre >= DESTEK_TABAN else self.aktifYuva

	@staticmethod
	def GrupSecildi():
		return 0 != net.GetMainActorSkillGroup()

	# ---------------------------------------------------------------- grup
	def GrupAdlariniKur(self):
		race = net.GetMainActorRace()
		adlar = GRUP_ADLARI.get(chr.RaceToJob(race), {})
		grup = net.GetMainActorSkillGroup()
		if grup == 0:
			yazilar = (adlar.get(1, ""), adlar.get(2, ""))
		else:
			yazilar = (adlar.get(grup, ""), localeInfo.SKILL_GROUP_HORSE)
		for (dugme, yazi) in zip(self.grupDugmeleri, yazilar):
			dugme.YaziAyarla(yazi)
			dugme.Show()
		self.grupAdi.Hide()

	def GrupSec(self, sekme):
		self.sekme = sekme
		grup = net.GetMainActorSkillGroup()
		if grup != 0:
			self.secilenGrup = (grup - 1) if sekme == 0 else SAYFA_BINICILIK
		else:
			self.secilenGrup = sekme
		for (i, dugme) in enumerate(self.grupDugmeleri):
			dugme.SeciliAyarla(i == sekme)
		playerSettingModule.RegisterSkill(net.GetMainActorRace(), self.secilenGrup + 1, net.GetMainActorEmpire())
		self.Yenile()

	def DestekSayfa(self, yon):
		self.destekSayfa = max(0, self.destekSayfa + yon)
		self.Yenile()

	# ---------------------------------------------------------------- yenile
	def Yenile(self):
		self.__AktifYenile()
		self.__DestekYenile()
		self.__ArtiYenile()

	def __Temizle(self, yuva, hucre):
		durum = None
		if app.ENABLE_SLOT_WINDOW_EX:
			try:
				durum = (yuva.IsActivatedSlot(hucre), yuva.GetSlotCoolTime(hucre))
			except Exception:
				durum = None
		yuva.ClearSlot(hucre)
		return durum

	def __Geri(self, yuva, hucre, durum):
		if not durum:
			return
		(aktif, (sure, gecen)) = durum
		if aktif:
			yuva.ActivateSlot(hucre)
		if sure:
			yuva.SetSlotCoolTime(hucre, sure, gecen)

	def __AktifYenile(self):
		yuva = self.aktifYuva
		taban = self.__Taban()
		for k in range(KART):
			hucre = k + 1
			kaynak = taban + hucre
			durum = self.__Temizle(yuva, hucre)
			beceri = player.GetSkillIndex(kaynak)
			ikon = self.hdIkonlar[k]
			if 0 == beceri:
				self.adlar[k].SetText("")
				self.kademeler[k].SetText("")
				ikon.Hide()
				continue
			kademe = player.GetSkillGrade(kaynak)
			seviye = player.GetSkillLevel(kaynak)
			gosterKademe = min(kademe, skill.SKILL_GRADE_COUNT - 1)
			if player.SKILL_INDEX_RIDING == beceri:
				seviye = {1 : seviye + 19, 2 : seviye + 29, 3 : 40}.get(kademe, seviye)
				gosterKademe = 0
			yuva.SetSkillSlotNew(hucre, beceri, gosterKademe, seviye)
			yuva.SetSlotCount(hucre, 0)
			yuva.SetCoverButton(hucre, BOS, BOS, BOS, BOS)
			if not self.GrupSecildi():
				yuva.DisableCoverButton(hucre)
			self.__Geri(yuva, hucre, durum)
			try:
				self.adlar[k].SetText(skill.GetSkillName(beceri, gosterKademe))
			except Exception:
				self.adlar[k].SetText(skill.GetSkillName(beceri))
			(yazi, renk) = P.KademeYazisi(kademe if player.SKILL_INDEX_RIDING != beceri else 0, seviye)
			self.kademeler[k].SetText(yazi)
			self.kademeler[k].SetPackedFontColor(renk)
			dosya = None
			try:
				import uiAkravonSkillIcon
				dosya = uiAkravonSkillIcon.GetIcon(beceri, kademe)
			except Exception:
				dosya = None
			if dosya:
				try:
					ikon.LoadImage(dosya.replace("akravon_skill/", "akravon_skill_oct/"))	# orijinal gibi sekizgen
					ikon.SetScale(44.0 / 96.0, 44.0 / 96.0)
				except Exception:
					ikon.LoadImage(dosya)
					ikon.SetScale(48.0 / 64.0, 48.0 / 64.0)
				ikon.Show()
			else:
				ikon.Hide()
		yuva.RefreshSlot()

	def __DestekYenile(self):
		yuva = self.destekYuva
		ogrenilen = []
		for i in range(13):
			kaynak = DESTEK_TABAN + i
			beceri = player.GetSkillIndex(kaynak)
			if beceri and beceri in DESTEK_LISTE and beceri not in DESTEK_GIZLI:
				ogrenilen.append(kaynak)
		sayfaSayisi = max(1, (len(ogrenilen) + DESTEK_SAYFA - 1) // DESTEK_SAYFA)
		self.destekSayfa = max(0, min(self.destekSayfa, sayfaSayisi - 1))
		self.destekHucre = {}
		gorunen = ogrenilen[self.destekSayfa * DESTEK_SAYFA:(self.destekSayfa + 1) * DESTEK_SAYFA]
		for j in range(DESTEK_SAYFA):
			hucre = DESTEK_TABAN + j
			durum = self.__Temizle(yuva, hucre)
			if j >= len(gorunen):
				self.destekAdlar[j].SetText("")
				self.destekKademeler[j].SetText("")
				self.destekIkonlar[j].Hide()
				continue
			kaynak = gorunen[j]
			self.destekHucre[kaynak] = hucre
			beceri = player.GetSkillIndex(kaynak)
			kademe = player.GetSkillGrade(kaynak)
			seviye = player.GetSkillLevel(kaynak)
			yuva.SetSkillSlot(hucre, beceri, seviye)
			try:
				yuva.HideSlotBaseImage(hucre)
			except Exception:
				pass
			if skill.CanUseSkill(beceri):
				yuva.SetCoverButton(hucre, BOS, BOS, BOS, BOS)
			self.__Geri(yuva, hucre, durum)
			self.destekAdlar[j].SetText(skill.GetSkillName(beceri))
			(yazi, renk) = P.KademeYazisi(kademe, seviye)
			self.destekKademeler[j].SetText(yazi)
			self.destekKademeler[j].SetPackedFontColor(renk)
			ikon = self.destekIkonlar[j]
			if beceri in DESTEK_IKON:
				ikon.LoadImage("d:/ymir work/ui/akravon_skill_orj/%s.png" % DESTEK_IKON[beceri])
				ikon.SetScale(48.0 / 96.0, 48.0 / 96.0)
				ikon.Show()
			else:
				ikon.Hide()
		yuva.RefreshSlot()
		self.sayfaYazi.SetText("%d/%d" % (self.destekSayfa + 1, sayfaSayisi))
		for parca in (self.solOk, self.sagOk, self.sayfaYazi):
			if sayfaSayisi > 1:
				parca.Show()
			else:
				parca.Hide()

	def __ArtiYenile(self):
		binicilik = self.secilenGrup == SAYFA_BINICILIK
		puan = player.GetStatus(player.SKILL_HORSE if binicilik else player.SKILL_ACTIVE)
		self.kalan.SetText(str(max(0, puan)))
		taban = self.__Taban()
		for k in range(KART):
			kaynak = taban + k + 1
			beceri = player.GetSkillIndex(kaynak)
			goster = False
			if puan > 0 and beceri and self.GrupSecildi() and player.GetSkillGrade(kaynak) == 0:
				seviye = player.GetSkillLevel(kaynak)
				if binicilik:
					goster = player.GetStatus(player.LEVEL) >= skill.GetSkillLevelLimit(beceri) and seviye < 20
				else:
					goster = skill.CanLevelUpSkill(beceri, seviye)
			if goster:
				self.artilar[k].Show()
			else:
				self.artilar[k].Hide()

	def __BeceriArttir(self, k):
		beceri = player.GetSkillIndex(self.__Taban() + k + 1)
		if beceri:
			net.SendChatPacket("/skillup %d" % beceri)

	# ---------------------------------------------------------------- yuva olaylari
	def __Tut(self, hucre):
		# beceriyi tut: hizli yuvaya surukle
		fare = mouseModule.mouseController
		if fare.isAttached():
			fare.DeattachObject()
			return
		kaynak = self.__Kaynak(hucre)
		beceri = player.GetSkillIndex(kaynak)
		if beceri and skill.CanUseSkill(beceri):
			fare.AttachObject(self.pencere, player.SLOT_TYPE_SKILL, kaynak, beceri)

	def __BosYuva(self, hucre):
		mouseModule.mouseController.DeattachObject()

	def __Kullan(self, hucre):
		kaynak = self.__Kaynak(hucre)
		beceri = player.GetSkillIndex(kaynak)
		if not beceri:
			return
		if skill.GetSkillType(beceri) == skill.SKILL_TYPE_ACTIVE and not self.GrupSecildi():
			return
		if skill.CanUseSkill(beceri):
			player.ClickSkillSlot(kaynak)
			return
		mouseModule.mouseController.DeattachObject()

	def IpucuAyarla(self, ipucu):
		self.ipucu = ipucu

	def __IpucuGoster(self, hucre):
		if mouseModule.mouseController.isAttached() or not self.ipucu:
			return
		kaynak = self.__Kaynak(hucre)
		beceri = player.GetSkillIndex(kaynak)
		if beceri:
			self.ipucu.SetSkillNew(kaynak, beceri, player.GetSkillGrade(kaynak), player.GetSkillLevel(kaynak))

	def IpucuGizle(self):
		if self.ipucu:
			self.ipucu.HideToolTip()

	# ---------------------------------------------------------------- motor bildirimleri (oyuncu beceri yuvasi ile gelir)
	def BeceriKullanildi(self, kaynak, sure):
		hucre = self.__Hucre(kaynak)
		if hucre:
			self.__Yuva(hucre).SetSlotCoolTime(hucre, sure)

	def BeceriAcildi(self, kaynak):
		hucre = self.__Hucre(kaynak)
		if hucre:
			self.__Yuva(hucre).ActivateSlot(hucre)

	def BeceriKapandi(self, kaynak):
		hucre = self.__Hucre(kaynak)
		if hucre:
			self.__Yuva(hucre).DeactivateSlot(hucre)

	def Yok(self):
		for dugme in self.grupDugmeleri + self.artilar + [self.solOk, self.sagOk]:
			dugme.Yok()
		self.grupDugmeleri = []
		self.artilar = []
		self.ipucu = None
		self.pencere = None
