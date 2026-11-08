# -*- coding: utf-8 -*-
# AKRAVON: karakter penceresinin resme gomulu yazilari (sekmeler, Seviye/EXP kutulari, bolum basliklari, stat adlari)
# ayarlar penceresinin dilinde yeniden cizilir: lacivert zemin, altin cizgi, Marcellus/Lato yazi.
# ayni boyutta png + ayni adli .sub (locale/tr/ui/windows/) -> pencere yerlesimi ve kod degismez.
import os
import sys
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
FONTS = os.path.join(HERE, '..', '..', 'data', 'fonts')
SS = 4
GOLD = (206, 166, 92, 255)
GOLD_DARK = (112, 86, 46, 255)
HEADER = (226, 194, 128, 255)
TEXT = (232, 226, 210, 255)
MUTED = (150, 142, 124, 255)
DARK = (40, 24, 8, 255)


def Font(name, px):
	return ImageFont.truetype(os.path.join(FONTS, name), px * SS)


def Canvas(w, h):
	return Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))


def Done(big, w, h):
	return big.resize((w, h), Image.LANCZOS)


def Text(d, x, y, text, font, fill, anchor = 'la'):
	d.text((x * SS, y * SS), text, font = font, fill = fill, anchor = anchor)


def Plate(d, x, y, w, h, active):
	r = (x * SS, y * SS, (x + w) * SS - 1, (y + h) * SS - 1)
	if active:
		d.rounded_rectangle(r, radius = 3 * SS, fill = (150, 112, 56, 255), outline = (240, 206, 132, 255), width = SS)
		d.rounded_rectangle((r[0] + 2 * SS, r[1] + 2 * SS, r[2] - 2 * SS, r[1] + h * SS // 2), radius = 2 * SS, fill = (190, 150, 84, 120))
	else:
		d.rounded_rectangle(r, radius = 3 * SS, fill = (10, 15, 32, 240), outline = GOLD_DARK, width = SS)


ASSETS = 'C:/AkravonTest/ui_out/mobile_ui/akravon'


def Tabs(active):
	w, h = 253, 37
	big = Canvas(w, h)
	d = ImageDraw.Draw(big)
	# ust ince altin cizgi (sayfa ile sekmeleri ayirir)
	d.line([(8 * SS, 2 * SS), ((w - 8) * SS, 2 * SS)], fill = GOLD_DARK, width = SS)
	tabs = ((6, 53, u'Durum'), (61, 67, u'Yetenekler'), (130, 61, u'Duygular'), (192, 55, u'Görevler'))
	font = Font('latobd.ttf', 11)
	for (i, (x, tw, label)) in enumerate(tabs):
		on = i == active
		skin = Image.open(os.path.join(ASSETS, 'plate_gold_normal.png' if on else 'picker_row_normal.png')).convert('RGBA')
		big.alpha_composite(skin.resize((tw * SS, 26 * SS), Image.LANCZOS), (x * SS, 6 * SS))
		Text(d, x + tw / 2.0, 6 + 13, label, font, DARK if on else TEXT, 'mm')
	return Done(big, w, h)


def ValueBox(w, h, label):
	big = Canvas(w, h)
	d = ImageDraw.Draw(big)
	d.rounded_rectangle((0, 0, w * SS - 1, h * SS - 1), radius = 3 * SS, fill = (5, 8, 18, 235), outline = GOLD_DARK, width = SS)
	d.line([(4 * SS, 16 * SS), ((w - 4) * SS, 16 * SS)], fill = (90, 70, 40, 200), width = SS)
	Text(d, w / 2.0, 8.5, label, Font('latobd.ttf', 11), HEADER, 'mm')
	return Done(big, w, h)


def Header(w, h, label, size = 12):
	big = Canvas(w, h)
	d = ImageDraw.Draw(big)
	Text(d, 2, h / 2.0, label, Font('marcellus.ttf', size), HEADER, 'lm')
	return Done(big, w, h)


def Column(w, h, rows, top, step, size = 10, align = 'left'):
	big = Canvas(w, h)
	d = ImageDraw.Draw(big)
	font = Font('latobd.ttf', size)
	for (i, label) in enumerate(rows):
		lines = label.split('\n')
		cy = top + i * step
		for (j, line) in enumerate(lines):
			y = cy + (j - (len(lines) - 1) / 2.0) * (size + 0.5)
			Text(d, 1 if align == 'left' else w / 2.0, y, line, font, TEXT, 'lm' if align == 'left' else 'mm')
	return Done(big, w, h)


def Small(w, h, label, color = HEADER, size = 10):
	big = Canvas(w, h)
	d = ImageDraw.Draw(big)
	Text(d, w / 2.0, h / 2.0, label, Font('latobd.ttf', size), color, 'mm')
	return Done(big, w, h)


def Build(out):
	global ASSETS
	ASSETS = os.path.join(out, 'mobile_ui', 'akravon')
	images = {
		'tab_1': Tabs(0), 'tab_2': Tabs(1), 'tab_3': Tabs(2), 'tab_4': Tabs(3),
		'label_level': ValueBox(37, 42, u'Seviye'),
		'label_cur_exp': ValueBox(90, 42, u'EXP'),
		'label_last_exp': ValueBox(90, 42, u'Gerekli EXP'),
		'label_std': Header(113, 15, u'Karakter Durumu'),
		'label_ext': Header(113, 15, u'Nitelikler'),
		# satirlar: deger kutulari 23 px aralikla (uiscript/characterwindow.py)
		'label_std_item1': Column(36, 91, [u'VIT', u'INT', u'STR', u'DEX'], 10, 23, 11),
		'label_std_item2': Column(46, 91, [u'HP', u'SP', u'Saldırı', u'Savunma'], 11, 23, 9),
		'label_ext_item1': Column(53, 65, [u'Hareket\nHızı', u'Saldırı\nHızı', u'Büyü\nHızı'], 10, 23, 9),
		'label_ext_item2': Column(53, 65, [u'Büyü\nHasarı', u'Büyü\nSavunma', u'Savuşturma'], 9, 23, 9),
		'label_uppt': Small(75, 14, u'Kalan Puan', HEADER, 9),
		'label_skill_active': Small(45, 15, u'Aktif'),
		'label_skill_passive': Small(47, 17, u'Pasif'),
		'label_skill_support': Small(75, 15, u'Destek'),
		'label_skill_weapon': Small(36, 17, u'Silah', TEXT),
		'label_skill_low': Small(35, 17, u'Acemi', TEXT),
		'label_skill_middle': Small(37, 17, u'Usta', TEXT),
		'label_skill_high': Small(34, 17, u'Büyük', TEXT),
		'title_skill': Header(72, 17, u'Yetenekler'),
		'title_status': Header(48, 17, u'Durum'),
	}
	for (name, image) in images.items():
		png = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy', 'loc_%s.png' % name)
		os.makedirs(os.path.dirname(png), exist_ok = True)
		image.save(png)
		sub = os.path.join(out, 'locale', 'tr', 'ui', 'windows', name + '.sub')
		os.makedirs(os.path.dirname(sub), exist_ok = True)
		text = 'title subImage\r\nversion 1.0\r\nimage "akravon_legacy/loc_%s.png"\r\nleft 0\r\ntop 0\r\nright %d\r\nbottom %d\r\n' % (name, image.width, image.height)
		open(sub, 'wb').write(text.encode('latin1'))
	print('karakter penceresi yazilari: %d' % len(images))


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
