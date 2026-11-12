# -*- coding: utf-8 -*-
# AKRAVON ortak tema bilesenleri - REFERANS: Karakter penceresi (kurallar: 5_Test_Sunucusu/AKRAVON_Tema_Kurallari.md)
# Gorseller: skin/build_akravon_theme.py (2x, oyunda 0.5 olcek -> keskin).
#   ThemeTab   : yazili dugme / sekme. normal (lacivert) / secili (altin, koyu yazi) / basili (kahve, yazi 1 px asagi)
#   ThemeIcon  : arti / eksi / carpi / ok. Gorsel 32-34 px, dokunma alani 44 px (gorselden buyuk, ortali)
#   AttachIcon : motor dugmesinin (ui.Button) ustune ThemeIcon koyar; olay eski dugmeden okunur
#   AttachRadio: motor sekmesinin (ui.RadioButton) ustune ThemeTab koyar; secili durum ve yazi sekmeden okunur
# Basili gorunum dokunus kisa olsa da HOLD saniye gorunur (parmak cekilince "iceri girip cikma" hissi).
# Butun gorseller onceden yuklenir; basista yalniz goster / gizle (basista gorsel yuklemek dokunusu kaybettirir).
from __future__ import division
import app
import ui
import uiMobileStyle

IMG = "d:/ymir work/ui/akravon_legacy/"
TEXT = uiMobileStyle.Color(232, 221, 197)	# krem (manifest text)
DARK = uiMobileStyle.Color(40, 24, 8)	# secili (altin) dugmede yazi
FONT_TAB = "Marcellus:20"	# sol sekmeler (122 px: en uzun HAREKETLER kenara degmez)
FONT_BUTTON = "Marcellus:20"	# alt sekmeler (Devam Edenler, bonus)
FONT_SMALL = "Marcellus:18"	# dar dugmeler (84 px: Bicak / Binicilik)
TOUCH = 44	# en kucuk dokunma alani
HOLD = 0.12	# basili gorunum en az bu kadar


class _Press(uiMobileStyle.Pressable):
	# ortak: basili gorunum en az HOLD saniye
	def __init__(self, width, height):
		self.holdUntil = 0.0
		uiMobileStyle.Pressable.__init__(self, width, height)

	def IsPressedLook(self):
		return self.pressed or app.GetTime() < self.holdUntil

	def OnMouseLeftButtonUp(self):
		if self.pressed:
			self.holdUntil = app.GetTime() + HOLD
		return uiMobileStyle.Pressable.OnMouseLeftButtonUp(self)

	def Run(self):
		self.holdUntil = app.GetTime() + HOLD
		uiMobileStyle.Pressable.Run(self)
		self.RefreshVisual()

	def OnUpdate(self):
		if self.holdUntil and app.GetTime() >= self.holdUntil and not self.pressed:
			self.holdUntil = 0.0
			self.RefreshVisual()


