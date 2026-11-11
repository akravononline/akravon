# -*- coding: utf-8 -*-
# AKRAVON: ayarlar penceresi yuvarlak + / - dugmeleri -> akravon_legacy/opt_round_1.png (normal) / _3.png (basili); 40x40
import os
from PIL import Image, ImageDraw
SS = 4
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, '..', '..', 'gorseller', 'ymir work', 'ui', 'akravon_legacy')
GOLD = (222, 184, 110, 255)


def Round(down):
	S = 40
	big = Image.new('RGBA', (S * SS, S * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	fill = (150, 112, 56, 255) if down else (8, 11, 22, 255)
	d.ellipse((SS, SS, S * SS - SS - 1, S * SS - SS - 1), fill = fill, outline = GOLD, width = int(1.5 * SS))
	return big.resize((S, S), Image.LANCZOS)


os.makedirs(OUT, exist_ok = True)
Round(False).save(os.path.join(OUT, 'opt_round_1.png'))
Round(True).save(os.path.join(OUT, 'opt_round_3.png'))
print('ok')
