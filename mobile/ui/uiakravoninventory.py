# -*- coding: utf-8 -*-
# AKRAVON: envanter penceresi - kayan ekipman paneli, cekmece kulpu, kare kapat, sekme yazilari
# yerlesim: skin/build_akravon_inventory.py (olculer ayni olmali)
from __future__ import division
import app
import math
import wndMgr
import ui
import uiMobileStyle

IMG = "d:/ymir work/ui/akravon_legacy/"
CELL = 50
ROWS = 7
PAGES = 5
IW = 5 * CELL + 40
EW = 4 * 60 - 10 + 40
TAB = 30
GAP = 10
W = TAB + EW + GAP + IW
RIGHT_MARGIN = 10
TOP_Y = 0
PULL_Y = 190
SLIDE_TIME = 0.2
ENTER_TIME = 0.28
CFG = "akravon_envanter.cfg"
GOLD = uiMobileStyle.Color(222, 184, 110)
TAB_ON = uiMobileStyle.Color(255, 238, 190)
TAB_OFF = uiMobileStyle.Color(104, 98, 88)


def LoadOpen():
	try:
		return open(CFG).read().strip() == "1"
	except Exception:
		return False


def SaveOpen(value):
	try:
		open(CFG, "w").write("1" if value else "0")
	except Exception:
		pass


