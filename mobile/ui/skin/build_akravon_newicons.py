# -*- coding: utf-8 -*-
# AKRAVON: yeni beceri ikonlari - game-icons.net (CC BY 3.0, skin/ikonlar/gameicons/license.txt)
# beyaz vektor ikon -> altin degrade + lacivert yuvarlak zemin + kademe parltisi (normal / M mavi / G mor / P altin)
# cikti: <out>/ymir work/ui/akravon_skill/<klasor>/<ad>_0N.png (build_akravon_skillicons.py ciktisinin uzerine)
import glob
import os
import shutil
import sys
from PIL import Image, ImageDraw, ImageFilter, ImageChops

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'ikonlar', 'gameicons')
DL = 'C:/AkravonTest/dl/gameicons/icons/ffffff/transparent/1x1'
SIZE = 64
SS = 4

# beceri numarasi -> game-icons adi
ICONS = {
	1: 'sword-slice', 2: 'sword-spin', 3: 'enrage', 4: 'sparkling-sabre', 5: 'sprint', 6: 'heart-plus',
	16: 'spectre', 17: 'punch-blast', 18: 'high-punch', 19: 'muscle-up', 20: 'sword-wound', 21: 'ringed-beam',
	31: 'backstab', 32: 'daggers', 33: 'spinning-blades', 34: 'invisible', 35: 'poison-cloud', 36: 'poison-bottle',
	46: 'arrow-flights', 47: 'arrow-cluster', 48: 'flaming-arrow', 49: 'running-shoe', 50: 'bow-arrow', 51: 'lightning-bow',
	61: 'glowing-hands', 62: 'dragon-spiral', 63: 'rune-sword', 64: 'terror', 65: 'magic-shield', 66: 'shield-disabled',
	76: 'orb-wand', 77: 'fire-ray', 78: 'spark-spirit', 79: 'barrier', 80: 'ghost', 81: 'double-ringed-orb',
	91: 'scroll-unfurled', 92: 'dragon-breath', 93: 'sonic-shout', 94: 'prayer', 95: 'shield-reflect', 96: 'dragon-orb',
	106: 'focused-lightning', 107: 'lightning-storm', 108: 'thunder-struck', 109: 'healing', 110: 'feathered-wing', 111: 'sword-brandish',
	170: 'claw-slashes', 171: 'wolf-head', 172: 'direwolf', 173: 'paw', 174: 'wolf-howl', 175: 'werewolf',
}

# kademe: (kenar rengi, parilti rengi, zemin ic, zemin dis)
GRADES = (
	((176, 140, 80), None, (24, 34, 62), (6, 9, 20)),			# normal
	((120, 170, 240), (80, 140, 255), (22, 40, 78), (6, 10, 26)),	# M
	((190, 130, 240), (160, 90, 255), (40, 26, 72), (10, 6, 24)),	# G
	((250, 214, 120), (255, 196, 80), (70, 50, 20), (18, 12, 6)),	# P
)


def Source(name):
	path = os.path.join(SRC, name + '.png')
	if not os.path.exists(path):
		found = glob.glob(os.path.join(DL, '*', name + '.png'))
		if not found:
			raise SystemExit('ikon yok: %s' % name)
		os.makedirs(SRC, exist_ok = True)
		shutil.copy(found[0], path)
	return Image.open(path).convert('RGBA')


def Gradient(size, top, bottom):
	im = Image.new('RGBA', (size, size))
	d = ImageDraw.Draw(im)
	for y in range(size):
		t = y / float(size - 1)
		d.line([(0, y), (size, y)], fill = tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3)) + (255,))
	return im


