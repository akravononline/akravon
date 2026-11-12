# -*- coding: utf-8 -*-
# AKRAVON Karakter penceresi (REFERANS pencere). Sifirdan, mobil; kurallar: 5_Test_Sunucusu/AKRAVON_Mobil_Arayuz_Kurallari.md
# Sayfalar ayri dosyalarda:
#   Durum      uiakravonkarakterdurum.py      Yetenekler  uiakravonkarakterbeceri.py
#   Hareketler uiakravonkarakterhareket.py    Gorevler    uiakravonkaraktergorev.py      Bonuslar uiakravonbonus.py
# Oyun (interfacemodule) bu pencereyi uiCharacter.CharacterWindow olarak olusturur ve asagidaki "OYUN ARAYUZU"
# fonksiyonlarini cagirir; adlari degismez, ici bizim.
import app
import net
import player
import chr
import localeInfo
import playerSettingModule
import ui
import wndMgr
import uiAkravonParca as P
import uiAkravonDugme
import uiAkravonPencere

GENISLIK = 620
YUKSEKLIK = 520
SAYFA_X = 170	# sag alan
SAYFA_Y = 44
SEKMELER = (("STATUS", "Durum"), ("SKILL", "Yetenekler"), ("EMOTICON", "Hareketler"), ("QUEST", "Görevler"), ("BONUS", "Bonuslar"))

PORTRE = {
	playerSettingModule.RACE_WARRIOR_M : "icon/face/warrior_m.tga",
	playerSettingModule.RACE_WARRIOR_W : "icon/face/warrior_w.tga",
	playerSettingModule.RACE_ASSASSIN_M : "icon/face/assassin_m.tga",
	playerSettingModule.RACE_ASSASSIN_W : "icon/face/assassin_w.tga",
	playerSettingModule.RACE_SURA_M : "icon/face/sura_m.tga",
	playerSettingModule.RACE_SURA_W : "icon/face/sura_w.tga",
	playerSettingModule.RACE_SHAMAN_M : "icon/face/shaman_m.tga",
	playerSettingModule.RACE_SHAMAN_W : "icon/face/shaman_w.tga",
}


def _DereceRengi(derece):
	import colorInfo
	iyi = 31 - derece	# 0..30: TITLE_RGB_GOOD_31..1, 31 normal, 32..35 EVIL_1..4
	if 0 <= derece <= 30:
		renk = getattr(colorInfo, "TITLE_RGB_GOOD_%d" % iyi, colorInfo.TITLE_RGB_NORMAL)
	elif 32 <= derece <= 35:
		renk = getattr(colorInfo, "TITLE_RGB_EVIL_%d" % (derece - 31), colorInfo.TITLE_RGB_NORMAL)
	else:
		renk = colorInfo.TITLE_RGB_NORMAL
	return ui.GenerateColor(renk[0], renk[1], renk[2])


