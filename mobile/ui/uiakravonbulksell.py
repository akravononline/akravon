# -*- coding: utf-8 -*-
# AKRAVON: NPC dukkaninda toplu satis.
#   SAT dugmesi -> toplu secim modu: envanterde esyalara dokun (yesil parlar), altta "Secilen / Toplam" yazar.
#   SAT dugmesine tekrar basinca onay penceresi: toplam yang; ozellikli esya varsa ayrica uyari.
#   Sunucuya yeni paket yok: secilen her esya icin mevcut tekli satis paketi sirayla gonderilir.
import app
import chat
import item
import net
import player
import snd
import ui
import localeInfo
import uiMobileStyle

active = False
selected = {}	# genel envanter yuvasi -> (vnum, adet)
lastToggle = {}	# ayni dokunus iki olay uretir: ikincisi yutulur
shopWindow = None
summaryText = None
originalTake = None
GREEN = (0.40, 0.31, 0.07, 1.0)	# aktif = mavi (tasarim dili)


def _Chase(inventory, locals):
	import uiAkravonSlotFx
	uiAkravonSlotFx.Refresh(inventory)


def _ItemPrice(vnum, count):
	item.SelectItem(vnum)
	price = item.GetISellItemPrice()
	if item.Is1GoldItem():
		return count / price / 5
	return price * max(1, count) / 5


def _IsSpecial(slot):
	# ozellikli: oyunun kendi "degerli esya" kontrolu, bonus (efsun) veya tas takili
	try:
		if player.IsValuableItem(player.SlotTypeToInvenType(player.SLOT_TYPE_INVENTORY), slot):
			return True
	except Exception:
		pass
	try:
		for index in range(player.ATTRIBUTE_SLOT_MAX_NUM):
			if player.GetItemAttribute(slot, index)[0] != 0:
				return True
	except Exception:
		pass
	return False


def _Totals():
	total = 0
	special = 0
	for (slot, (vnum, count)) in selected.items():
		total += _ItemPrice(vnum, count)
		if _IsSpecial(slot):
			special += 1
	return (total, special)


def _Inventory():
	try:
		import constInfo
		interface = constInfo.GetInterfaceInstance()
		return interface.wndInventory
	except Exception:
		return None


SUMMARY_FONTS = (21, 19, 17, 15, 13)
SUMMARY_WIDTH = 262


def _SetSummary(count, total):
	# ozet satiri tahtaya sigmali: buyuk tutarda yazi tipi kucultulur; yine sigmazsa "Yang" kisaltilir
	money = localeInfo.NumberToMoneyString(total)
	plain = "Se\xe7ilen: %d   Toplam: %s" % (count, money)
	colored = "Se\xe7ilen: |cffFFD24A%d|r   Toplam: |cffFFD24A%s|r" % (count, money)
	for size in SUMMARY_FONTS:
		font = "Marcellus:%d" % size
		if uiMobileStyle.TextWidth(plain, font) <= SUMMARY_WIDTH:
			break
	else:
		short = money.replace("Yang", "Y")
		plain = "Se\xe7ilen: %d   Toplam: %s" % (count, short)
		colored = "Se\xe7ilen: |cffFFD24A%d|r   Toplam: |cffFFD24A%s|r" % (count, short)
		font = "Marcellus:%d" % SUMMARY_FONTS[-1]
	summaryText.SetFontName(font)
	summaryText.SetText(colored)


def _Refresh():
	# secilenleri yesil parlat, ozet satirini guncelle
	inventory = _Inventory()
	if inventory:
		try:
			wndItem = inventory.wndItem
			page = inventory.inventoryPageIndex
			size = player.INVENTORY_PAGE_SIZE
			locals = {}
			for local in range(size):
				slot = local + size * page
				if active and slot in selected:
					wndItem.ActivateSlot(local, GREEN[0], GREEN[1], GREEN[2], GREEN[3])
					item.SelectItem(selected[slot][0])
					locals[local] = max(1, min(3, item.GetItemSize()[1]))
			_Chase(inventory, locals)
		except Exception:
			pass
	if summaryText:
		if not active:
			summaryText.SetText("")
		elif not selected:
			summaryText.SetText("Satmak istedi\xf0in e\xfeyalara dokun")
		else:
			(total, special) = _Totals()
			_SetSummary(len(selected), total)
	if shopWindow:
		try:
			label = "SAT" if not selected else "SAT (%d)" % len(selected)
			shopWindow.GetChild("SellButton").SetText(label)
		except Exception:
			pass


def _Leave():
	global active
	active = False
	selected.clear()
	_Refresh()
	inventory = _Inventory()
	if inventory:
		try:
			inventory.RefreshBagSlotWindow()
		except Exception:
			pass