def Render(glyph, grade):
	(rim, glow, inner, outer) = GRADES[grade]
	big = SIZE * SS
	out = Image.new('RGBA', (big, big), (0, 0, 0, 0))
	# zemin: radyal lacivert
	bg = Image.new('RGBA', (big, big), outer + (255,))
	g = Image.new('L', (big, big), 0)
	ImageDraw.Draw(g).ellipse([big * 0.12, big * 0.08, big * 0.88, big * 0.84], fill = 255)
	g = g.filter(ImageFilter.GaussianBlur(big * 0.14))
	bg = Image.composite(Image.new('RGBA', (big, big), inner + (255,)), bg, g)
	circle = Image.new('L', (big, big), 0)
	ImageDraw.Draw(circle).ellipse([SS, SS, big - SS, big - SS], fill = 255)
	out.paste(bg, (0, 0), circle)
	# sembol: altin degrade, ortada %62
	gs = int(big * 0.62)
	mask = glyph.getchannel('A').resize((gs, gs), Image.LANCZOS)
	off = ((big - gs) // 2, (big - gs) // 2)
	if glow:
		halo = Image.new('L', (big, big), 0)
		halo.paste(mask, off)
		halo = halo.filter(ImageFilter.GaussianBlur(SS * 3))
		out.alpha_composite(Image.composite(Image.new('RGBA', (big, big), glow + (200,)), Image.new('RGBA', (big, big), (0, 0, 0, 0)), halo))
	shadow = Image.new('L', (big, big), 0)
	shadow.paste(mask, (off[0] + SS, off[1] + SS * 2))
	shadow = shadow.filter(ImageFilter.GaussianBlur(SS * 1.5))
	out.alpha_composite(Image.composite(Image.new('RGBA', (big, big), (0, 0, 0, 170)), Image.new('RGBA', (big, big), (0, 0, 0, 0)), shadow))
	gold = Gradient(gs, (255, 236, 176), (196, 140, 58) if grade != 3 else (236, 170, 60))
	layer = Image.new('RGBA', (big, big), (0, 0, 0, 0))
	layer.paste(gold, off, mask)
	out.alpha_composite(layer)
	# kenar
	d = ImageDraw.Draw(out)
	d.ellipse([SS, SS, big - SS, big - SS], outline = (16, 10, 4, 230), width = int(SS * 2.5))
	d.ellipse([SS * 2.5, SS * 2.5, big - SS * 2.5, big - SS * 2.5], outline = rim + (255,), width = int(SS * 1.6))
	out.putalpha(ImageChops.multiply(out.getchannel('A'), circle))
	return out.resize((SIZE, SIZE), Image.LANCZOS)


def Build(out):
	sys.path.insert(0, os.path.join(HERE, '..'))
	import importlib
	table = importlib.import_module('uiakravonskillicon').TABLE
	count = 0
	for (vnum, name) in sorted(ICONS.items()):
		base = table.get(vnum)
		if not base:
			continue
		glyph = Source(name)
		for grade in range(4):
			path = os.path.join(out, 'ymir work', 'ui', 'akravon_skill', base + '_%02d.png' % (grade + 1))
			os.makedirs(os.path.dirname(path), exist_ok = True)
			Render(glyph, grade).save(path)
		count += 1
	# P kademesi (_04) eski ikonlarda yoktu: tabloya ekle
	mod = os.path.join(HERE, '..', 'uiakravonskillicon.py')
	text = open(mod, encoding = 'utf-8').read()
	extra = sorted(set(table[v] + '_%02d' % g for v in ICONS if v in table for g in range(1, 5)))
	marker = '# AKRAVON yeni ikonlar'
	if marker not in text:
		text = text.replace('TABLE = {', 'MADE.update([%s])  %s\nTABLE = {' % (', '.join('"%s"' % e for e in extra), marker), 1)
		open(mod, 'w', encoding = 'utf-8', newline = '\n').write(text)
	lic = os.path.join(SRC, 'license.txt')
	if not os.path.exists(lic) and os.path.exists('C:/AkravonTest/dl/gameicons/icons/license.txt'):
		shutil.copy('C:/AkravonTest/dl/gameicons/icons/license.txt', lic)
	print('yeni beceri ikonu: %d beceri x 4 kademe' % count)


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
