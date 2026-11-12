# -*- coding: utf-8 -*-
# AKRAVON ortak dokunmatik dugmeler (kurallar: 5_Test_Sunucusu/AKRAVON_Mobil_Arayuz_Kurallari.md)
# Eski PC dugmeleri (ui.Button / ui.RadioButton) kullanilmaz. Gorseller: skin/build_akravon_theme.py (2x, oyunda 0.5).
#   YaziDugme : yazili dugme / sekme. normal (lacivert) - secili (altin, koyu yazi) - basili (kahve, yazi 1 px asagi)
#   IkonDugme : arti / eksi / carpi / sol ok / sag ok. Gorsel 32-34 px, dokunma alani en az 44 px
# Dokunma: tek dokunus = tek is (dokunma + fare olayi ayni anda gelirse ikincisi yutulur).
# Basili gorunum: parmak basiliyken ve biraktiktan sonra BASILI_SURE saniye (kisa dokunusta da "iceri girip cikma").
# Gorseller onceden yuklenir; basista yalniz goster / gizle (basista gorsel yuklemek dokunusu kaybettirir).
from __future__ import division
import app
import ui
import uiMobileStyle

GORSEL = "d:/ymir work/ui/akravon_legacy/"
KREM = uiMobileStyle.Color(232, 221, 197)
KOYU = uiMobileStyle.Color(40, 24, 8)
YAZI_SEKME = "Marcellus:20"	# sol sekmeler
YAZI_DUGME = "Marcellus:20"	# dugme / alt sekme
YAZI_DAR = "Marcellus:18"	# dar dugme (84 px)
DOKUNMA = 44
BASILI_SURE = 0.12
CIFT_ENGEL = 0.35


class _Dokunmatik(ui.Window):
	def __init__(self, genislik, yukseklik):
		ui.Window.__init__(self)
		self.SetSize(genislik, yukseklik)
		ui.EnableSingleTap(self)
		self.SetTouchTapEvent(ui.__mem_func__(self.__Dokundu))
		self.olay = None
		self.olayArgumanlari = ()
		self.basili = False
		self.basiliBitis = 0.0
		self.sonCalisma = -10.0
		self.etkin = True

	def __del__(self):
		ui.Window.__del__(self)

	def Yok(self):
		self.olay = None
		self.olayArgumanlari = ()
		self.ClearTouchEvents() if hasattr(self, "ClearTouchEvents") else None
		self.Hide()

	def OlayAyarla(self, olay, *argumanlar):
		self.olay = olay
		self.olayArgumanlari = argumanlar

	def EtkinAyarla(self, etkin):
		self.etkin = bool(etkin)
		self.Gorunum()

	def BasiliGorunuyor(self):
		return self.basili or app.GetTime() < self.basiliBitis

	def __Calistir(self):
		simdi = app.GetTime()
		if simdi - self.sonCalisma < CIFT_ENGEL:
			return
		self.sonCalisma = simdi
		self.basiliBitis = simdi + BASILI_SURE
		self.Gorunum()
		if self.etkin and self.olay:
			self.olay(*self.olayArgumanlari)

	def __Dokundu(self, x, y):
		self.__Calistir()

	def OnMouseLeftButtonDown(self):
		if self.etkin:
			self.basili = True
			self.Gorunum()
		return True

	def OnMouseLeftButtonUp(self):
		basiliydi = self.basili
		self.basili = False
		if basiliydi:
			self.basiliBitis = app.GetTime() + BASILI_SURE
		self.Gorunum()
		if basiliydi and self.IsIn():
			self.__Calistir()
		return True

	def OnMouseOverOut(self):
		if self.basili:
			self.basili = False
			self.Gorunum()

	def OnUpdate(self):
		if self.basiliBitis and not self.basili and app.GetTime() >= self.basiliBitis:
			self.basiliBitis = 0.0
			self.Gorunum()

	def Gorunum(self):
		pass


