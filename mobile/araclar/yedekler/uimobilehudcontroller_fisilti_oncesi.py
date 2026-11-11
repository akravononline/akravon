from __future__ import division
import app
import mouseModule
import player
import wndMgr
import ui
import uiScriptLocale
import uiMobileCompat
import uiMobileHud
import uiMobileStyle
import uiMobileStyle
import uiMobileChrome
import uiVirtualJoystick
import uiAttackButton

from _weakref import proxy

STATE_HIDDEN = 0
STATE_MOVEMENT_ONLY = 1
STATE_FULL = 2

BLOCKING_WINDOW_NAMES = ("wndInventory", "wndCharacter", "wndSpecialStorage", "wndMessenger", "wndDragonSoul", "dlgExchange", "dlgShop", "wndSafebox", "wndMall")
QUICK_SLOT_REFRESH_INTERVAL = 0.5
EVENT_BUTTON_NAMES = ("wheelShowButton", "wheelEventShowButton", "publisherShowButton")
USE_AKR_CHAT = True	# sifirdan yazilan sohbet; False: onceki yollar
USE_CHAT_BAR = False	# True: uiakravonchatbar (deneme); False: orijinal sohbet + balon dugmesi
ORIGINAL_CHAT = True	# True: orijinal Metin2 sohbeti; False: mobil sohbet cubugu (geri almak icin tek satir)


class NullChatPart(object):
	def __getattr__(self, name):
		return lambda *args, **kwargs: None


class NullChatController(object):
	def __init__(self, interface):
		self.interface = proxy(interface)
		# telefonda Enter tusu yok: dokununca orijinal sohbet yazi kutusunu acan kucuk dugme
		self.button = uiMobileStyle.Pressable(52, 52)
		self.icon = ui.ExpandedImageBox()
		self.icon.SetParent(self.button)
		self.icon.AddFlag("not_pick")
		self.icon.LoadImage("mobile_ui/akravon/hud/chat_icon.png")
		scale = 38.0 / max(1, self.icon.GetWidth())
		self.icon.SetScale(scale, scale)
		self.icon.SetPosition(7, 7)
		self.icon.Show()
		self.button.SetEvent(ui.__mem_func__(self.__Open))
		self.button.Show()
		self.placed = False

	def __Open(self):
		try:
			window = self.interface.wndChat
			window.Show()
			window.OpenChat()
		except Exception:
			import dbg
			dbg.TraceError("NullChat.Open hata")

	def Restore(self):
		try:
			window = self.interface.wndChat
			if window:
				window.Show()
				window.Refresh()
				import uiAkravonChatStyle
				uiAkravonChatStyle.Apply(window, self.interface)
		except Exception:
			pass

	def Update(self):
		try:
			window = self.interface.wndChat
			import uiAkravonChatStyle
			uiAkravonChatStyle.Apply(window, self.interface)
			uiAkravonChatStyle.Update(window)
			# sohbet balonu: yazma kapaliyken joystigin altinda sol alt kosede; yazarken gizli
			if window.IsEditMode():
				self.button.Hide()
			else:
				self.button.SetPosition(8, wndMgr.GetScreenHeight() - 60)
				self.button.Show()
		except Exception:
			pass

	def Show(self):
		self.Restore()
		self.button.Show()

	def Hide(self):
		self.button.Hide()

	def Destroy(self):
		self.button.Destroy()
		self.icon = None
		self.button = None

	def __getattr__(self, name):
		return lambda *args, **kwargs: None


AFFECT_TOGGLE_NAME = "AffectShowHideBtn"
AFFECT_LABEL_NAME = "AffectShowHideLabel"
AFFECT_SWITCH_IMAGE = "mobile_ui/cynex/affect_switch_%s.png"
AFFECT_DESKTOP_IMAGES = ("refine/checkbox.tga", "refine/checked.tga")
AFFECT_DESKTOP_CHECK_Y = -4
AFFECT_LABEL_GAP = 6
AFFECT_DESKTOP_LABEL_COLOR = 0xFFFFFFFF
# Cynex: barbaros mini harita kisayollari (biyolog, uzaktan market, siralama) dokunmayla da calisiyor - gizlenmez
MINIMAP_HIDDEN_BUTTONS = ()
# AKRAVON: kare harita - masaustu halka dugmeleri gizlenir; yerine ortak kitten kucuk yuvarlak zoom dugmeleri (haritanin
# sag alt kosesi) ve haritanin altinda kisayol sirasi. Hepsi pencerenin icinde (ebeveyn disi secilmez).
MINIMAP_DESKTOP_BUTTONS = ("btnMucadele", "AtlasShowButton", "MiniMapHideButton", "ScaleUpButton", "ScaleDownButton", "MiniMapShowButton")
MINIMAP_FRAME_ASSET = "map_frame"
MINIMAP_BACKGROUND_NAME = "OpenWindowBGI"
MINIMAP_SIZE = (136, 137)
MINIMAP_ZOOM_SIZE = 30	# AKRAVON: mobil dokunma (24 kucuktu)
# (kit adi, mini harita metodu, x, y)
MINIMAP_ZOOM_BUTTONS = (("zoomout", "ScaleDown", 3, 104), ("zoomin", "ScaleUp", 103, 104))	# kare cercevenin sol alt / sag alt kosesi
# (mini harita ozniteligi, kit adi): sol -> sag
MENU_TILE_COLUMNS = 4


MENU_BASE_TILES = 3	# ok tablosundaki sabit karolar (mesaj, ayar, oto av); ustteki dortlu tablonun ilk satiri
MENU_SHORTCUT_LABELS = {"ranking" : "Sıralama", "biolog" : "Biyolog", "market" : "Pazar", "swords" : "Mücadele"}
MENU_TILE_LABEL_COLOR = uiMobileStyle.Color(250, 228, 176)
MENU_TILE_LABEL_HEIGHT = 24
MENU_TILE_CELL_WIDTH = 78
MENU_ROW_LABELS = ("Envanter", "Market", "Karakter", "Kapat")
MENU_BOARD_ENABLED = False	# tablo tahtasi (kapali: yuvarlak ikon + alt metin)
MENU_FLAT_ICONS = False	# tablo acikken yuvarlaksiz ikonlar
MENU_BOARD_PADDING = 16
MENU_BOARD_PATH = "mobile_ui/akravon/hud/menu_board_%s.png"
MENU_BOARD_CORNER = 12