def _Toggle(slot):
	now = app.GetTime()
	if now - lastToggle.get(slot, -10.0) < 0.35:
		return
	lastToggle[slot] = now
	vnum = player.GetItemIndex(slot)
	if vnum == 0:
		return
	item.SelectItem(vnum)
	if item.IsAntiFlag(item.ANTIFLAG_SELL):
		chat.AppendChat(chat.CHAT_TYPE_INFO, "Bu e\xfeya sat\xfdlamaz.")
		return
	if slot in selected:
		del selected[slot]
		inventory = _Inventory()
		if inventory:
			try:
				inventory.wndItem.DeactivateSlot(slot - player.INVENTORY_PAGE_SIZE * inventory.inventoryPageIndex)
			except Exception:
				pass
	else:
		selected[slot] = (vnum, max(1, player.GetItemCount(slot)))
	_Refresh()


def _Take(playerModule, slot):
	# uiMobileChat.TakeItemSlot yerine: toplu secim aciksa dokunus secimi degistirir, esya kullanilmaz
	global active
	if active:
		try:
			import shop
			if not shopWindow or not shopWindow.IsShow():
				_Leave()
				return originalTake(playerModule, slot)
		except Exception:
			_Leave()
			return originalTake(playerModule, slot)
		if slot < player.EQUIPMENT_SLOT_START:
			_Toggle(slot)
			return True
	return originalTake(playerModule, slot)


def _Confirm():
	import uiCommon
	(total, special) = _Totals()
	dialog = uiCommon.QuestionDialog2()
	dialog.SetText1("|cffFFD24A%d|r e\xfeya sat\xfdlacak. Toplam: |cffFFD24A%s|r" % (len(selected), localeInfo.NumberToMoneyString(total)))
	if special:
		dialog.SetText2("|cffFFD24A\xd6zellikli e\xfeya var (%d adet)!|r Satmak istiyor musun?" % special)
	else:
		dialog.SetText2("Satmak istiyor musun?")
	dialog.SetAcceptEvent(lambda: _Sell(dialog))
	dialog.SetCancelEvent(lambda: dialog.Close())
	dialog.Open()
	global confirmDialog
	confirmDialog = dialog


confirmDialog = None


def _Sell(dialog):
	dialog.Close()
	itemType = player.SlotTypeToInvenType(player.SLOT_TYPE_INVENTORY)
	for (slot, (vnum, count)) in list(selected.items()):
		if player.GetItemIndex(slot) != vnum:
			continue
		net.SendShopSellPacketNew(slot, count, itemType)
	snd.PlaySound("sound/ui/money.wav")
	selected.clear()
	_Refresh()
	inventory = _Inventory()
	if inventory:
		try:
			inventory.RefreshBagSlotWindow()
		except Exception:
			pass
	try:
		shopWindow.GetChild("SellButton").Down()
	except Exception:
		pass


def _OnSellDown(window):
	global active
	active = True
	selected.clear()
	try:
		window.GetChild("BuyButton").SetUp()
	except Exception:
		pass
	_Refresh()


def _OnSellUp(window):
	if selected:
		_Confirm()
		try:
			window.GetChild("SellButton").Down()	# onaydan sonra da mod acik kalir
		except Exception:
			pass
		return
	_Leave()
	app.SetCursor(app.NORMAL)


def _OnBuyDown(window):
	_Leave()
	window.OnBuy()


def _Bind(window):
	window.GetChild("SellButton").SetToggleDownEvent(lambda: _OnSellDown(window))
	window.GetChild("SellButton").SetToggleUpEvent(lambda: _OnSellUp(window))
	window.GetChild("BuyButton").SetToggleDownEvent(lambda: _OnBuyDown(window))


def Attach(window):
	global shopWindow, summaryText, originalTake
	shopWindow = window
	board = window.GetChild("board")
	if summaryText is None or getattr(summaryText, "akrOwner", None) is not board:
		summaryText = uiMobileStyle.CreateText(board, "", board.GetWidth() // 2, SUMMARY_Y, uiMobileStyle.Color(250, 236, 210), "Marcellus:21", "center")
		summaryText.akrOwner = board
	import uiMobileChat
	if originalTake is None:
		originalTake = uiMobileChat.TakeItemSlot
		uiMobileChat.TakeItemSlot = _Take
	_Bind(window)
	cls = window.__class__
	if not getattr(cls, "akrBulkPatched", False):
		cls.akrBulkPatched = True
		originalOpen = cls.Open

		def Open(self, *args):
			result = originalOpen(self, *args)
			try:
				_Bind(self)	# oyun olaylari LoadDialog sonunda baglar: bizimkini ondan sonra bagla
				_Leave()
			except Exception:
				import dbg
				import traceback
				dbg.TraceError("uiAkravonBulkSell.Open: " + traceback.format_exc())
			return result
		cls.Open = Open
	inventory = _Inventory()
	if inventory and not getattr(inventory, "akrBulkHook", False):
		inventory.akrBulkHook = True
		original = inventory.RefreshBagSlotWindow

		def RefreshBagSlotWindow(*args):
			result = original(*args)
			if active:
				_Refresh()
			return result
		inventory.RefreshBagSlotWindow = RefreshBagSlotWindow


SUMMARY_Y = 468
