# -*- coding: utf-8 -*-
# AKRAVON: oyuncu menusu gorselleri
#  pm_btn<W>_1 / _3 : sb_btn190'dan 3 parcali (kose 22 px) uretilen, metne gore secilen genislikte altin pahli dugme (W = 100..260, 10'ar)
#  pm_ic_<ad> : 24 px altin ikonlar (fisilti, ticaret, duello, grup, arkadas, lonca, engel, ...)
import os
import math
from PIL import Image, ImageDraw

SS = 8
HERE = os.path.dirname(os.path.abspath(__file__))
LEGACY = os.path.join(HERE, '..', '..', 'gorseller', 'ymir work', 'ui', 'akravon_legacy')
GOLD = (232, 196, 120, 255)
CAP = 22


def Buttons():
	for state in (1, 3):
		src = Image.open(os.path.join(LEGACY, 'sb_btn190_%d.png' % state)).convert('RGBA')
		w, h = src.size
		left = src.crop((0, 0, CAP, h))
		right = src.crop((w - CAP, 0, w, h))
		mid = src.crop((CAP, 0, w - CAP, h))
		for width in list(range(100, 261, 10)) + [140, 193, 238]:
			out = Image.new('RGBA', (width, h), (0, 0, 0, 0))
			out.paste(left, (0, 0))
			out.paste(right, (width - CAP, 0))
			# orta: kaynagin ortasindaki 1 px'lik dikey seridi (ust/alt kenar cizgisi dahil) yatay uzat
			strip = src.crop((w // 2, 0, w // 2 + 1, h)).resize((width - CAP * 2, h), Image.NEAREST)
			out.paste(strip, (CAP, 0))
			out.save(os.path.join(LEGACY, 'pm_btn%d_%d.png' % (width, state)))


def Canvas():
	im = Image.new('RGBA', (24 * SS, 24 * SS), (0, 0, 0, 0))
	return im, ImageDraw.Draw(im)


def Save(im, name):
	im.resize((24, 24), Image.LANCZOS).save(os.path.join(LEGACY, 'pm_ic_%s.png' % name))


def P(v):
	return v * SS


def Whisper():
	im, d = Canvas()
	d.rounded_rectangle((P(3), P(4), P(21), P(16)), radius = P(4), fill = GOLD)
	d.polygon([(P(8), P(15)), (P(7), P(21)), (P(13), P(16))], fill = GOLD)
	for x in (8, 12, 16):
		d.ellipse((P(x - 1), P(9), P(x + 1), P(11)), fill = (20, 32, 60, 255))
	Save(im, 'whisper')


def Trade():
	im, d = Canvas()
	w = P(2.4)
	d.line((P(4), P(8), P(19), P(8)), fill = GOLD, width = int(w))
	d.polygon([(P(20), P(8)), (P(15), P(4)), (P(15), P(12))], fill = GOLD)
	d.line((P(5), P(16), P(20), P(16)), fill = GOLD, width = int(w))
	d.polygon([(P(4), P(16)), (P(9), P(12)), (P(9), P(20))], fill = GOLD)
	Save(im, 'trade')


def Duel():
	im, d = Canvas()
	w = int(P(2.6))
	d.line((P(5), P(4), P(19), P(18)), fill = GOLD, width = w)
	d.line((P(19), P(4), P(5), P(18)), fill = GOLD, width = w)
	d.line((P(3), P(15), P(9), P(21)), fill = GOLD, width = w)
	d.line((P(21), P(15), P(15), P(21)), fill = GOLD, width = w)
	Save(im, 'duel')


def Person(d, cx, top, scale = 1.0):
	d.ellipse((P(cx - 3 * scale), P(top), P(cx + 3 * scale), P(top + 6 * scale)), fill = GOLD)
	d.pieslice((P(cx - 6 * scale), P(top + 8 * scale), P(cx + 6 * scale), P(top + 8 * scale + 12 * scale)), 180, 360, fill = GOLD)


def Group():
	im, d = Canvas()
	Person(d, 12, 3, 1.0)
	Person(d, 5, 8, 0.7)
	Person(d, 19, 8, 0.7)
	Save(im, 'group')


def Friend():
	im, d = Canvas()
	Person(d, 9, 3, 1.0)
	d.rectangle((P(17), P(10), P(23), P(12)), fill = GOLD)
	d.rectangle((P(19), P(8), P(21), P(14)), fill = GOLD)
	Save(im, 'friend')


def Guild():
	im, d = Canvas()
	d.polygon([(P(4), P(3)), (P(20), P(3)), (P(20), P(13)), (P(12), P(21)), (P(4), P(13))], fill = GOLD)
	d.polygon([(P(8), P(7)), (P(16), P(7)), (P(16), P(12)), (P(12), P(16)), (P(8), P(12))], fill = (20, 32, 60, 255))
	Save(im, 'guild')


def Block():
	im, d = Canvas()
	d.ellipse((P(3), P(3), P(21), P(21)), outline = GOLD, width = int(P(2.6)))
	d.line((P(6), P(18), P(18), P(6)), fill = GOLD, width = int(P(2.6)))
	Save(im, 'block')


def Exit():
	im, d = Canvas()
	d.rectangle((P(4), P(4), P(14), P(20)), outline = GOLD, width = int(P(2.2)))
	d.line((P(10), P(12), P(21), P(12)), fill = GOLD, width = int(P(2.4)))
	d.polygon([(P(22), P(12)), (P(17), P(8)), (P(17), P(16))], fill = GOLD)
	Save(im, 'exit')


def Revenge():
	im, d = Canvas()
	d.pieslice((P(3), P(3), P(21), P(21)), 40, 320, fill = GOLD)
	d.ellipse((P(8), P(8), P(16), P(16)), fill = (20, 32, 60, 255))
	Save(im, 'revenge')


def Lights():
	for (name, fill) in (('off', (14, 24, 48, 255)), ('on', (70, 200, 90, 255))):
		im = Image.new('RGBA', (30 * SS, 30 * SS), (0, 0, 0, 0))
		d = ImageDraw.Draw(im)
		d.rounded_rectangle((P(1), P(1), P(29), P(29)), radius = P(5), fill = (150, 118, 58, 255))
		d.rounded_rectangle((P(4), P(4), P(26), P(26)), radius = P(3), fill = fill)
		if name == 'on':
			d.line((P(9), P(15), P(13), P(19)), fill = (230, 255, 235, 255), width = int(P(3)))
			d.line((P(13), P(19), P(21), P(10)), fill = (230, 255, 235, 255), width = int(P(3)))
		im.resize((30, 30), Image.LANCZOS).save(os.path.join(LEGACY, 'ex_light_%s.png' % name))


if __name__ == '__main__':
	Lights()
	Buttons()
	for f in (Whisper, Trade, Duel, Group, Friend, Guild, Block, Exit, Revenge):
		f()
	print('tamam')
