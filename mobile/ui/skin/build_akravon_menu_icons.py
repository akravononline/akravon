# -*- coding: utf-8 -*-
# AKRAVON: menu butonlari - suslu yuvarlak cerceve + kum-altin kabartmali ikon.
# 1) ref_menu/tablo_N.png (7 ikon tablosu, saydam) -> tek tek ikonlar (ref_menu/ikon/<ad>.png)
# 2) cerceve_suslu.png + ikon -> mobile_ui/akravon/hud/btn_<ad>_{normal,down,active}[_m|_s|_xs].png
# kullanim: python build_akravon_menu_icons.py [cikis_klasoru]
import os
import sys

import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
REF = os.path.join(HERE, 'ref_menu')
OUT = sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/menu_out'
ICON_DIR = os.path.join(REF, 'ikon')

# tablo dosyasi -> (satirdaki ikon sayisi listesi, adlar soldan saga, yukaridan asagi)
SHEETS = {
	'tablo_1.png': ([3, 3, 3], ['bot', 'kumsaati', 'pusula', 'kitap', 'demirci', 'hedef', 'cikis', 'anlasma', 'yemek']),
	'tablo_2.png': ([4, 4], ['soru', 'kupa', 'taj', 'kafatasi', 'kalp', 'ates', 'simsek', 'goz']),
	'tablo_3.png': ([4, 4], ['kese', 'takvim', 'arti', 'eksi', 'unlem', 'carpi', 'zil', 'bilgi']),
	'tablo_4.png': ([4, 4], ['ayarlist', 'yenile', 'kalkan', 'sandik', 'ferman', 'harita', 'iksir', 'elmas']),
	'tablo_5.png': ([4, 4], ['mektup', 'bayrak', 'ev', 'kilit', 'anahtar', 'mercek', 'hediye', 'uyari']),
	'tablo_6.png': ([4, 4], ['canta', 'pazar', 'karakter', 'yukari', 'siralama', 'grup', 'disli', 'otoav']),
	'tablo_8.png': ([5], ['somun', 'dondur', 'baglanti', 'tezgah', 'kure']),
	'tablo_7.png': ([4, 4], ['kilic', 'bitki', 'pazar2', 'para', 'binek', 'sohbet', 'ucak', 'onay']),
}

# oyun dugmesi -> ikon adi
BUTTONS = {
	'inventory': 'canta',
	'itemshop': 'elmas',
	'character': 'karakter',
	'more': 'yukari',
	'less': 'yukari_ters',
	'messenger': 'grup',
	'system': 'disli',
	'autohunt': 'otoav',
	# sag sutun
	'pickup': 'para',
	'mount': 'binek',
	# harita yani / ustu
	'zoomin': 'arti',
	'zoomout': 'eksi',
	'collapse': 'yukari',
	'ranking': 'siralama',
	'biolog': 'kitap',
	'market': 'tezgah',
	'swords': 'kilic',
	# sohbet ve digerleri
	'chat': 'sohbet',
	'send': 'ucak',
	'bell': 'zil',
	'event': 'takvim',
	'quest': 'ferman',
	'refresh': 'dondur',
	'wrench': 'somun',
	'shield': 'kalkan',
	'timer': 'kumsaati',
	'channel': 'kure',
	'link': 'baglanti',
	'pin': 'hedef',
	'close': 'carpi',
	# ekipman paneli (envanter)
	'depo': 'sandik',
	'kostum': 'hediye',
}
SIZES = {'': 136, '_m': 112, '_s': 72, '_xs': 34}
FIT_RADIUS = 0.335	# ikonun en uzak noktasi / cerceve capi
FIT_LIMIT = 0.62	# ikon kutusu / cerceve capi
# ikon rengi: 'altin' (tablodaki kum-altin) | 'fildisi' (soluk bej-gri) | 'halka' (cerceve halkasinin kendi bronz rengi)
TONE = os.environ.get('AKRAVON_TONE', 'altin')
RING_STOPS = [(0.0, (8, 6, 4)), (0.22, (44, 34, 22)), (0.45, (84, 66, 46)), (0.70, (140, 114, 84)), (0.90, (196, 170, 140)), (1.0, (228, 208, 177))]
IVORY_STOPS = [(0.0, (22, 19, 17)), (0.30, (92, 84, 74)), (0.65, (170, 158, 140)), (1.0, (232, 222, 202))]


