# -*- coding: utf-8 -*-
# AKRAVON ortak tema (referans: Karakter penceresi). Kaynak: kullanici paketi V2 (skin/akravon_ui_paketi/assets).
# Karar 2026-11-12: arka plan / cerceve / kartlar BIZIM; paketten yalniz dugmeler, oklar, arti / eksi, carpi.
# Gorseller yeniden cizilmez: 9 parca ile boyutlanir, LANCZOS ile olceklenir; metal kenarlar bizim cerceve altinina cekilir.
# Hepsi 2x uretilir (oyunda ExpandedImageBox SetScale 0.5): telefonda bulaniklik olmaz.
#   akr_btn_<w>x<h>_{normal,selected,pressed}.png   yazili dugme / sekme (w x h = oyundaki boyut)
#   akr_{plus,minus}_{normal,pressed,disabled}.png   64x64  (oyunda 32)
#   akr_close_{normal,pressed}.png                   68x68  (oyunda 34)
#   akr_arrow_{l,r}_{normal,pressed}.png             68x68  (oyunda 34)
#   akr_clear.png                                     1x1 seffaf (eski hazir sekme seridi gorselinin yerine)
import colorsys
import json
import os
import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
PACK = os.path.join(HERE, 'akravon_ui_paketi')
SRC = os.path.join(PACK, 'assets')
OUT = os.path.join(HERE, '..', '..', 'gorseller', 'ymir work', 'ui', 'akravon_legacy')
MANIFEST = json.load(open(os.path.join(PACK, 'manifest.json'), encoding = 'utf-8'))['assets']

# yazili dugme boyutlari (oyundaki): sol sekme, beceri grubu, gorev sekmeleri, bonus sekmeleri
BUTTON_SIZES = ((122, 40), (84, 36), (150, 36), (110, 36), (100, 36), (125, 36), (142, 44), (67, 42), (97, 36), (210, 36), (126, 44), (190, 44))

# bizim cerceve altini (cw_board2 kenari olculdu): ton 0.108, doygunluk 0.50; paket metali 0.104 / 0.68 (bronza kaciyor)
TARGET_HUE = 0.108
SAT_SCALE = 0.74


def Recolor(image):
	# yalniz metal (altin / bronz) pikseller: ton bizim altina, doygunluk dusurulur; lacivert / kahve dolgu degismez
	a = np.array(image.convert('RGBA')).astype(np.float32) / 255.0
	rgb = a[..., :3]
	mx = rgb.max(axis = 2)
	mn = rgb.min(axis = 2)
	v = mx
	s = np.where(mx > 0, (mx - mn) / np.maximum(mx, 1e-6), 0)
	r, g, b = rgb[..., 0], rgb[..., 1], rgb[..., 2]
	d = np.maximum(mx - mn, 1e-6)
	h = np.where(mx == r, ((g - b) / d) % 6, np.where(mx == g, (b - r) / d + 2, (r - g) / d + 4)) / 6.0
	metal = (h > 0.04) & (h < 0.17) & (s > 0.30) & (v > 0.40)
	h2 = np.where(metal, TARGET_HUE + (h - TARGET_HUE) * 0.5, h)
	s2 = np.where(metal, s * SAT_SCALE, s)
	out = np.zeros_like(rgb)
	flat_h, flat_s, flat_v = h2.reshape(-1), s2.reshape(-1), v.reshape(-1)
	conv = np.array([colorsys.hsv_to_rgb(x, y, z) for (x, y, z) in zip(flat_h, flat_s, flat_v)]).reshape(rgb.shape)
	out = np.where(metal[..., None], conv, rgb)
	a[..., :3] = out
	return Image.fromarray((a * 255).clip(0, 255).astype(np.uint8), 'RGBA')


CACHE = {}


def Load(name):
	if name not in CACHE:
		CACHE[name] = Recolor(Image.open(os.path.join(SRC, name + '.png')))
	return CACHE[name].copy()


def Save(image, name):
	image.save(os.path.join(OUT, name + '.png'))