class MenuBoard(ui.Window):
	# AKRAVON: envanter/ekipman penceresindeki tahta ile ayni cizim (9 parca, kose 12 px)
	def __init__(self):
		ui.Window.__init__(self)
		self.pieces = {}
		for key in ("c", "t", "b", "l", "r", "tl", "tr", "bl", "br"):
			image = ui.ExpandedImageBox()
			image.SetParent(self)
			image.AddFlag("not_pick")
			image.LoadImage(MENU_BOARD_PATH % key)
			image.Show()
			self.pieces[key] = image
		self.boardSizes = {}
		for (key, image) in self.pieces.items():
			self.boardSizes[key] = (max(1, image.GetWidth()), max(1, image.GetHeight()))

	def __del__(self):
		ui.Window.__del__(self)

	def Show(self):
		if MENU_BOARD_ENABLED:
			ui.Window.Show(self)

	def SetSize(self, width, height):
		ui.Window.SetSize(self, width, height)
		corner = MENU_BOARD_CORNER
		middleWidth = max(1, width - corner * 2)
		middleHeight = max(1, height - corner * 2)
		self.__Place("c", corner, corner, middleWidth, middleHeight)
		self.__Place("t", corner, 0, middleWidth, corner)
		self.__Place("b", corner, height - corner, middleWidth, corner)
		self.__Place("l", 0, corner, corner, middleHeight)
		self.__Place("r", width - corner, corner, corner, middleHeight)
		self.__Place("tl", 0, 0, corner, corner)
		self.__Place("tr", width - corner, 0, corner, corner)
		self.__Place("bl", 0, height - corner, corner, corner)
		self.__Place("br", width - corner, height - corner, corner, corner)

	def __Place(self, key, x, y, width, height):
		image = self.pieces[key]
		(assetWidth, assetHeight) = self.boardSizes[key]
		image.SetPosition(x, y)
		image.SetScale(float(width) / assetWidth, float(height) / assetHeight)


