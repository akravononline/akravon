# AKRAVON: oyuncu durum paneli ("suzulen" tasarim) - tamamen uretilir, referans gorsel kullanmaz
# portre halkasi + ondan uzanan egik uclu HP / SP cubuklari + ince EXP cizgisi
# cikti: status_panel (yuvalar + halka), status_hp / status_sp / status_exp (dolgu parcalari), hepsi 2 kat cozunurluk
import math
import numpy as np
import cv2
from PIL import Image, ImageDraw, ImageFilter

K = 2	# cikti carpani
SS = 4	# cizim icin ek ornekleme
S = K * SS
W, H = 262, 70	# mantiksal panel olcusu

RING_CENTER = (34, 35)
RING_RADIUS = 32.0
RING_WIDTH = 2.6
HAIR = 1.0	# cubuk yuvasinin bronz ince cizgisi
FILL_X = 62	# dolgularin gorunen baslangici (halkanin hemen sagi)

# x0: yuvanin sol ucu (halkanin arkasinda), x1: ust sag kose, y/h: yuva, slant: sag ucun egimi
BARS = {
	'hp': {'x0': 44, 'x1': 252, 'y': 23, 'h': 15, 'slant': 9, 'top': (212, 62, 62), 'bottom': (122, 24, 34)},
	'sp': {'x0': 44, 'x1': 234, 'y': 41, 'h': 11, 'slant': 7, 'top': (76, 124, 222), 'bottom': (30, 58, 146)},
	'exp': {'x0': 44, 'x1': 216, 'y': 55, 'h': 8, 'slant': 5, 'top': (244, 184, 72), 'bottom': (196, 122, 30)},
}
BRONZE_LIGHT = (178, 142, 88)
BRONZE_MID = (126, 96, 54)
BRONZE_DARK = (66, 47, 26)
NAVY_TOP = (13, 21, 40)
NAVY_BOTTOM = (6, 10, 21)


def Size():
	return (W * S, H * S)


def Para(x0, x1, y, h, slant, grow = 0.0):
	# egik sag uclu paralelkenar; grow kadar her yone buyur
	g = grow
	topRight = x1 + slant * g / float(h) + g
	bottomRight = x1 - slant - slant * g / float(h) + g
	return [(x0, y - g), (topRight, y - g), (bottomRight, y + h + g), (x0, y + h + g)]


def PolyMask(points):
	mask = Image.new('L', Size(), 0)
	ImageDraw.Draw(mask).polygon([(x * S, y * S) for (x, y) in points], fill = 255)
	return mask


def CircleMask(cx, cy, radius):
	mask = Image.new('L', Size(), 0)
	ImageDraw.Draw(mask).ellipse([(cx - radius) * S, (cy - radius) * S, (cx + radius) * S, (cy + radius) * S], fill = 255)
	return mask


def Subtract(a, b):
	return Image.fromarray(np.clip(np.asarray(a).astype(np.int16) - np.asarray(b).astype(np.int16), 0, 255).astype(np.uint8))


def VGrad(top, bottom, y0, y1):
	# y0..y1 (mantiksal) arasinda dikey gecis; disinda uc renkler
	ys = (np.arange(H * S, dtype = np.float32) / S - y0) / float(max(1e-6, y1 - y0))
	t = np.clip(ys, 0.0, 1.0)[:, None, None]
	row = np.array(top, np.float32) * (1 - t) + np.array(bottom, np.float32) * t
	return Image.fromarray(np.repeat(row, W * S, axis = 1).astype(np.uint8), 'RGB').convert('RGBA')


