# -*- coding: utf-8 -*-
# AKRAVON: lonca dugmesi (btn_guild_*) - menudeki diger ikonlarla AYNI renk / cerceve.
# Yontem: build_akravon_menu_icons.py'nin cerceve + Compose'u; bayrak ikonunun renk dagilimi tezgah/kitap/canta/karakter
# ikonlarina (histogram) esitlenir, sonra oyun ici ekran goruntusunden OLCULEN hedefe dogrusal duzeltilir.
# OYUN ICI OLCULEN HEDEF (Biyolog/Pazar ikon pikselleri, %10/%50/%90): (154,131,102) (182,158,127) (210,187,157)
# kullanim: python build_akravon_guild_icon.py [cikis]   -> btn_guild_* (normal/down/active, flat, 136/112/72/34)
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = (sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/lonca_out') + '/'
src = open(os.path.join(HERE, 'build_akravon_menu_icons.py'), encoding='utf-8').read().rstrip()
assert src.endswith('Main()')
ns = {'__file__': os.path.join(HERE, 'build_akravon_menu_icons.py'), '__name__': 'm'}
sys.argv = ['x', OUT]
exec(compile(src[:-6], 'm', 'exec'), ns)
Image = ns['Image']
os.makedirs(OUT, exist_ok = True)


def Opaque(name):
	a = np.asarray(ns['LoadIcon'](name)).astype(float)
	return a[a[..., 3] > 200][:, :3]


ref = np.concatenate([Opaque('tezgah'), Opaque('kitap'), Opaque('canta'), Opaque('karakter')])
ic = ns['LoadIcon']('bayrak')
arr = np.asarray(ic).astype(float)
m = arr[..., 3] > 128
px = arr[m][:, :3]
res = np.empty_like(px)
for k in range(3):
	order = np.argsort(px[:, k])
	ranks = np.empty(len(px))
	ranks[order] = np.arange(len(px)) / max(1, len(px) - 1)
	res[:, k] = np.quantile(ref[:, k], ranks)
cur = np.array([[163, 134, 97], [202, 167, 120], [234, 197, 146]], float)	# histogram esitlemesinden sonra oyunda olculen
tgt = np.array([[154, 131, 102], [182, 158, 127], [210, 187, 157]], float)
for k in range(3):
	(g, b) = np.polyfit(cur[:, k], tgt[:, k], 1)
	res[:, k] = res[:, k] * g + b
arr[m, :3] = np.clip(res, 0, 255)
icon = Image.fromarray(arr.astype(np.uint8))
icon.putalpha(ic.split()[-1])
frame = ns['LoadFrame']()
for state in ('normal', 'down', 'active'):
	for (suffix, size) in ns['SIZES'].items():
		ns['Compose'](frame, icon, size * 2, state).resize((size, size), Image.LANCZOS).save(OUT + 'btn_guild_%s%s.png' % (state, suffix))
		ns['ComposeFlat'](icon, size * 2, state).resize((size, size), Image.LANCZOS).save(OUT + 'btn_guild_flat_%s%s.png' % (state, suffix))
print('tamam ->', OUT)
