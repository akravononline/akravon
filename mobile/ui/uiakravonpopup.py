# -*- coding: utf-8 -*-
# AKRAVON: onay / mesaj kutulari (uicommon.QuestionDialog*, PopupDialog*) - bizim tasarim, buyuk dokunma dugmeleri
from __future__ import division
import ui
import uiMobileStyle

TEXT = uiMobileStyle.Color(250, 236, 210)
W = 460
FONT = "Lato:23"


class Label(object):
	# dugme yazisi: ui.Button'in kucuk varsayilan yazisi yerine bizim yazi tipi
	def __init__(self, button):
		self.line = uiMobileStyle.CreateText(button, "", 95, 12, TEXT, "Marcellus:19", "center")

	def Set(self, text):
		self.line.SetText(uiMobileStyle.TrUpper(text))	# dugme yazilari BUYUK harf


def Style(dialog, count):
	keep = []
	dialog.akrKeep = keep
	for name in ("message", "message1", "message2", "message3"):
		try:
			dialog.GetChild(name).Hide()
		except Exception:
			pass
	board = dialog.GetChild("board")
	lines = []
	for i in range(4):
		line = uiMobileStyle.CreateText(board, "", W // 2, 30 + i * 32, TEXT, FONT, "center")
		lines.append(line)
		keep.append(line)
	state = {"parts" : ["", "", ""]}

	def Set(index, text):
		state["parts"][index] = text
		wrapped = []
		for part in state["parts"]:
			if part:
				wrapped.extend(uiMobileStyle.WrapLines(part, W - 50, FONT))
		n = min(4, len(wrapped))
		start = 70 - n * 16
		for i in range(4):
			lines[i].SetText(wrapped[i] if i < len(wrapped) else "")
			lines[i].SetPosition(W // 2, start + i * 32)

	dialog.SetText = lambda text: Set(0, text)
	dialog.SetText1 = lambda text: Set(0, text)
	dialog.SetText2 = lambda text: Set(1, text)
	dialog.SetText3 = lambda text: Set(2, text)
	dialog.SetWidth = lambda width: None

	for attr in ("acceptButton", "accceptButton", "cancelButton"):
		button = getattr(dialog, attr, None)
		if not button:
			continue
		initial = button.GetText()	# betikten gelen ilk yazi (EVET / HAYIR) yeni etikete tasinir
		if not initial:	# betikte yazi yoksa (yere atma onayi gibi) varsayilan etiket
			initial = {"acceptButton" : "Evet", "accceptButton" : "Tamam", "cancelButton" : "Hay\xfdr"}.get(attr, "")
		label = Label(button)
		keep.append(label)
		label.Set(initial)
		button.SetText = label.Set
	dialog.SetAcceptText = lambda text: dialog.acceptButton.SetText(text)
	dialog.SetCancelText = lambda text: dialog.cancelButton.SetText(text)
	dialog.SetButtonName = lambda text: dialog.accceptButton.SetText(text)