def Noise(scale, seed):
	rng = np.random.RandomState(seed)
	small = rng.rand(H * S // scale + 3, W * S // scale + 3).astype(np.float32)
	return cv2.resize(small, (W * S, H * S), interpolation = cv2.INTER_CUBIC)


def Tint(image, amount, seed, scale = 6):
	# yuzeye ince dokulu gurultu (tas / deri hissi)
	n = (Noise(scale, seed) - 0.5) * 2.0 * amount
	a = np.asarray(image).astype(np.float32)
	a[..., :3] = np.clip(a[..., :3] * (1.0 + n[..., None]), 0, 255)
	return Image.fromarray(a.astype(np.uint8), 'RGBA')


def Paint(canvas, shape, mask):
	layer = Image.new('RGBA', Size(), (0, 0, 0, 0))
	layer.paste(shape, (0, 0), mask)
	canvas.alpha_composite(layer)


def Veins(seed):
	# koyu lacivert tas: ince damarlar (cok hafif)
	n = 0.6 * Noise(70, seed) + 0.4 * Noise(28, seed + 1)
	veins = np.clip(1.0 - np.abs(np.sin(n * 15.0)) * 7.0, 0.0, 1.0) ** 2
	return cv2.GaussianBlur(veins, (0, 0), 0.9 * S / 4.0)


def Navy(y0, y1, seed):
	base = VGrad(NAVY_TOP, NAVY_BOTTOM, y0, y1)
	a = np.asarray(base).astype(np.float32)
	v = Veins(seed)[..., None]
	a[..., :3] = np.clip(a[..., :3] + v * (np.array((52, 76, 112), np.float32) - a[..., :3]) * 0.38, 0, 255)
	return Tint(Image.fromarray(a.astype(np.uint8), 'RGBA'), 0.05, seed + 2, 5)


def Bronze(y0, y1, seed):
	base = VGrad(BRONZE_LIGHT, BRONZE_DARK, y0, y1)
	a = np.asarray(base).astype(np.float32)
	a[..., :3] = a[..., :3] * 0.7 + np.array(BRONZE_MID, np.float32) * 0.3
	base = Image.fromarray(np.clip(a, 0, 255).astype(np.uint8), 'RGBA')
	return Tint(base, 0.10, seed, 3)


def Scrim(canvas, shapes, extra):
	# ince, yumusak koyu zemin: parlak arka planlarda yazilarin okunmasini saglar (kutu gibi gorunmez)
	union = Image.new('L', Size(), 0)
	for mask in shapes:
		union = Image.fromarray(np.maximum(np.asarray(union), np.asarray(mask)))
	d = ImageDraw.Draw(union)
	for (x0, y0, x1, y1) in extra:
		d.rectangle([x0 * S, y0 * S, x1 * S, y1 * S], fill = 255)
	soft = union.filter(ImageFilter.GaussianBlur(3.2 * S)).point(lambda v: int(min(255, v * 0.62)))
	layer = Image.new('RGBA', Size(), (3, 6, 14, 255))
	layer.putalpha(soft)
	canvas.alpha_composite(layer)


def Shadows(canvas, shapes):
	union = Image.new('L', Size(), 0)
	for mask in shapes:
		union = Image.fromarray(np.maximum(np.asarray(union), np.asarray(mask)))
	shifted = Image.new('L', Size(), 0)
	shifted.paste(union, (0, int(1.2 * S)))
	soft = shifted.filter(ImageFilter.GaussianBlur(1.7 * S))
	soft = soft.point(lambda v: int(v * 0.55))
	layer = Image.new('RGBA', Size(), (0, 0, 0, 255))
	layer.putalpha(soft)
	canvas.alpha_composite(layer)


def Fill(top, bottom, y, h, seed):
	fill = Tint(VGrad(top, bottom, y, y + h), 0.07, seed, 4)
	fa = np.asarray(fill).astype(np.float32)
	rows = (np.arange(H * S, dtype = np.float32) / S - y) / float(h)
	gloss = np.clip(1.0 - rows * 3.2, 0.0, 1.0)[:, None, None] * 0.28
	fa[..., :3] = np.clip(fa[..., :3] + gloss * (255 - fa[..., :3]), 0, 255)
	return Image.fromarray(fa.astype(np.uint8), 'RGBA')


def Slot(canvas, outer, inner, y, h):
	# yuva: ince bronz cizgi + koyu lacivert ic + ust ic golge
	Paint(canvas, Bronze(y - HAIR, y + h + HAIR, 3), outer)
	Paint(canvas, Navy(y, y + h, 11), inner)
	band = Image.new('L', Size(), 0)
	ImageDraw.Draw(band).rectangle([0, y * S, W * S, (y + max(1.2, h * 0.22)) * S], fill = 255)
	Paint(canvas, Image.new('RGBA', Size(), (0, 0, 0, 90)), Image.fromarray(np.minimum(np.asarray(band), np.asarray(inner))))


def BuildStatus(save):
	canvas = Image.new('RGBA', Size(), (0, 0, 0, 0))
	cx, cy = RING_CENTER
	ringOuter = CircleMask(cx, cy, RING_RADIUS)

	outers = {}
	inners = {}
	for (name, b) in BARS.items():
		outers[name] = PolyMask(Para(b['x0'], b['x1'], b['y'], b['h'], b['slant'], HAIR))
		inners[name] = PolyMask(Para(b['x0'], b['x1'], b['y'], b['h'], b['slant']))
	Scrim(canvas, list(outers.values()) + [ringOuter], ())
	Shadows(canvas, list(outers.values()) + [ringOuter])

	for (name, b) in BARS.items():
		Slot(canvas, outers[name], inners[name], b['y'], b['h'])
	# portre halkasi: lacivert damarli ic + dokulu bronz halka
	inner = CircleMask(cx, cy, RING_RADIUS - RING_WIDTH)
	Paint(canvas, Navy(cy - RING_RADIUS, cy + RING_RADIUS, 21), inner)
	ring = Subtract(ringOuter, inner)
	Paint(canvas, Bronze(cy - RING_RADIUS, cy + RING_RADIUS, 5), ring)
	# halkanin ic kenarinda ince koyu cizgi + ust kenarda soluk isik
	edge = Subtract(CircleMask(cx, cy, RING_RADIUS - RING_WIDTH + 0.7), CircleMask(cx, cy, RING_RADIUS - RING_WIDTH - 0.5))
	Paint(canvas, Image.new('RGBA', Size(), (4, 6, 12, 200)), edge)
	glow = Image.new('RGBA', Size(), (0, 0, 0, 0))
	r = (RING_RADIUS - 0.7) * S
	ImageDraw.Draw(glow).arc([cx * S - r, cy * S - r, cx * S + r, cy * S + r], 205, 300, fill = (236, 204, 150, 120), width = int(0.9 * S))
	canvas.alpha_composite(glow)

	save('status_panel', canvas.resize((W * K, H * K), Image.LANCZOS))

	# dolgu parcalari: yuvanin ici; sol uc halkanin kavisine gore kesilir
	ringClip = CircleMask(cx, cy, RING_RADIUS + 0.2)
	for (name, b) in BARS.items():
		mask = Subtract(inners[name], ringClip)
		layer = Image.new('RGBA', Size(), (0, 0, 0, 0))
		layer.paste(Fill(b['top'], b['bottom'], b['y'], b['h'], 40 + len(name)), (0, 0), mask)
		top, bottom = int(math.floor(b['y'])), int(math.ceil(b['y'] + b['h']))
		right = int(math.ceil(b['x1']))
		piece = layer.crop((FILL_X * S, top * S, right * S, bottom * S)).resize(((right - FILL_X) * K, (bottom - top) * K), Image.LANCZOS)
		save('status_%s' % name, piece)
		print('  %s dolgu: x=%d y=%d  boyut %dx%d' % (name, FILL_X, top, right - FILL_X, bottom - top))
	print('  halka merkezi: %d, %d  yaricap %.1f  panel %dx%d' % (RING_CENTER + (RING_RADIUS, W, H)))
