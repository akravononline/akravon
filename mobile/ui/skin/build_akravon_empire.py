# -*- coding: utf-8 -*-
# AKRAVON: krallik (bolge) secme ekrani gorselleri -> ymir work/ui/akravon_legacy/emp_*.png
import os, sys
from PIL import Image, ImageDraw, ImageEnhance, ImageFilter
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_akravon_charwin as cw
SS = 4
GOLD = (222, 184, 110, 255)
CW, CH = 380, 420
REGIONS = {'a': (200, 56, 46), 'b': (226, 184, 60), 'c': (70, 122, 214)}	# kirmizi / sari / mavi bolge


def Emblem(color):
	# renkli sancak: ust cubuk + alta dogru sivri kumas, altin cizgi, ince isik
	w, h = 120, 150
	big = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	d.rounded_rectangle((6 * SS, 6 * SS, (w - 6) * SS, 16 * SS), 4 * SS, fill = (150, 112, 56, 255), outline = GOLD, width = SS)
	body = [(14 * SS, 16 * SS), ((w - 14) * SS, 16 * SS), ((w - 14) * SS, (h - 38) * SS), ((w // 2) * SS, (h - 8) * SS), (14 * SS, (h - 38) * SS)]
	d.polygon(body, fill = color + (255,))
	cloth = Image.new('RGBA', big.size, (0, 0, 0, 0))
	cd = ImageDraw.Draw(cloth)
	for i in range(h * SS):
		t = i / float(h * SS)
		shade = int(60 * (1 - t))
		cd.line((0, i, w * SS, i), fill = (255, 255, 255, shade))
	mask = Image.new('L', big.size, 0)
	ImageDraw.Draw(mask).polygon(body, fill = 255)
	big.alpha_composite(Image.composite(cloth, Image.new('RGBA', big.size, (0, 0, 0, 0)), mask))
	d = ImageDraw.Draw(big)
	d.line(body + [body[0]], fill = GOLD, width = int(2 * SS), joint = 'curve')
	inner = [(22 * SS, 24 * SS), ((w - 22) * SS, 24 * SS), ((w - 22) * SS, (h - 42) * SS), ((w // 2) * SS, (h - 18) * SS), (22 * SS, (h - 42) * SS)]
	d.line(inner + [inner[0]], fill = (255, 236, 190, 150), width = SS)
	# orta simge: elmas
	cx, cy, r = (w // 2) * SS, 62 * SS, 14 * SS
	d.polygon([(cx, cy - r), (cx + r, cy), (cx, cy + r), (cx - r, cy)], fill = (255, 232, 170, 255), outline = (255, 255, 255, 120))
	return big.resize((w, h), Image.LANCZOS)


def Src(name):
	return Image.open(os.path.join(cw.OUT_DIR, 'mobile_ui', 'akravon', name + '.png')).convert('RGBA')


def Slice9(src, w, h, margin, corner):
	# 9 parca: koseler korunur, kenar ve orta alan gerilir (karakter ekranindaki cerceve bozulmadan buyur)
	(sw, sh) = src.size
	out = Image.new('RGBA', (w, h), (0, 0, 0, 0))
	xs = [0, margin, sw - margin, sw]
	ys = [0, margin, sh - margin, sh]
	dx = [0, corner, w - corner, w]
	dy = [0, corner, h - corner, h]
	for j in range(3):
		for i in range(3):
			part = src.crop((xs[i], ys[j], xs[i + 1], ys[j + 1]))
			size = (max(1, dx[i + 1] - dx[i]), max(1, dy[j + 1] - dy[j]))
			out.alpha_composite(part.resize(size, Image.LANCZOS), (dx[i], dy[j]))
	return out


def Build(out):
	folder = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy')
	os.makedirs(folder, exist_ok = True)
	panel = Src('create_panel')
	card = Slice9(panel, CW, CH, 56, 34)
	off = ImageEnhance.Brightness(card).enhance(0.6)
	glow = Image.new('RGBA', (CW + 40, CH + 40), (0, 0, 0, 0))
	ImageDraw.Draw(glow).rounded_rectangle((20, 20, CW + 19, CH + 19), 10, outline = (255, 200, 100, 255), width = 6)
	glow = glow.filter(ImageFilter.GaussianBlur(8))
	on = card.copy()
	frame = Image.new('RGBA', (CW, CH), (0, 0, 0, 0))
	ImageDraw.Draw(frame).rounded_rectangle((2, 2, CW - 3, CH - 3), 6, outline = (255, 214, 120, 255), width = 3)
	on.alpha_composite(frame)
	off.save(os.path.join(folder, 'emp_card_off.png'))
	on.save(os.path.join(folder, 'emp_card_on.png'))
	glow.save(os.path.join(folder, 'emp_glow.png'))
	for (key, color) in REGIONS.items():
		Emblem(color).save(os.path.join(folder, 'emp_emblem_%s.png' % key))
	# dugmeler: karakter ekranindaki uzun yuva cercevesi (create_card normal / aktif)
	normal = Src('create_card_normal')
	active = Src('create_card_active')
	def Btn(w, h, state):
		base = active if state != 'up' else normal
		im = Slice9(base, w, h, 40, int(h * 0.38))
		if state == 'down':
			im = ImageEnhance.Brightness(im).enhance(0.72)
		return im
	for (i, state) in ((1, 'up'), (2, 'over'), (3, 'down')):
		Btn(300, 56, state).save(os.path.join(folder, 'emp_btn300_%d.png' % i))
		Btn(130, 46, state).save(os.path.join(folder, 'emp_back_%d.png' % i))
	print('krallik: gorseller hazir')


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
