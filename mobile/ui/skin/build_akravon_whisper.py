# -*- coding: utf-8 -*-
# AKRAVON: fisilti gorselleri
#  mail_icon / mail_icon_green : HUD'da Sohbet ikonunun altina gelen zarf (Sohbet ikonuyla ayni bej / metal stil; green = okunmamis yanip sonme katmani)
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


ART = os.path.join(HERE, 'ref_menu', 'ikon', 'zarf_tas.png')	# el yapimi tas kabartmali zarf (kaynak sanat)


def Envelope(tint = None):
	# kullanicinin tas kabartmali zarfi: kenar kirlerini temizle, kirp, 46 genislige indir, rengi Sohbet ikonuyla ayni tona cek
	art = Image.open(ART).convert('RGBA')
	a = np.array(art).astype(np.float32)
	# kenardaki yari saydam kir / benek: sert esik + acilim (erozyon sonra genisleme) ile ana govde disinda her sey silinir
	solid = Image.fromarray(((a[..., 3] > 200) * 255).astype(np.uint8), 'L')
	solid = solid.filter(ImageFilter.MinFilter(15)).filter(ImageFilter.MaxFilter(17))
	keep = np.array(solid) > 128
	a[..., 3] = np.where(keep, a[..., 3], 0)
	ys, xs = np.where(a[..., 3] > 0)
	art = Image.fromarray(a.astype(np.uint8), 'RGBA').crop((xs.min(), ys.min(), xs.max() + 1, ys.max() + 1))
	width = 40
	height = int(round(art.size[1] * width / float(art.size[0])))
	art = art.resize((width, height), Image.LANCZOS)
	px = np.array(art).astype(np.float32)
	mask = px[..., 3] > 200
	target = MeanColor(os.path.join(HUD, 'chat_icon.png'))
	current = px[mask][:, :3].mean(axis = 0)
	for channel in range(3):
		px[..., channel] = np.clip(px[..., channel] * (target[channel] / float(current[channel])), 0, 255)
	if tint:
		for channel in range(3):
			px[..., channel] = np.clip(px[..., channel] * 0.45 + tint[channel] * 0.75, 0, 255)
	icon = Image.fromarray(px.astype(np.uint8), 'RGBA')
	canvas = Image.new('RGBA', (54, 40), (0, 0, 0, 0))	# Gorev / Sohbet ikon sutunuyla ayni genislik (54): hizali
	canvas.alpha_composite(icon, ((54 - icon.size[0]) // 2, (40 - icon.size[1]) // 2))
	return canvas


def Minimize(state):
	big = Image.new('RGBA', (32 * SS, 32 * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	fill = (150, 112, 56, 255) if state == 'down' else (8, 11, 22, 255)
	d.rounded_rectangle((0, 0, 32 * SS - 1, 32 * SS - 1), 5 * SS, fill = fill, outline = GOLD, width = SS)
	col = (24, 16, 6, 255) if state == 'down' else GOLD
	d.line((8 * SS, 22 * SS, 24 * SS, 22 * SS), fill = col, width = 3 * SS)
	return big.resize((32, 32), Image.LANCZOS)


def Corner(fill, edge):
	# 12x12 kose: yaricap 11 yuvarlak, 1 px kenar, dolgu opak; kenar / dolgu duz seritlerle ayni renk
	size = 36
	big = Image.new('RGBA', (size * SS, size * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	d.rounded_rectangle((0, 0, size * SS - 1, size * SS - 1), 11 * SS, fill = fill + (255,), outline = edge + (255,), width = SS)
	return big.resize((size, size), Image.LANCZOS)


def Corners(im, prefix, out):
	c = 12
	w, h = im.size
	boxes = {'tl': (0, 0, c, c), 'tr': (w - c, 0, w, c), 'bl': (0, h - c, c, h), 'br': (w - c, h - c, w, h)}
	for (key, box) in boxes.items():
		im.crop(box).save(os.path.join(out, '%s_%s.png' % (prefix, key)))


if __name__ == '__main__':
	hud = sys.argv[1] if len(sys.argv) > 1 else HUD
	legacy = sys.argv[2] if len(sys.argv) > 2 else LEGACY
	os.makedirs(hud, exist_ok = True)
	os.makedirs(legacy, exist_ok = True)
	Envelope().save(os.path.join(hud, 'mail_icon.png'))
	Envelope((70, 235, 110)).save(os.path.join(hud, 'mail_icon_green.png'))
	Minimize('up').save(os.path.join(legacy, 'wsp_min_1.png'))
	Minimize('down').save(os.path.join(legacy, 'wsp_min_3.png'))
	Corners(Corner((28, 98, 70), (96, 196, 140)), 'wsp_me', legacy)
	Corners(Corner((34, 54, 104), (110, 146, 224)), 'wsp_other', legacy)
	print('fisilti gorselleri ->', os.path.abspath(hud), os.path.abspath(legacy))
