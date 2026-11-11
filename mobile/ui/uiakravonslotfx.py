# -*- coding: utf-8 -*-
# AKRAVON: envanter slot efektleri (orijinal Metin2 aktif yuva efekti, renklendirilmis)
#   aktif calisan esya (otomatik iksir, Ruzgarin Ayakkabisi) : MAVI kovalayan isik
#   toplu satis icin secilen esya                              : SARI-ALTIN kovalayan isik
#   yeni alinan esya (yerden / pazardan)                       : YESIL nabiz atan halka, birkac saniye sonra soner
# Bu dosyada Turkce harf yok.
import app
import item
import math
import player
import ui
import wndMgr

CELL = 58
FRAMES = 13
FPS = 14.0
NEW_SECONDS = 8.0
PATH = "mobile_ui/akravon/hud/"

state = {"inventory" : None, "ticker" : None, "frame" : -1}
overlays = {}	# yuva (sayfa ici) -> resim
newSeen = {}	# genel yuva -> ilk gorulme zamani


def _Rows(vnum):
	item.SelectItem(vnum)
	return max(1, min(3, item.GetItemSize()[1]))


def _Kind(inventory, slot, now):
	vnum = player.GetItemIndex(slot)
	if vnum == 0:
		newSeen.pop(slot, None)
		return None
	try:
		import uiAkravonBulkSell
		if uiAkravonBulkSell.active and slot in uiAkravonBulkSell.selected:
			return "sell"
	except Exception:
		pass
	try:
		import constInfo
		if constInfo.IS_AUTO_POTION(vnum) and player.GetItemMetinSocket(slot, 0) != 0:
			return "active"
		# Ruzgarin Ayakkabisi: envanterde gizlenir, efekt ekipman yuvasinda (uiakravoninventory)
	except Exception:
		pass
	if slot in (getattr(inventory, "liHighlightedItems", None) or []):
		pass	# AKRAVON: yeni esya yesil cercevesi kalkti; yalniz motorun sari yildizi
	else:
		newSeen.pop(slot, None)
	return None


def _File(kind, rows, frame):
	if kind == "new":
		return PATH + "akrfx_green_%d.png" % rows
	color = "gold" if kind == "sell" else "blue"
	return PATH + "akrfx_%s_%d_%02d.png" % (color, rows, frame)


class Ticker(ui.Window):
	def __init__(self, parent):
		ui.Window.__init__(self)
		self.SetParent(parent)
		self.AddFlag("not_pick")
		self.SetSize(1, 1)
		self.Show()

	def __del__(self):
		ui.Window.__del__(self)

	def OnUpdate(self):
		if not overlays:
			return
		now = app.GetTime()
		frame = int(now * FPS) % FRAMES
		for (local, image) in list(overlays.items()):
			if image.akrKind == "new":
				alpha = 0.4 + 0.6 * (0.5 + 0.5 * math.sin(now * 4.5))
				wndMgr.SetDiffuseColor(image.hWnd, 1.0, 1.0, 1.0, alpha)
			elif frame != state["frame"]:
				image.LoadImage(_File(image.akrKind, image.akrRows, frame))
				image.SetScale(CELL / 64.0, CELL / 64.0)
		state["frame"] = frame
		# yeni esya suresi dolunca kalk
		inventory = state["inventory"]
		if inventory:
			page = inventory.inventoryPageIndex
			size = player.INVENTORY_PAGE_SIZE
			for (local, image) in list(overlays.items()):
				if image.akrKind == "new" and now - newSeen.get(local + size * page, now) > NEW_SECONDS:
					Refresh(inventory)
					break


def Refresh(inventory = None):
	inventory = inventory or state["inventory"]
	if inventory is None:
		return
	try:
		wndItem = inventory.wndItem
		page = inventory.inventoryPageIndex
		size = player.INVENTORY_PAGE_SIZE
		now = app.GetTime()
		wanted = {}
		for local in range(size):
			slot = local + size * page
			kind = _Kind(inventory, slot, now)
			if kind:
				wanted[local] = (kind, _Rows(player.GetItemIndex(slot)))
		for local in list(overlays.keys()):
			image = overlays[local]
			if local not in wanted or wanted[local] != (image.akrKind, image.akrRows):
				image.Hide()
				del overlays[local]
		for (local, (kind, rows)) in wanted.items():
			if local in overlays:
				continue
			image = ui.ExpandedImageBox()
			image.akrKind = kind
			image.akrRows = rows
			image.SetParent(wndItem)
			image.AddFlag("not_pick")
			image.LoadImage(_File(kind, rows, 0))
			image.SetScale(CELL / 64.0, CELL / 64.0)
			image.SetPosition((local % 6) * CELL, (local // 6) * CELL)
			image.Show()
			image.SetTop()
			overlays[local] = image
		state["frame"] = -1
	except Exception:
		import dbg
		import traceback
		dbg.TraceError("uiAkravonSlotFx.Refresh: " + traceback.format_exc())


def Install(interface):
	# her karede cagrilir; envanter penceresi basina bir kez kurulur
	try:
		inventory = getattr(interface, "wndInventory", None)
	except ReferenceError:
		return
	if inventory is None or state["inventory"] is inventory:
		return
	state["inventory"] = inventory
	overlays.clear()
	newSeen.clear()
	state["ticker"] = Ticker(inventory.wndItem)
	original = inventory.RefreshBagSlotWindow

	def RefreshBagSlotWindow(*args):
		result = original(*args)
		Refresh(inventory)
		return result
	inventory.RefreshBagSlotWindow = RefreshBagSlotWindow
	Refresh(inventory)