class MenuTile(ui.Window):
	# AKRAVON: acilir tablodaki hucre - yuvarlak ikon + altinda isim
	def __init__(self, kitName, size, label = ""):
		ui.Window.__init__(self)
		self.SetSize(MENU_TILE_CELL_WIDTH, size + MENU_TILE_LABEL_HEIGHT)
		self.button = uiMobileChrome.HudButton(size, kitName + ("_flat" if MENU_FLAT_ICONS else ""))
		self.button.SetParent(self)
		self.button.SetPosition((MENU_TILE_CELL_WIDTH - size) // 2, 0)
		self.button.Show()
		self.label = None
		if label:
			self.label = uiMobileStyle.CreateText(self, label, MENU_TILE_CELL_WIDTH // 2, size + 3, MENU_TILE_LABEL_COLOR, "LatoBd:15", "center", True)

	def SetEvent(self, event):
		self.button.SetEvent(event)

	def Destroy(self):
		self.button.Destroy()
		self.button = None
		self.label = None


marketPopup = [None]


def InMarketZone():
	# pazar yalniz Carsi haritasinda (metin2_map_privateshop) kurulur / acilir
	try:
		import background
		return "privateshop" in str(background.GetCurrentMapName()).lower()
	except Exception:
		return True


def MarketGuard(original):
	def Run():
		if InMarketZone():
			original()
			return
		import uiCommon
		if marketPopup[0] is None:
			marketPopup[0] = uiCommon.PopupDialog()
		marketPopup[0].SetText("Pazar yaln\xfdzca \xc7ar\xfd\xfe'ta kurulabilir. \xc7ar\xfd\xfe G\xf6revlisi ile gidebilirsin.")
		marketPopup[0].accceptButton.SetText("Tamam")
		marketPopup[0].Open()
	return Run


MINIMAP_SHORTCUTS = (("btnRank", "ranking"), ("btnBiyolog", "biolog"), ("btnUM", "market"))
MINIMAP_MARGIN = (14, 24)	# ust kenardan uzak; ustteki dugme satiriyla ayni hizada
# Canli Yayin afisi (publisher_live.png 175x150, afis y 35-110): mobilde olcekli, masaustu X yerine yuvarlak X
PUBLISHER_BUTTON_NAME = "publisherShowButton"
PUBLISHER_CLOSE_NAME = "publisherCloseButton"
PUBLISHER_SCALE = 0.7
PUBLISHER_SIZE = (175, 150)
PUBLISHER_CLOSE_ASSET = "close_28"
PUBLISHER_CLOSE_TOP = 10
TASKBAR_TOOLTIP_NAMES = ("tooltipHP", "tooltipSP", "tooltipST", "tooltipEXP")
# Cynex: gorev cubugu gizliyken beceri yayi da ayni olaylari alsin
TASKBAR_HOOKS = (("RefreshQuickSlot", "RefreshQuickSlot"), ("OnUseSkill", "OnUseSkill"), ("OnActivateSkill", "OnActivateSkill"), ("OnDeactivateSkill", "OnDeactivateSkill"))


def _MakeTaskBarHook(function, taskBar, arc, arcName):
	def Hooked(*args):
		result = function(taskBar, *args)
		try:
			getattr(arc, arcName)(*args)
		except ReferenceError:
			pass
		return result
	return Hooked


class MobileHudController(object):
	def __init__(self, gameWindow, interface):
		self.editResumed = False
		self.interface = proxy(interface)
		self.gameWindow = proxy(gameWindow)
		self.state = None
		self.arcVisible = None
		self.arcAttached = False
		self.floatingPositions = {}
		self.hiddenMinimapButtons = {}
		self.minimapDesktopButtons = {}
		self.minimapButtons = []
		self.minimapTapWindow = None
		self.publisherStyled = None
		self.affectToggleDesktop = None
		self.nextQuickSlotRefresh = 0.0
		self.menuHighlights = {}

		layout = uiMobileHud.GetTouchLayout(wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())
		self.eventRow = layout["eventRow"]
		self.affectTogglePosition = layout["affectToggle"]

		(x, y, size) = layout["joystick"]
		zone = layout["joystickZone"] if uiMobileHud.IsDynamicJoystick() else None
		self.joystick = uiVirtualJoystick.VirtualJoystick(gameWindow, size, zone, layout["runLock"])
		self.joystick.Place(x, y)
		uiMobileHud.SetController(proxy(self))

		(x, y, size) = layout["attack"]
		self.attackButton = uiAttackButton.AttackButton(size)
		self.attackButton.SetPosition(x, y)

		self.skillArc = uiMobileHud.SkillArc(interface.wndTaskBar, layout)
		self.taskBarHooks = []
		self.__HookTaskBar(interface.wndTaskBar)

		(utilityPositions, utilitySize) = layout["utilities"]
		self.utilities = uiMobileChrome.ButtonRow(uiMobileChrome.UTILITY_BUTTONS, utilitySize, layout.get("utilitySizes"))
		self.utilities.SetEvent("pickup", uiMobileChrome.RunPickup)
		self.utilities.SetEvent("autohunt", self.__ToggleAutoHunt)	# AKRAVON: yeni mobil oto av paneli
		autoHuntButton = self.utilities.GetButton("autohunt")
		if autoHuntButton and hasattr(autoHuntButton, "EnablePressRipple"):
			autoHuntButton.EnablePressRipple((0.45, 1.0, 0.45))	# beceri slotlariyla ayni basma efekti, yesil
		self.utilities.SetEvent("mount", uiMobileChrome.RunMount)
		self.utilities.PlaceVertical(utilityPositions)
		self.utilityOrigin = utilityPositions[0]

		self.statusBar = uiMobileChrome.StatusBar()
		self.__RedirectSystemDialog(interface)
		self.statusBar.SetPosition(*layout["statusBar"])
		self.statusBar.SetEvent(self.__GetInterfaceEvent("ToggleCharacterWindowStatusPage"))

		(menuX, menuY, menuSize, menuGap) = layout["menuBar"]
		self.menuMoreOpen = False
		import uiMobileOptions
		uiMobileOptions.ApplyStartupQuality()	# AKRAVON: cihaza gore otomatik grafik kalitesi
		self.menuExtra = []
		self.menuGeometry = (menuX, menuY, menuSize, menuGap, len(uiMobileChrome.MENU_BUTTONS))
		self.menuBoard = MenuBoard()
		self.menuBoard.AddFlag("not_pick")
		self.menuBoard.Hide()
		self.menuBar = uiMobileChrome.ButtonRow(uiMobileChrome.MENU_BUTTONS, menuSize)
		self.menuBar.SetEvent("inventory", self.__GetInterfaceEvent("ToggleInventoryWindow"))
		self.menuBar.SetEvent("itemshop", self.__GetInterfaceEvent("ItemShopSendOpen"))
		self.menuBar.SetEvent("character", self.__GetInterfaceEvent("ToggleCharacterWindowStatusPage"))
		self.menuBar.SetEvent("more", self.__ToggleMenuMore)
		self.menuBar.PlaceHorizontal(menuX, menuY, menuSize, menuGap)
		self.guildTile = None
		self.guildShown = False

		import uiAkravon
		self.menuMore = ui.Window("UI")
		self.menuMore.AddFlag("float")
		self.menuMore.AddFlag("not_pick")
		self.menuMore.Hide()
		self.menuBoard.Hide()
		self.menuRowLabels = []
		for (index, text) in enumerate(MENU_ROW_LABELS):
			self.menuRowLabels.append(uiMobileStyle.CreateText(self.menuMore, text, index * (menuSize + menuGap) + menuSize // 2, menuSize + 3, MENU_TILE_LABEL_COLOR, "LatoBd:15", "center", True))
		self.menuTiles = []
		# AKRAVON: ok'a basinca acilan tablo - ustteki dortlu dahil tum butonlar isimleriyle
		self.__AddMenuTile("messenger", self.__GetInterfaceEvent("ToggleMessenger"), "Mesajlar")
		self.__AddMenuTile("system", self.__ToggleSystemMenu, "Ayarlar")
		self.__AddMenuTile("autohunt", self.__OpenAutoHuntSettings, "Oto Av")

		if USE_AKR_CHAT:
			# AKRAVON: sifirdan yazilan mobil sohbet (uiakravonchat.py); hata olursa eski yola dusulur
			self.chatToggle = NullChatPart()
			try:
				import uiAkravonChat
				self.chatController = uiAkravonChat.Controller(interface)
				self.chatController.SetAnchor(layout["joystick"])
			except Exception:
				import dbg
				dbg.TraceError("uiAkravonChat kurulamadi")
				self.chatController = NullChatController(interface)
			self.chatController.Restore()
		elif ORIGINAL_CHAT:
			# AKRAVON deneme: oyunun kendi (orijinal Metin2) sohbet penceresi; mobil sohbet cubugu / paneli kurulmaz
			self.chatToggle = NullChatPart()
			try:
				if not USE_CHAT_BAR:
					raise RuntimeError("chat bar kapali")
				import uiAkravonChatBar
				self.chatController = uiAkravonChatBar.Controller(interface)
			except Exception:
				import dbg
				import exception
				dbg.TraceError("uiAkravonChatBar kurulamadi")
				self.chatController = NullChatController(interface)
			self.chatController.Restore()
		else:
			(pillX, pillY, pillWidth, pillHeight) = layout["chatPill"]
			self.chatToggle = uiMobileChrome.ChatToggle(pillWidth, pillHeight)
			self.chatToggle.SetPosition(pillX, pillY)
			self.chatController = uiMobileChrome.ChatController(interface, self.chatToggle)
			self.chatController.Place()
			self.chatController.Collapse()

		self.notifications = None
		gameButtonWindow = getattr(interface, "wndGameButton", None)
		if gameButtonWindow:
			self.notifications = uiMobileChrome.NotificationLayout(gameButtonWindow)
			self.notifications.Apply(uiMobileHud.SCREEN_EDGE, layout["notificationY"])

		self.affectShower = None
		self.affectShowerPosition = None
		affectShower = getattr(gameWindow, "affectShower", None)
		if affectShower:
			self.affectShower = proxy(affectShower)
			self.affectShowerPosition = affectShower.GetGlobalPosition()
			affectShower.SetPosition(*layout["affectShower"])

		self.__ArrangeFloatingWindows()
		import uiAkravonQuest
		uiAkravonQuest.Install(interface)	# AKRAVON: soldaki gorev mektuplari yerine gorev dugmesi + tablo

	def __HookTaskBar(self, taskBar):
		if not taskBar:
			return
		taskBarProxy = proxy(taskBar)
		arcProxy = proxy(self.skillArc)
		for (name, arcName) in TASKBAR_HOOKS:
			method = getattr(taskBar.__class__, name, None)
			function = getattr(method, "im_func", None)
			if function is None:
				continue
			setattr(taskBar, name, _MakeTaskBarHook(function, taskBarProxy, arcProxy, arcName))
			self.taskBarHooks.append(name)
		self.taskBarProxy = taskBarProxy

	def __UnhookTaskBar(self):
		taskBar = getattr(self, "taskBarProxy", None)
		for name in getattr(self, "taskBarHooks", []):
			try:
				if name in taskBar.__dict__:
					del taskBar.__dict__[name]
			except (ReferenceError, AttributeError):
				pass
		self.taskBarHooks = []
		self.taskBarProxy = None

	@staticmethod
	def __ToggleSettings():
		import uiMobileOptions
		uiMobileOptions.Open(3)

	@staticmethod
	def __ToggleAutoHunt():
		import uiAkravonAutoHunt
		uiAkravonAutoHunt.ToggleHunt()

	@staticmethod
	def __OpenAutoHuntSettings():
		import uiAkravonAutoHunt
		uiAkravonAutoHunt.Toggle()

	def __ToggleSystemMenu(self):
		import uiMobileSystem
		uiMobileSystem.Toggle(self.interface)

	def __RedirectSystemDialog(self, interface):
		# AKRAVON: ESC / eski "Sistem" penceresi yerine yeni ayarlar menusu acilir
		self.systemDialog = None
		dialog = getattr(interface, "dlgSystem", None)
		if not dialog:
			return
		try:
			weakInterface = proxy(interface)
		except TypeError:
			weakInterface = interface

		def OpenMobileMenu():
			import uiMobileSystem
			uiMobileSystem.Open(weakInterface)

		dialog.OpenDialog = OpenMobileMenu
		self.systemDialog = proxy(dialog)

	def __RestoreSystemDialog(self):
		dialog = getattr(self, "systemDialog", None)
		self.systemDialog = None
		if not dialog:
			return
		try:
			if "OpenDialog" in dialog.__dict__:
				del dialog.__dict__["OpenDialog"]
		except (ReferenceError, AttributeError):
			pass

	def __AddMenuTile(self, kitName, event, label = ""):
		tile = MenuTile(kitName, self.menuGeometry[2], label)
		tile.SetParent(self.menuMore)
		if event:
			tile.SetEvent(event)
		tile.Show()
		self.menuTiles.append(tile)
		self.__LayoutMenuTiles()
		return tile

	def __LayoutMenuTiles(self):
		(menuX, menuY, menuSize, menuGap, count) = self.menuGeometry
		cols = MENU_TILE_COLUMNS
		step = menuSize + menuGap
		rowHeight = menuSize + MENU_TILE_LABEL_HEIGHT + 4
		shownTiles = [tile for tile in self.menuTiles if tile is not self.guildTile or self.guildShown]
		for tile in self.menuTiles:
			if tile is self.guildTile and not self.guildShown:
				tile.Hide()
		extraRows = (len(shownTiles) + cols - 1) // cols
		barWidth = cols * step - menuGap
		pad = MENU_BOARD_PADDING
		for (i, tile) in enumerate(shownTiles):
			if tile is self.guildTile:
				tile.Show()
			tile.SetPosition((i % cols) * step + menuSize // 2 - MENU_TILE_CELL_WIDTH // 2, (1 + i // cols) * rowHeight)
		height = (1 + extraRows) * rowHeight
		self.menuMore.SetSize(barWidth, height)
		self.menuMore.SetPosition(menuX, menuY)
		self.menuBoard.SetPosition(menuX - pad, menuY - pad)
		self.menuBoard.SetSize(barWidth + pad * 2, height + pad * 2 - 4)

	def __OpenGuildWindow(self):
		# AKRAVON: lonca penceresi ekranin ortasinda acilir (eskiden sol ust kosede kaliyordu)
		toggle = getattr(self.interface, "ToggleGuildWindow", None)
		if toggle:
			toggle()
		window = getattr(self.interface, "wndGuild", None)
		if window and window.IsShow():
			window.SetPosition(max(0, (wndMgr.GetScreenWidth() - window.GetWidth()) // 2), max(0, (wndMgr.GetScreenHeight() - window.GetHeight()) // 2))
			window.SetTop()

	def __ToggleMenuMore(self):
		self.menuMoreOpen = not self.menuMoreOpen
		button = self.menuBar.GetButton("more")
		if button:
			button.SetIcon(("less_flat" if MENU_FLAT_ICONS else "less") if self.menuMoreOpen else "more")
		for name in ("inventory", "itemshop", "character"):
			flatButton = self.menuBar.GetButton(name)
			if flatButton:
				flatButton.SetIcon(name + "_flat" if (self.menuMoreOpen and MENU_FLAT_ICONS) else name)
		if self.menuMoreOpen and self.state != STATE_HIDDEN:
			self.menuMore.Show()
			self.menuBoard.Show()
			self.menuMore.SetTop()	# tablo beceri slotlarinin ustunde
		else:
			self.menuMore.Hide()
			self.menuBoard.Hide()
		for extra in self.menuExtra:
			if self.menuMoreOpen and self.state != STATE_HIDDEN:
				extra.Show()
			else:
				extra.Hide()

	def __GetInterfaceEvent(self, name):
		method = getattr(self.interface, name, None)
		if method is None:
			return None
		return ui.__mem_func__(method)

	def Destroy(self, restoreDesktop):
		uiMobileHud.SetController(None)
		self.__UnhookTaskBar()
		self.__RestoreSystemDialog()

		if restoreDesktop:
			self.__RestoreDesktop()
		else:
			import uiMobileSettings
			uiMobileSettings.Hide()

		self.skillArc.Destroy()
		self.attackButton.Destroy()
		self.joystick.Destroy()
		import uiMobileSystem
		uiMobileSystem.Destroy()
		if not restoreDesktop:	# AKRAVON: joystick / saldiri boyutu degisince HUD yeniden kurulur; ayar penceresi acik kalsin
			import uiMobileOptions
			uiMobileOptions.Destroy()
		import uiAkravonAutoHunt
		uiAkravonAutoHunt.Destroy()
		import uiAkravonQuest
		uiAkravonQuest.Destroy()
		self.utilities.Destroy()
		self.menuBar.Destroy()
		for tile in self.menuTiles:
			tile.Destroy()
		self.menuTiles = []
		self.menuMore.Hide()
		self.menuBoard.Hide()
		self.menuMore = None
		self.statusBar.Destroy()
		self.chatController.Destroy()
		self.chatToggle.Destroy()

		self.skillArc = None
		self.attackButton = None
		self.joystick = None
		self.utilities = None
		self.menuBar = None
		self.menuMore = None
		self.statusBar = None
		self.chatController = None
		self.chatToggle = None
		self.notifications = None
		self.affectShower = None
		self.floatingPositions = {}
		self.gameWindow = None
		self.interface = None

	def __RestoreDesktop(self):
		try:
			taskBar = self.interface.wndTaskBar
		except ReferenceError:
			return
		if taskBar:
			taskBar.Show()
		self.chatController.Restore()
		if self.notifications:
			self.notifications.Restore()
		if self.affectShower and self.affectShowerPosition:
			try:
				self.affectShower.SetPosition(*self.affectShowerPosition)
			except ReferenceError:
				pass
		for (window, position) in self.floatingPositions.values():
			try:
				window.SetPosition(*position)
			except ReferenceError:
				pass
		for button in self.hiddenMinimapButtons.values():
			try:
				button.Show()
			except ReferenceError:
				pass
		self.hiddenMinimapButtons = {}
		self.__RestoreMinimapButtons()
		self.__RestorePublisher()
		self.__RestoreAffectToggle()

	def __GetGameWindowChild(self, name):
		try:
			return getattr(self.gameWindow, name, None)
		except ReferenceError:
			return None

	def __IsTracked(self, name, window):
		stored = self.floatingPositions.get(name)
		if not stored:
			return False
		try:
			return stored[0] == window
		except ReferenceError:
			return False

	def __MoveFloatingWindow(self, name, window, x, y):
		if not self.__IsTracked(name, window):
			self.floatingPositions[name] = (proxy(window), tuple(window.GetLocalPosition()))
		if tuple(window.GetLocalPosition()) != (x, y):
			window.SetPosition(x, y)

	def __ArrangeFloatingWindows(self):
		self.__StylePublisher()
		(rightX, rowY) = self.eventRow
		for name in EVENT_BUTTON_NAMES:
			window = self.__GetGameWindowChild(name)
			if not window or not window.IsShow():
				continue
			rightX -= window.GetWidth()
			self.__MoveFloatingWindow(name, window, rightX, rowY)
			rightX -= uiMobileHud.WINDOW_GAP

		toggle = self.__GetGameWindowChild(AFFECT_TOGGLE_NAME)
		if toggle:
			self.__StyleAffectToggle(toggle)
			self.__MoveFloatingWindow(AFFECT_TOGGLE_NAME, toggle, *self.affectTogglePosition)

		self.__HideMinimapButtons()
		self.__BuildMinimapButtons()
		self.__ArrangeWhisperButtons()

	def __ArrangeWhisperButtons(self):
		# Cynex: fisilti/[DUYURU] dugmeleri sag sutundaki Topla/Oto Av/Binek ile cakismasin - solunda dizilir
		buttons = getattr(self.interface, "whisperButtonList", None)
		if not buttons:
			return
		(utilityX, utilityY) = self.utilityOrigin
		y = utilityY
		for button in buttons:
			if "DUYURU" in str(getattr(button, "name", "")).upper():
				button.Hide()	# AKRAVON: duyuru zarfi yok
				continue
			x = utilityX - button.GetWidth() - uiMobileHud.WINDOW_GAP * 2
			if button.GetLocalPosition() != (x, y):
				button.SetPosition(x, y)
			y += button.GetHeight() + uiMobileHud.WINDOW_GAP

	def __StyleAffectToggle(self, toggle):
		if self.affectToggleDesktop:
			return
		label = self.__GetGameWindowChild(AFFECT_LABEL_NAME)
		if not label or not toggle.backgroundImage or not toggle.checkImage:
			return
		self.affectToggleDesktop = (proxy(toggle), proxy(label), label.GetText(), tuple(label.GetLocalPosition()), (toggle.GetWidth(), toggle.GetHeight()))
		toggle.backgroundImage.LoadImage(AFFECT_SWITCH_IMAGE % "off")
		toggle.checkImage.LoadImage(AFFECT_SWITCH_IMAGE % "on")
		toggle.checkImage.SetPosition(0, 0)
		switchWidth = toggle.backgroundImage.GetWidth()
		switchHeight = toggle.backgroundImage.GetHeight()
		label.SetText(uiScriptLocale.MOBILE_AFFECT_TOGGLE)
		label.SetPackedFontColor(uiMobileStyle.GOLD_COLOR)
		label.SetPosition(switchWidth + AFFECT_LABEL_GAP, (switchHeight - 14) // 2)
		toggle.SetSize(switchWidth + AFFECT_LABEL_GAP + label.GetTextSize()[0], switchHeight)

	def __RestoreAffectToggle(self):
		if not self.affectToggleDesktop:
			return
		(toggle, label, text, position, size) = self.affectToggleDesktop
		self.affectToggleDesktop = None
		try:
			toggle.backgroundImage.LoadImage(AFFECT_DESKTOP_IMAGES[0])
			toggle.checkImage.LoadImage(AFFECT_DESKTOP_IMAGES[1])
			toggle.checkImage.SetPosition(0, AFFECT_DESKTOP_CHECK_Y)
			toggle.SetSize(*size)
			label.SetText(text)
			label.SetPackedFontColor(AFFECT_DESKTOP_LABEL_COLOR)
			label.SetPosition(*position)
		except ReferenceError:
			pass

	def __HideMinimapButtons(self):
		miniMap = getattr(self.interface, "wndMiniMap", None)
		if not miniMap:
			return
		for name in MINIMAP_HIDDEN_BUTTONS:
			button = getattr(miniMap, name, None)
			if button and button.IsShow():
				button.Hide()
				self.hiddenMinimapButtons[name] = proxy(button)

	def __BuildMinimapButtons(self):
		miniMap = getattr(self.interface, "wndMiniMap", None)
		if not miniMap:
			return
		for name in MINIMAP_DESKTOP_BUTTONS:
			button = getattr(miniMap, name, None)
			if button and button.IsShow():
				button.Hide()
				self.minimapDesktopButtons[name] = proxy(button)
		for (attribute, kitName) in MINIMAP_SHORTCUTS:
			button = getattr(miniMap, attribute, None)
			if button and button.IsShow():
				button.Hide()
				self.minimapDesktopButtons[attribute] = proxy(button)

		openWindow = getattr(miniMap, "OpenWindow", None)
		if not self.minimapButtons and openWindow:
			self.__StyleMinimap(miniMap, openWindow)

		# haritaya dokunus = buyuk harita (dugmeler kardes: dokunuslari kendilerinde kalir)
		mapWindow = getattr(miniMap, "MiniMapWindow", None)
		if mapWindow and not self.minimapTapWindow:
			toggle = ui.__mem_func__(miniMap.ToggleAtlasWindow)
			import uiAkravonAtlas
			uiAkravonAtlas.Install(miniMap)	# buyuk harita ortada acilir, dokununca yol tarifi + oto yuruyus
			mapWindow.SetTouchTapEvent(lambda x, y: toggle())
			self.minimapTapWindow = proxy(mapWindow)

	def __StyleMinimap(self, miniMap, openWindow):
		(width, height) = MINIMAP_SIZE
		try:
			background = miniMap.GetChild(MINIMAP_BACKGROUND_NAME)
			background.Hide()
			self.minimapBackground = proxy(background)
		except Exception:
			self.minimapBackground = None

		self.minimapFrame = uiMobileChrome.HudImage(openWindow, MINIMAP_FRAME_ASSET, 0, 0)
		# AKRAVON: haritanin altindaki sunucu adi ("AKRAVON, CH1") kaldirildi
		for hiddenName in ("ServerInfo", "Saat"):	# sunucu adi ve tarih / saat haritanin altindan kaldirildi
			try:
				hiddenText = miniMap.GetChild(hiddenName)
				hiddenText.Hide()
				hiddenText.Show = lambda *args: None	# uiminimap her karede yeniden gosteriyor
			except Exception:
				pass

		# kenarlara yapismasin: sag ve ustten bosluk
		self.minimapOrigin = (miniMap, tuple(miniMap.GetLocalPosition()))
		miniMap.SetPosition(wndMgr.GetScreenWidth() - width - MINIMAP_MARGIN[0] - int(uiMobileHud.SAFE_RIGHT * uiMobileHud.RIGHT_SAFE_SHARE), MINIMAP_MARGIN[1] + uiMobileHud.SAFE_TOP)	# AKRAVON: guvenli alan

		for (kitName, methodName, x, y) in MINIMAP_ZOOM_BUTTONS:
			method = getattr(miniMap, methodName, None)
			if not method:
				continue
			button = uiMobileChrome.HudButton(MINIMAP_ZOOM_SIZE, kitName)
			button.SetParent(openWindow)
			button.SetPosition(x, y)
			button.SetEvent(ui.__mem_func__(method))
			button.Show()
			self.minimapButtons.append(button)

		# haritayi gizle / goster
		closeWindow = getattr(miniMap, "CloseWindow", None)
		for (parent, kitName, methodName, x, y) in ((openWindow, "close", "HideMiniMap", 103, 3), (closeWindow, "more", "ShowMiniMap", width - MINIMAP_ZOOM_SIZE - 6, 6)):
			method = getattr(miniMap, methodName, None)
			if not parent or not method:
				continue
			button = uiMobileChrome.HudButton(MINIMAP_ZOOM_SIZE, kitName)
			button.SetParent(parent)
			button.SetPosition(x, y)
			button.SetEvent(ui.__mem_func__(method))
			button.Show()
			self.minimapButtons.append(button)

		# kisayollar (siralama, biyolog, pazar, mucadele) haritanin altindan kalkti: acilir menude durur
		(menuX, menuY, menuSize, menuGap, count) = self.menuGeometry
		shortcuts = [(attribute, kitName) for (attribute, kitName) in MINIMAP_SHORTCUTS if getattr(miniMap, attribute, None)]
		for (attribute, kitName) in shortcuts:
			callback = ui.__mem_func__(getattr(miniMap, attribute).CallEvent)
			if kitName == "market":
				callback = MarketGuard(callback)
			self.__AddMenuTile(kitName, callback, MENU_SHORTCUT_LABELS.get(kitName, ""))
		# AKRAVON: Lonca - Pazar'in yaninda; yalniz bir loncadaysan gorunur
		self.guildTile = self.__AddMenuTile("guild", self.__OpenGuildWindow, "Lonca")
		self.__LayoutMenuTiles()

	def __RestoreMinimapButtons(self):
		for button in self.minimapButtons:
			button.Hide()
			button.Destroy()
		self.minimapButtons = []
		for tile in self.menuTiles[MENU_BASE_TILES:]:
			tile.Hide()
			tile.Destroy()
		self.menuTiles = self.menuTiles[:MENU_BASE_TILES]
		self.guildTile = None
		self.__LayoutMenuTiles()
		frame = getattr(self, "minimapFrame", None)
		if frame:
			frame.Hide()
			self.minimapFrame = None
		background = getattr(self, "minimapBackground", None)
		if background:
			try:
				background.Show()
			except ReferenceError:
				pass
			self.minimapBackground = None
		origin = getattr(self, "minimapOrigin", None)
		if origin:
			try:
				origin[0].SetPosition(*origin[1])
			except ReferenceError:
				pass
			self.minimapOrigin = None
		for button in self.minimapDesktopButtons.values():
			try:
				button.Show()
			except ReferenceError:
				pass
		self.minimapDesktopButtons = {}
		if self.minimapTapWindow:
			try:
				self.minimapTapWindow.eventTouchTap = None
				if "OnTouchTap" in self.minimapTapWindow.__dict__:
					del self.minimapTapWindow.OnTouchTap
			except ReferenceError:
				pass
			self.minimapTapWindow = None

	def __StylePublisher(self):
		button = self.__GetGameWindowChild(PUBLISHER_BUTTON_NAME)
		if not button:
			return
		if self.publisherStyled:
			try:
				if self.publisherStyled[0] == button:
					return
			except ReferenceError:
				pass
			self.__RestorePublisher()
		desktopClose = self.__GetGameWindowChild(PUBLISHER_CLOSE_NAME)
		(width, height) = PUBLISHER_SIZE
		scaledWidth = int(width * PUBLISHER_SCALE)
		scaledHeight = int(height * PUBLISHER_SCALE)
		button.SetButtonScale(PUBLISHER_SCALE, PUBLISHER_SCALE)
		button.SetSize(scaledWidth, scaledHeight)
		close = uiMobileStyle.StateImageButton(PUBLISHER_CLOSE_ASSET)
		close.SetParent(button)
		close.SetPosition(scaledWidth - close.GetWidth(), PUBLISHER_CLOSE_TOP)
		if desktopClose:
			desktopClose.Hide()
			close.SetEvent(ui.__mem_func__(desktopClose.CallEvent))
		close.Show()
		self.publisherStyled = (proxy(button), proxy(desktopClose) if desktopClose else None, close)

	def __RestorePublisher(self):
		if not self.publisherStyled:
			return
		(button, desktopClose, close) = self.publisherStyled
		self.publisherStyled = None
		close.Hide()
		close.Destroy()
		try:
			button.SetButtonScale(1.0, 1.0)
			button.SetSize(*PUBLISHER_SIZE)
			if desktopClose and button.IsShow():
				desktopClose.Show()
		except ReferenceError:
			pass

	def __IsWindowOpen(self, name):
		window = self.interface.__dict__.get(name)
		if not window:
			return False
		try:
			return bool(window.IsShow())
		except Exception:
			return False

	def __IsCombatBlocked(self):
		for name in BLOCKING_WINDOW_NAMES:
			if self.__IsWindowOpen(name):
				return True
		try:
			import uiAkravonAtlas
			if uiAkravonAtlas.atlasWindow and uiAkravonAtlas.atlasWindow.IsShow():
				return True	# buyuk harita acikken beceri yuvalari / saldiri / yardimcilar gizlenir
		except Exception:
			pass
		return False

	def __GetState(self):
		if uiMobileHud.IsCameraMode():
			return STATE_HIDDEN
		if player.IsObserverMode() or self.__IsCombatBlocked():
			return STATE_MOVEMENT_ONLY
		return STATE_FULL

	def __ApplyState(self, state):
		if state == STATE_HIDDEN:
			self.joystick.Stop()
			self.joystick.Hide()
			self.statusBar.Hide()
			self.menuBar.Hide()
			self.menuMore.Hide()
			self.menuBoard.Hide()
			for extra in self.menuExtra:
				extra.Hide()
			self.chatController.Hide()
			import uiAkravonQuest
			uiAkravonQuest.SetVisible(False)
		else:
			import uiAkravonQuest
			uiAkravonQuest.SetVisible(True)
			self.joystick.Show()
			self.statusBar.Show()
			self.menuBar.Show()
			for extra in self.menuExtra:
				if self.menuMoreOpen:
					extra.Show()
				else:
					extra.Hide()
			if self.menuMoreOpen:
				self.menuMore.Show()
				self.menuBoard.Show()
			else:
				self.menuMore.Hide()
				self.menuBoard.Hide()
			self.chatController.Show()

		if state == STATE_FULL:
			self.attackButton.Show()
			self.utilities.Show()
		else:
			self.attackButton.Stop()
			self.attackButton.Hide()
			self.utilities.Hide()

	# Hizli erisim yayi C paneli acikken de gorunur (beceri uzun basisla yaya birakilir) ve imlece nesne takiliyken
	# her durumda (envanterden iksir); saldiri dugmesi ve yardimcilar yine gizli. O durumlarda yay ustte: 16:9
	# ekranda C panelinin altinda kalmasin.
	def __RefreshSkillArc(self, state):
		attached = bool(mouseModule.mouseController.isAttached())
		if state == STATE_FULL:
			wanted = True
		elif state == STATE_HIDDEN or player.IsObserverMode():
			wanted = False
		else:
			wanted = self.__IsWindowOpen("wndCharacter")	# AKRAVON: esya surukleyince hizli slotlar one gelmez (slot duzenleme ayri)

		raiseArc = wanted and state != STATE_FULL and (not self.arcVisible or (attached and not self.arcAttached))
		self.arcAttached = attached
		if wanted != self.arcVisible:
			self.arcVisible = wanted
			if wanted:
				self.skillArc.Show()
			else:
				self.skillArc.Hide()
		if raiseArc:
			self.skillArc.SetTop()
			# AKRAVON: acik pencereler beceri yuvalarinin ustunde kalsin (yuvalar ust katmana cikinca pencerelerin onune geciyordu)
			for name in ("wndInventory", "wndCharacter", "wndSafebox", "wndMall", "wndGuild", "wndMessenger", "wndDragonSoul"):
				try:
					window = getattr(self.interface, name, None)
					if window and window.IsShow():
						window.SetTop()
				except Exception:
					pass

	def __HideTaskBar(self):
		taskBar = self.interface.wndTaskBar
		if not taskBar:
			return
		if taskBar.IsShow():
			taskBar.Hide()
		for name in TASKBAR_TOOLTIP_NAMES:
			tooltip = getattr(taskBar, name, None)
			if tooltip and tooltip.IsShow():
				tooltip.Hide()

	def __RefreshMenuHighlights(self):
		button = self.utilities.GetButton("autohunt")
		if button:
			import uiAkravonAutoHunt
			running = bool(uiAkravonAutoHunt.IsRunning())
			if self.menuHighlights.get("autohunt") != running:
				self.menuHighlights["autohunt"] = running
				button.SetRunning(running)
		for (buttonName, windowName) in (("inventory", "wndInventory"), ("character", "wndCharacter")):
			button = self.menuBar.GetButton(buttonName)
			if not button:
				continue
			isOpen = self.__IsWindowOpen(windowName)
			if self.menuHighlights.get(buttonName) != isOpen:
				self.menuHighlights[buttonName] = isOpen
				button.SetHighlight(isOpen)

	def __RefreshPointAlert(self):
		if self.notifications:
			self.notifications.HidePointButtons()
		button = self.menuBar.GetButton("character")
		if button:
			button.SetAlert(player.GetStatus(player.STAT) > 0 or player.GetStatus(player.SKILL_ACTIVE) > 0)

	def __RefreshLinkTooltip(self):
		# AKRAVON: sohbetteki esya karti acikken ekranda herhangi bir yere ilk dokunus karti kapatir
		tooltip = getattr(self.interface, "hyperlinkItemTooltip", None)
		if not tooltip:
			return
		try:
			shown = tooltip.IsShow()
		except ReferenceError:
			return
		touching = app.GetTouchCount() > 0 if hasattr(app, "GetTouchCount") else False
		if not shown:
			self.linkTooltipArmed = False
			return
		if not touching:
			self.linkTooltipArmed = True
		elif getattr(self, "linkTooltipArmed", False):
			self.linkTooltipArmed = False
			tooltip.Hide()

	# ------------------------------------------------------------ HUD duzenleme (uiMobileHudEdit)

	def GetEditRects(self):
		# gercek HUD ogelerinin su anki dikdortgenleri: (anahtar, x, y, genislik, yukseklik)
		layout = uiMobileHud.GetTouchLayout(wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())
		rects = []
		(x, y, size) = layout["joystick"]
		rects.append(("joystick", x, y, size, size))
		(x, y, size) = layout["attack"]
		rects.append(("attack", x, y, size, size))
		(positions, utilityBase) = layout["utilities"]
		for (name, (ux, uy)) in zip(uiMobileHud.UTILITY_NAMES, positions):
			usize = layout["utilitySizes"][name]
			rects.append((name, ux, uy, usize, usize))
		arc = self.skillArc
		for index in xrange(arc.count):
			(sx, sy) = arc.slots[index].GetLocalPosition()
			rects.append(("skill%d" % index, sx, sy, arc.slots[index].GetWidth(), arc.slots[index].GetHeight()))
		(pagePosition, pageSize) = layout["pageButton"]
		rects.append(("page", pagePosition[0], pagePosition[1], pageSize, pageSize))
		(editPosition, editSize) = layout["editButton"]
		rects.append(("edit", editPosition[0], editPosition[1], editSize, editSize))
		return rects

	def MoveHudElement(self, key, x, y):
		if key == "joystick":
			self.joystick.Place(x, y)
		elif key == "attack":
			self.attackButton.SetPosition(x, y)
		elif key in uiMobileHud.UTILITY_NAMES:
			button = self.utilities.GetButton(key)
			if button:
				button.SetPosition(x, y)
		elif key == "page":
			self.skillArc.pageButton.SetPosition(x, y)
		elif key == "edit":
			self.skillArc.editor.toggle.SetPosition(x, y)
		elif key.startswith("skill"):
			self.skillArc.slots[int(key[5:])].SetPosition(x, y)

	def SaveSkillPositions(self):
		self.skillArc.SavePositions()

	def Update(self):
		import uiAkravonQuest
		uiAkravonQuest.Update()
		# AKRAVON: lonca dugmesi - loncaya girince / ayrilinca kendiliginden belirir / kaybolur
		try:
			wantGuild = player.GetGuildID() != 0
			if wantGuild != self.guildShown:
				self.guildShown = wantGuild
				self.__LayoutMenuTiles()
		except Exception:
			pass
		# AKRAVON: dokunmatikte "fare cikti" olayi gelmedigi icin takili kalan esya / beceri ipuclari 3 sn sonra kapanir
		try:
			now = app.GetTime()
			if not hasattr(self, "tipSince"):
				self.tipSince = {}
			# 3 sn sonra kapanir; parmak ayni yerde kaldigi icin motor her karede yeniden acar -> parmak kalkip baska yere
			# dokunulana kadar ShowToolTip engellenir (tekrar acilma hatasi)
			import wndMgr
			mouse = wndMgr.GetMousePosition()
			block = getattr(self, "tipBlock", None)
			if block and (abs(block[0] - mouse[0]) > 6 or abs(block[1] - mouse[1]) > 6):
				self.tipBlock = None
			for name in ("tooltipItem", "tooltipSkill"):
				tip = getattr(self.interface, name, None)
				if tip and not getattr(tip, "akrGuard", False):
					tip.akrGuard = True
					tip.akrShow = tip.ShowToolTip
					def Guarded(tip = tip, owner = self):
						if getattr(owner, "tipBlock", None):
							return
						tip.akrShow()
					tip.ShowToolTip = Guarded
				if tip and tip.IsShow():
					started = self.tipSince.setdefault(name, now)
					if now - started > 3.0:
						self.tipBlock = mouse
						tip.HideToolTip()
						self.tipSince.pop(name, None)
				else:
					self.tipSince.pop(name, None)
		except Exception:
			pass
		import uiAkravonSlotFx
		uiAkravonSlotFx.Install(self.interface)
		# AKRAVON: orijinal Metin2 gibi - saldiri tusu basiliyken yon verirsen karakter olarak (ata binikken de) oldugu yerde
		# o yone doner ve saldirmaya devam eder. Motor bunu yalniz yon TUSU bayragi kurulunca yapar; joystick bayragi kurmadigi
		# icin saldiri yone donmuyordu. Saldiri basiliyken joystick yonu sekiz yone yuvarlanip motorun yon tuslarina verilir.
		attackHeld = bool(getattr(self.attackButton, "isActive", False))
		wanted = {}
		if attackHeld and self.joystick.isActive:
			limit = max(1.0, self.joystick.radius * 0.35)
			offsetX = self.joystick.knobOffsetX
			offsetY = self.joystick.knobOffsetY
			wanted = {app.DIK_LEFT : offsetX < -limit, app.DIK_RIGHT : offsetX > limit, app.DIK_UP : offsetY < -limit, app.DIK_DOWN : offsetY > limit}
		held = getattr(self, "attackDirKeys", {})
		for key in (app.DIK_LEFT, app.DIK_RIGHT, app.DIK_UP, app.DIK_DOWN):
			state = bool(wanted.get(key, False))
			if state != bool(held.get(key, False)):
				player.SetSingleDIKKeyState(key, state)
		self.attackDirKeys = dict((key, bool(wanted.get(key, False))) for key in (app.DIK_LEFT, app.DIK_RIGHT, app.DIK_UP, app.DIK_DOWN))
		self.skillArc.UpdateFlash()
		import uiAkravonAutoHunt
		uiAkravonAutoHunt.Update(bool(self.joystick.isActive), self.utilities.GetButton("autohunt") if self.utilities else None)
		import uiAkravonAtlas
		uiAkravonAtlas.Update(bool(self.joystick.isActive or getattr(self.attackButton, "isActive", False)))
		if uiMobileHud.IsEditing():
			self.joystick.Stop()
			self.attackButton.Stop()
			if not self.editResumed:
				self.editResumed = True
				import uiMobileHudEdit
				uiMobileHudEdit.Resume()
		state = self.__GetState()
		if state != self.state:
			self.state = state
			self.__ApplyState(state)
		self.__RefreshSkillArc(state)
		self.__RefreshLinkTooltip()

		self.__HideTaskBar()
		self.__RefreshMenuHighlights()
		self.__RefreshPointAlert()

		if state != STATE_HIDDEN:
			self.chatController.Update()

		now = app.GetTime()
		if now >= self.nextQuickSlotRefresh:
			self.nextQuickSlotRefresh = now + QUICK_SLOT_REFRESH_INTERVAL
			self.skillArc.RefreshQuickSlot()
			self.__ArrangeFloatingWindows()
