# -*- coding: utf-8 -*-
# AKRAVON: kullanicinin urettigi beceri ikonlari -> oyun
# skin/ikonlar/skill_<numara>_<kademe>.png (kademe 1 normal, 2 M, 3 G, 4 P; kare veya yuvarlak, herhangi boyut)
# -> <out>/ymir work/ui/akravon_skill/<klasor>/<ad>_0<kademe>.png (64 px, yuvarlak) + uiakravonskillicon.MADE
import glob
import os
import re
import sys
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, 'ikonlar')
SIZE = 64
MARKER = '# AKRAVON kullanici ikonlari'


def Round(path):
	im = Image.open(path).convert('RGBA')
	side = min(im.size)
	im = im.crop(((im.width - side) // 2, (im.height - side) // 2, (im.width + side) // 2, (im.height + side) // 2))
	big = im.resize((256, 256), Image.LANCZOS)
	mask = Image.new('L', (1024, 1024), 0)
	ImageDraw.Draw(mask).ellipse((4, 4, 1020, 1020), fill = 255)
	mask = mask.resize((256, 256), Image.LANCZOS)
	alpha = Image.eval(big.getchannel('A'), lambda v: v)
	from PIL import ImageChops
	big.putalpha(ImageChops.multiply(alpha, mask))
	return big.resize((SIZE, SIZE), Image.LANCZOS)


def Build(out):
	sys.path.insert(0, os.path.join(HERE, '..'))
	import importlib
	module = importlib.import_module('uiakravonskillicon')
	table = module.TABLE
	made = []
	for path in sorted(glob.glob(os.path.join(SRC, 'skill_*_*.png'))):
		m = re.match(r'skill_(\d+)_([1-4])\.png$', os.path.basename(path))
		if not m:
			continue
		(vnum, grade) = (int(m.group(1)), int(m.group(2)))
		base = table.get(vnum)
		if not base:
			print('ikon: %s - beceri %d tabloda yok' % (os.path.basename(path), vnum))
			continue
		dst = os.path.join(out, 'ymir work', 'ui', 'akravon_skill', base + '_%02d.png' % grade)
		os.makedirs(os.path.dirname(dst), exist_ok = True)
		Round(path).save(dst)
		made.append(base + '_%02d' % grade)
	# P kademesi (_04) eski ikonlarda yok: tabloya eklenir
	mod = os.path.join(HERE, '..', 'uiakravonskillicon.py')
	text = open(mod, encoding = 'utf-8').read()
	text = '\n'.join(l for l in text.split('\n') if MARKER not in l)
	if made:
		text = text.replace('TABLE = {', 'MADE.update([%s])  %s\nTABLE = {' % (', '.join('"%s"' % e for e in made), MARKER), 1)
	open(mod, 'w', encoding = 'utf-8', newline = '\n').write(text)
	print('kullanici ikonu: %d dosya' % len(made))


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
