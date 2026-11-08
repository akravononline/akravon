# -*- coding: utf-8 -*-
# AKRAVON: harita yol tarifi gorselleri - altin ayak izi (sol / sag) ve hedef igne
# cikti: <out>/mobile_ui/akravon/atlas/foot_l.png, foot_r.png, pin.png
import os
import sys
from PIL import Image, ImageDraw, ImageFilter

out = sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out'
folder = os.path.join(out, 'mobile_ui', 'akravon', 'atlas')
os.makedirs(folder, exist_ok=True)

GOLD = (255, 214, 120, 255)
GOLD_DARK = (120, 80, 20, 255)
S = 4	# ust ornekleme


def Foot(left):
	w, h = 22, 32
	img = Image.new('RGBA', (w * S, h * S), (0, 0, 0, 0))
	d = ImageDraw.Draw(img)
	# taban + topuk
	d.ellipse((5 * S, 9 * S, 17 * S, 25 * S), fill=GOLD, outline=GOLD_DARK, width=S // 2)
	d.ellipse((6 * S, 22 * S, 16 * S, 31 * S), fill=GOLD, outline=GOLD_DARK, width=S // 2)
	# parmaklar
	for (x, y, r) in ((6, 6, 2.4), (9.6, 3.6, 2.2), (13.2, 3.6, 2.2), (16.4, 5.4, 2.0)):
		d.ellipse(((x - r) * S, (y - r) * S, (x + r) * S, (y + r) * S), fill=GOLD, outline=GOLD_DARK, width=S // 2)
	img = img.resize((w, h), Image.LANCZOS)
	if not left:
		img = img.transpose(Image.FLIP_LEFT_RIGHT)
	return img


def Pin():
	w, h = 30, 42
	img = Image.new('RGBA', (w * S, h * S), (0, 0, 0, 0))
	d = ImageDraw.Draw(img)
	d.polygon([(15 * S, 40 * S), (4 * S, 17 * S), (26 * S, 17 * S)], fill=(215, 40, 40, 255), outline=(90, 10, 10, 255))
	d.ellipse((3 * S, 3 * S, 27 * S, 27 * S), fill=(215, 40, 40, 255), outline=(90, 10, 10, 255), width=S)
	d.ellipse((10 * S, 10 * S, 20 * S, 20 * S), fill=(255, 235, 200, 255))
	return img.resize((w, h), Image.LANCZOS)


Foot(True).save(os.path.join(folder, 'foot_l.png'))
Foot(False).save(os.path.join(folder, 'foot_r.png'))
Pin().save(os.path.join(folder, 'pin.png'))
print('atlas gorselleri yazildi:', folder)
