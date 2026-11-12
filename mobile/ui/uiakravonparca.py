# -*- coding: utf-8 -*-
# AKRAVON ortak arayuz parcalari: renkler, yazi tipleri, yazi / resim olusturma (tum AKRAVON pencereleri kullanir).
# Kurallar: 5_Test_Sunucusu/AKRAVON_Mobil_Arayuz_Kurallari.md
import ui
import uiMobileStyle

GORSEL = "d:/ymir work/ui/akravon_legacy/"

# renkler
ALTIN = uiMobileStyle.Color(227, 194, 128)	# baslik / bolum basligi / deger vurgusu
SOLUK = uiMobileStyle.Color(161, 150, 130)	# aciklama / etiket
PARLAK = uiMobileStyle.Color(245, 240, 224)	# deger / ad
KREM = uiMobileStyle.Color(232, 221, 197)

# yazi tipleri
BASLIK = "Marcellus:22"	# pencere basligi
BOLUM = "Marcellus:20"	# bolum basligi (Karakter Durumu, Savas Degerleri ...)
DEGER_BUYUK = "Marcellus:24"
AD_BUYUK = "Marcellus:22"
GOVDE = "Lato:16"
GOVDE_BUYUK = "Lato:17"
GOVDE_KUCUK = "Lato:15"
NOT = "Lato:14"


def Yazi(ebeveyn, x, y, metin = "", renk = PARLAK, tip = GOVDE, hiza = "left"):
	return uiMobileStyle.CreateText(ebeveyn, metin, x, y, renk, tip, hiza)


def Resim(ebeveyn, x, y, dosya, genis = False):
	resim = ui.ExpandedImageBox()	# mobil motorda duz ImageBox cizilmiyor
	resim.SetParent(ebeveyn)
	resim.AddFlag("not_pick")
	resim.LoadImage(dosya if dosya.startswith("d:") or "/" in dosya else GORSEL + dosya)
	resim.SetScale(1.0, 1.0)	# olcek verilmeyen genis resim cizilmiyor
	resim.SetPosition(x, y)
	resim.Show()
	return resim


def Pencere(ebeveyn, x, y, genislik, yukseklik, secilmez = True):
	pencere = ui.Window()
	pencere.SetParent(ebeveyn)
	if secilmez:
		pencere.AddFlag("not_pick")
	pencere.SetPosition(x, y)
	pencere.SetSize(genislik, yukseklik)
	pencere.Show()
	return pencere


def KademeYazisi(kademe, seviye):
	# beceri kademesi: Perfect / Grand Master N / Master N / Level N
	if kademe >= 3:
		return ("Perfect", 0xFFFF7770)
	if kademe == 2:
		return ("Grand Master %d" % seviye, 0xFFFFD16A)
	if kademe == 1:
		return ("Master %d" % seviye, 0xFF85D8FF)
	if seviye > 0:
		return ("Level %d" % seviye, 0xFFE8E2D2)
	return ("", 0xFFE8E2D2)
