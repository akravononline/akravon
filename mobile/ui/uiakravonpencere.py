# -*- coding: utf-8 -*-
# AKRAVON pencere yoneticisi (kurallar: 5_Test_Sunucusu/AKRAVON_Mobil_Arayuz_Kurallari.md)
#   - Ayni anda TEK pencere. Yeni pencere acilinca, onunla BAGLI olmayan acik pencereler kapanir.
#   - Bagli pencereler (BAGLANTI) birlikte acik kalabilir (ornek: envanter + ticaret).
#   - Pencere acikken savas HUD'u gizlenir (AcikVarMi -> uiMobileHudController).
#   - Karakter yuruyunce "yuruyunce kapanir" isaretli pencereler kapanir (Guncelle her karede cagrilir).
# Her AKRAVON penceresi kendi Ac / Kapat'inda Acildi(ad, pencere) / Kapandi(ad) cagirir. Tahmin (her karede tarama) yok.
import player
import app

# ad -> birlikte acik kalabilecegi pencereler (iki yonlu sayilir)
BAGLANTI = {
	"envanter" : ("ticaret", "dukkan", "depo", "nesne_market", "yukseltme", "ozel_envanter", "ejderha_tasi"),
}

# sunucu oturumu olan pencereler: yuruyunce biz kapatmayiz (kendi kurallari var: ticaret mesafe, dukkan NPC uzakligi)
OTURUM = ("ticaret", "dukkan", "depo", "nesne_market", "yukseltme", "envanter", "ozel_envanter", "ejderha_tasi")	# envanter: yururken iksir icilebilsin (karar 2026-11-12)

YURUME_ESIGI = 150	# oyun birimi (1.5 m): bu kadar yer degisince "yurudu" sayilir

acik = {}	# ad -> pencere
sonKonum = None
depoKontrol = 0.0	# depo acilinca bu zamana kadar envanter acik / ekipman kapali tutulur


def _Bagli(a, b):
	return b in BAGLANTI.get(a, ()) or a in BAGLANTI.get(b, ())


def _Kapat(ad, pencere):
	try:
		if hasattr(pencere, "Kapat"):
			pencere.Kapat()
		elif hasattr(pencere, "Close"):
			pencere.Close()
		else:
			pencere.Hide()
	except Exception:
		import dbg
		import traceback
		dbg.TraceError("uiAkravonPencere kapatma (%s): %s" % (ad, traceback.format_exc()))
	acik.pop(ad, None)


def EkranaSigdir(pencere, pay = 8):
	# AKRAVON: tek yerden - acilan her pencere ekranin icinde kalir (sag / alt kenardan tasarsa iceri cekilir)
	try:
		import wndMgr
		(sw, sh) = (wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())
		(w, h) = (pencere.GetWidth(), pencere.GetHeight())
		(x, y) = pencere.GetGlobalPosition()
		nx = max(pay, min(x, sw - w - pay)) if w + pay * 2 <= sw else max(0, (sw - w) // 2)
		ny = max(pay, min(y, sh - h - pay)) if h + pay * 2 <= sh else max(0, (sh - h) // 2)
		if (nx, ny) != (x, y) and not pencere.GetParentProxy():
			pencere.SetPosition(nx, ny)
	except Exception:
		pass


def Acildi(ad, pencere):
	# yeni pencere: bagli olmayan acik pencereler kapanir
	global sonKonum
	EkranaSigdir(pencere)
	for (digerAd, digerPencere) in list(acik.items()):
		if digerAd != ad and not _Bagli(ad, digerAd):
			_Kapat(digerAd, digerPencere)
	acik[ad] = pencere
	sonKonum = _Konum()


def Kapandi(ad):
	acik.pop(ad, None)


def AcikMi(ad):
	return ad in acik


def AcikVarMi():
	return bool(acik)


def HepsiniKapat():
	for (ad, pencere) in list(acik.items()):
		_Kapat(ad, pencere)


def _Konum():
	try:
		(x, y, z) = player.GetMainCharacterPosition()
		return (x, y)
	except Exception:
		return None


def _DepoYaninda():
	# depo / nesne market acildiktan kisa sure sonra: envanter acik, ekipman kapali (baska bir kod kapatsa bile geri getirir)
	if not arayuz:
		return
	if "depo" not in acik and "nesne_market" not in acik and "ticaret" not in acik:
		return
	envanter = getattr(arayuz[0], "wndInventory", None)
	if not envanter:
		return
	if not envanter.IsShow():
		envanter.Show()
	try:
		envanter.akravonSkin.EkipmaniKapat()
	except Exception:
		pass


def Guncelle():
	# her karede (HUD guncellemesinden): karakter yuruduyse oturumsuz pencereler kapanir
	global sonKonum
	if depoKontrol and app.GetTime() < depoKontrol:
		_DepoYaninda()
	if not acik:
		sonKonum = None
		return
	simdi = _Konum()
	if simdi is None:
		return
	if sonKonum is None:
		sonKonum = simdi
		return
	if abs(simdi[0] - sonKonum[0]) + abs(simdi[1] - sonKonum[1]) < YURUME_ESIGI:
		return
	sonKonum = simdi
	for (ad, pencere) in list(acik.items()):
		if ad not in OTURUM:
			_Kapat(ad, pencere)


# oyunun hazir pencereleri (interfacemodule): Show / Hide yoneticiye bildirilir
HAZIR = (("wndInventory", "envanter"), ("wndSafebox", "depo"), ("wndMall", "nesne_market"), ("dlgExchange", "ticaret"),
	("dlgShop", "dukkan"), ("wndSpecialStorage", "ozel_envanter"), ("wndDragonSoul", "ejderha_tasi"), ("wndMessenger", "mesajlar"), ("wndGuild", "lonca"), ("dlgRefineNew", "yukseltme"))


def _Bagla1(pencere, ad):
	eskiShow = pencere.Show
	eskiHide = pencere.Hide

	def Show(*args, **kwargs):
		sonuc = eskiShow(*args, **kwargs)
		Acildi(ad, pencere)
		if ad in ("depo", "nesne_market", "ticaret"):
			global depoKontrol
			depoKontrol = app.GetTime() + 1.5
		if ad in ("depo", "nesne_market", "ticaret") and arayuz:	# depo acilinca envanter yaninda acilir (esya tasimak icin)
			envanter = getattr(arayuz[0], "wndInventory", None)
			if envanter and not envanter.IsShow():
				envanter.Show()
			try:
				envanter.akravonSkin.EkipmaniKapat()	# depo yaninda yalniz canta
			except Exception:
				pass
		return sonuc

	def Hide(*args, **kwargs):
		Kapandi(ad)
		return eskiHide(*args, **kwargs)
	pencere.Show = Show
	pencere.Hide = Hide


arayuz = []


def Bagla(interface):
	if not arayuz:
		arayuz.append(interface)
	for (alan, ad) in HAZIR:
		pencere = getattr(interface, alan, None)
		if pencere is None or getattr(pencere, "akrPencereBagli", False):
			continue
		pencere.akrPencereBagli = True
		_Bagla1(pencere, ad)
