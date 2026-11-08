# -*- coding: utf-8 -*-
# AKRAVON: yeni yuvarlak menu ikon levhalari -> mobile_ui/akravon/hud/btn_<ad>_<durum>[_m|_s|_xs].png
# kaynak: skin/src/ikon_levha_menu_13.png, skin/src/ikon_levha_sohbet_arac.png   (HUD'daki yeni ikon dili)
# NOT: beceri ikonlarina / beceri HUD'una dokunmaz.
import os, sys
import numpy as np
import cv2
from PIL import Image, ImageDraw, ImageEnhance, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
out = sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out'
SIZES = ((136, ''), (112, '_m'), (72, '_s'), (34, '_xs'))

# levha, sira (soldan saga, yukaridan asagi) -> dugme adi
SHEETS = (
	('ikon_levha_menu_13.png', ('inventory', 'itemshop', 'character', 'less', 'ranking', 'messenger', 'system', 'autohunt', 'swords', 'biolog', 'market', 'pickup', 'mount')),
	('ikon_levha_sohbet_arac.png', ('chat', 'send', 'link', 'refresh', 'wrench')),
)


def Circles(path):
	a = np.asarray(Image.open(path).convert('RGBA'))
	m = (a[..., 3] > 230).astype(np.uint8)
	m = cv2.morphologyEx(m, cv2.MORPH_OPEN, np.ones((9, 9), np.uint8))
	cnt, lab, st, cen = cv2.connectedComponentsWithStats(m, 8)
	boxes = [tuple(int(v) for v in s[:4]) for s in st[1:] if s[4] > 20000]
	boxes.sort(key = lambda b: (b[1] // 200, b[0]))
	return boxes


def Cut(sheet, box):
	(x, y, w, h) = box
	side = max(w, h)
	cx, cy = x + w / 2.0, y + h / 2.0
	crop = sheet.crop((int(cx - side / 2.0) - 2, int(cy - side / 2.0) - 2, int(cx + side / 2.0) + 2, int(cy + side / 2.0) + 2)).convert('RGBA')
	big = crop.width * 3
	mask = Image.new('L', (big, big), 0)
	ImageDraw.Draw(mask).ellipse((6, 6, big - 7, big - 7), fill = 255)		# kenar lekelerini ve parilti artigini temizler
	mask = mask.resize(crop.size, Image.LANCZOS)
	crop.putalpha(ImageChops_multiply(crop.getchannel('A'), mask))
	return crop


def ImageChops_multiply(a, b):
	from PIL import ImageChops
	return ImageChops.multiply(a, b)


def Arrow(icon):
	# yuvarlak ikonun ortasindaki ok: lacivert zemin saydam, bej ok kalir; 40x39 (0.5 olcekte cizilir)
	w = icon.width
	c = icon.crop((int(w * 0.22), int(w * 0.22), int(w * 0.78), int(w * 0.78))).convert('RGBA')
	px = np.asarray(c).astype(np.float32)
	lum = px[..., :3].mean(axis = 2)
	alpha = np.clip((lum - 70.0) * 255.0 / 60.0, 0, 255)
	px[..., 3] = np.minimum(px[..., 3], alpha)
	res = Image.fromarray(px.astype(np.uint8), 'RGBA')
	bbox = res.getchannel('A').point(lambda v: 255 if v > 40 else 0).getbbox()
	res = res.crop(bbox)
	canvas = Image.new('RGBA', (40, 39), (0, 0, 0, 0))
	res.thumbnail((36, 30), Image.LANCZOS)
	canvas.alpha_composite(res, ((40 - res.width) // 2, (39 - res.height) // 2))
	return canvas


def Variant(im, state):
	rgb = im.convert('RGB')
	alpha = im.getchannel('A')
	if state == 'down':
		rgb = ImageEnhance.Brightness(rgb).enhance(0.72)
	elif state == 'active':
		rgb = ImageEnhance.Brightness(rgb).enhance(1.12)
	res = rgb.convert('RGBA')
	res.putalpha(alpha)
	if state == 'active':
		# etkin: dis altin halka
		w = res.width
		ring = Image.new('RGBA', res.size, (0, 0, 0, 0))
		ImageDraw.Draw(ring).ellipse((2, 2, w - 3, w - 3), outline = (255, 214, 120, 255), width = max(2, w // 40))
		res.alpha_composite(ring)
	return res


def Save(folder, name, im):
	for (px, suffix) in SIZES:
		for state in ('normal', 'down', 'active'):
			v = Variant(im, state).resize((px, px), Image.LANCZOS)
			if state == 'down':
				pad = max(1, px // 36)
				frame = Image.new('RGBA', (px, px), (0, 0, 0, 0))
				frame.alpha_composite(v.resize((px - 2 * pad, px - 2 * pad), Image.LANCZOS), (pad, pad))
				v = frame
			v.save(os.path.join(folder, 'btn_%s_%s%s.png' % (name, state, suffix)))


def Build(out):
	folder = os.path.join(out, 'mobile_ui', 'akravon', 'hud')
	os.makedirs(folder, exist_ok = True)
	done = []
	for (fileName, names) in SHEETS:
		path = os.path.join(HERE, 'src', fileName)
		sheet = Image.open(path).convert('RGBA')
		boxes = Circles(path)
		if len(boxes) != len(names):
			raise SystemExit('%s: %d ikon bulundu, %d bekleniyordu' % (fileName, len(boxes), len(names)))
		for (name, box) in zip(names, boxes):
			icon = Cut(sheet, box)
			Save(folder, name, icon)
			done.append(name)
			if name == 'chat':
				# sohbet cubugu simgesi (chat_icon.png) - ayni yuvarlak ikon, mevcut dosya boyutunda
				size = Image.open(os.path.join(folder, 'chat_icon.png')).size[0]
				icon.resize((size, size), Image.LANCZOS).save(os.path.join(folder, 'chat_icon.png'))
			if name == 'refresh':
				icon.resize((256, 256), Image.LANCZOS).save(os.path.join(folder, 'hud_skill_pages.png'))	# hizli slot sayfa dugmesi
			if name == 'wrench':
				icon.resize((256, 256), Image.LANCZOS).save(os.path.join(folder, 'hud_skill_edit.png'))	# yerlesim duzenleme dugmesi
			if name == 'less':
				Arrow(icon).save(os.path.join(folder, 'chat_arrow.png'))	# sohbet cubugu oku (acik/kapali)
			if name == 'less':
				Save(folder, 'more', icon.rotate(180))		# menu kapaliyken asagi ok
	print('menu ikonlari: ' + ', '.join(done))


if __name__ == '__main__':
	Build(out)
