# -*- coding: utf-8 -*-
# AKRAVON: genel giris kutusu (uicommon.InputDialog) - karakter penceresi dili, mobil boyut
import ui
import uiMobileStyle

IMG = "d:/ymir work/ui/akravon_legacy/"
W, H = 340, 176
GOLD = uiMobileStyle.Color(222, 184, 110)


def Style(dialog):
	board = dialog.board
	for part in list(getattr(board, "Corners", [])) + list(getattr(board, "Lines", [])) + [getattr(board, "Base", None)]:
		if part:
			part.Hide()
	titleBar = getattr(board, "titleBar", None)
	if titleBar:
		titleBar.Hide()
	board.AddFlag("attach")
	keep = []
	bg = ui.ExpandedImageBox()
	bg.SetParent(board)
	bg.AddFlag("not_pick")
	bg.LoadImage(IMG + "dlg_board.png")
	bg.Show()
	keep.append(bg)
	title = uiMobileStyle.CreateText(board, "", W // 2, 14, GOLD, "Marcellus:20", "center")
	keep.append(title)
	keep.append(uiMobileStyle.CreateBar(board, uiMobileStyle.Color(96, 74, 42), 16, 46, W - 32, 1))
	close = ui.Button()
	close.SetParent(board)
	close.SetUpVisual(IMG + "close_1.png")
	close.SetOverVisual(IMG + "close_2.png")
	close.SetDownVisual(IMG + "close_3.png")
	close.SetPosition(W - 40, 10)
	close.Show()
	keep.append(close)
	field = ui.ExpandedImageBox()
	field.SetParent(board)
	field.AddFlag("not_pick")
	field.LoadImage(IMG + "dlg_field.png")
	field.SetPosition(24, 60)
	field.Show()
	keep.append(field)
	slot = dialog.inputSlot
	slot.SetWindowHorizontalAlignLeft()
	slot.SetPosition(36, 71)
	slot.SetSize(W - 72, 24)
	import wndMgr
	wndMgr.SetColor(slot.hWnd, 0, 0, 0)	# Bar3D beyaz cerceve saydam
	for part in (getattr(slot, "Base", None),) + tuple(getattr(slot, "Lines", []) or ()):
		if part:
			part.Hide()
	dialog.inputValue.SetSize(W - 72, 24)
	try:
		dialog.inputValue.SetFontName("Lato:19")
		dialog.inputValue.SetText(dialog.inputValue.GetText())	# yazi ornegi yeni yazi tipiyle yeniden kurulsun
	except Exception:
		pass
	for (button, x) in ((dialog.acceptButton, 24), (dialog.cancelButton, W - 24 - 140)):
		button.SetUpVisual(IMG + "dlg_btn_1.png")
		button.SetOverVisual(IMG + "dlg_btn_2.png")
		button.SetDownVisual(IMG + "dlg_btn_3.png")
		button.SetWindowHorizontalAlignLeft()
		button.SetPosition(x, 116)
		label = button.GetText()
		button.SetText("")
		text = uiMobileStyle.CreateText(button, label, 70, 11, uiMobileStyle.Color(250, 236, 210), "LatoBd:17", "center")
		keep.append(text)
	# zemin panonun ustune eklendi: giris / dugmeler zeminden sonra eklenen katmana tasinir
	holder = ui.Window()
	holder.SetParent(board)
	holder.AddFlag("not_pick")
	holder.SetSize(W, H)
	holder.Show()
	keep.append(holder)
	for child in (dialog.acceptButton, dialog.cancelButton, close, title):
		child.SetParent(holder)
		child.Show()
	# giris cizgisi: eski cerceveli slot gizlenir, yazi alani dogrudan kutunun icine
	value = dialog.inputValue
	value.SetParent(holder)
	value.SetPosition(38, 68)
	value.Show()
	slot.Hide()
	ui.ScriptWindow.SetSize(dialog, W, H)
	board.SetSize(W, H)
	dialog.akrKeep = keep
	dialog.akrTitle = title
	dialog.akrClose = close
	(setTitle, setCancel) = (dialog.SetTitle, dialog.SetCancelEvent)

	def SetTitle(name):
		setTitle(name)
		title.SetText(name)

	def SetCancelEvent(event):
		setCancel(event)
		close.SetEvent(event)

	dialog.SetTitle = SetTitle
	dialog.SetCancelEvent = SetCancelEvent


def SetCancel(dialog, event):
	close = getattr(dialog, "akrClose", None)
	if close:
		close.SetEvent(event)
