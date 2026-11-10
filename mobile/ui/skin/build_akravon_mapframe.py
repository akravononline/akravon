# -*- coding: utf-8 -*-
# AKRAVON: kare harita cercevesi - yuvarlak suslu cercevenin bronzu + koseli suslerden (ic kisim saydam: harita gorunur)
# cikti: <out>/mobile_ui/akravon/hud/map_frame.png (272x274, olcek 0.5 -> 136x137 harita)
import os
import sys
import numpy as np
from PIL import Image, ImageDraw, ImageFilter
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/menu_out'
src = open(os.path.join(HERE, 'build_akravon_menu_icons.py'), encoding='utf-8').read().replace('\nMain()\n', '\n')
ns = {'__file__': os.path.join(HERE, 'build_akravon_menu_icons.py')}
argv = sys.argv
sys.argv = ['x', OUT]
exec(compile(src, 'menu_icons', 'exec'), ns)
sys.argv = argv
ring = ns['LoadFrame']()
n = ring.width

K = 4
W, H = 272 * K, 274 * K
img = Image.new('RGBA', (W, H), (0, 0, 0, 0))
d = ImageDraw.Draw(img)
BRONZE = [(30, 22, 14), (92, 72, 48), (160, 132, 98), (214, 190, 156), (168, 140, 104), (110, 86, 58), (52, 40, 26)]
band = 9 * K	# dis kabartma
for (i, color) in enumerate(BRONZE):
	inset = int(i * band / len(BRONZE))
	d.rectangle((inset, inset, W - 1 - inset, H - 1 - inset), outline=color + (255,), width=int(band / len(BRONZE)) + 1)
gap = band + 3 * K
d.rectangle((band, band, W - 1 - band, H - 1 - band), outline=(10, 8, 6, 235), width=3 * K)
d.rectangle((gap, gap, W - 1 - gap, H - 1 - gap), outline=(150, 124, 92, 255), width=int(1.5 * K))
d.rectangle((gap + int(1.5 * K), gap + int(1.5 * K), W - 1 - gap - int(1.5 * K), H - 1 - gap - int(1.5 * K)), outline=(20, 15, 10, 200), width=K)
# hafif doku
noise = (np.random.RandomState(7).rand(H, W) * 26 - 13).astype(np.float32)
arr = np.asarray(img).astype(np.float32)
mask = arr[..., 3] > 0
for c in range(3):
	arr[..., c] = np.where(mask, np.clip(arr[..., c] + noise, 0, 255), arr[..., c])
img = Image.fromarray(arr.astype(np.uint8))
# susler (yuvarlak cerceveden)
top = ring.crop((430, 18, 528, 66))
side = ring.crop((24, 416, 72, 514))
scale = (W * 0.26) / 939.0 * 939.0 / 272.0 / 4.0	# yaklasik: sus genisligi cercevenin ~%14'u
tw = int(W * 0.15)
th = int(top.height * tw / top.width)
topS = top.resize((tw, th), Image.LANCZOS)
sw = th
sh = int(side.height * sw / side.width)
sideS = side.resize((sw, sh), Image.LANCZOS)
cy = gap // 2
img.alpha_composite(topS, ((W - tw) // 2, max(0, cy - th // 2)))
img.alpha_composite(topS.rotate(180), ((W - tw) // 2, min(H - th, H - cy - th // 2)))
img.alpha_composite(sideS, (max(0, cy - sw // 2), (H - sh) // 2))
img.alpha_composite(sideS.transpose(Image.FLIP_LEFT_RIGHT), (min(W - sw, W - cy - sw // 2), (H - sh) // 2))
shadow = img.split()[-1].filter(ImageFilter.GaussianBlur(2 * K))
out = img.resize((272, 274), Image.LANCZOS)
dest = os.path.join(OUT, 'mobile_ui', 'akravon', 'hud')
os.makedirs(dest, exist_ok=True)
out.save(os.path.join(dest, 'map_frame.png'))
print('tamam', os.path.join(dest, 'map_frame.png'))