def _Resim(ebeveyn, dosya, olcek, x = 0, y = 0, gorunur = False):
	resim = ui.ExpandedImageBox()
	resim.SetParent(ebeveyn)
	resim.AddFlag("not_pick")
	resim.LoadImage(dosya)
	resim.SetScale(olcek, olcek)
	resim.SetPosition(x, y)
	if gorunur:
		resim.Show()
	else:
		resim.Hide()
	return resim


class YaziDugme(_Dokunmatik):
	# genislik x yukseklik icin akr_btn_<g>x<y>_* gorselleri uretilmis olmali (build_akravon_theme.BUTTON_SIZES)
	def __init__(self, genislik, yukseklik, yazi = "", yaziTipi = YAZI_DUGME):
		_Dokunmatik.__init__(self, genislik, yukseklik)
		self.secili = False
		self.durum = "normal"
		self.resimler = {}
		for durum in ("normal", "selected", "pressed"):
			self.resimler[durum] = _Resim(self, GORSEL + "akr_btn_%dx%d_%s.png" % (genislik, yukseklik, durum), 0.5, gorunur = durum == "normal")
		boyut = int(yaziTipi.split(":")[1])
		self.yaziY = max(0, (yukseklik - boyut) // 2 - 1)
		self.etiket = uiMobileStyle.CreateText(self, "", genislik // 2, self.yaziY, KREM, yaziTipi, "center")
		self.YaziAyarla(yazi)

	def Yok(self):
		_Dokunmatik.Yok(self)
		self.resimler = {}
		self.etiket = None

	def YaziAyarla(self, yazi):
		self.etiket.SetText(uiMobileStyle.TrUpper(yazi or ""))

	def SeciliAyarla(self, secili):
		if bool(secili) != self.secili:
			self.secili = bool(secili)
			self.Gorunum()

	def Gorunum(self):
		if not self.resimler or self.etiket is None:
			return
		basili = self.BasiliGorunuyor()
		durum = "selected" if (basili or self.secili) else "normal"	# basili = altin (kahve kullanilmaz)
		if durum != self.durum:
			self.resimler[self.durum].Hide()
			self.resimler[durum].Show()
			self.durum = durum
		self.etiket.SetPackedFontColor(KOYU if durum == "selected" else KREM)
		self.etiket.SetPosition(self.GetWidth() // 2, self.yaziY + (1 if basili else 0))


class IkonDugme(_Dokunmatik):
	# tur: plus / minus / close / arrow_l / arrow_r ; boyut: gorunen gorsel ; dokunma: dokunma kutusu (ortali)
	def __init__(self, tur, boyut = 32, dokunma = DOKUNMA):
		dokunma = max(boyut, dokunma)
		_Dokunmatik.__init__(self, dokunma, dokunma)
		self.durum = "normal"
		kaynak = 68.0 if tur in ("close", "arrow_l", "arrow_r") else 64.0
		kenar = (dokunma - boyut) // 2
		self.resimler = {}
		for durum in ("normal", "pressed"):
			self.resimler[durum] = _Resim(self, GORSEL + "akr_%s_%s.png" % (tur, durum), boyut / kaynak, kenar, kenar, durum == "normal")
		self.boyut = boyut

	def Yok(self):
		_Dokunmatik.Yok(self)
		self.resimler = {}

	def GorselKonumu(self, x, y):
		# gorselin sol ust kosesini (x, y) yapar; dokunma kutusu etrafinda kalir
		kenar = (self.GetWidth() - self.boyut) // 2
		self.SetPosition(x - kenar, y - kenar)

	def Gorunum(self):
		if not self.resimler:
			return
		durum = "pressed" if self.BasiliGorunuyor() else "normal"
		if durum != self.durum:
			self.resimler[self.durum].Hide()
			self.resimler[durum].Show()
			self.durum = durum


class Dokunma(_Dokunmatik):
	# gorselsiz dokunma alani (kart / satir dokununca acilan seyler icin)
	def __init__(self, genislik, yukseklik):
		_Dokunmatik.__init__(self, genislik, yukseklik)