def NineSlice(name, w, h, scale = None):
	src = Load(name)
	(sw, sh) = src.size
	(l, t, r, b) = MANIFEST.get(name, {}).get('nine_slice_ltrb') or (sw // 4, sh // 4, sw // 4, sh // 4)
	s = scale if scale else float(h) / sh
	L, T, R, B = [max(1, int(round(v * s))) for v in (l, t, r, b)]
	out = Image.new('RGBA', (w, h), (0, 0, 0, 0))
	xs = ((0, l, 0, L), (l, sw - r, L, w - R), (sw - r, sw, w - R, w))
	ys = ((0, t, 0, T), (t, sh - b, T, h - B), (sh - b, sh, h - B, h))
	for (x0, x1, dx0, dx1) in xs:
		for (y0, y1, dy0, dy1) in ys:
			if dx1 <= dx0 or dy1 <= dy0:
				continue
			out.alpha_composite(src.crop((x0, y0, x1, y1)).resize((dx1 - dx0, dy1 - dy0), Image.LANCZOS), (dx0, dy0))
	return out


def Uniform(name, size):
	return Load(name).resize((size, size), Image.LANCZOS)


def Gold(name):
	# basili gorunum: lacivert ic altin dolguya, ortadaki isaret koyu kahveye doner (kenar altin kalir)
	src = Load(name)
	a = np.array(src).astype(np.float32)
	w, h = src.size
	fill = np.array(Load('button_selected').crop((120, 40, 280, 88)).resize((w, h), Image.LANCZOS)).astype(np.float32)
	rgb = a[..., :3] / 255.0
	mx = rgb.max(axis = 2)
	mn = rgb.min(axis = 2)
	s = np.where(mx > 0, (mx - mn) / np.maximum(mx, 1e-6), 0)
	yy, xx = np.mgrid[0:h, 0:w]
	merkez = np.hypot(xx - w / 2.0, yy - h / 2.0) < min(w, h) * 0.30
	altin = (rgb[..., 0] > rgb[..., 2] + 0.08) & (s > 0.25) & (mx > 0.35)
	lacivert = (a[..., 3] > 0) & ~altin
	a[..., :3] = np.where(lacivert[..., None], fill[..., :3], a[..., :3])
	isaret = merkez & altin
	a[..., :3] = np.where(isaret[..., None], np.array([60, 38, 12], np.float32), a[..., :3])
	return Image.fromarray(a.clip(0, 255).astype(np.uint8), 'RGBA')


def Guild():
	# lonca: dugmeler (gd_btn*: up / over = lacivert, down = altin), sekmeler (gd_tabN_on altin + koyu yazi, off lacivert + krem yazi), arti
	from PIL import ImageDraw, ImageFont
	import glob
	for path in glob.glob(os.path.join(OUT, 'gd_btn*_up.png')):
		name = os.path.basename(path)[:-len('_up.png')]
		(w, h) = Image.open(path).size
		normal = NineSlice('button_normal', w, h)
		normal.save(os.path.join(OUT, name + '_up.png'))
		normal.save(os.path.join(OUT, name + '_over.png'))
		NineSlice('button_selected', w, h).save(os.path.join(OUT, name + '_down.png'))
	font = ImageFont.truetype(os.path.join(HERE, '..', '..', 'data', 'fonts', 'marcellus.ttf'), 20 * 4)
	labels = (u'BİLGİ', u'TAHTA', u'ÜYELER', u'ARAZİ', u'BECERİLER', u'RÜTBELER')
	for (i, label) in enumerate(labels):
		for on in (True, False):
			bg = NineSlice('button_selected' if on else 'button_normal', 140, 40)
			big = Image.new('RGBA', (140 * 4, 40 * 4), (0, 0, 0, 0))
			ImageDraw.Draw(big).text((70 * 4, 20 * 4), label, font = font, fill = (40, 24, 8, 255) if on else (232, 221, 197, 255), anchor = 'mm')
			bg.alpha_composite(big.resize((140, 40), Image.LANCZOS))
			bg.save(os.path.join(OUT, 'gd_tab%d_%s.png' % (i, 'on' if on else 'off')))
	for (state, src) in (('up', 'plus_normal'), ('over', 'plus_normal'), ('down', None)):
		im = Gold('plus_normal') if src is None else Load(src)
		im.resize((30, 30), Image.LANCZOS).save(os.path.join(OUT, 'gd_plus_%s.png' % state))


def Build():
	for (w, h) in BUTTON_SIZES:
		for state in ('normal', 'selected', 'pressed'):
			Save(NineSlice('button_' + state, w * 2, h * 2), 'akr_btn_%dx%d_%s' % (w, h, state))
	for state in ('normal', 'pressed', 'disabled'):
		Save(Uniform('plus_' + state, 64), 'akr_plus_' + state)
		Save(Uniform('minus_' + state, 64), 'akr_minus_' + state)
	for (kind, src, size) in (('plus', 'plus_normal', 64), ('minus', 'minus_normal', 64), ('close', 'close_normal', 68), ('arrow_l', 'arrow_left_normal', 68), ('arrow_r', 'arrow_right_normal', 68)):
		Save(Gold(src).resize((size, size), Image.LANCZOS), 'akr_%s_gold' % kind)
	for state in ('normal', 'pressed'):
		Save(Uniform('close_' + state, 68), 'akr_close_' + state)
		Save(Uniform('arrow_left_' + state, 68), 'akr_arrow_l_' + state)
		Save(Uniform('arrow_right_' + state, 68), 'akr_arrow_r_' + state)
	Image.new('RGBA', (1, 1), (0, 0, 0, 0)).save(os.path.join(OUT, 'akr_clear.png'))
	# eski ortak gorsellerin USTUNE tema (ayni ad, ayni boyut): kod degismeden her yerde yeni gorunum
	#   inv_close_1/3: kare carpi (normal / basili altin), 34 px    sb_btnW_1/2/3: dugme (normal, normal, basili altin), 44 yuksek
	#   sb_field: yazi kutusu (paket input_normal)
	Uniform('close_normal', 34).save(os.path.join(OUT, 'inv_close_1.png'))
	Gold('close_normal').resize((34, 34), Image.LANCZOS).save(os.path.join(OUT, 'inv_close_3.png'))
	for w in (120, 140, 170):
		normal = NineSlice('button_normal', w, 44)
		normal.save(os.path.join(OUT, 'sb_btn%d_1.png' % w))
		normal.save(os.path.join(OUT, 'sb_btn%d_2.png' % w))
		NineSlice('button_selected', w, 44).save(os.path.join(OUT, 'sb_btn%d_3.png' % w))
	NineSlice('input_normal', 150, 40).save(os.path.join(OUT, 'sb_field.png'))
	# pazar dugmeleri pz_btnW_1/2/3 (normal, secili altin, basili altin)
	for (w, h) in ((168, 52), (74, 46), (88, 46), (360, 44), (248, 52), (112, 52)):
		NineSlice('button_normal', w, h).save(os.path.join(OUT, 'pz_btn%d_1.png' % w))
		gold = NineSlice('button_selected', w, h)
		gold.save(os.path.join(OUT, 'pz_btn%d_2.png' % w))
		gold.save(os.path.join(OUT, 'pz_btn%d_3.png' % w))
	# ortak soru / uyari kutusu dugmesi (pp_btn_1 normal, pp_btn_3 basili altin) ve pazar sekmesi / alanlari
	NineSlice('button_normal', 190, 44).save(os.path.join(OUT, 'pp_btn_1.png'))
	NineSlice('button_selected', 190, 44).save(os.path.join(OUT, 'pp_btn_3.png'))
	NineSlice('button_selected', 69, 46).save(os.path.join(OUT, 'pz_tabs_on.png'))
	# envanter / depo sayfa sekmeleri: market deposuyla ayni (secili = altin kapsul)
	NineSlice('button_selected', 69, 42).save(os.path.join(OUT, 'inv_tab_on.png'))
	for n in (1, 2, 3, 4, 5):
		NineSlice('button_selected', 290 // n if n > 1 else 290, 42).save(os.path.join(OUT, 'sb_tab_on_%d.png' % n))
	NineSlice('input_normal', 348, 52).save(os.path.join(OUT, 'pz_field_348.png'))
	NineSlice('input_normal', 168, 46).save(os.path.join(OUT, 'pz_field_168.png'))
	NineSlice('input_normal', 360, 52).save(os.path.join(OUT, 'pz_field_wide.png'))
	NineSlice('input_normal', 348, 104).save(os.path.join(OUT, 'pz_field_card.png'))	# teklif karti
	Guild()
	print('tema: tamam')


if __name__ == '__main__':
	Build()
