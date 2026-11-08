# -*- coding: utf-8 -*-
# AKRAVON: Siralama penceresi - ayni boyutlarda lacivert/altin gorseller (uirank.py degismez)
import io, os, sys, zipfile
from PIL import Image, ImageDraw, ImageEnhance
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_akravon_charwin as cw
import build_akravon_inventory as inv
FILES = os.path.join(HERE, '..', '..', '..', '..', '2_Oyun_Dosyalari', 'files', 'mobile')
GOLD = (222, 184, 110, 255)
SS = 4


def Box(w, h, fill, line, r = 5, width = 1):
	b = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	ImageDraw.Draw(b).rounded_rectangle((0, 0, w * SS - 1, h * SS - 1), r * SS, fill = fill, outline = line, width = width * SS)
	return b.resize((w, h), Image.LANCZOS)


def Tint(src):
	# eski kahverengi zemin lacivert-koyuya cekilir, simgeler korunur
	im = src.convert('RGBA')
	navy = Image.new('RGBA', im.size, (14, 20, 38, 255))
	out = Image.blend(im, navy, 0.55)
	out.putalpha(im.getchannel('A'))
	return out


def Rows(w, h, n):
	im = Image.new('RGBA', (w, h), (0, 0, 0, 0))
	rh = h / float(n)
	medal = {0: (120, 92, 30, 240), 1: (90, 96, 112, 240), 2: (104, 66, 34, 240)}
	for i in range(n):
		y0, y1 = int(i * rh) + 1, int((i + 1) * rh) - 1
		fill = medal.get(i, (8, 11, 22, 235))
		line = GOLD if i < 3 else (64, 58, 44, 255)
		im.alpha_composite(Box(w - 2, y1 - y0, fill, line, 4), (1, y0))
	return im


FONT = os.path.join(HERE, '..', '..', 'data', 'fonts', 'latobd.ttf')
TABS = ((2, 58), (61, 122), (125, 186), (189, 250), (253, 314))


def Header(w, h):
	from PIL import ImageFont
	im = Box(w, h, (14, 20, 38, 250), GOLD, 4)
	d = ImageDraw.Draw(im)
	f = ImageFont.truetype(FONT, 12)
	for (x, label) in ((15, u'Sıra'), (85, u'İsim'), (175, u'Bayrak'), (300, u'Değer')):
		d.text((x, h // 2), label, font = f, fill = GOLD, anchor = 'mm')
	return im


def RaceBar(src, sel):
	src = src.convert('RGBA')
	im = Image.new('RGBA', src.size, (0, 0, 0, 0))
	for (i, (x0, x1)) in enumerate(TABS):
		on = i == sel
		# serit basliga gomulu: 26 px yukseklik, ustte/altta bosluk
		im.alpha_composite(Box(x1 - x0 - 4, 26, (60, 46, 22, 250) if on else (14, 20, 38, 245), GOLD if on else (96, 74, 42, 255), 5, 2 if on else 1), (x0 + 2, 4))
		cx = (x0 + x1) // 2
		face = src.crop((cx - 16, 7, cx + 16, 29)).resize((26, 18), Image.LANCZOS)
		if not on:
			face = ImageEnhance.Brightness(face).enhance(0.7)
		im.alpha_composite(face, (cx - 13, 8))
	return im


def Build(out):
	pa = zipfile.ZipFile(os.path.join(FILES, '13_patchadditions2_1.zip'))
	read = lambda n: Image.open(io.BytesIO(pa.read(n)))
	imgs = {
		'rank_window/board_base.png': cw.Board(788, 408),
		'rank_window/left_boardbase.png': inv.Slot(165, 340),
		'rank_window/render_boardbase.png': inv.Slot(165, 340),
		'rank_window/borderbg.png': inv.Slot(375, 340),
		'rank_window/right_list.tga': Rows(349, 283, 11),
		'rank_window/right_title_old.tga': Header(349, 23),
		'ymir work/ui/game/rank/norm.png': Box(145, 25, (14, 20, 38, 245), GOLD),
		'ymir work/ui/game/rank/norm1.png': Box(145, 25, (176, 136, 70, 255), GOLD),
		'ymir work/ui/game/rank/subbutton.png': Box(125, 25, (8, 11, 22, 235), (96, 74, 42, 255)),
		'ymir work/ui/game/rank/subbuttonbasili.png': Box(125, 25, (60, 46, 22, 245), GOLD),
	}
	for i in range(5):
		for sub in ('', 'y/'):
			n = 'rank_window/race/%srace_%d.tga' % (sub, i)
			imgs[n] = RaceBar(read(n), i)
	# kaydirma cubugu: lacivert iz, altin tutamak
	sb = 'rank_window/scrollbar/'
	track = lambda w, h: Image.new('RGBA', (w, h), (8, 11, 22, 235))
	for (n, size) in (('scrollbar_top', (13, 2)), ('scrollbar_bottom', (13, 2)), ('scrollbar_middle', (13, 1))):
		im = track(*size)
		d = ImageDraw.Draw(im)
		d.line((0, 0, 0, size[1]), fill = (96, 74, 42, 255))
		d.line((12, 0, 12, size[1]), fill = (96, 74, 42, 255))
		imgs[sb + n + '.tga'] = im
	thumb = (176, 136, 70, 255)
	for (n, size) in (('scrollbar_middle_top', (9, 8)), ('scrollbar_middle_bottom', (9, 8)), ('scrollbar_middle_middle', (9, 10)), ('scrollbar_middle_topscale', (9, 1)), ('scrollbar_middle_bottomscale', (9, 1))):
		im = Image.new('RGBA', size, (0, 0, 0, 0))
		d = ImageDraw.Draw(im)
		if n.endswith('_top'):
			d.rounded_rectangle((0, 0, 8, 12), 4, fill = thumb)
		elif n.endswith('_bottom'):
			d.rounded_rectangle((0, -5, 8, 7), 4, fill = thumb)
		else:
			d.rectangle((0, 0, 8, size[1]), fill = thumb)
		imgs[sb + n + '.tga'] = im
	for (name, im) in imgs.items():
		path = os.path.join(out, name)
		os.makedirs(os.path.dirname(path), exist_ok = True)
		im.save(path, format = 'TGA' if name.endswith('.tga') else 'PNG')
	print('siralama: %d gorsel' % len(imgs))


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