def ApplyTone(icon):
	stops = {'fildisi': IVORY_STOPS, 'halka': RING_STOPS}.get(TONE)
	if stops is None:
		return icon
	arr = np.asarray(icon).astype(np.float32)
	lum = (arr[..., 0] * 0.3 + arr[..., 1] * 0.59 + arr[..., 2] * 0.11) / 255.0
	lum = np.clip(lum * 1.25, 0, 1)	# hafif acik ton
	xs = [t[0] for t in stops]
	for k in range(3):
		arr[..., k] = np.interp(lum, xs, [t[1][k] for t in stops])
	return Image.fromarray(arr.astype(np.uint8))


def CleanIcon(rgba):
	rgb = rgba[..., :3].astype(np.float32)
	alpha = rgba[..., 3].astype(np.float32)
	core = (alpha > 235).astype(np.float32)
	core = cv2.erode(core, np.ones((3, 3), np.uint8))
	blur = cv2.GaussianBlur(rgb * core[..., None], (0, 0), 6)
	weight = cv2.GaussianBlur(core, (0, 0), 6)[..., None]
	filled = blur / np.maximum(weight, 1e-3)
	use = (alpha < 240)[..., None] & (weight > 0.02)
	rgb = np.where(use, filled, rgb)
	alpha = np.clip((alpha - 70.0) / 150.0, 0, 1) * 255.0
	alpha = cv2.erode(alpha, np.ones((2, 2), np.uint8))
	return np.dstack([np.clip(rgb, 0, 255), alpha]).astype(np.uint8)


def ExtractSheet(fileName, rowCounts, names):
	image = Image.open(os.path.join(REF, fileName)).convert('RGBA')
	data = np.asarray(image)
	mask = (data[..., 3] > 40).astype(np.uint8)
	merged = cv2.dilate(mask, np.ones((31, 31), np.uint8))
	(count, labels, stats, centroids) = cv2.connectedComponentsWithStats(merged)
	items = [(stats[i, cv2.CC_STAT_AREA], i) for i in range(1, count)]
	items.sort(reverse = True)
	items = items[:sum(rowCounts)]
	items.sort(key = lambda it: centroids[it[1]][1])
	ordered = []
	start = 0
	for rowCount in rowCounts:
		row = sorted(items[start:start + rowCount], key = lambda it: centroids[it[1]][0])
		ordered += row
		start += rowCount
	for ((area, index), name) in zip(ordered, names):
		region = (labels == index) & (mask > 0)
		ys, xs = np.where(region)
		(x0, x1, y0, y1) = (xs.min(), xs.max() + 1, ys.min(), ys.max() + 1)
		crop = data[y0:y1, x0:x1].copy()
		crop[..., 3] = np.where(region[y0:y1, x0:x1], crop[..., 3], 0)
		Image.fromarray(CleanIcon(crop)).save(os.path.join(ICON_DIR, name + '.png'))


def LoadFrame():
	frame = Image.open(os.path.join(REF, 'cerceve_suslu.png')).convert('RGB')
	gray = np.asarray(frame).astype(float).mean(2)
	ys, xs = np.where(gray > 28)
	(x0, x1, y0, y1) = (xs.min(), xs.max(), ys.min(), ys.max())
	size = max(x1 - x0, y1 - y0) + 1
	(cx, cy) = ((x0 + x1) / 2, (y0 + y1) / 2)
	square = frame.crop((int(cx - size / 2), int(cy - size / 2), int(cx + size / 2), int(cy + size / 2))).convert('RGBA')
	circle = Image.new('L', square.size, 0)
	ImageDraw.Draw(circle).ellipse((1, 1, square.width - 2, square.height - 2), fill = 255)
	square.putalpha(circle.filter(ImageFilter.GaussianBlur(1.2)))
	return square


