# -*- coding: utf-8 -*-
# AKRAVON: Arkadaslar penceresi gorselleri -> <out>/ymir work/ui/akravon_legacy/msg_*.png
import os, sys
from PIL import Image, ImageDraw, ImageEnhance
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_akravon_charwin as cw
import build_akravon_inventory as inv
GOLD = (222, 184, 110, 255)
SS = 4
W, H = 300, 460
BW, BH = 50, 44


def Glyph(name, col):
	big = Image.new('RGBA', (24 * SS, 24 * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	s = SS
	w = int(2.6 * s)
	if name == 'add':
		d.ellipse((5 * s, 2 * s, 13 * s, 10 * s), outline = col, width = w)
		d.arc((1 * s, 11 * s, 17 * s, 27 * s), 200, 340, fill = col, width = w)
		d.line((19 * s, 8 * s, 19 * s, 18 * s), fill = col, width = w)
		d.line((14 * s, 13 * s, 24 * s, 13 * s), fill = col, width = w)
	elif name == 'whisper':
		d.rounded_rectangle((2 * s, 4 * s, 22 * s, 17 * s), 4 * s, outline = col, width = w)
		d.polygon([(7 * s, 16 * s), (7 * s, 22 * s), (13 * s, 16 * s)], fill = col)
	elif name == 'remove':
		d.line((5 * s, 5 * s, 19 * s, 19 * s), fill = col, width = w)
		d.line((5 * s, 19 * s, 19 * s, 5 * s), fill = col, width = w)
	elif name == 'block':
		d.ellipse((3 * s, 3 * s, 21 * s, 21 * s), outline = col, width = w)
		d.line((7 * s, 17 * s, 17 * s, 7 * s), fill = col, width = w)
	elif name == 'guild':
		d.polygon([(12 * s, 2 * s), (21 * s, 6 * s), (19 * s, 16 * s), (12 * s, 22 * s), (5 * s, 16 * s), (3 * s, 6 * s)], outline = col, width = w)
		d.line((12 * s, 7 * s, 12 * s, 17 * s), fill = col, width = w)
	return big.resize((24, 24), Image.LANCZOS)


def Button(name, state):
	im = inv.Slot(BW, BH)
	if state == 1:
		im = ImageEnhance.Brightness(im).enhance(1.5)
	elif state == 2:
		d = ImageDraw.Draw(im)
		d.rounded_rectangle((0, 0, BW - 1, BH - 1), 6, fill = (150, 112, 56, 255), outline = GOLD, width = 1)
	col = {0: GOLD, 1: (255, 226, 160, 255), 2: (24, 16, 6, 255), 3: (90, 84, 70, 255)}[state]
	g = Glyph(name, col)
	im.alpha_composite(g, ((BW - 24) // 2, (BH - 24) // 2))
	return im


def Arrow(opened):
	big = Image.new('RGBA', (16 * SS, 16 * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	if opened:
		d.polygon([(3 * SS, 5 * SS), (13 * SS, 5 * SS), (8 * SS, 12 * SS)], fill = GOLD)
	else:
		d.polygon([(5 * SS, 3 * SS), (12 * SS, 8 * SS), (5 * SS, 13 * SS)], fill = GOLD)
	return big.resize((16, 16), Image.LANCZOS)


def Dot(online):
	big = Image.new('RGBA', (16 * SS, 16 * SS), (0, 0, 0, 0))
	ImageDraw.Draw(big).ellipse((4 * SS, 4 * SS, 12 * SS, 12 * SS), fill = (110, 220, 120, 255) if online else (110, 104, 92, 255))
	return big.resize((16, 16), Image.LANCZOS)


def Build(out):
	folder = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy')
	os.makedirs(folder, exist_ok = True)
	images = {'msg_board': cw.Board(W, H), 'msg_list': inv.Slot(W - 32, H - 52 - 96),
		'msg_open': Arrow(True), 'msg_close': Arrow(False), 'msg_on': Dot(True), 'msg_off': Dot(False)}
	images['dlg_board'] = cw.Board(340, 176)
	images['dlg_field'] = inv.Slot(292, 44)
	src = Image.open(os.path.join(out, 'mobile_ui', 'akravon', 'create_button_normal.png')).convert('RGBA').resize((140, 42), Image.LANCZOS)
	images['dlg_btn_1'] = src
	images['dlg_btn_2'] = ImageEnhance.Brightness(src).enhance(1.15)
	images['dlg_btn_3'] = Image.open(os.path.join(out, 'mobile_ui', 'akravon', 'create_button_down.png')).convert('RGBA').resize((140, 42), Image.LANCZOS)
	for name in ('add', 'whisper', 'remove', 'block', 'guild'):
		for state in range(4):
			images['msg_%s_%d' % (name, state + 1)] = Button(name, state)
	for (n, im) in images.items():
		im.save(os.path.join(folder, n + '.png'))
	print('arkadaslar: %d gorsel' % len(images))


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
