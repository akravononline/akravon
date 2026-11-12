# -*- coding: utf-8 -*-
# AKRAVON Karakter penceresi - DURUM sayfasi (statlar + savas degerleri). Pencere: uiakravonkarakter.py
# Stat degerleri esya bonusu olmadan gosterilir (orijinal 2008: VIT / INT / STR / DEX en fazla 90).
import net
import player
import item
import constInfo
import chat
import uiAkravonParca as P
import uiAkravonDugme

STATLAR = (
	# (anahtar, ad, aciklama, puan turu, komut)
	("HTH", "VIT", "Can ve savunma", player.HT, "/stat ht"),
	("INT", "INT", "Büyü ve SP", player.IQ, "/stat iq"),
	("STR", "STR", "Saldırı gücü", player.ST, "/stat st"),
	("DEX", "DEX", "İsabet, kaçınma", player.DX, "/stat dx"),
)

SATIR_Y = 50	# ilk stat satiri
SATIR_ADIM = 54
SAVAS_Y = 288	# Savas Degerleri bolumu
SUTUN = 219	# ikinci sutun
SUTUN_GENIS = 207


def _BonusluDegil(puanTuru):
	# sunucu stati esya bonuslu yollar; takili esyalarin STR / DEX / CON / INT bonuslari dusulur
	toplam = player.GetStatus(puanTuru)
	tur = {player.ST : item.APPLY_STR, player.DX : item.APPLY_DEX, player.HT : item.APPLY_CON, player.IQ : item.APPLY_INT}.get(puanTuru)
	bonus = 0
	try:
		for yer in xrange(32):
			yuva = player.EQUIPMENT_SLOT_START + yer
			vnum = player.GetItemIndex(yuva)
			if vnum == 0:
				continue
			item.SelectItem(vnum)
			for i in xrange(item.ITEM_APPLY_MAX_NUM):
				(etkiTuru, etkiDegeri) = item.GetAffect(i)
				if etkiTuru == tur:
					bonus += etkiDegeri
			for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
				(efsunTuru, efsunDegeri) = player.GetItemAttribute(player.INVENTORY, yuva, i)
				if efsunTuru == tur:
					bonus += efsunDegeri
	except Exception:
		bonus = 0
	return max(0, toplam - bonus)


def _Aralik(en, ust):
	if en == ust:
		return "%d" % en
	return "%d-%d" % (en, ust)


class DurumSayfasi(object):
	def __init__(self, sayfa):
		self.sayfa = sayfa
		P.Yazi(sayfa, 0, 22, "Karakter Durumu", P.ALTIN, P.BOLUM)
		self.kalanEtiket = P.Yazi(sayfa, 394, 24, "Kalan Puan:", P.SOLUK, P.GOVDE_KUCUK, "right")
		self.kalanDeger = P.Yazi(sayfa, 426, 24, "0", P.ALTIN, P.NOT, "right")
		self.degerler = {}
		self.artilar = {}
		for (sira, (anahtar, ad, aciklama, puanTuru, komut)) in enumerate(STATLAR):
			y = SATIR_Y + sira * SATIR_ADIM
			satir = P.Resim(sayfa, 0, y, "cw_statrow.png")
			P.Yazi(satir, 16, 5, ad, P.ALTIN, P.AD_BUYUK)
			P.Yazi(satir, 16, 29, aciklama, P.SOLUK, P.GOVDE)
			self.degerler[anahtar] = P.Yazi(satir, 352, 11, "0", P.PARLAK, P.DEGER_BUYUK, "center")
			arti = uiAkravonDugme.IkonDugme("plus", 32)
			arti.SetParent(sayfa)
			arti.GorselKonumu(381, y + 9)
			arti.OlayAyarla(self.__StatArttir, komut)
			self.artilar[anahtar] = arti
		P.Yazi(sayfa, 0, SAVAS_Y, "Savaş Değerleri", P.ALTIN, P.BOLUM)
		self.savas = {}
		for (sira, (anahtar, ad)) in enumerate((("ATT", "Saldırı"), ("DEF", "Savunma"), ("MATT", "Büyü Hasarı"), ("MDEF", "Büyü Savunması"),
				("MSPD", "Hareket Hızı"), ("ASPD", "Saldırı Hızı"), ("CSPD", "Büyü Hızı"), ("ER", "Savuşturma"))):
			x = (sira % 2) * SUTUN
			y = SAVAS_Y + 32 + (sira // 2) * 30
			P.Yazi(sayfa, x, y + 1, ad, P.SOLUK, P.GOVDE_BUYUK)
			self.savas[anahtar] = P.Yazi(sayfa, x + SUTUN_GENIS, y + 1, "0", P.PARLAK, "Lato:18", "right")
			P.Resim(sayfa, x, y + 26, "cw_line.png")

	def __StatArttir(self, komut):
		if player.GetStatus(player.STAT) <= 0:
			chat.AppendChat(chat.CHAT_TYPE_INFO, "Dağıtılacak statü puanı yok.")
			return
		net.SendChatPacket(komut)

	def Yenile(self):
		for (anahtar, ad, aciklama, puanTuru, komut) in STATLAR:
			self.degerler[anahtar].SetText(str(_BonusluDegil(puanTuru)))
		kalan = player.GetStatus(player.STAT)
		self.kalanDeger.SetText(str(max(0, kalan)))
		for arti in self.artilar.values():
			if kalan > 0:
				arti.Show()
			else:
				arti.Hide()
		if kalan > 0:
			self.kalanEtiket.Show()
			self.kalanDeger.Show()
		else:
			self.kalanEtiket.Hide()
			self.kalanDeger.Hide()
		saldiriEk = player.GetStatus(player.ATT_BONUS) + player.GetStatus(player.ATTACKER_BONUS)
		self.savas["ATT"].SetText(_Aralik(player.GetStatus(player.ATT_MIN) + saldiriEk, player.GetStatus(player.ATT_MAX) + saldiriEk))
		savunma = player.GetStatus(player.DEF_GRADE)
		if constInfo.ADD_DEF_BONUS_ENABLE:
			savunma += player.GetStatus(player.DEF_BONUS)
		self.savas["DEF"].SetText("%d" % savunma)
		buyu = player.GetStatus(player.MAG_ATT)
		self.savas["MATT"].SetText(_Aralik(buyu + player.GetStatus(player.MIN_MAGIC_WEP), buyu + player.GetStatus(player.MAX_MAGIC_WEP)))
		self.savas["MDEF"].SetText(str(player.GetStatus(player.MAG_DEF)))
		self.savas["MSPD"].SetText(str(player.GetStatus(player.MOVING_SPEED)))
		self.savas["ASPD"].SetText(str(player.GetStatus(player.ATT_SPEED)))
		self.savas["CSPD"].SetText(str(player.GetStatus(player.CASTING_SPEED)))
		self.savas["ER"].SetText(str(player.GetStatus(player.EVADE_RATE)))

	def Yok(self):
		for arti in self.artilar.values():
			arti.Yok()
		self.artilar = {}
