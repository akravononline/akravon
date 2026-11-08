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
MINIMAP_DESKTOP_BUTTONS = ("AtlasShowButton", "MiniMapHideButton", "ScaleUpButton", "ScaleDownButton", "MiniMapShowButton")
MINIMAP_FRAME_ASSET = "map_frame"
MINIMAP_BACKGROUND_NAME = "OpenWindowBGI"
MINIMAP_SIZE = (136, 137)
MINIMAP_ZOOM_SIZE = 30	# AKRAVON: mobil dokunma (24 kucuktu)
# (kit adi, mini harita metodu, x, y)
MINIMAP_ZOOM_BUTTONS = (("zoomout", "ScaleDown", 60, 94), ("zoomin", "ScaleUp", 94, 94))
# (mini harita ozniteligi, kit adi): sol -> sag
MENU_TILE_COLUMNS = 4


class MenuTile(ui.Window):
	def __init__(self, kitName, size):
		ui.Window.__init__(self)
		self.SetSize(size, size)
		self.button = uiMobileChrome.HudButton(size, kitName)
		self.button.SetParent(self)
		self.button.SetPosition(0, 0)
		self.button.Show()

	def SetEvent(self, event):
		self.button.SetEvent(event)

	def Destroy(self):
		self.button.Destroy()
		self.button = None


MINIMAP_SHORTCUTS = (("btnRank", "ranking"), ("btnBiyolog", "biolog"), ("btnUM", "market"), ("btnMucadele", "swords"))
MINIMAP_MARGIN = (14, 12)
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
		self.menuBar = uiMobileChrome.ButtonRow(uiMobileChrome.MENU_BUTTONS, menuSize)
		self.menuBar.SetEvent("inventory", self.__GetInterfaceEvent("ToggleInventoryWindow"))
		self.menuBar.SetEvent("itemshop", self.__GetInterfaceEvent("ItemShopSendOpen"))
		self.menuBar.SetEvent("character", self.__GetInterfaceEvent("ToggleCharacterWindowStatusPage"))
		self.menuBar.SetEvent("more", self.__ToggleMenuMore)
		self.menuBar.PlaceHorizontal(menuX, menuY, menuSize, menuGap)

		import uiAkravon
		self.menuMore = ui.Window("UI")
		self.menuMore.AddFlag("float")
		self.menuMore.AddFlag("not_pick")
		self.menuMore.Hide()
		self.menuTiles = []
		self.__AddMenuTile("messenger", self.__GetInterfaceEvent("ToggleMessenger"))
		self.__AddMenuTile("system", self.__ToggleSystemMenu)
		self.__AddMenuTile("autohunt", self.__OpenAutoHuntSettings)

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

	def __AddMenuTile(self, kitName, event):
		tile = MenuTile(kitName, self.menuGeometry[2])
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
		rows = max(1, (len(self.menuTiles) + cols - 1) // cols)
		step = menuSize + menuGap
		w = cols * step - menuGap
		h = rows * step - menuGap
		for (i, tile) in enumerate(self.menuTiles):
			tile.SetPosition((i % cols) * step, (i // cols) * step)
		self.menuMore.SetSize(w, h)
		self.menuMore.SetPosition(menuX, menuY + step)

	def __ToggleMenuMore(self):
		self.menuMoreOpen = not self.menuMoreOpen
		button = self.menuBar.GetButton("more")
		if button:
			button.SetIcon("less" if self.menuMoreOpen else "more")
		if self.menuMoreOpen and self.state != STATE_HIDDEN:
			self.menuMore.Show()
		else:
			self.menuMore.Hide()
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
		import uiMobileOptions
		uiMobileOptions.Destroy()
		import uiAkravonAutoHunt
		uiAkravonAutoHunt.Destroy()
		self.utilities.Destroy()
		self.menuBar.Destroy()
		for tile in self.menuTiles:
			tile.Destroy()
		self.menuTiles = []
		self.menuMore.Hide()
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
		miniMap.SetPosition(wndMgr.GetScreenWidth() - width - MINIMAP_MARGIN[0] - uiMobileHud.SAFE_RIGHT, MINIMAP_MARGIN[1] + uiMobileHud.SAFE_TOP)	# AKRAVON: guvenli alan

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
		for (parent, kitName, methodName, x, y) in ((openWindow, "less", "HideMiniMap", 4, 94), (closeWindow, "more", "ShowMiniMap", width - MINIMAP_ZOOM_SIZE - 6, 6)):
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
			self.__AddMenuTile(kitName, ui.__mem_func__(getattr(miniMap, attribute).CallEvent))

	def __RestoreMinimapButtons(self):
		for button in self.minimapButtons:
			button.Hide()
			button.Destroy()
		self.minimapButtons = []
		for tile in self.menuTiles[3:]:
			tile.Hide()
			tile.Destroy()
		self.menuTiles = self.menuTiles[:3]
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
			for extra in self.menuExtra:
				extra.Hide()
			self.chatController.Hide()
		else:
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
			else:
				self.menuMore.Hide()
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
