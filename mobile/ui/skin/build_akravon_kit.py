# AKRAVON: ortak HUD cizim kiti - tum oyun ici butonlar / yuvalar / kilit ayni dili kullanir
# dil: koyu lacivert damarli tas yuzey + eskitilmis mat bronz ince cizgi + krem ince cizgili ikonlar; altin yok
# kaynak: skin/hud_line_icons_a.png ve hud_line_icons_b.png (3x3 ikon sayfalari, yesil zemin)
import math
import os
import numpy as np
import cv2
from PIL import Image, ImageDraw, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
ICON_SHEETS = {'a': os.path.join(HERE, 'hud_line_icons_a.png'), 'b': os.path.join(HERE, 'hud_line_icons_b.png')}
ICON_NAMES = {
	'a': ('inventory', 'character', 'itemshop', 'messenger', 'system', 'event', 'quest', 'channel', 'lock_closed'),
	'b': ('pickup', 'autohunt', 'mount', 'chat', 'shield', 'bell', 'timer', 'pin', 'attack'),
}
CREAM = (234, 223, 196)
BRONZE_LIGHT = (176, 140, 86)
BRONZE_MID = (126, 96, 54)
BRONZE_DARK = (66, 47, 26)
NAVY_TOP = (16, 25, 46)
NAVY_BOTTOM = (7, 12, 24)
SS = 4	# cizim icin ek ornekleme


def Gradient(width, height, top, bottom):
	t = np.linspace(0.0, 1.0, height, dtype = np.float32)[:, None, None]
	row = np.array(top, np.float32) * (1 - t) + np.array(bottom, np.float32) * t
	return np.repeat(row, width, axis = 1)