class CharacterWindow(ui.Window):
	def __init__(self):
		ui.Window.__init__(self)
		self.AddFlag("float")
		self.SetSize(GENISLIK, YUKSEKLIK)
		self.durumAdi = "STATUS"
		self.yuklendi = False
		self.ipucuBeceri = None
		self.sekmeDugmeleri = {}
		self.sayfalar = {}
		self.durum = None
		self.beceri = None
		self.hareket = None
		self.gorev = None
		self.bonus = None
		self.kapat = None
		self.dereceDokunma = None
		# kurulum ilk acilista (oyun yuklenirken olusturulan yazi / resimler mobilde cizilmiyor)

	def __del__(self):
		ui.Window.__del__(self)

	# ---------------------------------------------------------------- kurulum
	def __Kur(self):
		P.Resim(self, 0, 0, "cw_board2.png")
		P.Yazi(self, GENISLIK // 2, 13, "Karakter", P.ALTIN, P.BASLIK, "center")
		P.Resim(self, 16, 50, "cw_hline.png")
		P.Resim(self, 150, 68, "cw_vline.png")
		self.kapat = uiAkravonDugme.IkonDugme("close", 34)
		self.kapat.SetParent(self)
		self.kapat.GorselKonumu(GENISLIK - 20 - 34, 12)
		self.kapat.OlayAyarla(self.Close)
		self.kapat.Show()

		# sol: portre, ad, lonca, derece
		P.Resim(self, 28, 68, "cw_face.png")
		self.portre = ui.ExpandedImageBox()
		self.portre.SetParent(self)
		self.portre.AddFlag("not_pick")
		self.portre.SetPosition(32, 72)
		self.portre.Show()
		self.ad = P.Yazi(self, 75, 172, "", P.ALTIN, "Marcellus:20", "center")
		self.lonca = P.Yazi(self, 75, 196, "", P.SOLUK, P.GOVDE_KUCUK, "center")
		P.Resim(self, 14, 216, "cw_aligncard.png")
		self.dereceAdi = P.Yazi(self, 24, 224, "-", P.PARLAK, "Marcellus:19")
		self.dereceDegeri = P.Yazi(self, 24, 248, "Puan : 0", P.ALTIN, P.NOT)
		derece = uiAkravonDugme.Dokunma(122, 56)
		derece.SetParent(self)
		derece.SetPosition(14, 216)
		derece.OlayAyarla(self.__DereceAc)
		derece.Show()
		self.dereceDokunma = derece

		# sol sekmeler
		for (sira, (anahtar, yazi)) in enumerate(SEKMELER):
			dugme = uiAkravonDugme.YaziDugme(122, 40, yazi, uiAkravonDugme.YAZI_SEKME)
			dugme.SetParent(self)
			dugme.SetPosition(14, 288 + sira * 44)
			dugme.OlayAyarla(self.SetState, anahtar)
			dugme.Show()
			self.sekmeDugmeleri[anahtar] = dugme

		# sayfalar (sag alan)
		import uiAkravonKarakterDurum
		import uiAkravonKarakterBeceri
		import uiAkravonKarakterHareket
		import uiAkravonKarakterGorev
		for (anahtar, yazi) in SEKMELER:
			self.sayfalar[anahtar] = P.Pencere(self, SAYFA_X, SAYFA_Y, 426, YUKSEKLIK - SAYFA_Y, False)
		self.durum = uiAkravonKarakterDurum.DurumSayfasi(self.sayfalar["STATUS"])
		self.beceri = uiAkravonKarakterBeceri.BeceriSayfasi(self.sayfalar["SKILL"], self)
		self.hareket = uiAkravonKarakterHareket.HareketSayfasi(self.sayfalar["EMOTICON"], self)
		self.gorev = uiAkravonKarakterGorev.GorevSayfasi(self.sayfalar["QUEST"])
		import uiAkravonBonus
		self.bonus = uiAkravonBonus.BonusPanel(426, 424)
		self.bonus.SetParent(self.sayfalar["BONUS"])
		self.bonus.SetPosition(0, 22)
		self.bonus.Show()
		self.yuklendi = True
		self.SetState("STATUS")

	def __DereceAc(self):
		import uiAkravonAlign
		uiAkravonAlign.Open(self)

	# ---------------------------------------------------------------- ac / kapat
	def Show(self):
		if not self.yuklendi:
			self.__Kur()
			if self.ipucuBeceri:
				self.beceri.IpucuAyarla(self.ipucuBeceri)
			self.RefreshCharacter()
		ui.Window.Show(self)
		try:
			import uiMobileHud
			self.SetPosition(uiMobileHud.SCREEN_EDGE + 8, max(8, min(96, wndMgr.GetScreenHeight() - YUKSEKLIK - 8)))
		except Exception:
			pass
		uiAkravonPencere.Acildi("karakter", self)
		self.__SayfaYenile(self.durumAdi)

	def Hide(self):
		self.__IpuclariGizle()
		ui.Window.Hide(self)
		uiAkravonPencere.Kapandi("karakter")

	def Close(self):
		self.Hide()

	def Kapat(self):
		self.Hide()

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def __IpuclariGizle(self):
		for parca in (getattr(self, "beceri", None), getattr(self, "hareket", None)):
			if parca:
				parca.IpucuGizle()

	def OnUpdate(self):
		if self.durumAdi == "QUEST" and self.gorev:
			self.gorev.SureleriGuncelle()

	# ---------------------------------------------------------------- sekme
	def SetState(self, anahtar):
		if anahtar not in self.sayfalar:
			anahtar = "STATUS"
		self.durumAdi = anahtar
		self.__IpuclariGizle()
		for (ad, sayfa) in self.sayfalar.items():
			if ad == anahtar:
				sayfa.Show()
			else:
				sayfa.Hide()
		for (ad, dugme) in self.sekmeDugmeleri.items():
			dugme.SeciliAyarla(ad == anahtar)
		self.__SayfaYenile(anahtar)

	def GetState(self):
		return self.durumAdi

	def __SayfaYenile(self, anahtar):
		if not self.yuklendi:
			return
		if anahtar == "STATUS":
			self.durum.Yenile()
		elif anahtar == "SKILL":
			self.beceri.Yenile()
		elif anahtar == "QUEST":
			self.gorev.Yenile()
		elif anahtar == "BONUS":
			self.bonus.Refresh()

	# ---------------------------------------------------------------- OYUN ARAYUZU (interfacemodule / game cagirir)
	def SetSkillToolTip(self, ipucu):
		self.ipucuBeceri = ipucu
		if self.beceri:
			self.beceri.IpucuAyarla(ipucu)

	def RefreshStatus(self):
		if self.yuklendi:
			self.durum.Yenile()
			self.RefreshAlignment()
			if self.durumAdi == "BONUS":
				self.bonus.Refresh()

	def RefreshSkill(self):
		if self.yuklendi:
			self.beceri.Yenile()

	def RefreshQuest(self):
		if self.yuklendi:
			self.gorev.Yenile()

	def RefreshAlignment(self):
		if not self.yuklendi:
			return
		(puan, derece) = player.GetAlignmentData()
		try:
			self.dereceAdi.SetText(localeInfo.TITLE_NAME_LIST[derece])
		except Exception:
			self.dereceAdi.SetText("-")
		self.dereceAdi.SetPackedFontColor(_DereceRengi(derece))
		try:
			yazi = localeInfo.NumberToMoneyString(puan).replace(" Yang", "")
		except Exception:
			yazi = str(puan)
		self.dereceDegeri.SetText("Puan : " + yazi)

	def RefreshCharacter(self):
		if not self.yuklendi:
			return
		self.ad.SetText(player.GetName())
		lonca = player.GetGuildName()
		self.lonca.SetText(lonca or "")
		self.ad.SetPosition(75, 172 if lonca else 178)
		irk = net.GetMainActorRace()
		dosya = PORTRE.get(irk)
		if dosya:
			try:
				self.portre.LoadImage(dosya)
				self.portre.SetScale(2.0, 2.0)
				self.portre.Show()
			except Exception:
				self.portre.Hide()
		else:
			self.portre.Hide()
		grup = net.GetMainActorSkillGroup()
		if grup:
			playerSettingModule.RegisterSkill(irk, grup, net.GetMainActorEmpire())
		self.beceri.GrupAdlariniKur()
		self.beceri.GrupSec(0)
		self.RefreshAlignment()

	def OnUseSkill(self, kaynak, sure):
		if self.beceri:
			self.beceri.BeceriKullanildi(kaynak, sure)

	def OnActivateSkill(self, kaynak):
		if self.beceri:
			self.beceri.BeceriAcildi(kaynak)

	def OnDeactivateSkill(self, kaynak):
		if self.beceri:
			self.beceri.BeceriKapandi(kaynak)

	def OverOutItem(self):
		self.__IpuclariGizle()

	def ActEmotion(self, no):
		if not self.yuklendi:
			self.Show()
			self.Hide()
		self.hareket.Yap(no)

	def SelectSkillGroup(self, sekme):
		if self.beceri:
			self.beceri.GrupSec(sekme)

	def Destroy(self):
		self.Hide()
		for parca in (self.durum, self.beceri, self.hareket, self.gorev):
			if parca:
				parca.Yok()
		for dugme in list(self.sekmeDugmeleri.values()) + [self.kapat, self.dereceDokunma]:
			if dugme:
				dugme.Yok()
		self.sekmeDugmeleri = {}
		self.durum = self.beceri = self.hareket = self.gorev = None
		if self.bonus:
			self.bonus.Hide()
		self.bonus = None
		self.yuklendi = False