class PullTab(uiMobileStyle.Pressable):
	def __init__(self, parent, event):
		uiMobileStyle.Pressable.__init__(self, TAB + 10, 110)	# dokunma alani gorselden genis
		self.SetParent(parent)
		self.image = ui.ExpandedImageBox()
		self.image.SetParent(self)
		self.image.AddFlag("not_pick")
		self.image.LoadImage(IMG + "inv_pull.png")
		self.image.Show()
		self.chevron = ui.ExpandedImageBox()
		self.chevron.SetParent(self)
		self.chevron.AddFlag("not_pick")
		self.chevron.Show()
		self.SetEvent(event)
		self.SetOpened(False)
		self.Show()

	def SetOpened(self, opened):
		self.chevron.LoadImage(IMG + ("inv_chev_close.png" if opened else "inv_chev_open.png"))
		self.chevron.SetPosition((TAB - self.chevron.GetWidth()) // 2 + 1, (110 - self.chevron.GetHeight()) // 2)

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.image = None
		self.chevron = None


class CloseButton(uiMobileStyle.Pressable):
	def __init__(self, parent, x, y, event):
		uiMobileStyle.Pressable.__init__(self, 40, 40)
		self.SetParent(parent)
		self.SetPosition(x - 4, y - 4)
		self.image = ui.ExpandedImageBox()
		self.image.SetParent(self)
		self.image.AddFlag("not_pick")
		self.image.SetPosition(4, 4)
		self.image.LoadImage(IMG + "inv_close_1.png")
		self.image.Show()
		self.SetEvent(event)
		self.Show()

	def RefreshVisual(self):
		if getattr(self, "image", None):
			self.image.LoadImage(IMG + ("inv_close_3.png" if self.pressed else "inv_close_1.png"))

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.image = None


class DragHandle(ui.Window):
	# AKRAVON: pencere yalniz baslik seridinden tasinir (bos alana dokunmak tasimaz)
	def __init__(self, parent, owner, width, height):
		ui.Window.__init__(self)
		self.SetParent(parent)
		self.owner = owner
		self.SetPosition(0, 0)
		self.SetSize(width, height)
		self.dragging = False
		self.grab = (0, 0)
		self.Show()

	def OnMouseLeftButtonDown(self):
		(mx, my) = wndMgr.GetMousePosition()
		(wx, wy) = self.owner.window.GetGlobalPosition()
		self.grab = (mx - wx, my - wy)
		self.dragging = True

	def OnMouseLeftButtonUp(self):
		self.dragging = False

	def Drag(self):
		if not self.dragging:
			return
		(mx, my) = wndMgr.GetMousePosition()
		x = max(-W + 120, min(wndMgr.GetScreenWidth() - 120, mx - self.grab[0]))
		y = max(0, min(wndMgr.GetScreenHeight() - 90, my - self.grab[1]))
		self.owner.window.SetPosition(x, y)


class Slider(ui.Window):
	# ekipman acilma orani (0 kapali .. 1 acik) - her karede pencereyi yeniden yerlestirir
	def __init__(self, owner):
		ui.Window.__init__(self)
		self.owner = owner
		self.SetParent(owner.window)
		self.AddFlag("not_pick")
		self.SetSize(0, 0)
		self.Show()

	def OnUpdate(self):
		self.owner.Step()


class Skin(object):
	def __init__(self, window):
		self.window = window
		self.board = window.GetChild("board")
		self.equipBoard = window.GetChild("EquipBoard")
		self.opened = LoadOpen()
		self.ratio = 1.0 if self.opened else 0.0
		self.lastTime = app.GetTime()
		self.keep = []

		try:
			window.GetChild("TitleBar").Hide()
		except Exception:
			pass
		# AKRAVON: oto pelerin yok - dugme ve pencere kapatilir
		for name in ("disbandBtn", "wndBraveCape"):
			other = getattr(window, name, None)
			if other:
				other.Hide()
		window.ClickBraveCape = lambda *a: None
		# AKRAVON: kostum dugmesi simdilik kaldirildi (kostum penceresi ekrana sigmiyor); depo dugmesi silah yuvasinin altina,
		# ayakkabi hizasina alindi
		try:
			for (buttonName, art, x) in (("MallButton", "depo", 74), ("CostumeButton", "kostum", 142)):
				button = window.GetChild(buttonName)
				kind = "depo" if art == "depo" else "kost"
				button.SetUpVisual(IMG + "inv_%s_up.png" % kind)
				button.SetOverVisual(IMG + "inv_%s_over.png" % kind)
				button.SetDownVisual(IMG + "inv_%s_down.png" % kind)
				button.SetPosition(x, 315)
				button.Show()
		except Exception:
			pass
		# uiInventory.RefreshBagSlotWindow wndItemDelete'i bayraksiz okur; esya silme sistemi kapaliyken ozellik hic olusmaz -> varsayilan None
		if not hasattr(window, "wndItemDelete"):
			window.wndItemDelete = None
		# Ruzgarin Ayakkabisi acikken: envanterde gizlenir, ekipmandaki ozel (seker) yuvada mavi donen efektle gorunur, yuvaya dokununca kapanir
		self.ruzgarSlot = None
		self.ruzgarFx = None
		self.ruzgarTap = None
		try:
			base = window.GetChild("Equipment_Base")
			fx = ui.ExpandedImageBox()
			fx.SetParent(base)
			fx.AddFlag("not_pick")
			fx.LoadImage("mobile_ui/akravon/hud/akrfx_blue_1_00.png")
			fx.SetScale(50 / 64.0, 50 / 64.0)
			fx.Hide()
			self.ruzgarFx = fx
			tap = uiMobileStyle.Pressable(50, 50)
			tap.SetParent(base)
			tap.SetEvent(self.RuzgarKapat)
			tap.Hide()
			self.ruzgarTap = tap
			original = window.RefreshBagSlotWindow
			skin = self

			def RefreshBagSlotWindow(*args):
				result = original(*args)
				skin.HideRuzgarInInventory()
				return result
			window.RefreshBagSlotWindow = RefreshBagSlotWindow
		except Exception:
			pass

		# secili sayfa sekmesinde dalgalanan mavi dolgu (envanterdeki aktif slotla ayni dil)
		self.activeTab = 0
		self.tabW = (IW - 40) // PAGES
		self.waveBars = []
		for k in range(15):
			bar = uiMobileStyle.CreateBar(self.board, 0x00000000, 0, 0, 1, 2)
			self.waveBars.append(bar)
		self.keep.append(uiMobileStyle.CreateText(self.board, "Envanter", IW // 2, 13, GOLD, "Marcellus:22", "center"))
		self.keep.append(uiMobileStyle.CreateText(self.equipBoard, "Ekipman", EW // 2, 13, GOLD, "Marcellus:22", "center"))
		self.keep.append(uiMobileStyle.CreateBar(self.board, uiMobileStyle.Color(96, 74, 42), 16, 52, IW - 32, 1))
		self.keep.append(uiMobileStyle.CreateBar(self.equipBoard, uiMobileStyle.Color(96, 74, 42), 16, 52, EW - 32, 1))
		self.close = CloseButton(self.board, IW - 46, 12, ui.__mem_func__(window.Close))
		self.pull = PullTab(window, self.Toggle)
		self.handles = [DragHandle(self.board, self, IW - 60, 52), DragHandle(self.equipBoard, self, EW, 52)]
		for name in ("DSSButton", "Equipment_Tab_01", "Equipment_Tab_02", "Equipment_Tab_Strip"):
			try:
				window.GetChild(name).Hide()
			except Exception:
				pass
		try:
			window.GetChild("EditButton").Hide()	# envanter duzenleme dugmesi kaldirildi
		except Exception:
			pass
		self.slider = Slider(self)

		if hasattr(wndMgr, "SetSlotUnitSize"):
			wndMgr.SetSlotUnitSize(window.GetChild("ItemSlot").hWnd, CELL, CELL, 4)
			wndMgr.SetSlotUnitSize(window.GetChild("EquipmentSlot").hWnd, 50, 50, 4)

		# sekme yazilari: dugme ici yazi gorunmuyor - panoya ayri katman (dokunmayi engellemez)
		self.tabLabels = []
		tw = (IW - 40) // PAGES
		for i in xrange(len(getattr(window, "inventoryTab", []))):
			try:
				window.GetChild("Inventory_Tab_%02d_Print" % (i + 1)).Hide()
			except Exception:
				pass
			label = uiMobileStyle.CreateText(self.board, ("I", "II", "III", "IV", "V", "VI")[i], 20 + i * tw + tw // 2, 71, TAB_OFF, "Marcellus:20", "center")
			self.tabLabels.append(label)
		try:
			window.GetChild("Money").SetFontName("LatoBd:17")
			window.GetChild("Money").SetPackedFontColor(GOLD)
		except Exception:
			pass
		self.Layout()

	def RuzgarInventorySlot(self):
		import player
		for slot in range(player.INVENTORY_PAGE_SIZE * 5):
			if player.GetItemIndex(slot) == 72701:
				return slot
		return -1

	def HideRuzgarInInventory(self):
		# Ruzgar acikken envanter yuvasinda gorunmez (ekipmanda gorunur)
		try:
			import constInfo
			import player
			if not getattr(constInfo, "AKRAVON_RUZGAR_ACTIVE", 0):
				return
			slot = self.RuzgarInventorySlot()
			page = self.window.inventoryPageIndex
			local = slot - player.INVENTORY_PAGE_SIZE * page
			if slot >= 0 and 0 <= local < player.INVENTORY_PAGE_SIZE:
				self.window.wndItem.ClearSlot(local)
				self.window.wndItem.RefreshSlot()
		except Exception:
			pass

	def RuzgarKapat(self):
		import net
		slot = self.RuzgarInventorySlot()
		if slot >= 0:
			net.SendItemUsePacket(slot)

	def RefreshTabs(self, page):
		self.activeTab = page
		for (i, label) in enumerate(getattr(self, "tabLabels", [])):
			label.SetPackedFontColor(TAB_ON if i == page else TAB_OFF)

	def Toggle(self):
		self.opened = not self.opened
		SaveOpen(self.opened)
		self.pull.SetOpened(self.opened)

	def TabWave(self, now):
		tw = self.tabW
		x0 = 20 + self.activeTab * tw + 6
		for (k, bar) in enumerate(self.waveBars):
			bar.Hide()
			continue
			wave = 0.5 + 0.5 * math.sin(now * 4.0 - k * 0.55)
			wave2 = 0.5 + 0.5 * math.sin(now * 2.3 + k * 0.3 + 1.3)
			a = int(255 * (0.10 + 0.22 * (0.6 * wave + 0.4 * wave2)))
			bar.SetColor((a << 24) | (115 << 16) | (184 << 8) | 255)
			bar.SetPosition(x0, 62 + 6 + k * 2)
			bar.SetSize(tw - 12, 2)

	def Step(self):
		now = app.GetTime()
		self.TabWave(now)
		for handle in getattr(self, "handles", []):
			handle.Drag()
		gap = now - self.lastTime
		dt = max(0.0, min(0.1, gap))
		self.lastTime = now
		# AKRAVON: pencere her acildiginda sagdan kayarak girer (kapali kaldigi sure yeni acilis sayilir)
		if gap > 0.4 or not hasattr(self, "enterT"):
			self.enterT = 1.0	# AKRAVON: acilista kayma yok - envanter dogrudan yerinde belirir
			self.needY = True
		entering = self.enterT < 1.0
		if entering:
			self.enterT = min(1.0, self.enterT + dt / ENTER_TIME)
			self.Layout()
		cape = getattr(self.window, "wndBraveCape", None)
		if cape and cape.IsShow():
			cape.Hide()
		try:
			import constInfo
			import player
			now = app.GetTime(); equip = self.window.GetChild("EquipmentSlot")
			slots = (player.EQUIPMENT_SLOT_START + 7, player.EQUIPMENT_SLOT_START + 8)
			wanted = None
			if getattr(constInfo, "AKRAVON_RUZGAR_ACTIVE", 0) and self.equipBoard.IsShow():
				for slot in slots:
					if player.GetItemIndex(slot) == 0 or slot == self.ruzgarSlot:
						wanted = slot
						break
			if wanted is not None:
				if self.ruzgarSlot != wanted or now - getattr(self, "ruzgarTime", 0.0) > 0.3:
					equip.SetItemSlot(wanted, 72701, 0)
					equip.RefreshSlot()
					self.ruzgarSlot = wanted
					self.ruzgarTime = now
			elif self.ruzgarSlot is not None:
				if player.GetItemIndex(self.ruzgarSlot) == 0:
					equip.ClearSlot(self.ruzgarSlot)
					equip.RefreshSlot()
				self.ruzgarSlot = None
				self.ruzgarSet = False
			# mavi donen efekt + dokunma alani
			if self.ruzgarFx and self.ruzgarTap:
				if self.ruzgarSlot is not None:
					x = 120 if self.ruzgarSlot == player.EQUIPMENT_SLOT_START + 7 else 180
					self.ruzgarFx.LoadImage("mobile_ui/akravon/hud/akrfx_blue_1_%02d.png" % (int(now * 14) % 13))
					self.ruzgarFx.SetScale(50 / 64.0, 50 / 64.0)
					self.ruzgarFx.SetPosition(x, 180)
					self.ruzgarFx.Show()
					self.ruzgarTap.SetPosition(x, 180)
					self.ruzgarTap.Show()
				else:
					self.ruzgarFx.Hide()
					self.ruzgarTap.Hide()
		except Exception:
			pass
		# AKRAVON: kostum acikken ekipmanin yerine gecer (ust uste binmez)
		costume = getattr(self.window, "wndCostume", None)
		if costume and costume.IsShow():
			# AKRAVON: kostum paneli ekipmanin SOLUNDA, ayni hizada acilir (sigmazsa ekipmanin onunde)
			(ex, ey) = self.equipBoard.GetGlobalPosition()
			if not self.equipBoard.IsShow():
				(bx, by) = self.board.GetGlobalPosition()
				(ex, ey) = (bx - EW - GAP, by)
			left = ex - costume.GetWidth() - GAP
			costume.SetPosition(left if left >= 0 else ex, ey)
			self.costumeShown = True
		target = 1.0 if self.opened else 0.0
		if self.ratio == target:
			return
		delta = dt / SLIDE_TIME
		self.ratio = min(target, self.ratio + delta) if target > self.ratio else max(target, self.ratio - delta)
		self.Layout()

	def Layout(self):
		# envanter ekranda sabit; ekipman onun arkasindan sola kayar, kulp ekipmanin solunda gider
		t = self.ratio * self.ratio * (3 - 2 * self.ratio)	# yumusak baslangic / bitis
		hidden = int(round((1.0 - t) * (EW + GAP)))
		# envanterin sag kenari yerinde kalir (pencere suruklenmis olabilir)
		(x, y) = self.window.GetGlobalPosition()
		off = getattr(self, "enterOff", 0)
		e = getattr(self, "enterT", 1.0)
		newOff = int(round((1.0 - (1.0 - (1.0 - e) ** 3)) * (W + RIGHT_MARGIN + 40))) if e < 1.0 else 0
		self.enterOff = newOff
		right = x + self.window.GetWidth() - off + newOff
		if not getattr(self, "placed", False) or getattr(self, "needY", False):
			import uiMobileHud
			right = wndMgr.GetScreenWidth() - RIGHT_MARGIN - getattr(uiMobileHud, "SAFE_RIGHT", 0) + newOff	# kavisli kenar payi
			self.placed = True
		if getattr(self, "needY", False):
			y = max(0, (wndMgr.GetScreenHeight() - self.board.GetHeight()) // 2)	# ustten / alttan esit bosluk
			self.needY = False
		self.window.SetPosition(right - W + hidden, y)
		self.window.SetSize(W - hidden, self.board.GetHeight())	# pencere yalniz gorunen panel kadar: bosluga dokunmak tasima baslatmaz
		self.board.SetPosition(TAB + EW + GAP - hidden, 0)
		self.equipBoard.SetPosition(TAB, 0)
		self.pull.SetPosition(-6 if False else 0, PULL_Y)
		self.pull.SetOpened(self.opened)
		if t <= 0.0:
			self.equipBoard.Hide()
		else:
			self.equipBoard.Show()

	def Destroy(self):
		if self.slider:
			self.slider.Hide()
		self.slider = None
		self.keep = []
		self.window = None


def Apply(window):
	try:
		return _Apply(window)
	except Exception:
		import traceback
		open("akravon_hata.txt", "w").write(traceback.format_exc())
		raise


def _Apply(window):
	skin = Skin(window)
	window.akravonSkin = skin
	original = window.SetInventoryPage

	def SetInventoryPage(page):
		original(page)
		skin.RefreshTabs(page)

	window.SetInventoryPage = SetInventoryPage
	skin.RefreshTabs(0)
	return skin