def Noise(width, height, scale, seed):
	rng = np.random.RandomState(seed)
	small = rng.rand(height // scale + 3, width // scale + 3).astype(np.float32)
	return cv2.resize(small, (width, height), interpolation = cv2.INTER_CUBIC)


def NavyTexture(width, height, seed, lift = 0.0, top = NAVY_TOP, bottom = NAVY_BOTTOM):
	# damarli koyu lacivert tas; lift > 0 aktif durumda biraz acar
	base = Gradient(width, height, top, bottom)
	n = 0.6 * Noise(width, height, max(8, width // 3), seed) + 0.4 * Noise(width, height, max(4, width // 8), seed + 1)
	veins = np.clip(1.0 - np.abs(np.sin(n * 15.0)) * 7.0, 0.0, 1.0) ** 2
	veins = cv2.GaussianBlur(veins, (0, 0), max(0.6, width / 220.0))[..., None]
	out = base + veins * (np.array((54, 78, 114), np.float32) - base) * 0.34
	out += (Noise(width, height, 3, seed + 2) - 0.5)[..., None] * 7.0
	out += lift
	return np.clip(out, 0, 255).astype(np.uint8)


def BronzeTexture(width, height, seed, brighten = 1.0):
	top = np.array(BRONZE_LIGHT, np.float32) * brighten
	base = Gradient(width, height, top, BRONZE_DARK)
	base = base * 0.7 + np.array(BRONZE_MID, np.float32) * 0.3
	base *= (1.0 + (Noise(width, height, 5, seed) - 0.5) * 0.3)[..., None]
	return np.clip(base, 0, 255).astype(np.uint8)


def PolyMask(size, points, ss = SS):
	mask = Image.new('L', (size[0] * ss, size[1] * ss), 0)
	ImageDraw.Draw(mask).polygon([(x * ss, y * ss) for (x, y) in points], fill = 255)
	return mask.resize(size, Image.LANCZOS)


def CutRect(x0, y0, x1, y1, cut):
	# kesik (pahli) koseli dikdortgen
	return [(x0 + cut, y0), (x1 - cut, y0), (x1, y0 + cut), (x1, y1 - cut), (x1 - cut, y1), (x0 + cut, y1), (x0, y1 - cut), (x0, y0 + cut)]


def Layer(texture, mask):
	img = Image.fromarray(texture, 'RGB').convert('RGBA')
	img.putalpha(mask)
	return img


def Shadow(size, mask, offset, blur, strength):
	shifted = Image.new('L', size, 0)
	shifted.paste(mask, (0, offset))
	soft = shifted.filter(ImageFilter.GaussianBlur(blur)).point(lambda v: int(v * strength))
	layer = Image.new('RGBA', size, (0, 0, 0, 255))
	layer.putalpha(soft)
	return layer


def Plate(px, state, seed = 3):
	# kare buton plakasi (px x px): kesik koseli, ince bronz cizgi, damarli lacivert ic
	size = (px, px)
	pad = max(3, px // 22)
	hair = max(2.0, px / 56.0)
	cut = px * 0.11
	canvas = Image.new('RGBA', size, (0, 0, 0, 0))
	outerPts = CutRect(pad, pad, px - pad, px - pad, cut)
	innerPts = CutRect(pad + hair, pad + hair, px - pad - hair, px - pad - hair, max(1.0, cut - hair * 0.4))
	outer, inner = PolyMask(size, outerPts), PolyMask(size, innerPts)
	canvas.alpha_composite(Shadow(size, outer, max(1, px // 36), px / 40.0, 0.55 if state != 'down' else 0.3))
	bright = {'normal' : 1.0, 'active' : 1.28, 'down' : 0.78}[state]
	canvas.alpha_composite(Layer(BronzeTexture(px, px, seed, bright), outer))
	lift = {'normal' : 0.0, 'active' : 9.0, 'down' : -3.0}[state]
	canvas.alpha_composite(Layer(NavyTexture(px, px, seed + 10, lift), inner))
	# ic ust golge (oyuk his)
	band = Image.new('L', size, 0)
	ImageDraw.Draw(band).rectangle([0, 0, px, pad + hair + px * 0.12], fill = 255)
	shade = Image.new('RGBA', size, (0, 0, 0, 255))
	shade.putalpha(Image.fromarray(np.minimum(np.asarray(band), np.asarray(inner)) // 3))
	canvas.alpha_composite(shade)
	if state == 'active':
		glow = Image.new('RGBA', size, (232, 176, 92, 255))
		edge = Image.fromarray(np.clip(np.asarray(outer).astype(np.int16) - np.asarray(PolyMask(size, CutRect(pad + hair * 3, pad + hair * 3, px - pad - hair * 3, px - pad - hair * 3, cut * 0.8))).astype(np.int16), 0, 255).astype(np.uint8))
		glow.putalpha(edge.filter(ImageFilter.GaussianBlur(px / 30.0)).point(lambda v: int(v * 0.26)))
		canvas.alpha_composite(glow)
	return canvas


def LoadIcons():
	icons = {}
	for (key, path) in ICON_SHEETS.items():
		sheet = np.asarray(Image.open(path).convert('RGB')).astype(np.float32)
		h, w = sheet.shape[:2]
		# yesil zemini sil, kenarlardaki yesil tasmasini kirp; ikonlarin kendi bronz rengi korunur
		d = sheet[..., 1] - np.maximum(sheet[..., 0], sheet[..., 2])
		alpha = np.clip((95.0 - d) / 70.0, 0.0, 1.0)
		alpha = np.where(d <= 25, 1.0, alpha)
		green = np.minimum(sheet[..., 1], np.maximum(sheet[..., 0], sheet[..., 2]) + 6)
		rgba = np.dstack([sheet[..., 0], green, sheet[..., 2], alpha * 255]).astype(np.uint8)
		cw, ch = w // 3, h // 3
		for index, name in enumerate(ICON_NAMES[key]):
			cell = Image.fromarray(rgba[(index // 3) * ch:(index // 3 + 1) * ch, (index % 3) * cw:(index % 3 + 1) * cw], 'RGBA')
			bbox = cell.getchannel('A').point(lambda v: 255 if v > 40 else 0).getbbox()
			icons[name] = cell.crop(bbox)
	return icons


def FitIcon(icon, box):
	k = box / float(max(icon.width, icon.height))
	return icon.resize((max(1, int(round(icon.width * k))), max(1, int(round(icon.height * k)))), Image.LANCZOS)


def LineCanvas(size):
	return Image.new('RGBA', (size * SS, size * SS), (0, 0, 0, 0))


def Stroke(size):
	return max(2, int(round(size * SS * 0.055)))


def DrawLock(size, closed):
	canvas = LineCanvas(size)
	d = ImageDraw.Draw(canvas)
	s = size * SS
	w = Stroke(size)
	color = CREAM + (255,)
	bodyTop = s * 0.46
	d.rounded_rectangle([s * 0.2, bodyTop, s * 0.8, s * 0.9], radius = s * 0.09, outline = color, width = w)
	d.ellipse([s * 0.45, s * 0.58, s * 0.55, s * 0.68], outline = color, width = w)
	d.line([(s * 0.5, s * 0.67), (s * 0.5, s * 0.77)], fill = color, width = w)
	left, right = s * 0.3, s * 0.7
	if closed:
		d.arc([left, s * 0.08, right, s * 0.08 + (right - left)], 180, 360, fill = color, width = w)
		d.line([(left, s * 0.08 + (right - left) / 2), (left, bodyTop)], fill = color, width = w)
		d.line([(right, s * 0.08 + (right - left) / 2), (right, bodyTop)], fill = color, width = w)
	else:
		d.arc([left, s * 0.08, right, s * 0.08 + (right - left)], 180, 360, fill = color, width = w)
		d.line([(right, s * 0.08 + (right - left) / 2), (right, s * 0.26)], fill = color, width = w)
		d.line([(left, s * 0.08 + (right - left) / 2), (left, s * 0.3)], fill = color, width = w)
	return canvas.resize((size, size), Image.LANCZOS)


def DrawChevron(size, up):
	canvas = LineCanvas(size)
	d = ImageDraw.Draw(canvas)
	s = size * SS
	w = Stroke(size) + 8
	color = BRONZE_LIGHT + (255,)
	y0, y1 = (s * 0.62, s * 0.36) if up else (s * 0.36, s * 0.62)
	d.line([(s * 0.26, y0), (s * 0.5, y1), (s * 0.74, y0)], fill = color, width = w, joint = 'curve')
	for x in (s * 0.26, s * 0.74):
		d.ellipse([x - w / 2, y0 - w / 2, x + w / 2, y0 + w / 2], fill = color)
	d.ellipse([s * 0.5 - w / 2, y1 - w / 2, s * 0.5 + w / 2, y1 + w / 2], fill = color)
	return canvas.resize((size, size), Image.LANCZOS)


def DrawPlus(size):
	canvas = LineCanvas(size)
	d = ImageDraw.Draw(canvas)
	s = size * SS
	w = Stroke(size) + 3
	color = CREAM + (255,)
	d.line([(s * 0.5, s * 0.26), (s * 0.5, s * 0.74)], fill = color, width = w)
	d.line([(s * 0.26, s * 0.5), (s * 0.74, s * 0.5)], fill = color, width = w)
	for (x, y) in ((0.5, 0.26), (0.5, 0.74), (0.26, 0.5), (0.74, 0.5)):
		d.ellipse([s * x - w / 2, s * y - w / 2, s * x + w / 2, s * y + w / 2], fill = color)
	return canvas.resize((size, size), Image.LANCZOS)


def DrawSign(size, plus):
	# kabartmali bronz arti / eksi: koyu golge + govde + ust isik
	canvas = LineCanvas(size)
	d = ImageDraw.Draw(canvas)
	s = size * SS
	w = int(s * 0.12)
	half = s * 0.24
	cx = cy = s * 0.5
	segments = [((cx - half, cy), (cx + half, cy))]
	if plus:
		segments.append(((cx, cy - half), (cx, cy + half)))
	layers = ((int(s * 0.012) + 2, BRONZE_DARK + (255,)), (0, (206, 172, 112, 255)), (-int(s * 0.012) - 1, (232, 204, 150, 255)))
	for (offset, color) in layers:
		inset = 0 if offset >= 0 else w * 0.34
		for ((x0, y0), (x1, y1)) in segments:
			d.line([(x0, y0 + offset), (x1, y1 + offset)], fill = color, width = int(w - inset))
			for (x, y) in ((x0, y0), (x1, y1)):
				r = (w - inset) / 2.0
				d.ellipse([x - r, y + offset - r, x + r, y + offset + r], fill = color)
	return canvas.resize((size, size), Image.LANCZOS)


BUTTON_PX = 136
BUTTON_ICON_RATIO = 0.5


def RoundPlate(disc, state):
	# yuvarlak plaka: kullanicinin yuvarlak cercevesi (elmassiz); aktifte ince sicak isik, basili halde koyulasir
	px = BUTTON_PX
	plate = disc.resize((px, px), Image.LANCZOS)
	if state == 'active':
		ring = np.asarray(plate).copy()
		yy, xx = np.mgrid[0:px, 0:px]
		edge = np.hypot(xx - px / 2.0 + 0.5, yy - px / 2.0 + 0.5) > px / 2.0 * 0.86
		glow = Image.new('RGBA', (px, px), (232, 176, 92, 255))
		mask = Image.fromarray((edge * ring[..., 3]).astype(np.uint8)).filter(ImageFilter.GaussianBlur(px / 28.0)).point(lambda v: int(v * 0.5))
		glow.putalpha(mask)
		plate.alpha_composite(glow)
	elif state == 'down':
		a = np.asarray(plate).astype(np.float32)
		a[..., :3] *= 0.72
		plate = Image.fromarray(a.astype(np.uint8), 'RGBA')
	return plate


def LoadChevron():
	# kabartmali cift ok (asagi); yukari hali dikey cevrilerek turetilir
	sheet = np.asarray(Image.open(os.path.join(HERE, 'hud_chevron_source.png')).convert('RGB')).astype(np.float32)
	d = sheet[..., 1] - np.maximum(sheet[..., 0], sheet[..., 2])
	alpha = np.where(d <= 25, 1.0, np.clip((95.0 - d) / 70.0, 0.0, 1.0))
	green = np.minimum(sheet[..., 1], np.maximum(sheet[..., 0], sheet[..., 2]) + 6)
	# bakiri diger ikonlarin soluk bronz tonuna yaklastir
	r = sheet[..., 0] * 0.93 + 8
	rgba = np.dstack([r, green, sheet[..., 2] + 6, alpha * 255]).clip(0, 255).astype(np.uint8)
	img = Image.fromarray(rgba, 'RGBA')
	return img.crop(img.getchannel('A').point(lambda v: 255 if v > 40 else 0).getbbox())


def MapFrame(save):
	# kare harita cercevesi (272x274 = 136x137 mantiksal): pahli koseler, eskitilmis bronz serit, ici saydam
	w, h = 272, 274
	size = (w, h)
	canvas = Image.new('RGBA', size, (0, 0, 0, 0))
	t = 8
	cut = 18
	outer = PolyMask(size, CutRect(1, 1, w - 1, h - 1, cut))
	inner = PolyMask(size, CutRect(1 + t, 1 + t, w - 1 - t, h - 1 - t, cut - 6))
	ring = Image.fromarray(np.clip(np.asarray(outer).astype(np.int16) - np.asarray(inner).astype(np.int16), 0, 255).astype(np.uint8))
	canvas.alpha_composite(Layer(BronzeTexture(w, h, 9), ring))
	# ic kenarda koyu ince cizgi, dis kenarda soluk isik
	edge = Image.fromarray(np.clip(np.asarray(PolyMask(size, CutRect(1 + t, 1 + t, w - 1 - t, h - 1 - t, cut - 6))).astype(np.int16) - np.asarray(PolyMask(size, CutRect(1 + t + 3, 1 + t + 3, w - 1 - t - 3, h - 1 - t - 3, cut - 8))).astype(np.int16), 0, 255).astype(np.uint8))
	shade = Image.new('RGBA', size, (6, 4, 2, 255))
	shade.putalpha(edge.point(lambda v: int(v * 0.8)))
	canvas.alpha_composite(shade)
	save('map_frame', canvas)


CHAT_CLOSED = (380, 38)	# mantiksal olculer (dosyalar 2 kat)
CHAT_OPEN_WIDTH = 600
CHAT_TOP_HEIGHT = 44	# acik panelin ust parcasi (sekme satiri); govde dikey kutulanir
CHAT_BODY_TILE = 100
CHAT_NAVY_TOP = (14, 22, 42)
CHAT_NAVY_BOTTOM = (10, 17, 34)


def ChatDock(save, name, logical, seed, alpha):
	# alt kenara yapisik sohbet seridi (ters U): ust ve yan bronz ince cizgi, alt acik, pahli ust koseler
	w, h = logical[0] * 2, logical[1] * 2
	size = (w, h)
	cut = 18
	t = 3
	outerMask = PolyMask(size, [(0, h), (0, cut), (cut, 0), (w - cut, 0), (w, cut), (w, h)])
	innerMask = PolyMask(size, [(t, h), (t, cut + 1), (cut + 2, t), (w - cut - 2, t), (w - t, cut + 1), (w - t, h)])
	ring = Image.fromarray(np.clip(np.asarray(outerMask).astype(np.int16) - np.asarray(innerMask).astype(np.int16), 0, 255).astype(np.uint8))
	canvas = Image.new('RGBA', size, (0, 0, 0, 0))
	navy = Layer(NavyTexture(w, h, seed), innerMask)
	navy.putalpha(innerMask.point(lambda v: int(v * alpha)))
	canvas.alpha_composite(navy)
	canvas.alpha_composite(Layer(BronzeTexture(w, h, seed + 2), ring))
	save(name, canvas)


def ChatOpenParts(save):
	# acik panel: ust parca (pahli koseler + tutamac) ve dikey dosenebilen govde; govde ayni renkte duz, dikis gorunmez
	w = CHAT_OPEN_WIDTH * 2
	t = 3
	cut = 18
	alpha = 0.94
	h = CHAT_TOP_HEIGHT * 2
	size = (w, h)
	outerMask = PolyMask(size, [(0, h), (0, cut), (cut, 0), (w - cut, 0), (w, cut), (w, h)])
	innerMask = PolyMask(size, [(t, h), (t, cut + 1), (cut + 2, t), (w - cut - 2, t), (w - t, cut + 1), (w - t, h)])
	ring = Image.fromarray(np.clip(np.asarray(outerMask).astype(np.int16) - np.asarray(innerMask).astype(np.int16), 0, 255).astype(np.uint8))
	top = Image.new('RGBA', size, (0, 0, 0, 0))
	navy = Layer(NavyTexture(w, h, 35, 0.0, CHAT_NAVY_TOP, CHAT_NAVY_BOTTOM), innerMask)
	navy.putalpha(innerMask.point(lambda v: int(v * alpha)))
	top.alpha_composite(navy)
	top.alpha_composite(Layer(BronzeTexture(w, h, 37), ring))
	# tutamac: ortada kisa bronz cubuk (yukari surukle = pencereyi buyut)
	grip = Image.new('RGBA', (size[0] * 4, size[1] * 4), (0, 0, 0, 0))
	gw, gh = 72 * 4, 6 * 4
	gx, gy = (w * 4 - gw) // 2, 12 * 4
	ImageDraw.Draw(grip).rounded_rectangle([gx, gy, gx + gw, gy + gh], radius = gh // 2, fill = BRONZE_MID + (255,))
	ImageDraw.Draw(grip).rounded_rectangle([gx, gy, gx + gw, gy + gh // 2], radius = gh // 4, fill = BRONZE_LIGHT + (220,))
	top.alpha_composite(grip.resize(size, Image.LANCZOS))
	save('chat_dock_top', top)

	bh = CHAT_BODY_TILE * 2
	bsize = (w, bh)
	bodyInner = PolyMask(bsize, [(t, 0), (w - t, 0), (w - t, bh), (t, bh)])
	bodyRing = Image.fromarray(np.clip(255 - np.asarray(bodyInner).astype(np.int16), 0, 255).astype(np.uint8))
	body = Image.new('RGBA', bsize, (0, 0, 0, 0))
	flat = Layer(NavyTexture(w, bh, 39, 0.0, CHAT_NAVY_BOTTOM, CHAT_NAVY_BOTTOM), bodyInner)
	flat.putalpha(bodyInner.point(lambda v: int(v * alpha)))
	body.alpha_composite(flat)
	body.alpha_composite(Layer(BronzeTexture(w, bh, 41), bodyRing))
	save('chat_dock_body', body)


def ChatBadge(save):
	# okunmamis sayisi rozeti: lacivert ic + bronz halka
	px = 44
	ss = 4
	size = (px, px)
	canvas = Image.new('RGBA', size, (0, 0, 0, 0))
	outer = Image.new('L', (px * ss, px * ss), 0)
	ImageDraw.Draw(outer).ellipse([0, 0, px * ss - 1, px * ss - 1], fill = 255)
	inner = Image.new('L', (px * ss, px * ss), 0)
	ImageDraw.Draw(inner).ellipse([4 * ss, 4 * ss, (px - 4) * ss - 1, (px - 4) * ss - 1], fill = 255)
	outer, inner = outer.resize(size, Image.LANCZOS), inner.resize(size, Image.LANCZOS)
	ring = Image.fromarray(np.clip(np.asarray(outer).astype(np.int16) - np.asarray(inner).astype(np.int16), 0, 255).astype(np.uint8))
	canvas.alpha_composite(Layer(NavyTexture(px, px, 41, 6.0), inner))
	canvas.alpha_composite(Layer(BronzeTexture(px, px, 43), ring))
	save('chat_badge_dot', canvas)


USER_ICONS = set()


def FitInCircle(icon, radius):
	# AKRAVON: gorsel agirlik merkezi dugme ortasina gelir; en uzak opak nokta daire icinde kalir
	alpha = np.asarray(icon.getchannel('A')).astype(np.float32)
	ys, xs = np.nonzero(alpha > 40)
	weight = alpha[ys, xs]
	(cx, cy) = (float((xs * weight).sum() / weight.sum()), float((ys * weight).sum() / weight.sum()))
	reach = float(np.sqrt((xs - cx) ** 2 + (ys - cy) ** 2).max())
	k = min(radius / reach, BUTTON_PX * 0.58 / max(icon.width, icon.height))	# yuvarlak ikonlar da asiri buyumesin
	fit = icon.resize((max(1, int(round(icon.width * k))), max(1, int(round(icon.height * k)))), Image.LANCZOS)
	return (fit, int(round(BUTTON_PX / 2.0 - cx * k)), int(round(BUTTON_PX / 2.0 - cy * k)))


APPROVED_HUD_ICONS = {
	'inventory': 'inventory', 'itemshop': 'itemshop', 'character': 'character',
	'more': 'less', 'less': 'collapse',
	'messenger': 'messenger', 'system': 'system', 'autohunt': 'autohunt',
	'shield': 'rank', 'quest': 'biologist', 'pin': 'remotemarket', 'attack': 'attack',
	'pickup': 'pickup', 'mount': 'mount',
	'collapse': 'less', 'link': 'link', 'send': 'send',
	'zoomin': 'zoomin', 'zoomout': 'zoomout',
}


def IvoryIcon(icon):
	pixels = np.asarray(icon.convert('RGBA')).copy()
	luma = np.asarray(icon.convert('L')).astype(np.float32)
	opaque = pixels[..., 3] > 128
	if not np.any(opaque):
		raise ValueError('HUD icon has no opaque pixels')
	low, high = np.percentile(luma[opaque], (2, 98))
	shade = np.clip((luma - low) / max(1.0, high - low), 0, 1)
	# Air Sword reference: median (247, 236, 189), highlight (255, 248, 205).
	for channel, palette in enumerate(((170, 247, 255), (147, 236, 248), (107, 189, 205))):
		pixels[..., channel] = np.interp(shade, (0, 0.5, 1), palette).astype(np.uint8)
	return Image.fromarray(pixels)


def BuildApprovedHud(save):
	folder = os.path.join(HERE, 'ikonlar', 'hud_approved')
	base = Image.open(os.path.join(folder, 'base.png')).convert('RGBA')
	bounds = base.getchannel('A').point(lambda value: 255 if value > 128 else 0).getbbox()
	if not bounds:
		raise ValueError('Approved HUD base is empty')
	left, top, right, bottom = bounds
	side = max(right - left, bottom - top) + 12
	cx, cy = (left + right) / 2.0, (top + bottom) / 2.0
	x, y = int(round(cx - side / 2.0)), int(round(cy - side / 2.0))
	disc = base.crop((x, y, x + side, y + side))
	pixels = np.asarray(disc).copy()
	rgb = pixels[..., :3].astype(np.float32)
	gold = (rgb[..., 0] > rgb[..., 1] * 1.05) & (rgb[..., 1] > rgb[..., 2] * 1.2) & (rgb[..., 0] > 25)
	pixels[..., :3] = np.where(gold[..., None], np.minimum(rgb * 1.18, 255), rgb).astype(np.uint8)
	disc = Image.fromarray(pixels)
	iconSize = int(round(BUTTON_PX * base.width * 0.44 / side))
	for name, source in APPROVED_HUD_ICONS.items():
		icon = Image.open(os.path.join(folder, source + '.png')).convert('RGBA')
		icon = IvoryIcon(icon)
		fit = FitIcon(icon, int(round(side * iconSize / float(BUTTON_PX))))
		for state in ('normal', 'active', 'down'):
			plate = disc.copy()
			shift = int(round(side / float(BUTTON_PX))) if state == 'down' else 0
			plate.alpha_composite(fit, ((side - fit.width) // 2,
									   (side - fit.height) // 2 + shift))
			if state != 'normal':
				pixels = np.asarray(plate).copy()
				pixels[..., :3] = np.clip(pixels[..., :3].astype(np.float32) *
										 (0.72 if state == 'down' else 1.08), 0, 255).astype(np.uint8)
				plate = Image.fromarray(pixels)
			for suffix, size in (('', BUTTON_PX), ('_m', 112), ('_s', 72)):
				save('btn_%s_%s%s' % (name, state, suffix),
					 plate.resize((size, size), Image.LANCZOS))
	# The separate system window's _xs images are composed in BuildKit.


def BuildKit(save, disc):
	ChatDock(save, 'chat_dock_closed', CHAT_CLOSED, 31, 0.95)
	ChatOpenParts(save)
	ChatBadge(save)
	MapFrame(save)
	icons = LoadIcons()
	USER_ICONS.clear()
	# AKRAVON: kullanicinin urettigi HUD ikonlari (skin/ikonlar/hud/<ad>.png) eskilerinin yerine gecer
	userDir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'ikonlar', 'hud')
	if os.path.isdir(userDir):
		for fileName in os.listdir(userDir):
			if fileName.endswith('.png'):
				user = Image.open(os.path.join(userDir, fileName)).convert('RGBA')
				# hafif altin ton: parlaklik korunur, renk %65 altina cekilir
				from PIL import ImageOps
				gold = ImageOps.colorize(ImageOps.autocontrast(user.convert('L'), cutoff = 1), black = (34, 22, 8), mid = (186, 140, 66), white = (255, 238, 186)).convert('RGBA')
				tinted = Image.blend(user, gold, 0.0)	# AKRAVON: renkli set (metal set: ikonlar/hud_metal, 0.65)
				tinted.putalpha(user.getchannel('A'))
				icons[fileName[:-4]] = tinted
				USER_ICONS.add(fileName[:-4])
	# AKRAVON: kullanicinin yuvarlak plakasi (skin/ikonlar/hud_disc.png)
	userDisc = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'ikonlar', 'hud_disc.png')
	if os.path.exists(userDisc):
		disc = Image.open(userDisc).convert('RGBA')
	chevron = LoadChevron()
	icons['more'] = chevron
	icons['less'] = chevron.transpose(Image.FLIP_TOP_BOTTOM)
	icons['collapse'] = FitIcon(DrawChevron(256, False), 256)
	icons['send'] = chevron.rotate(90, expand = True)
	icons['link'] = icons['inventory']
	icons['zoomin'] = DrawSign(256, True)
	icons['zoomout'] = DrawSign(256, False)
	icons['plus'] = FitIcon(DrawPlus(256), 256)

	for (name, icon) in icons.items():
		if name in ('lock_closed', 'plus'):
			continue
		for state in ('normal', 'active', 'down'):
			plate = RoundPlate(disc, state)
			fit = FitIcon(icon, int(BUTTON_PX * (0.64 if name in USER_ICONS else 0.46 if name in ('more', 'less') else 0.40 if name == 'collapse' else 0.62 if name in ('zoomin', 'zoomout') else 0.52)))
			shift = 1 if state == 'down' else 0
			if name in USER_ICONS:
				(fit, ox, oy) = FitInCircle(icon, BUTTON_PX * 0.36)
				plate.alpha_composite(fit, (ox, oy + shift))
			else:
				ox, oy = (BUTTON_PX - fit.width) // 2, (BUTTON_PX - fit.height) // 2
				plate.alpha_composite(fit, (ox, oy + shift))
			save('btn_%s_%s' % (name, state), plate)
			# kucuk butonlar (<= 44 px) icin ayri 72 px surum: buyuk resmi 4 kat kucultmek kenarlari bozuyor
			small = plate.resize((72, 72), Image.LANCZOS)
			save('btn_%s_%s_s' % (name, state), small)
			save('btn_%s_%s_m' % (name, state), plate.resize((112, 112), Image.LANCZOS))
			# menu satiri ikonu: ekrandaki boyutla birebir (26 UI px x 1.25 = 33 px), motor kucultmesin
			if name in ('system', 'channel', 'itemshop', 'character', 'less', 'quest'):
				smallPlate = RoundPlate(disc, state)
				smallPlate.alpha_composite(IvoryIcon(fit), (ox, oy + shift))
			else:
				smallPlate = plate
			save('btn_%s_%s_xs' % (name, state), smallPlate.resize((34, 34), Image.LANCZOS))

	# kilit ikonu (disk uzerine oturur); acik/kapali ayrimi kodda renk ve parlaklikla yapilir
	lock = FitIcon(icons['lock_closed'], 128)
	canvas = Image.new('RGBA', (128, 128), (0, 0, 0, 0))
	canvas.alpha_composite(lock, ((128 - lock.width) // 2, (128 - lock.height) // 2))
	save('lock_open', canvas)
	save('lock_closed', canvas)
	save('chat_arrow', IvoryIcon(FitIcon(icons['less'], 40)))
	BuildApprovedHud(save)
	return icons
