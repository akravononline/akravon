# -*- coding: utf-8 -*-
# AKRAVON: lonca imzasi (16x12) / simgesi (64x128) secme penceresi - oyunun upload klasoru yerine hazir takim; kendi tasarimimiz
# uiUploadMark.MarkSelectDialog / SymbolSelectDialog bu sinifa devreder (orijinalden_degisen/uiuploadmark.py)
from __future__ import division
import app
import ui
import wndMgr
import uiMobileStyle
import uiAkravon
import uiAkravonQuest
import uiAkravonOffer

BOARD_W = 500
PAD = 18
TITLE_H = 54
GOLD = uiMobileStyle.Color(222, 184, 110)
CREAM = uiMobileStyle.Color(250, 236, 210)

KINDS = {
	"mark" : {"id" : 0, "ext" : "bmp", "title" : "Lonca İmzası", "filters" : ("tga", "bmp", "jpg"), "prefix" : "imza_", "size" : (16, 12), "scale" : 4, "cols" : 4, "rows" : 3, "cell" : (104, 84)},
	"symbol" : {"id" : 1, "ext" : "jpg", "title" : "Lonca Simgesi", "filters" : ("jpg",), "prefix" : "simge_", "size" : (64, 128), "scale" : 1, "cols" : 5, "rows" : 1, "cell" : (84, 150)},
}