class ThemeTab(_Press):
	def __init__(self, width, height, text, font = FONT_BUTTON):
		_Press.__init__(self, width, height)
		self.selected = False
		self.shown = "normal"
		self.images = {}
		for state in ("normal", "selected", "pressed"):
			image = ui.ExpandedImageBox()
			image.SetParent(self)
			image.AddFlag("not_pick")
			image.LoadImage(IMG + "akr_btn_%dx%d_%s.png" % (width, height, state))
			image.SetScale(0.5, 0.5)
			image.SetPosition(0, 0)
			if state == "normal":
				image.Show()
			else:
				image.Hide()
			self.images[state] = image
		size = int(font.split(":")[1])
		self.textY = max(0, (height - size) // 2 - 1)
		self.label = uiMobileStyle.CreateText(self, "", width // 2, self.textY, TEXT, font, "center")
		self.SetText(text)

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.images = {}
		self.label = None

	def SetText(self, text):
		self.label.SetText(uiMobileStyle.TrUpper(text))

	def SetSelected(self, selected):
		if bool(selected) != self.selected:
			self.selected = bool(selected)
			self.RefreshVisual()

	def RefreshVisual(self):
		if not getattr(self, "images", None) or getattr(self, "label", None) is None:
			return
		pressed = self.IsPressedLook()
		state = "selected" if (pressed or self.selected) else "normal"	# basili = altin (kahve yok)
		if state != self.shown:
			self.images[self.shown].Hide()
			self.images[state].Show()
			self.shown = state
		self.label.SetPackedFontColor(DARK if state == "selected" else TEXT)
		self.label.SetPosition(self.GetWidth() // 2, self.textY + (1 if pressed else 0))


class ThemeIcon(_Press):
	# kind: plus / minus / close / arrow_l / arrow_r ; size: gorsel (oyun px), touch: dokunma alani
	def __init__(self, kind, size, touch = TOUCH):
		touch = max(size, touch)
		_Press.__init__(self, touch, touch)
		self.shown = "normal"
		self.images = {}
		source = 68 if kind in ("close", "arrow_l", "arrow_r") else 64
		offset = (touch - size) // 2
		for state in ("normal", "pressed"):
			image = ui.ExpandedImageBox()
			image.SetParent(self)
			image.AddFlag("not_pick")
			image.LoadImage(IMG + "akr_%s_%s.png" % (kind, "gold" if state == "pressed" else state))	# basili = altin
			image.SetScale(size / float(source), size / float(source))
			image.SetPosition(offset, offset)
			if state == "normal":
				image.Show()
			else:
				image.Hide()
			self.images[state] = image

	def Destroy(self):
		uiMobileStyle.Pressable.Destroy(self)
		self.images = {}

	def RefreshVisual(self):
		if not getattr(self, "images", None):
			return
		state = "pressed" if self.IsPressedLook() else "normal"
		if state != self.shown:
			self.images[self.shown].Hide()
			self.images[state].Show()
			self.shown = state


def _Fire(button):
	function = getattr(button, "eventFunc", None)
	if function:
		function(*(getattr(button, "eventArgs", ()) or ()))


def AttachIcon(button, kind, size, touch = TOUCH, width = None, height = None, parent = None):
	# motor dugmesinin cocugu dokunus almiyor: ikon dugmenin EBEVEYNINE (parent) konur, dugme gizlenir.
	# Dugmenin Show / Hide'i ikona aktarilir (stat artisi puan varken gorunur vb.)
	width = width or button.GetWidth()
	height = height or button.GetHeight()
	icon = ThemeIcon(kind, size, touch)
	if parent is None:
		icon.SetParent(button)
		icon.SetPosition((width - icon.GetWidth()) // 2, (height - icon.GetHeight()) // 2)
	else:
		(bx, by) = button.GetGlobalPosition()
		(px, py) = parent.GetGlobalPosition()
		icon.SetParent(parent)
		icon.SetPosition(bx - px + (width - icon.GetWidth()) // 2, by - py + (height - icon.GetHeight()) // 2)
		gorunur = button.IsShow()
		ui.Window.Hide(button)
		button.Show = icon.Show
		button.Hide = icon.Hide
		button.IsShow = icon.IsShow
		if not gorunur:
			icon.Hide()
			icon.SetEvent(lambda button = button: _Fire(button))
			button.akrThemeIcon = icon
			return icon
	icon.SetEvent(lambda button = button: _Fire(button))
	icon.Show()
	button.akrThemeIcon = icon
	return icon


class _RadioSkin(ThemeTab):
	# motor sekmesini (ui.RadioButton) giydirir: secili = sekme basili (IsDown), yazi sekmenin yazisindan kopyalanir
	def __init__(self, button, width, height, text, font):
		ThemeTab.__init__(self, width, height, text, font)
		self.button = button
		self.mirrorText = text is None
		self.lastText = None

	def OnUpdate(self):
		ThemeTab.OnUpdate(self)
		button = self.button
		try:
			self.SetSelected(button.IsDown())
		except Exception:
			pass
		if self.mirrorText:
			try:
				text = button.GetText()
			except Exception:
				text = ""
			if text != self.lastText:
				self.lastText = text
				self.SetText(text or "")
				hidden = getattr(button, "ButtonText", None)
				if hidden:
					hidden.Hide()


def AttachRadio(button, width, height, text = None, font = FONT_BUTTON, dy = 0):
	# text None: yazi motor sekmesinden (SetText) okunur
	skin = _RadioSkin(button, width, height, text or "", font)
	skin.mirrorText = text is None
	skin.SetParent(button)
	skin.SetPosition(0, dy)

	def Tap(button = button):
		button.Down()
		_Fire(button)
	skin.SetEvent(Tap)
	skin.Show()
	button.akrThemeSkin = skin
	return skin
