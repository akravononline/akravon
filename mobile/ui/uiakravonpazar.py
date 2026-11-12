# -*- coding: utf-8 -*-
# AKRAVON: pazar (ikashop) pencereleri - sifirdan, bizim tasarim (Depo Sifresi dili), mobil dokunma boyutlari
# gorseller: skin/build_akravon_ikashop.py (pz_*.png). Sunucu komutlari uiikashop.py icindeki ikashop.* cagrilari aynen kalir.
from __future__ import division
import item
import ui
import uiMobileStyle
import localeInfo
import wndMgr

IMG = "d:/ymir work/ui/akravon_legacy/"
DARK = uiMobileStyle.Color(40, 24, 8)	# altin dugmede yazi
GOLD = uiMobileStyle.Color(222, 184, 110)
TEXT = uiMobileStyle.Color(250, 236, 210)
YELLOW = uiMobileStyle.Color(255, 214, 90)
SOFT = uiMobileStyle.Color(190, 178, 152)
LINE = uiMobileStyle.Color(96, 74, 42)
W = 372


class PazarButton(uiMobileStyle.Pressable):
	# dokunmaya uygun buyuk dugme: gorsel (normal/secili/basili) + ortada yazi
	def __init__(self, parent, x, y, width, height, prefix, text, event = None, font = "Marcellus:20", *args):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.SetParent(parent)
		self.SetPosition(x, y)
		self.prefix = prefix
		self.selected = False
		self.image = ui.ExpandedImageBox()
		self.image.SetParent(self)
		self.image.AddFlag("not_pick")
		self.image.LoadImage(IMG + prefix % 1)
		self.image.Show()
		self.label = uiMobileStyle.CreateText(self, text, width // 2, max(0, (height - 24) // 2), TEXT, font, "center")
		if event:
			self.SetEvent(event, *args)
		self.Show()

	def RefreshVisual(self):
		if not getattr(self, "image", None):
			return
		if self.selected:
			state = 2
		else:
			state = 3 if (self.GetVisualState() == "down" or getattr(self, "goldUntil", 0)) else 1
		self.image.LoadImage(IMG + self.prefix % state)
		if getattr(self, "label", None):
			self.label.SetPackedFontColor(DARK if state != 1 else (YELLOW if self.selected else TEXT))

	def Run(self):
		# dokunmatik: basili durum gelmese de kisa sure altin
		import app
		self.goldUntil = app.GetTime() + 0.15
		self.RefreshVisual()
		uiMobileStyle.Pressable.Run(self)

	def OnUpdate(self):
		until = getattr(self, "goldUntil", 0)
		if until:
			import app
			if app.GetTime() >= until:
				self.goldUntil = 0
				self.RefreshVisual()

	# eski ui.Button uyumu
	def Enable(self):
		self.selected = False
		self.RefreshVisual()
		self.label.SetPackedFontColor(TEXT)

	def Disable(self):
		self.selected = True
		self.RefreshVisual()
		self.label.SetPackedFontColor(YELLOW)

	def SetText(self, text):
		self.label.SetText(text)

	def SetToolTipText(self, text):
		pass


class PazarTab(uiMobileStyle.Pressable):
	# envanter sekmesi gibi: secili olan cukur kapsul + acik yazi
	def __init__(self, parent, x, y, width, height, text, event, *args):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.SetParent(parent)
		self.SetPosition(x, y)
		self.on = ui.ExpandedImageBox()
		self.on.SetParent(self)
		self.on.AddFlag("not_pick")
		self.on.LoadImage(IMG + "pz_tabs_on.png")
		self.on.Hide()
		self.label = uiMobileStyle.CreateText(self, text, width // 2, 11, SOFT, "Marcellus:20", "center")
		self.SetEvent(event, *args)
		self.Show()

	def Enable(self):
		self.on.Hide()
		self.label.SetPackedFontColor(SOFT)

	def Disable(self):
		self.on.Show()	# secili sayfa: altin, koyu yazi
		self.label.SetPackedFontColor(DARK)

	def SetText(self, text):
		self.label.SetText(text)

	def SetToolTipText(self, text):
		pass

	def RefreshVisual(self):
		pass


class PazarClose(uiMobileStyle.Pressable):
	# kare kapat dugmesi (envanterle ayni dil) - 52x52 dokunma alani
	def __init__(self, parent, x, y, event):
		uiMobileStyle.Pressable.__init__(self, 56, 56)
		self.SetParent(parent)
		self.SetPosition(x, y)
		self.image = ui.ExpandedImageBox()
		self.image.SetParent(self)
		self.image.AddFlag("not_pick")
		self.image.LoadImage(IMG + "pz_close_1.png")
		self.image.SetPosition(8, 8)
		self.image.Show()
		self.SetEvent(event)
		self.Show()

	def RefreshVisual(self):
		if getattr(self, "image", None):
			self.image.LoadImage(IMG + ("pz_close_3.png" if self.GetVisualState() == "down" else "pz_close_1.png"))


def Frame(board, title, height):
	# eski cerceve parcalari gizlenir; kendi tahtamiz, baslik, ayirici cizgi ve kare kapat
	for name in ("centerImage", "leftSide", "rightSide", "bottomSide", "leftBottomCorner", "rightBottomCorner", "titleLeft", "titleRight", "titleCenter", "closeButton", "titleText"):
		widget = getattr(board, name, None)
		if widget:
			widget.Hide()
	board.akrBg = ui.ExpandedImageBox()
	board.akrBg.SetParent(board)
	board.akrBg.AddFlag("not_pick")
	board.akrBg.LoadImage(IMG + "pz_board_%d.png" % height)
	board.akrBg.SetPosition(0, 0)
	board.akrBg.Show()
	board.akrTitleY = 13
	board.titleText = uiMobileStyle.CreateText(board, title, W // 2, 13, GOLD, "Marcellus:22", "center")
	board.akrLine = uiMobileStyle.CreateBar(board, LINE, 16, 60, W - 32, 1)
	import uiAkravonInventory
	board.akrClose = uiAkravonInventory.CloseButton(board, W - 12 - 34, 12, board.Close)	# ortak carpi, basinca altin


def Field(board, x, y, width, height):
	image = ui.ExpandedImageBox()
	image.SetParent(board)
	image.AddFlag("not_pick")
	image.LoadImage(IMG + "pz_field_%d.png" % width)
	image.SetPosition(x, y)
	image.Show()
	return image


def Grid(board, x, y):
	box = ui.ExpandedImageBox()
	box.SetParent(board)
	box.AddFlag("not_pick")
	box.LoadImage(IMG + "pz_gridbox.png")
	box.SetPosition(x, y)
	box.Show()
	grid = ui.GridSlotWindow()
	grid.SetParent(box)
	grid.SetPosition(6, 6)
	grid.ArrangeSlot(0, 6, 6, 56, 56, 0, 0)
	grid.SetSlotBaseImage("ikashop/lite/common/slot/cell56.png", 1.0, 1.0, 1.0, 1.0)
	if hasattr(wndMgr, "SetSlotUnitSize"):
		wndMgr.SetSlotUnitSize(grid.hWnd, 56, 56, 4)
	grid.Show()
	return (box, grid)


def BuildMarket(board):
	# Marketim: baslik, pazar adi, 6x6 izgara, iki buyuk dugme
	import uiikashop
	Frame(board, "Marketim", 556)
	board.NameSlotBar = Field(board, 12, 66, 348, 52)
	board.NameEdit = ui.EditLine()
	board.NameEdit.SetParent(board.NameSlotBar)
	board.NameEdit.SetMax(40)
	board.NameEdit.SetSize(200, 26)
	board.NameEdit.SetPosition(16, 13)
	board.NameEdit.SetLimitWidth(200)
	board.NameEdit.SetEscapeEvent(ui.__mem_func__(board.OnPressNameEscapeKey))
	board.NameEdit.SetInfoMessage(uiikashop.locale.IKASHOP_BUSINESS_SHOP_TITLE)
	board.NameEdit.SetStatus(True)
	try:
		board.NameEdit.SetFontName("Lato:20")
		board.NameEdit.SetPackedFontColor(0xFFFFFFFF)
	except Exception:
		pass
	board.NameEdit.Show()

	(board.slotBox, board.slotWindow) = Grid(board, 12, 130)
	board.slotWindow.SetSelectEmptySlotEvent(board._OnClickEmptySlot)
	board.slotWindow.SetSelectItemSlotEvent(board._OnClickItemSlot)
	board.slotWindow.SetOverInItemEvent(board._ShowToolTipOnItemSlot)
	board.slotWindow.SetOverOutItemEvent(board._HideToolTipOnItemSlot)
	board.slotWindow.SetUnselectItemSlotEvent(board._OnRightClickItemSlot)

	# kalan sure: pazar adi alaninin sagi, sari
	board.durationBox = board.NameSlotBar
	board.durationText = uiMobileStyle.CreateText(board.NameSlotBar, "", 332, 15, YELLOW, "Lato:17", "right")
	board.durationExpiredIcon = ui.ImageBox()
	board.durationExpiredIcon.SetParent(board.NameSlotBar)
	board.durationExpiredIcon.Hide()

	# dugmeler: "PAZARI KUR" (pazar yok ya da suresi bitmis) ve "PAZARI KAPAT"; yaninda DEPO
	# alt satir: [KUR / KAPAT] [TEKLIFLER] [DEPO] - 3 x 112, arada 6
	board.durationIncreaseButton = PazarButton(board, 12, 490, 112, 52, "pz_btn112_%d.png", "KUR", board._OnClickDurationIncreaseButton, "Marcellus:20")
	board.durationIncreaseButton.Hide()
	board.cancelShopButton = PazarButton(board, 12, 490, 112, 52, "pz_btn112_%d.png", "KAPAT", board._OnClickCancelShopButton, "Marcellus:20")
	board.cancelShopButton.Hide()
	board.privateOfferOpenButton = PazarButton(board, 130, 490, 112, 52, "pz_btn112_%d.png", "TEKLİFLER", board.offersBoard.Toggle, "Marcellus:18")
	board.safeboxOpenButton = PazarButton(board, 248, 490, 112, 52, "pz_btn112_%d.png", "DEPO", board.safeboxBoard.Toggle, "Marcellus:20")
	board.auctionOpenButton = ui.Button()
	board.auctionOpenButton.SetParent(board)
	board.auctionOpenButton.Hide()
	board.safeboxItemCounter = board.CreateWidget(uiikashop.IkarusShopCounter, pos = (86, 0), parent = board.safeboxOpenButton)
	board.privateOfferCounter = board.CreateWidget(uiikashop.IkarusShopCounter, pos = (86, 0), parent = board.privateOfferOpenButton)
	board.auctionOfferCounter = board.CreateWidget(uiikashop.IkarusShopCounter, pos = (0, 0), parent = board.auctionOpenButton)
	BuildMarketSelect(board)


def BuildSafebox(board):
	# Market Deposu: ustte sayfa sekmeleri, 6x6 izgara, altta bakiye + AL + TUMU
	import uiikashop
	Frame(board, "Market Deposu", 556)
	strip = ui.ExpandedImageBox()
	strip.SetParent(board)
	strip.AddFlag("not_pick")
	strip.LoadImage(IMG + "pz_tabs_strip.png")
	strip.SetPosition(12, 72)
	strip.Show()
	board.akrStrip = strip
	board.selectPageButtons = []
	for i in xrange(5):
		board.selectPageButtons.append(PazarTab(board, 12 + i * 69, 72, 69, 46, uiikashop.RomeNumber(i + 1), board._OnSelectPage, i))
	(board.slotBox, board.slotWindow) = Grid(board, 12, 128)
	board.slotWindow.SetOverInItemEvent(board._ShowToolTipOnItemSlot)
	board.slotWindow.SetOverOutItemEvent(board._HideToolTipOnItemSlot)
	board.slotWindow.SetSelectItemSlotEvent(board._OnClickItemSlot)

	board.moneyBox = Field(board, 12, 490, 168, 46)
	board.moneyText = uiMobileStyle.CreateText(board.moneyBox, "0", 12, 11, YELLOW, "Lato:19", "left")
	board.moneyClaimButton = PazarButton(board, 190, 490, 74, 46, "pz_btn74_%d.png", "AL", board._OnClickClaimMoneyButton, "Marcellus:20")
	board.itemsClaimButton = PazarButton(board, 272, 490, 88, 46, "pz_btn88_%d.png", "TÜMÜ", board._OnClickClaimAllItemsButton, "Marcellus:20")
	board._OnSelectPage(0)


def BuildGuest(board):
	# Baska oyuncunun pazari: Marketim ile ayni tahta, 6x6 izgara, kalan sure.
	# Urune DOKUN: urun secilir (yesil), bilgisi acilir, altta SATIN AL / TEKLIF VER gorunur; secim yoksa gizli
	Frame(board, "Pazar", 556)
	board.durationBox = Field(board, 12, 66, 348, 52)
	board.durationText = uiMobileStyle.CreateText(board.durationBox, "", 332, 15, YELLOW, "Lato:17", "right")
	(board.slotBox, board.slotWindow) = Grid(board, 12, 130)
	board.slotWindow.SetSelectItemSlotEvent(board._OnClickItemSlot)
	board.slotWindow.SetOverInItemEvent(board._ShowToolTipOnItemSlot)
	board.slotWindow.SetOverOutItemEvent(board._HideToolTipOnItemSlot)
	board.akrHint = uiMobileStyle.CreateText(board, "Ürüne dokun", W // 2, 506, SOFT, "Lato:17", "center")
	board.akrBuy = PazarButton(board, 12, 490, 168, 52, "pz_btn168_%d.png", "SATIN AL", board._AkrBuy, "Marcellus:20")
	board.akrOffer = PazarButton(board, 192, 490, 168, 52, "pz_btn168_%d.png", "TEKLİF VER", board._AkrOffer, "Marcellus:20")
	board.akrBuy.Hide()
	board.akrOffer.Hide()
	board.akrSelected = None
	board.closeGmButton = PazarButton(board, 12, 66, 120, 52, "pz_btn168_%d.png", "GM KAPAT", board._OnClickGmCloseButton, "Marcellus:18")
	board.closeGmButton.Hide()
	board.akrTicker = TooltipTicker(board)


OFFER_ROWS = 3
OFFER_STEP = 116


def BuildOffers(board):
	# Teklifler: Marketim ile ayni tahta; her teklif bir kart (esya ikonu, ad, kimden, tutar) + KABUL / REDDET; sayfalar oklarla
	import uiAkravonDugme
	Frame(board, "Teklifler", 556)
	board.akrOffers = []
	board.akrOfferPage = 0
	board.akrRows = []
	for i in range(OFFER_ROWS):
		y = 72 + i * OFFER_STEP
		card = Field(board, 12, y, 348, 52)
		card.LoadImage(IMG + "pz_field_card.png")	# 348x88 hazir gorsel (olcekleme cizilmiyordu)
		icon = ui.ExpandedImageBox()
		icon.SetParent(board)
		icon.AddFlag("not_pick")
		icon.SetPosition(22, y + 20)
		name = uiMobileStyle.CreateText(board, "", 66, y + 12, TEXT, "Lato:18", "left")
		who = uiMobileStyle.CreateText(board, "", 66, y + 38, SOFT, "Lato:16", "left")
		price = uiMobileStyle.CreateText(board, "", 66, y + 64, YELLOW, "LatoBd:18", "left")
		accept = PazarButton(board, 264, y + 5, 88, 46, "pz_btn88_%d.png", "KABUL", None, "Marcellus:18")
		deny = PazarButton(board, 264, y + 53, 88, 46, "pz_btn88_%d.png", "REDDET", None, "Marcellus:18")
		accept.SetEvent(lambda b = board, k = i: OfferAction(b, k, True))
		deny.SetEvent(lambda b = board, k = i: OfferAction(b, k, False))
		board.akrRows.append((card, icon, name, who, price, accept, deny))
	board.akrEmpty = uiMobileStyle.CreateText(board, "Teklif yok", W // 2, 260, SOFT, "Lato:19", "center")
	board.akrPrev = uiAkravonDugme.IkonDugme("arrow_l", 34)
	board.akrPrev.SetParent(board)
	board.akrPrev.GorselKonumu(110, 500)
	board.akrPrev.OlayAyarla(lambda b = board: OfferPage(b, -1))
	board.akrPageText = uiMobileStyle.CreateText(board, "", W // 2, 507, YELLOW, "Lato:18", "center")
	board.akrNext = uiAkravonDugme.IkonDugme("arrow_r", 34)
	board.akrNext.SetParent(board)
	board.akrNext.GorselKonumu(W - 110 - 34, 500)
	board.akrNext.OlayAyarla(lambda b = board: OfferPage(b, 1))

	def SetElements(elements):
		board.akrServerOffers = list(elements)
		board.akrOffers = AllOffers(elements)
		board.akrOfferPage = 0
		OfferRefresh(board)
	board.SetElements = SetElements
	board.akrServerOffers = []
	board.akrOffers = AllOffers([])
	OfferRefresh(board)


LOCAL_OFFERS = []	# fisiltiyla gelen teklifler (sunucu teklif kaydi tutmadigi icin)


def OnWhisperOffer(buyer, line):
	# game.OnRecvWhisper: "[Pazar teklifi] ... #T<id>:<tutar>:<vnum>"
	import re
	m = re.search(r"#T(\d+):(\d+):(\d+)", str(line))
	if not m:
		return
	(itemId, amount, vnum) = (int(m.group(1)), long(m.group(2)), int(m.group(3)))
	for offer in LOCAL_OFFERS:
		if offer['item_id'] == itemId and offer['buyer_name'] == buyer:
			offer['price'] = amount
			break
	else:
		LOCAL_OFFERS.append({'local' : True, 'incoming' : True, 'buyer_name' : str(buyer), 'price' : amount, 'item_id' : itemId, 'item' : {'vnum' : vnum, 'count' : 1}})


def OffersOpened(board):
	# Teklifler acilinca fisilti teklifleri de listelenir (sunucu bos liste yollasa da)
	board.akrOffers = AllOffers(getattr(board, "akrServerOffers", []))
	board.akrOfferPage = 0
	OfferRefresh(board)


def AllOffers(serverOffers):
	return list(LOCAL_OFFERS) + list(serverOffers or [])


def OfferPage(board, delta):
	pages = max(1, (len(board.akrOffers) + OFFER_ROWS - 1) // OFFER_ROWS)
	board.akrOfferPage = max(0, min(pages - 1, board.akrOfferPage + delta))
	OfferRefresh(board)


def OfferRefresh(board):
	import item as itemModule
	import uiikashop
	offers = board.akrOffers
	pages = max(1, (len(offers) + OFFER_ROWS - 1) // OFFER_ROWS)
	start = board.akrOfferPage * OFFER_ROWS
	for (i, (card, icon, name, who, price, accept, deny)) in enumerate(board.akrRows):
		k = start + i
		if k >= len(offers):
			for part in (card, icon, name, who, price, accept, deny):
				part.Hide()
			continue
		data = offers[k]
		itemData = data['item']
		itemModule.SelectItem(itemData['vnum'])
		icon.LoadImage(itemModule.GetIconImageFileName())
		icon.SetScale(1.0, 1.0)
		count = itemData.get('count', 1)
		name.SetText(itemModule.GetItemName() + ((" (%d)" % count) if count > 1 else ""))
		incoming = data.get('incoming')
		who.SetText(("Kimden: %s" % data.get('buyer_name', "")) if incoming else ("Pazar: %s" % data.get('name', "")))
		price.SetText(uiikashop.locale.NumberToMoneyString(data['price']))
		deny.SetText("REDDET" if incoming else "GER\xdd AL")
		for part in (card, icon, name, who, price, deny):
			part.Show()
		if incoming:
			accept.Show()
		else:
			accept.Hide()
	if offers:
		board.akrEmpty.Hide()
	else:
		board.akrEmpty.Show()
	board.akrPageText.SetText("%d/%d" % (board.akrOfferPage + 1, pages))
	for part in (board.akrPrev, board.akrNext, board.akrPageText):
		if pages > 1:
			part.Show()
		else:
			part.Hide()


def OfferAction(board, k, accept):
	import uiikashop
	k = board.akrOfferPage * OFFER_ROWS + k
	if k >= len(board.akrOffers):
		return
	board.akrSelectedOffer = board.akrOffers[k]
	if accept:
		board.OpenQuestionDialog(uiikashop.locale.IKASHOP_OFFER_LIST_ACCEPT_OFFER_QUESTION, lambda b = board: _OfferDo(b, True))
	else:
		board.OpenQuestionDialog(uiikashop.locale.IKASHOP_OFFER_LIST_CANCEL_OFFER_QUESTION, lambda b = board: _OfferDo(b, False))


def _OfferDo(board, accept):
	import ikashop
	import net
	data = board.akrSelectedOffer
	board.questionDialog.Hide()
	if data.get('local'):
		# fisilti teklifi: kabul = urun fiyati teklife iner, aliciya haber; ret = aliciya haber
		if accept:
			ikashop.SendEditItem(data['item_id'], data['price'])
			net.SendWhisperPacket(data['buyer_name'], "[Pazar] Teklifin kabul edildi: fiyat %d Yang oldu, pazardan satin alabilirsin." % data['price'])
		else:
			net.SendWhisperPacket(data['buyer_name'], "[Pazar] Teklifin reddedildi.")
		if data in LOCAL_OFFERS:
			LOCAL_OFFERS.remove(data)
		board.akrOffers = AllOffers(getattr(board, "akrServerOffers", []))
		OfferRefresh(board)
		return
	if accept:
		ikashop.SendOfferAccept(data['id'])
	else:
		ikashop.SendOfferCancel(data['id'], data['ownerid'])


class TooltipTicker(ui.Window):
	# secili urunun bilgisi her karede pazar penceresinin ortasina konur (baska kod yerini degistirse de)
	def __init__(self, board):
		ui.Window.__init__(self)
		self.SetParent(board)
		self.AddFlag("not_pick")
		self.SetSize(1, 1)
		self.board = board
		self.Show()

	def OnUpdate(self):
		board = self.board
		if getattr(board, "akrSelected", None) is None or fixedTooltip is None or not fixedTooltip.IsShow():
			return
		(x, y) = board.GetGlobalPosition()
		tx = x + (W - fixedTooltip.GetWidth()) // 2
		ty = y + 130
		sw = wndMgr.GetScreenWidth()
		sh = wndMgr.GetScreenHeight()
		fixedTooltip.SetPosition(max(4, min(sw - fixedTooltip.GetWidth() - 4, tx)), max(4, min(sh - fixedTooltip.GetHeight() - 4, ty)))


def BuildMarketSelect(board):
	board.akrSelected = None
	board.akrEdit = PazarButton(board, 12, 490, 112, 52, "pz_btn112_%d.png", "FİYAT", lambda b = board: _MarketEdit(b), "Marcellus:20")
	board.akrRemove = PazarButton(board, 130, 490, 112, 52, "pz_btn112_%d.png", "GERİ AL", lambda b = board: _MarketRemove(b), "Marcellus:20")
	board.akrBack = PazarButton(board, 248, 490, 112, 52, "pz_btn112_%d.png", "VAZGEÇ", lambda b = board: MarketSelect(b, None), "Marcellus:20")
	for button in (board.akrEdit, board.akrRemove, board.akrBack):
		button.Hide()
	board.akrTicker = TooltipTicker(board)


def _MarketButtons(board, selected):
	normal = [board.privateOfferOpenButton, board.safeboxOpenButton]
	if selected:
		board.akrNormalShown = [w for w in [board.durationIncreaseButton, board.cancelShopButton] + normal if w.IsShow()]
		for w in board.akrNormalShown:
			w.Hide()
		for w in (board.akrEdit, board.akrRemove, board.akrBack):
			w.Show()
	else:
		for w in (board.akrEdit, board.akrRemove, board.akrBack):
			w.Hide()
		for w in getattr(board, "akrNormalShown", normal):
			w.Show()


def MarketSelect(board, slot):
	grid = board.slotWindow
	if board.akrSelected is not None:
		grid.DeactivateSlot(board.akrSelected)
	if slot is None or slot == board.akrSelected or slot not in board.shopInfo.get('items', {}):
		if board.akrSelected is not None:
			_MarketButtons(board, False)
		board.akrSelected = None
		board._HideToolTipOnItemSlot()
		ReleaseTooltip()
		grid.RefreshSlot()
		return
	if board.akrSelected is None:
		_MarketButtons(board, True)
	board.akrSelected = slot
	grid.ActivateSlot(slot, 0.3, 0.9, 0.3, 1.0)
	grid.RefreshSlot()
	board._ShowToolTipOnItemSlot(slot)
	global fixedTooltip
	fixedTooltip = board.GetToolTip()
	fixedTooltip.SetFollow(False)


def _MarketEdit(board):
	slot = board.akrSelected
	if slot is None:
		return
	data = board.shopInfo['items'][slot]
	MarketSelect(board, None)
	board.insertingPriceInfo = data
	board.OpenMoneyInputDialog(board._OnAcceptEditingItemPrice, itemVnum = data['vnum'], itemCount = max(1, data.get('count', 1)))


def _MarketRemove(board):
	import ikashop
	slot = board.akrSelected
	if slot is None:
		return
	data = board.shopInfo['items'][slot]
	MarketSelect(board, None)
	ikashop.SendRemoveItem(data['id'])


def GuestSelect(board, slot):
	# secili urun: yesil isaret, bilgi acik, alt dugmeler gorunur; ayni urune tekrar dokun = secim kalkar
	grid = board.slotWindow
	if board.akrSelected is not None:
		grid.DeactivateSlot(board.akrSelected)
	if slot is None or slot == board.akrSelected:
		board.akrSelected = None
		board.akrBuy.Hide()
		board.akrOffer.Hide()
		board.akrHint.Show()
		board._HideToolTipOnItemSlot()
		ReleaseTooltip()
		grid.RefreshSlot()
		return
	board.akrSelected = slot
	grid.ActivateSlot(slot, 0.3, 0.9, 0.3, 1.0)
	grid.RefreshSlot()
	board.akrHint.Hide()
	board.akrBuy.Show()
	board.akrOffer.Show()
	board._ShowToolTipOnItemSlot(slot)
	tooltip = board.GetToolTip()
	global fixedTooltip
	fixedTooltip = tooltip
	tooltip.SetFollow(False)	# fare / parmak konumunu izlemesin
	(x, y) = board.GetGlobalPosition()
	# bilgi pazar penceresinin ortasinda, izgaranin ustunde; ekrandan tasmaz
	tx = x + (W - tooltip.GetWidth()) // 2
	ty = y + 130
	sw = wndMgr.GetScreenWidth()
	sh = wndMgr.GetScreenHeight()
	tooltip.SetPosition(max(4, min(sw - tooltip.GetWidth() - 4, tx)), max(4, min(sh - tooltip.GetHeight() - 4, ty)))
	tooltip.SetTop()


class PriceDialog(ui.ScriptWindow):
	# "ne kadar yang'a satmak istiyorsun" - yatay: solda esya, sagda fiyat + ozet, altta buyuk dugmeler
	WIDTH = 560
	HEIGHT = 384

	def __init__(self, itemVnum, count):
		ui.ScriptWindow.__init__(self)
		self.minPrice = 0
		self.maxPrice = 0
		self.avgValue = 0
		self.count = count
		self.keep = []
		self.__Build(itemVnum)
		self.__Compact(True)	# ortalama fiyat gelene kadar kisa pencere
		self.SetCenterPosition()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def __Text(self, text, x, y, color = TEXT, font = "Lato:17", align = "center"):
		line = uiMobileStyle.CreateText(self, text, x, y, color, font, align)
		self.keep.append(line)
		return line

	def __Build(self, itemVnum):
		self.AddFlag("movable")
		self.AddFlag("float")
		self.bg = ui.ExpandedImageBox()
		self.bg.SetParent(self)
		self.bg.AddFlag("not_pick")
		self.bg.LoadImage(IMG + "pz_dialog.png")
		self.bg.Show()
		self.titleLine = self.__Text("Satış Fiyatı", self.WIDTH // 2, 13, GOLD, "Marcellus:22")
		self.keep.append(uiMobileStyle.CreateBar(self, LINE, 16, 60, self.WIDTH - 32, 1))

		item.SelectItem(itemVnum)
		plate = ui.ExpandedImageBox()
		plate.SetParent(self)
		plate.AddFlag("not_pick")
		plate.LoadImage(IMG + "pz_itemplate.png")
		plate.SetPosition(24, 78)
		plate.Show()
		self.keep.append(plate)
		icon = ui.ExpandedImageBox()
		icon.SetParent(plate)
		icon.AddFlag("not_pick")
		icon.LoadImage(item.GetIconImageFileName())
		(w, h) = (icon.GetWidth(), icon.GetHeight())
		scale = min(3.0, 112.0 / max(w, h))
		icon.SetScale(scale, scale)
		icon.SetPosition((130 - int(w * scale)) // 2, (130 - int(h * scale)) // 2)
		icon.Show()
		self.keep.append(icon)
		name = item.GetItemName()
		if self.count > 1:
			name = "%s  x%d" % (name, self.count)
		self.__Text(name, 89, 218, YELLOW, "Lato:16")

		field = ui.ExpandedImageBox()
		field.SetParent(self)
		field.AddFlag("not_pick")
		field.LoadImage(IMG + "pz_field_wide.png")
		field.SetPosition(176, 78)
		field.Show()
		self.keep.append(field)
		self.inputValue = ui.EditLine()
		self.inputValue.SetParent(field)
		self.inputValue.SetPosition(16, 15)
		self.inputValue.SetSize(324, 26)
		self.inputValue.SetMax(13)
		self.inputValue.SetNumberMode()
		self.inputValue.SetFontName("Lato:24")
		self.inputValue.SetPackedFontColor(0xFFFFFFFF)
		self.inputValue.OnIMEUpdate = ui.__mem_func__(self.__OnUpdate)
		self.inputValue.Show()
		self.moneyText = uiMobileStyle.CreateText(field, "0 Yang", 344, 17, YELLOW, "Lato:20", "right")
		self.keep.append(self.moneyText)
		# pazar ozeti: uc satir (etiket solda, deger sagda sari)
		card = ui.ExpandedImageBox()
		card.SetParent(self)
		card.AddFlag("not_pick")
		card.LoadImage(IMG + "pz_info.png")
		card.SetPosition(176, 146)
		card.Show()
		self.keep.append(card)
		self.__Text("Ortalama fiyat", 192, 156, TEXT, "Lato:17", "left")
		self.__Text("En düşük", 192, 183, TEXT, "Lato:17", "left")
		self.__Text("En yüksek", 192, 210, TEXT, "Lato:17", "left")
		self.avgText = self.__Text("-", 520, 156, YELLOW, "Lato:17", "right")
		self.minText = self.__Text("-", 520, 183, YELLOW, "Lato:17", "right")
		self.maxText = self.__Text("-", 520, 210, YELLOW, "Lato:17", "right")
		self.avgButton = PazarButton(self, 176, 254, 360, 44, "pz_btn360_%d.png", "ORTALAMA FİYATI KULLAN", self.__UseAverage, "Marcellus:18")
		self.avgButton.Hide()
		self.acceptButton = PazarButton(self, 24, 262, 248, 52, "pz_btn248_%d.png", "ONAYLA", None, "Marcellus:20")
		self.cancelButton = PazarButton(self, 288, 262, 248, 52, "pz_btn248_%d.png", "VAZGEÇ", None, "Marcellus:20")

	def __OnUpdate(self):
		ui.EditLine.OnIMEUpdate(self.inputValue)
		text = self.inputValue.GetText().lower().replace("k", "000")
		money = long(text) if text and text.isdigit() else 0
		self.moneyText.SetText(localeInfo.NumberToMoneyString(min(money, 9999999999999)))

	def __UseAverage(self):
		if self.avgValue > 0:
			self.SetValue(self.avgValue * self.count)

	# uiikashop uyumlulugu
	def Open(self):
		self.inputValue.SetText("")
		self.__OnUpdate()
		self.inputValue.SetFocus()
		# klavye acik: pencere ekranin ustunde (ONAYLA / VAZGEC klavyenin ustunde kalir)
		self.SetPosition((wndMgr.GetScreenWidth() - self.GetWidth()) // 2, max(6, int(wndMgr.GetScreenHeight() * 0.03)))
		self.SetTop()
		self.Show()

	def __Compact(self, compact):
		# ortalama fiyat dugmesi yoksa dugmeler yukari kayar, pencere kisalir (bos alan kalmaz)
		if compact:
			self.bg.LoadImage(IMG + "pz_dialog_short.png")
			self.acceptButton.SetPosition(24, 262)
			self.cancelButton.SetPosition(288, 262)
			self.SetSize(self.WIDTH, 330)
		else:
			self.bg.LoadImage(IMG + "pz_dialog.png")
			self.acceptButton.SetPosition(24, 314)
			self.cancelButton.SetPosition(288, 314)
			self.SetSize(self.WIDTH, self.HEIGHT)

	def Close(self):
		self.Hide()

	def SoftClose(self):
		self.Hide()
		if self.inputValue.IsFocus():
			self.inputValue.KillFocus()

	def Destroy(self):
		self.keep = []
		self.ClearDictionary()

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def SetTitle(self, name):
		pass

	def SetHeader(self, text):
		# AKRAVON: ayni pencere teklif icin de kullanilir ("Teklif Ver")
		self.titleLine.SetText(text)

	def SetFocus(self):
		self.inputValue.SetFocus()

	def SetAcceptEvent(self, event):
		self.acceptButton.SetEvent(event)
		self.inputValue.OnIMEReturn = event

	def SetCancelEvent(self, event):
		self.cancelButton.SetEvent(event)
		self.inputValue.OnPressEscapeKey = event

	def SetValue(self, value):
		if value == "":
			self.inputValue.SetText("")
		else:
			self.inputValue.SetText(str(value))
		self.__OnUpdate()

	def GetText(self):
		return self.inputValue.GetText()

	def GetMinPrice(self):
		return self.minPrice

	def GetMaxPrice(self):
		return self.maxPrice

	def SetAvgPrice(self, price):
		self.avgValue = long(price)
		if price == 0:
			self.avgText.SetText("bilgi yok")
			self.avgButton.Hide()
			self.__Compact(True)
		else:
			self.avgText.SetText(localeInfo.NumberToMoneyString(price))
			self.avgButton.Show()
			self.__Compact(False)

	def SetMinMaxPrice(self, minPrice, maxPrice):
		self.minPrice = long(minPrice)
		self.maxPrice = long(maxPrice)
		self.minText.SetText(localeInfo.NumberToMoneyString(minPrice) if minPrice else "-")
		self.maxText.SetText(localeInfo.NumberToMoneyString(maxPrice) if maxPrice else "-")


fixedTooltip = None


def ReleaseTooltip():
	global fixedTooltip
	if fixedTooltip is not None:
		try:
			fixedTooltip.SetFollow(True)
			fixedTooltip.HideToolTip()
		except Exception:
			pass
		fixedTooltip = None


def HideItemCard():
	ReleaseTooltip()
