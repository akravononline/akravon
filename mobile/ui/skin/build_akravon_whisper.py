# -*- coding: utf-8 -*-
# AKRAVON: fisilti gorselleri
#  mail_icon / mail_icon_red : HUD'da Sohbet ikonunun altina gelen zarf (Sohbet ikonuyla ayni bej / metal stil; red = yanip sonme katmani)
#  wsp_min_1 / wsp_min_3 : fisilti penceresi kucult dugmesi (kapat dugmesiyle ayni cerceve, ortada tire)
#  wsp_me_* / wsp_other_* : mesaj balonlari (9 parca, kose 12 px): kendi mesajimiz yesil, gelen mesaj mavi, ince acik kenar
import os
import sys
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

SS = 4
HERE = os.path.dirname(os.path.abspath(__file__))
PROJECT = os.path.join(HERE, '..', '..')
HUD = os.path.join(PROJECT, 'gorseller', 'mobile_ui', 'akravon', 'hud')
LEGACY = os.path.join(PROJECT, 'gorseller', 'ymir work', 'ui', 'akravon_legacy')
GOLD = (222, 184, 110, 255)


def MeanColor(path):
	im = np.array(Image.open(path).convert('RGBA')).astype(np.float32)
	mask = im[..., 3] > 200
	return tuple(int(v) for v in im[mask][:, :3].mean(axis = 0))


def Envelope(tint = None):
	# 48x45 tuval; govde: dolgulu yuvarlak dikdortgen, ustte V kapak cizgisi, hafif bej doku
	w, h = 48, 45
	base = MeanColor(os.path.join(HUD, 'chat_icon.png'))
	big = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	body = (3 * SS, 8 * SS, 45 * SS, 38 * SS)
	dark = tuple(int(c * 0.55) for c in base) + (255,)
	d.rounded_rectangle(body, 5 * SS, fill = base + (255,), outline = dark, width = 2 * SS)
	d.line((5 * SS, 11 * SS, 24 * SS, 26 * SS), fill = dark, width = 2 * SS)
	d.line((43 * SS, 11 * SS, 24 * SS, 26 * SS), fill = dark, width = 2 * SS)
	light = tuple(min(255, int(c * 1.25)) for c in base) + (255,)
	d.line((6 * SS, 36 * SS, 20 * SS, 24 * SS), fill = light, width = SS)
	d.line((42 * SS, 36 * SS, 28 * SS, 24 * SS), fill = light, width = SS)
	im = big.resize((w, h), Image.LANCZOS)
	if tint:
		px = np.array(im).astype(np.float32)
		px[..., 0] = np.clip(px[..., 0] * 0.55 + tint[0] * 0.6, 0, 255)
		px[..., 1] = np.clip(px[..., 1] * 0.55 + tint[1] * 0.25, 0, 255)
		px[..., 2] = np.clip(px[..., 2] * 0.55 + tint[2] * 0.25, 0, 255)
		im = Image.fromarray(px.astype(np.uint8), 'RGBA')
	return im


def Minimize(state):
	big = Image.new('RGBA', (32 * SS, 32 * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	fill = (150, 112, 56, 255) if state == 'down' else (8, 11, 22, 255)
	d.rounded_rectangle((0, 0, 32 * SS - 1, 32 * SS - 1), 5 * SS, fill = fill, outline = GOLD, width = SS)
	col = (24, 16, 6, 255) if state == 'down' else GOLD
	d.line((8 * SS, 22 * SS, 24 * SS, 22 * SS), fill = col, width = 3 * SS)
	return big.resize((32, 32), Image.LANCZOS)


def Bubble(fill, edge):
	# 36x36: yumusak kose (yaricap 11), ust yarida hafif parlama, 1 px acik kenar
	size = 36
	big = Image.new('RGBA', (size * SS, size * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	d.rounded_rectangle((0, 0, size * SS - 1, size * SS - 1), 11 * SS, fill = fill + (250,), outline = edge + (255,), width = SS)
	shine = Image.new('RGBA', big.size, (0, 0, 0, 0))
	sd = ImageDraw.Draw(shine)
	sd.rounded_rectangle((SS, SS, size * SS - SS - 1, size * SS // 2), 10 * SS, fill = (255, 255, 255, 18))
	big = Image.alpha_composite(big, shine)
	return big.resize((size, size), Image.LANCZOS)


def Slice(im, prefix, out):
	c = 12
	w, h = im.size
	pieces = {
		'tl': (0, 0, c, c), 'tr': (w - c, 0, w, c), 'bl': (0, h - c, c, h), 'br': (w - c, h - c, w, h),
		't': (c, 0, w - c, c), 'b': (c, h - c, w - c, h), 'l': (0, c, c, h - c), 'r': (w - c, c, w, h - c), 'c': (c, c, w - c, h - c),
	}
	for (key, box) in pieces.items():
		im.crop(box).save(os.path.join(out, '%s_%s.png' % (prefix, key)))


if __name__ == '__main__':
	hud = sys.argv[1] if len(sys.argv) > 1 else HUD
	legacy = sys.argv[2] if len(sys.argv) > 2 else LEGACY
	os.makedirs(hud, exist_ok = True)
	os.makedirs(legacy, exist_ok = True)
	Envelope().save(os.path.join(hud, 'mail_icon.png'))
	Envelope((255, 70, 60)).save(os.path.join(hud, 'mail_icon_red.png'))
	Minimize('up').save(os.path.join(legacy, 'wsp_min_1.png'))
	Minimize('down').save(os.path.join(legacy, 'wsp_min_3.png'))
	Slice(Bubble((28, 98, 70), (96, 196, 140)), 'wsp_me', legacy)
	Slice(Bubble((34, 54, 104), (110, 146, 224)), 'wsp_other', legacy)
	print('fisilti gorselleri ->', os.path.abspath(hud), os.path.abspath(legacy))
