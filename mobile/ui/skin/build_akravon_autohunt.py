# -*- coding: utf-8 -*-
# AKRAVON: oto av paneli gorselleri -> <out>/ymir work/ui/akravon_legacy/oa_*.png (birebir boyut, SS ile yumusak)
import os
import sys
from PIL import Image, ImageDraw, ImageFilter

SS = 4
GOLD = (222, 184, 110, 255)
GOLD_FILL = (176, 136, 70, 255)
NAVY = (14, 20, 38, 255)
ROW = (6, 9, 20, 205)
ROW_LINE = (64, 58, 44, 255)


def Done(big, w, h):
	return big.resize((w, h), Image.LANCZOS)


def RoundBox(w, h, r, fill, line, width = 1):
	big = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	ImageDraw.Draw(big).rounded_rectangle((0, 0, w * SS - 1, h * SS - 1), r * SS, fill = fill, outline = line, width = width * SS)
	return Done(big, w, h)


def Toggle(on):
	(w, h) = (46, 26)
	big = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	d.rounded_rectangle((0, 0, w * SS - 1, h * SS - 1), h * SS // 2, fill = GOLD_FILL if on else (40, 44, 60, 255), outline = GOLD, width = SS)
	k = (w - 13) if on else 13
	d.ellipse(((k - 10) * SS, 3 * SS, (k + 10) * SS, (h - 3) * SS), fill = (242, 232, 204, 255))
	return Done(big, w, h)


def Start(w, h, stop, down):
	big = Image.new('RGBA', ((w + 24) * SS, (h + 24) * SS), (0, 0, 0, 0))
	glow = Image.new('RGBA', big.size, (0, 0, 0, 0))
	ImageDraw.Draw(glow).rounded_rectangle((12 * SS, 12 * SS, (w + 12) * SS, (h + 12) * SS), 10 * SS, fill = (255, 120, 80, 110) if stop else (255, 190, 90, 110))
	big.alpha_composite(glow.filter(ImageFilter.GaussianBlur(7 * SS)))
	grad = Image.new('RGBA', (w * SS, h * SS))
	gd = ImageDraw.Draw(grad)
	(top, bottom) = (((196, 70, 52), (118, 28, 20)) if stop else ((240, 202, 124), (160, 116, 50)))
	if down:
		(top, bottom) = (bottom, top)
	for y in range(h * SS):
		t = y / float(h * SS - 1)
		gd.line((0, y, w * SS, y), fill = tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3)) + (255,))
	mask = Image.new('L', grad.size, 0)
	ImageDraw.Draw(mask).rounded_rectangle((0, 0, w * SS - 1, h * SS - 1), 10 * SS, fill = 255)
	big.paste(grad, (12 * SS, 12 * SS), mask)
	d = ImageDraw.Draw(big)
	d.rounded_rectangle((12 * SS, 12 * SS, (w + 12) * SS - 1, (h + 12) * SS - 1), 10 * SS, outline = (255, 232, 170, 255), width = 2 * SS)
	d.rounded_rectangle((16 * SS, 16 * SS, (w + 8) * SS, (h + 8) * SS), 8 * SS, outline = (120, 80, 30, 200), width = SS)
	return Done(big, w + 24, h + 24)


def Card(w, h, on):
	return RoundBox(w, h, 8, (34, 26, 12, 235) if on else ROW, GOLD if on else ROW_LINE, 2 if on else 1)


def Check():
	big = Image.new('RGBA', (22 * SS, 22 * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	d.ellipse((SS, SS, 21 * SS, 21 * SS), fill = GOLD_FILL, outline = (255, 232, 170, 255), width = SS)
	d.line([(6 * SS, 11 * SS), (10 * SS, 15 * SS), (16 * SS, 7 * SS)], fill = (24, 16, 6, 255), width = int(2.4 * SS))
	return Done(big, 22, 22)


def Knob():
	big = Image.new('RGBA', (22 * SS, 22 * SS), (0, 0, 0, 0))
	ImageDraw.Draw(big).ellipse((SS, SS, 21 * SS, 21 * SS), fill = (242, 232, 204, 255), outline = GOLD, width = 2 * SS)
	return Done(big, 22, 22)


def Build(out):
	folder = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy')
	os.makedirs(folder, exist_ok = True)
	images = {
		'oa_row': RoundBox(346, 50, 5, ROW, ROW_LINE),
		'oa_row_tall': RoundBox(346, 62, 5, ROW, ROW_LINE),
		'oa_toggle_on': Toggle(True), 'oa_toggle_off': Toggle(False),
		'oa_start': Start(346, 56, False, False), 'oa_start_down': Start(346, 56, False, True),
		'oa_stop': Start(346, 56, True, False), 'oa_stop_down': Start(346, 56, True, True),
		'oa_card_on': Card(106, 140, True), 'oa_card_off': Card(106, 140, False),
		'oa_chip': RoundBox(110, 28, 5, NAVY, (96, 74, 42, 255)),
		'oa_dew_on': RoundBox(346, 44, 6, (34, 26, 12, 235), GOLD, 2), 'oa_dew_off': RoundBox(346, 44, 6, ROW, ROW_LINE),
		'oa_tab_on': RoundBox(122, 42, 6, GOLD_FILL, GOLD), 'oa_tab_off': RoundBox(122, 42, 6, NAVY, GOLD),
		'oa_check': Check(), 'oa_knob': Knob(),
		'oa_track': RoundBox(318, 8, 4, (40, 44, 60, 255), (40, 44, 60, 255)),
		'oa_fill': RoundBox(318, 8, 4, GOLD_FILL, GOLD_FILL),
	}
	for (name, im) in images.items():
		im.save(os.path.join(folder, name + '.png'))
	print('oto av gorselleri: %d' % len(images))


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
