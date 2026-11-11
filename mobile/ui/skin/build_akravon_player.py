# AKRAVON: oyuncu durum paneli ("suzulen" tasarim) - tamamen uretilir, referans gorsel kullanmaz
# portre halkasi + ondan uzanan egik uclu HP / SP cubuklari + ince EXP cizgisi
# cikti: status_panel (yuvalar + halka), status_hp / status_sp / status_exp (dolgu parcalari), hepsi 2 kat cozunurluk
import math
import os
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
FILL_X = 70	# dolgularin gorunen baslangici (halkanin hemen sagi)

# x0: yuvanin sol ucu (halkanin arkasinda), x1: ust sag kose, y/h: yuva, slant: sag ucun egimi
BARS = {
	'hp': {'x0': 70, 'x1': 224, 'y': 20, 'h': 17, 'slant': 0, 'top': (212, 62, 62), 'bottom': (122, 24, 34)},
	'sp': {'x0': 70, 'x1': 224, 'y': 39, 'h': 16, 'slant': 0, 'top': (76, 124, 222), 'bottom': (30, 58, 146)},
	'st': {'x0': 70, 'x1': 224, 'y': 58, 'h': 10, 'slant': 0, 'top': (244, 184, 72), 'bottom': (196, 122, 30)},
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


CORNER = 4.0	# portre cercevesi: yuvarlak kose kare (halka degil)


def CircleMask(cx, cy, radius):
	# AKRAVON: ad eski kaldi; artik yuvarlatilmis kare maske (portre kare)
	mask = Image.new('L', Size(), 0)
	corner = max(2.0, CORNER - (RING_RADIUS - radius))
	ImageDraw.Draw(mask).rounded_rectangle([(cx - radius) * S, (cy - radius) * S, (cx + radius) * S, (cy + radius) * S], radius = corner * S, fill = 255)
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


def BuildExp(save):
	# 4 EXP kutusunun yuvasi ve dolgusu: mantiksal 54x16 / 52x14, 4x ornekleme, 2 kat cikti
	w, h, q = 53, 16, 16
	slot = Image.new('RGBA', (w * q, h * q), (0, 0, 0, 0))
	dr = ImageDraw.Draw(slot)
	dr.rounded_rectangle([0, 0, w * q - 1, h * q - 1], radius = 2.8 * q, fill = BRONZE_MID + (255,))
	dr.rounded_rectangle([0.5 * q, 0.5 * q, w * q - 0.5 * q - 1, h * q - 0.5 * q - 1], radius = 2.4 * q, fill = BRONZE_LIGHT + (255,))
	inner = Image.new('RGBA', slot.size, (0, 0, 0, 0))
	ImageDraw.Draw(inner).rounded_rectangle([1.0 * q, 1.0 * q, w * q - 1.0 * q - 1, h * q - 1.0 * q - 1], radius = 2.0 * q, fill = NAVY_TOP + (255,))
	slot.alpha_composite(inner)
	save('exp_slot', slot.resize((w * 2, h * 2), Image.LANCZOS))
	fw, fh = w - 2, h - 2
	fill = Image.new('RGBA', (fw * q, fh * q), (0, 0, 0, 0))
	mask = Image.new('L', fill.size, 0)
	ImageDraw.Draw(mask).rounded_rectangle([0, 0, fw * q - 1, fh * q - 1], radius = 2.0 * q, fill = 255)
	grad = OrbFill(fw * q, fh * q)
	fill.paste(grad, (0, 0), mask)
	save('exp_fill', fill.resize((fw * 2, fh * 2), Image.LANCZOS))


def OrbFill(w, h):
	# AKRAVON EXP dolgusu: zengin altin degrade + camsi ust parlaklik + alt isik yayi (orijinal kure rengine yakin, daha canli)
	stops = ((0.00, (255, 236, 150)), (0.30, (250, 200, 70)), (0.62, (226, 150, 26)), (0.86, (176, 104, 10)), (1.00, (255, 214, 110)))
	rows = np.zeros((h, 3), np.float32)
	for y in range(h):
		t = y / float(max(1, h - 1))
		for n in range(len(stops) - 1):
			t0, c0 = stops[n]
			t1, c1 = stops[n + 1]
			if t0 <= t <= t1:
				f = (t - t0) / float(max(1e-6, t1 - t0))
				rows[y] = np.array(c0, np.float32) * (1 - f) + np.array(c1, np.float32) * f
				break
	img = np.repeat(rows[:, None, :], w, axis = 1)
	ys, xs = np.mgrid[0:h, 0:w].astype(np.float32)
	# camsi parlaklik: ust %42, yatayda ortada guclu, kenarlara dogru sonuyor
	gloss = np.clip(1.0 - ys / (h * 0.42), 0.0, 1.0) ** 1.4 * (0.55 + 0.45 * (1.0 - np.abs(xs - w * 0.5) / (w * 0.5)))
	img = img + gloss[..., None] * (255.0 - img) * 0.45
	# yan kenarlarda hafif koyulasma (derinlik)
	edge = np.clip(np.minimum(xs, w - 1 - xs) / (w * 0.06), 0.0, 1.0)
	img = img * (0.82 + 0.18 * edge[..., None])
	out = np.zeros((h, w, 4), np.uint8)
	out[..., :3] = np.clip(img, 0, 255)
	out[..., 3] = 255
	return Image.fromarray(out, 'RGBA')


def VGradBox(top, bottom, w, h):
	t = (np.arange(h, dtype = np.float32) / max(1, h - 1))[:, None, None]
	row = np.array(top, np.float32) * (1 - t) + np.array(bottom, np.float32) * t
	return Image.fromarray(np.repeat(row, w, axis = 1).astype(np.uint8), 'RGB').convert('RGBA')


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
	ImageDraw.Draw(glow).line([((cx - RING_RADIUS + CORNER) * S, (cy - RING_RADIUS + 0.8) * S), ((cx + RING_RADIUS - CORNER) * S, (cy - RING_RADIUS + 0.8) * S)], fill = (236, 204, 150, 120), width = int(0.9 * S))
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