def LoadIcon(name):
	if name.endswith('_ters'):
		return Image.open(os.path.join(ICON_DIR, name[:-5] + '.png')).convert('RGBA').transpose(Image.FLIP_TOP_BOTTOM)
	return Image.open(os.path.join(ICON_DIR, name + '.png')).convert('RGBA')


def FitRatio(icon, size, radius = FIT_RADIUS, limit = FIT_LIMIT):
	# ikonun en uzak (opak) noktasi dairenin ic yaricapina degsin; bounding box de sinirda kalsin
	alpha = np.asarray(icon.split()[-1])
	(ys, xs) = np.where(alpha > 128)
	(cx, cy) = ((xs.min() + xs.max()) / 2.0, (ys.min() + ys.max()) / 2.0)
	rmax = float(np.max(np.hypot(xs - cx, ys - cy))) + 1.0
	return min(radius * size / rmax, limit * size / float(max(icon.width, icon.height)))


def Compose(frame, icon, size, state):
	base = frame.resize((size, size), Image.LANCZOS)
	ratio = FitRatio(icon, size)
	small = icon.resize((max(1, int(icon.width * ratio)), max(1, int(icon.height * ratio))), Image.LANCZOS)
	if state == 'active':
		arr = np.asarray(small).astype(np.float32)
		arr[..., :3] = np.clip(arr[..., :3] * 1.12 + 10, 0, 255)
		small = Image.fromarray(arr.astype(np.uint8))
	pos = ((size - small.width) // 2, (size - small.height) // 2)
	shadow = Image.new('RGBA', base.size, (0, 0, 0, 0))
	shadow.alpha_composite(small, (pos[0] + max(1, size // 70), pos[1] + max(1, size // 50)))
	shadowAlpha = shadow.split()[-1].point(lambda v: int(v * 0.6)).filter(ImageFilter.GaussianBlur(max(1, size / 60.0)))
	shade = Image.new('RGBA', base.size, (0, 0, 0, 255))
	shade.putalpha(shadowAlpha)
	base.alpha_composite(shade)
	base.alpha_composite(small, pos)
	if state == 'down':
		arr = np.asarray(base).astype(np.float32)
		arr[..., :3] *= 0.72
		base = Image.fromarray(arr.astype(np.uint8))
		inner = int(size * 0.94)
		canvas = Image.new('RGBA', base.size, (0, 0, 0, 0))
		canvas.alpha_composite(base.resize((inner, inner), Image.LANCZOS), ((size - inner) // 2, (size - inner) // 2))
		base = canvas
	elif state == 'active':
		glow = Image.new('RGBA', base.size, (0, 0, 0, 0))
		ring = ImageDraw.Draw(glow)
		ring.ellipse((size * 0.02, size * 0.02, size * 0.98, size * 0.98), outline = (255, 214, 120, 210), width = max(1, size // 28))
		base.alpha_composite(glow.filter(ImageFilter.GaussianBlur(max(1, size / 45.0))))
	return base


def ComposeFlat(icon, size, state):
	# cercevesiz (duz) ikon: tablo acikken yuvarlak olmadan
	base = Image.new('RGBA', (size, size), (0, 0, 0, 0))
	ratio = FitRatio(icon, size, 0.47, 0.90)
	small = icon.resize((max(1, int(icon.width * ratio)), max(1, int(icon.height * ratio))), Image.LANCZOS)
	if state == 'active':
		arr = np.asarray(small).astype(np.float32)
		arr[..., :3] = np.clip(arr[..., :3] * 1.15 + 12, 0, 255)
		small = Image.fromarray(arr.astype(np.uint8))
	pos = ((size - small.width) // 2, (size - small.height) // 2)
	shadow = Image.new('RGBA', base.size, (0, 0, 0, 0))
	shadow.alpha_composite(small, (pos[0] + max(1, size // 70), pos[1] + max(1, size // 50)))
	shadowAlpha = shadow.split()[-1].point(lambda v: int(v * 0.7)).filter(ImageFilter.GaussianBlur(max(1, size / 50.0)))
	shade = Image.new('RGBA', base.size, (0, 0, 0, 255))
	shade.putalpha(shadowAlpha)
	base.alpha_composite(shade)
	base.alpha_composite(small, pos)
	if state == 'down':
		arr = np.asarray(base).astype(np.float32)
		arr[..., :3] *= 0.72
		base = Image.fromarray(arr.astype(np.uint8))
	return base


def SaveStandalone(dest, frame):
	# sohbet cubugundaki cercevesiz ikonlar + rozet
	def Fit(icon, width, height):
		ratio = min(width * 2 / float(icon.width), height * 2 / float(icon.height))
		big = icon.resize((max(1, int(icon.width * ratio)), max(1, int(icon.height * ratio))), Image.LANCZOS)
		canvas = Image.new('RGBA', (width * 2, height * 2), (0, 0, 0, 0))
		canvas.alpha_composite(big, ((width * 2 - big.width) // 2, (height * 2 - big.height) // 2))
		return canvas.resize((width, height), Image.LANCZOS)
	ApplyTone(LoadIcon('sohbet')).convert('RGBA')
	Fit(ApplyTone(LoadIcon('sohbet')), 48, 45).save(os.path.join(dest, 'chat_icon.png'))
	chevron = ApplyTone(LoadIcon('yukari'))
	stack = Image.new('RGBA', (chevron.width, int(chevron.height * 1.75)), (0, 0, 0, 0))
	stack.alpha_composite(chevron, (0, 0))
	stack.alpha_composite(chevron, (0, int(chevron.height * 0.75)))
	Fit(stack, 40, 39).save(os.path.join(dest, 'chat_arrow.png'))
	frame.resize((44, 44), Image.LANCZOS).save(os.path.join(dest, 'chat_badge_dot.png'))
	# beceri halkasindaki ayar (duzenle) ve sayfa dugmeleri
	Compose(frame, ApplyTone(LoadIcon('somun')), 512, 'normal').resize((256, 256), Image.LANCZOS).save(os.path.join(dest, 'hud_skill_edit.png'))
	Compose(frame, ApplyTone(LoadIcon('dondur')), 512, 'normal').resize((256, 256), Image.LANCZOS).save(os.path.join(dest, 'hud_skill_pages.png'))


def SaveMapFrame(dest, frame):
	# minimap: kare cerceve yerine yuvarlak susu halka (ic kismi saydam; harita daire)
	(width, height) = (272, 274)
	size = 272
	ring = frame.resize((size * 2, size * 2), Image.LANCZOS)
	hole = Image.new('L', ring.size, 255)
	inner = int(size * 2 * 0.5 * 0.865)
	ImageDraw.Draw(hole).ellipse((size - inner, size - inner, size + inner, size + inner), fill = 0)
	hole = hole.filter(ImageFilter.GaussianBlur(1.5))
	alpha = np.minimum(np.asarray(ring.split()[-1]), np.asarray(hole))
	ring.putalpha(Image.fromarray(alpha.astype(np.uint8)))
	out = Image.new('RGBA', (width * 2, height * 2), (0, 0, 0, 0))
	out.alpha_composite(ring, (0, (height - size)))
	out.resize((width, height), Image.LANCZOS).save(os.path.join(dest, 'map_frame.png'))


def Main():
	os.makedirs(ICON_DIR, exist_ok = True)
	for (fileName, (rowCounts, names)) in SHEETS.items():
		ExtractSheet(fileName, rowCounts, names)
	frame = LoadFrame()
	dest = os.path.join(OUT, 'mobile_ui', 'akravon', 'hud')
	os.makedirs(dest, exist_ok = True)
	for (button, iconName) in BUTTONS.items():
		icon = ApplyTone(LoadIcon(iconName))
		for state in ('normal', 'down', 'active'):
			for (suffix, size) in SIZES.items():
				Compose(frame, icon, size * 2, state).resize((size, size), Image.LANCZOS).save(os.path.join(dest, 'btn_%s_%s%s.png' % (button, state, suffix)))
				ComposeFlat(icon, size * 2, state).resize((size, size), Image.LANCZOS).save(os.path.join(dest, 'btn_%s_flat_%s%s.png' % (button, state, suffix)))
	SaveStandalone(dest, frame)
	print('tamam', len(BUTTONS) * 6 * len(SIZES), 'dosya ->', dest)


Main()
