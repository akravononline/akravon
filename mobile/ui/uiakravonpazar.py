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
GOLD = uiMobileStyle.Color(222, 184, 110)
TEXT = uiMobileStyle.Color(250, 236, 210)
YELLOW = uiMobileStyle.Color(255, 214, 90)
SOFT = uiMobileStyle.Color(190, 178, 152)
LINE = uiMobileStyle.Color(96, 74, 42)
W = 372


class PazarButton(uiMobileStyle.Pressable):
	# dokunmaya uygun buyuk dugme: gorsel (normal/secili/basili) + ortada yazi
	def __init__(self, parent, x, y, width, height, prefix, text, event = None, font = "Marcellus:22", *args):
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
			state = 3 if self.GetVisualState() == "down" else 1
		self.image.LoadImage(IMG + self.prefix % state)

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
		self.label = uiMobileStyle.CreateText(self, text, width // 2, 10, SOFT, "Marcellus:22", "center")
		self.SetEvent(event, *args)
		self.Show()

	def Enable(self):
		self.on.Hide()
		self.label.SetPackedFontColor(SOFT)

	def Disable(self):
		self.on.Show()
		self.label.SetPackedFontColor(TEXT)

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
	board.titleText = uiMobileStyle.CreateText(board, title, W // 2, 13, GOLD, "Marcellus:24", "center")
	board.akrLine = uiMobileStyle.CreateBar(board, LINE, 16, 60, W - 32, 1)
	board.akrClose = PazarClose(board, W - 62, 2, board.Close)


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
	board.durationIncreaseButton = PazarButton(board, 12, 490, 168, 52, "pz_btn168_%d.png", "PAZARI KUR", board._OnClickDurationIncreaseButton, "Marcellus:21")
	board.durationIncreaseButton.Hide()
	board.cancelShopButton = PazarButton(board, 12, 490, 168, 52, "pz_btn168_%d.png", "PAZARI KAPAT", board._OnClickCancelShopButton, "Marcellus:21")
	board.cancelShopButton.Hide()
	board.safeboxOpenButton = PazarButton(board, 192, 490, 168, 52, "pz_btn168_%d.png", "DEPO", board.safeboxBoard.Toggle, "Marcellus:22")
	board.privateOfferOpenButton = ui.Button()
	board.privateOfferOpenButton.SetParent(board)
	board.privateOfferOpenButton.Hide()
	board.auctionOpenButton = ui.Button()
	board.auctionOpenButton.SetParent(board)
	board.auctionOpenButton.Hide()
	board.safeboxItemCounter = board.CreateWidget(uiikashop.IkarusShopCounter, pos = (140, 0), parent = board.safeboxOpenButton)
	board.privateOfferCounter = board.CreateWidget(uiikashop.IkarusShopCounter, pos = (0, 0), parent = board.privateOfferOpenButton)
	board.auctionOfferCounter = board.CreateWidget(uiikashop.IkarusShopCounter, pos = (0, 0), parent = board.auctionOpenButton)


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
	board.moneyClaimButton = PazarButton(board, 190, 490, 74, 46, "pz_btn74_%d.png", "AL", board._OnClickClaimMoneyButton, "Marcellus:22")
	board.itemsClaimButton = PazarButton(board, 272, 490, 88, 46, "pz_btn88_%d.png", "TÜMÜ", board._OnClickClaimAllItemsButton, "Marcellus:20")
	board._OnSelectPage(0)


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
		self.SetSize(self.WIDTH, self.HEIGHT)
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
		self.__Text("Satış Fiyatı", self.WIDTH // 2, 13, GOLD, "Marcellus:24")
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
		self.avgButton = PazarButton(self, 176, 254, 360, 44, "pz_btn360_%d.png", "ORTALAMA FİYATI KULLAN", self.__UseAverage, "Marcellus:17")
		self.avgButton.Hide()
		self.acceptButton = PazarButton(self, 24, 314, 248, 52, "pz_btn248_%d.png", "ONAYLA", None, "Marcellus:22")
		self.cancelButton = PazarButton(self, 288, 314, 248, 52, "pz_btn248_%d.png", "VAZGEÇ", None, "Marcellus:22")

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
		self.SetCenterPosition()
		self.SetTop()
		self.Show()

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
		else:
			self.avgText.SetText(localeInfo.NumberToMoneyString(price))
			self.avgButton.Show()

	def SetMinMaxPrice(self, minPrice, maxPrice):
		self.minPrice = long(minPrice)
		self.maxPrice = long(maxPrice)
		self.minText.SetText(localeInfo.NumberToMoneyString(minPrice) if minPrice else "-")
		self.maxText.SetText(localeInfo.NumberToMoneyString(maxPrice) if maxPrice else "-")