class Cell(uiMobileStyle.Pressable):
	def __init__(self, parent, width, height):
		uiMobileStyle.Pressable.__init__(self, width, height)
		self.SetParent(parent)
		self.box = uiAkravon.Box()
		self.box.SetParent(self)
		self.box.SetSize(width, height)
		self.box.Show()
		uiAkravon.SkinBox(self.box, "picker_row_normal", "plate_gold_normal")
		self.image = ui.ExpandedImageBox()
		self.image.SetParent(self)
		self.image.AddFlag("not_pick")
		self.fileName = None
		self.selected = False
		self.Hide()

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.box = None
		self.image = None

	def Bind(self, fileName, scale, size):
		self.fileName = fileName
		self.image.LoadImage("upload/" + fileName)
		self.image.SetScale(scale, scale)
		(w, h) = (size[0] * scale, size[1] * scale)
		self.image.SetPosition((self.GetWidth() - w) // 2, (self.GetHeight() - h) // 2)
		self.image.Show()
		self.Show()

	def SetSelected(self, selected):
		self.selected = selected
		self.RefreshVisual()

	def RefreshVisual(self):
		if getattr(self, "box", None):
			uiAkravon.SetSkinState(self.box, self.selected or self.pressed)


class Picker(ui.ScriptWindow):
	def __init__(self, kind):
		ui.ScriptWindow.__init__(self)
		self.AddFlag("float")	# SetTop calissin: lonca penceresinin onunde acilir
		self.kind = KINDS[kind]
		self.selectEvent = None
		self.files = []
		self.page = 0
		self.selected = None
		self.built = False

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def SAFE_SetSelectEvent(self, event):
		self.selectEvent = ui.__mem_func__(event)

	def __Build(self):
		self.built = True
		kind = self.kind
		(cw, ch) = kind["cell"]
		gap = 8
		cols = kind["cols"]
		rows = kind["rows"]
		gridW = cols * cw + (cols - 1) * gap
		gridH = rows * ch + (rows - 1) * gap
		self.gridX = (BOARD_W - gridW) // 2
		self.gridY = TITLE_H + 16
		navY = self.gridY + gridH + 12
		height = navY + 44 + 12 + 44 + 14 + 24 + 52 + PAD	# + galeri dugmesi
		self.board = uiAkravonQuest.Board9()
		self.board.SetParent(self)
		self.board.AddFlag("not_pick")
		self.board.Show()
		self.board.SetSize(BOARD_W, height)
		self.SetSize(BOARD_W, height)
		self.title = uiMobileStyle.CreateText(self, kind["title"], BOARD_W // 2, 15, GOLD, "Marcellus:22", "center")
		self.divider = uiMobileStyle.CreateBar(self, uiMobileStyle.Color(96, 74, 42), PAD, TITLE_H, BOARD_W - PAD * 2, 1)
		import uiAkravonInventory
		self.close = uiAkravonInventory.CloseButton(self, BOARD_W - PAD - 34, 11, ui.__mem_func__(self.Close))
		self.cells = []
		for index in range(cols * rows):
			cell = Cell(self, cw, ch)
			cell.SetPosition(self.gridX + (index % cols) * (cw + gap), self.gridY + (index // cols) * (ch + gap))
			cell.SetEvent(ui.__mem_func__(self.__OnCell), index)
			self.cells.append(cell)
		self.prev = uiAkravonQuest.ArrowButton(self, BOARD_W // 2 - 120, navY, "qs_up", ui.__mem_func__(self.__PrevPage))
		self.next = uiAkravonQuest.ArrowButton(self, BOARD_W // 2 + 76, navY, "qs_dn", ui.__mem_func__(self.__NextPage))
		self.counter = uiMobileStyle.CreateText(self, "", BOARD_W // 2, navY + 12, uiAkravon.HINT_COLOR, "Lato:17", "center")
		self.gallery = uiAkravonOffer.SkinButton("Galeriden Seç")
		self.gallery.SetParent(self)
		self.gallery.SetPosition(BOARD_W // 2 - 95, navY + 44 + 10)
		self.gallery.SetEvent(ui.__mem_func__(self.__OnGallery))
		self.gallery.Show()
		self.status = uiMobileStyle.CreateText(self, "", BOARD_W // 2, navY + 44 + 10 + 44 + 14, CREAM, "Lato:18", "center")
		self.save = uiAkravonOffer.SkinButton("Kaydet")
		self.save.SetParent(self)
		self.save.SetPosition(BOARD_W // 2 - 196, height - PAD - 44)
		self.save.SetEvent(ui.__mem_func__(self.__OnSave))
		self.save.Show()
		self.cancel = uiAkravonOffer.SkinButton("Kapat")
		self.cancel.SetParent(self)
		self.cancel.SetPosition(BOARD_W // 2 + 6, height - PAD - 44)
		self.cancel.SetEvent(ui.__mem_func__(self.Close))
		self.cancel.Show()

	def Open(self):
		if not self.built:
			self.__Build()
		self.__ScanFiles()
		self.page = 0
		self.selected = None
		self.status.SetText("")
		self.__Refresh()
		self.SetPosition((wndMgr.GetScreenWidth() - BOARD_W) // 2, max(10, (wndMgr.GetScreenHeight() - self.GetHeight()) // 2))
		self.Show()
		self.SetTop()

	def Close(self):
		self.Hide()

	def Destroy(self):
		for cell in getattr(self, "cells", []):
			cell.Destroy()
		self.cells = []
		for name in ("prev", "next", "save", "cancel", "close", "gallery"):
			widget = getattr(self, name, None)
			if widget:
				widget.Destroy()
			setattr(self, name, None)
		self.ClearDictionary()
		self.Hide()

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def __ScanFiles(self):
		files = []
		for extension in self.kind["filters"]:
			for name in app.GetFileList("upload/*." + extension):
				if name.lower().startswith(self.kind["prefix"]):
					files.append(name)
		files.sort(key = lambda name: ("ozel" not in name.lower(), name))	# kendi gorsellerin en basta
		self.files = files

	def __PerPage(self):
		return self.kind["cols"] * self.kind["rows"]

	def __Refresh(self):
		per = self.__PerPage()
		pages = max(1, (len(self.files) + per - 1) // per)
		self.page = max(0, min(self.page, pages - 1))
		start = self.page * per
		for (index, cell) in enumerate(self.cells):
			position = start + index
			if position < len(self.files):
				cell.Bind(self.files[position], self.kind["scale"], self.kind["size"])
				cell.SetSelected(self.selected == self.files[position])
			else:
				cell.Hide()
		self.counter.SetText("%d / %d" % (self.page + 1, pages))
		if not self.files:
			self.status.SetText("Henüz hazır imza yok")

	def __PrevPage(self):
		self.page -= 1
		self.__Refresh()

	def __NextPage(self):
		self.page += 1
		self.__Refresh()

	def __OnCell(self, index):
		position = self.page * self.__PerPage() + index
		if position < len(self.files):
			self.selected = self.files[position]
			self.status.SetText("")
			self.__Refresh()

	def __OnGallery(self):
		if not hasattr(app, "PickImage"):
			self.status.SetText("Bu özellik yeni sürümde (uygulamayı güncelle)")
			return
		(width, height) = self.kind["size"]
		self.customName = "%s_ozel_%d.%s" % (self.kind["prefix"].rstrip("_"), int(app.GetTime() * 10), self.kind["ext"])
		self.status.SetText("Galeri açılıyor...")
		app.PickImage(self.kind["id"], width, height, "upload/" + self.customName)
		self.picking = True

	def OnUpdate(self):
		if not getattr(self, "picking", False):
			return
		state = app.PickImageState()
		if state == 2:
			self.picking = False
			self.__ScanFiles()
			self.selected = self.customName
			self.page = 0
			self.status.SetText("Görselin eklendi, KAYDET'e bas")
			self.__Refresh()
		elif state == 3:
			self.picking = False
			self.status.SetText("Seçim iptal edildi")

	def __OnSave(self):
		if not self.selected:
			self.status.SetText("Önce bir seçim yap")
			return
		if self.selectEvent:
			self.selectEvent(self.selected)
		self.Close()
