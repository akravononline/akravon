# AKRAVON: karakter olusturma ekrani parcalari - skin/akravon_referans_olustur.png (onayli tasarim) uzerinden kesilir
# Referans 1365x768; parcalar oyun arayuzu olcegine (1280x720) indirilir, yazilar oyunda basilir.
import os
from PIL import Image, ImageEnhance

HERE = os.path.dirname(os.path.abspath(__file__))
REF = os.path.join(HERE, 'akravon_referans_olustur.png')
K = 1280.0 / 1365.0


def Lum(p):
	return (p[0] * 299 + p[1] * 587 + p[2] * 114) // 1000


def Clamp(v):
	return max(0, min(255, int(v)))


def Scale(im):
	return im.resize((max(1, round(im.width * K)), max(1, round(im.height * K))), Image.LANCZOS)


def Compose(ref, outer, inner, leftEnd, rightStart, strip, glow, innerWidth = None):
	# outer: kesilen alan (parlama payi dahil), inner: kartin kendi siniri; yazi/simge alani strip ile doldurulur
	crop = ref.crop(outer).convert('RGBA')
	ox, oy = outer[0], outer[1]
	lc = leftEnd - ox
	rc = rightStart - ox
	sx0, sx1 = strip[0] - ox, strip[1] - ox
	innerW = inner[2] - inner[0]
	if innerWidth is None:
		innerWidth = innerW
	extra = innerWidth - innerW
	outW = crop.width + extra
	out = Image.new('RGBA', (outW, crop.height))
	out.paste(crop.crop((0, 0, lc, crop.height)), (0, 0))
	middleW = outW - lc - (crop.width - rc)
	middle = crop.crop((sx0, 0, sx1, crop.height)).resize((middleW, crop.height), Image.BILINEAR)
	out.paste(middle, (lc, 0))
	out.paste(crop.crop((rc, 0, crop.width, crop.height)), (lc + middleW, 0))
	ix0, iy0 = inner[0] - ox, inner[1] - oy
	ix1, iy1 = inner[2] - ox + extra, inner[3] - oy
	px = out.load()
	for y in range(out.height):
		for x in range(out.width):
			r, g, b, a = px[x, y]
			if x < ix0 or x >= ix1 or y < iy0 or y >= iy1:
				alpha = Clamp((r - b - 22) * 3) if glow else 0
				px[x, y] = (r, g, b, alpha)
				continue
			dx = min(x - ix0, ix1 - 1 - x)
			dy = min(y - iy0, iy1 - 1 - y)
			if dx + dy < 4 and Lum((r, g, b)) < 70:
				px[x, y] = (r, g, b, 0)
	return out


def Erase(im, box, strip):
	# kutu icini ayni satirlardaki temiz seridin yatay uzatilmasiyla doldur
	fill = im.crop((strip[0], box[1], strip[1], box[3])).resize((box[2] - box[0], box[3] - box[1]), Image.BILINEAR)
	im.paste(fill, (box[0], box[1]))


def Bridge(im, x0, x1):
	# yazi alanini satir satir iki yandaki pikseller arasinda yumusak gecisle doldur
	px = im.load()
	for y in range(im.height):
		left = px[x0 - 1, y]
		right = px[x1, y]
		for x in range(x0, x1):
			k = (x - x0 + 1) / float(x1 - x0 + 1)
			px[x, y] = tuple(int(left[i] + (right[i] - left[i]) * k) for i in range(4))


def GoldAlpha(crop, low, span, bg):
	out = Image.new('RGBA', crop.size)
	src = crop.load()
	dst = out.load()
	for y in range(crop.height):
		for x in range(crop.width):
			r, g, b = src[x, y][:3]
			a = Clamp((Lum((r, g, b)) - low) * 255.0 / span)
			if a <= 0:
				dst[x, y] = (0, 0, 0, 0)
				continue
			k = a / 255.0
			c = tuple(Clamp((v - (1 - k) * bv) / k) for (v, bv) in zip((r, g, b), bg))
			dst[x, y] = c + (a,)
	return out


def Frame(ref):
	# ekran kenarindaki ince altin cerceve + ust orta elmas (alt kisim seritte)
	out = Image.new('RGBA', ref.size, (0, 0, 0, 0))
	src = ref.load()
	dst = out.load()
	for y in range(0, 662):
		for x in range(ref.width):
			edge = y < 30 or x < 16 or x >= ref.width - 16 or (y < 64 and (x < 64 or x >= ref.width - 64))
			if not edge:
				continue
			r, g, b = src[x, y]
			alpha = Clamp((r - b - 30) * 3) * min(1.0, Lum((r, g, b)) / 110.0)
			if alpha > 8:
				dst[x, y] = (r, g, b, int(alpha))
	return out.resize((1280, 720), Image.LANCZOS)


# AKRAVON: uretilmis parcalar (skin/create_parts_source.png, create_icons_source.png - yesil zemin)
# 2 kat cozunurlukte kaydedilir; oyunda 0.5 olcekle cizilir (HD_SCALE)
PARTS = os.path.join(HERE, 'create_parts_source.png')
ICONS = os.path.join(HERE, 'create_icons_source.png')
HD = 2
# ad: (kaynak kutusu, kaynak kenar paylari sol/ust/sag/alt, mantiksal boyut)
PART_SPECS = {
	'create_card_normal': ((67, 90, 740, 196), (34, 30, 34, 30), (262, 56)),
	'create_card_active': ((795, 89, 1470, 197), (34, 30, 34, 30), (262, 56)),
	'create_chip_normal': ((67, 249, 341, 325), (26, 24, 26, 24), (120, 40)),
	'create_chip_active': ((372, 247, 634, 326), (26, 24, 26, 24), (120, 40)),
	'create_shape': ((679, 248, 1465, 324), (92, 22, 92, 22), (246, 40)),
	'create_panel': ((325, 372, 1211, 781), (52, 52, 52, 52), (300, 436)),
	'create_back_normal': ((70, 837, 366, 921), (86, 26, 32, 26), (200, 48)),
	'create_field': ((402, 838, 1012, 917), (28, 24, 28, 24), (340, 40)),
	'create_button_normal': ((1048, 831, 1469, 925), (36, 30, 36, 30), (200, 48)),
	'select_card_normal': ((67, 90, 740, 196), (34, 30, 34, 30), (300, 70)),
	'select_card_active': ((795, 89, 1470, 197), (34, 30, 34, 30), (300, 70)),
	'select_panel': ((325, 372, 1211, 781), (52, 52, 52, 52), (300, 422)),
	'login_panel': ((325, 372, 1211, 781), (52, 52, 52, 52), (340, 454)),
	'account_panel': ((325, 372, 1211, 781), (52, 52, 52, 52), (362, 220)),
	'picker_panel': ((325, 372, 1211, 781), (52, 52, 52, 52), (440, 514)),
	'account_row_normal': ((67, 90, 740, 196), (34, 30, 34, 30), (338, 54)),
	'account_row_active': ((795, 89, 1470, 197), (34, 30, 34, 30), (338, 54)),
	'picker_server_normal': ((67, 90, 740, 196), (34, 30, 34, 30), (270, 60)),
	'picker_server_active': ((795, 89, 1470, 197), (34, 30, 34, 30), (270, 60)),
	'picker_row_normal': ((67, 90, 740, 196), (34, 30, 34, 30), (380, 50)),
	'picker_row_active': ((795, 89, 1470, 197), (34, 30, 34, 30), (380, 50)),
	'loading_bar': ((402, 838, 1012, 917), (28, 24, 28, 24), (640, 26)),
	'login_field': ((402, 838, 1012, 917), (28, 24, 28, 24), (288, 42)),
	'login_selector': ((402, 838, 1012, 917), (28, 24, 28, 24), (139, 38)),
	'login_field_focus': ((402, 838, 1012, 917), (28, 24, 28, 24), (288, 42)),
}
ICON_SPECS = {
	'warrior': (61, 326, 383, 657),
	'assassin': (466, 324, 718, 662),
	'sura': (805, 300, 1122, 694),
	'shaman': (1208, 277, 1461, 710),
}
ICON_SIZE = 36


def ChromaKey(im):
	# yesil zemini saydam yap, kenarlardaki yesil tasmasini temizle
	im = im.convert('RGBA')
	px = im.load()
	for y in range(im.height):
		for x in range(im.width):
			r, g, b, a = px[x, y]
			d = g - max(r, b)
			if d >= 90:
				px[x, y] = (0, 0, 0, 0)
				continue
			alpha = 255 if d <= 20 else Clamp(255 * (90 - d) / 70.0)
			px[x, y] = (r, min(g, max(r, b) + 8), b, alpha)
	return im


def NineSlice(src, insets, width, height, k):
	# kenar paylari k ile olceklenir, orta kisim esnetilir
	l, t, r, b = insets
	dl, dt, dr, db = [max(1, int(round(v * k))) for v in insets]
	out = Image.new('RGBA', (width, height), (0, 0, 0, 0))
	sx = (0, l, src.width - r, src.width)
	sy = (0, t, src.height - b, src.height)
	dx = (0, dl, width - dr, width)
	dy = (0, dt, height - db, height)
	for i in range(3):
		for j in range(3):
			box = (sx[i], sy[j], sx[i + 1], sy[j + 1])
			size = (dx[i + 1] - dx[i], dy[j + 1] - dy[j])
			if size[0] <= 0 or size[1] <= 0 or box[2] <= box[0] or box[3] <= box[1]:
				continue
			out.paste(src.crop(box).resize(size, Image.LANCZOS), (dx[i], dy[j]))
	return out


def BuildGenerated(save):
	parts = ChromaKey(Image.open(PARTS))
	for (name, (box, insets, (lw, lh))) in PART_SPECS.items():
		src = parts.crop(box)
		width, height = lw * HD, lh * HD
		k = min(height / float(src.height), 1.6)
		im = NineSlice(src, insets, width, height, k)
		if name.endswith('_focus'):
			im = ImageEnhance.Brightness(im).enhance(1.45)
		save(name, im)
		if name.endswith('_normal') and not name.startswith(('create_card', 'create_chip')):
			save(name.replace('_normal', '_down'), ImageEnhance.Brightness(im).enhance(0.78))

	icons = ChromaKey(Image.open(ICONS))
	for (name, box) in ICON_SPECS.items():
		icon = icons.crop(box)
		size = ICON_SIZE * HD
		icon.thumbnail((size, size), Image.LANCZOS)
		cell = Image.new('RGBA', (size, size), (0, 0, 0, 0))
		cell.alpha_composite(icon, ((size - icon.width) // 2, (size - icon.height) // 2))
		save('create_icon_%s' % name, cell)

	# AKRAVON: bos yuva karti - normal kartin soluk hali
	empty = NineSlice(parts.crop(PART_SPECS['select_card_normal'][0]), PART_SPECS['select_card_normal'][1], 300 * HD, 70 * HD, 1.6)
	alpha = empty.getchannel('A').point(lambda v: int(v * 0.72))
	empty = ImageEnhance.Brightness(empty).enhance(0.7)
	empty.putalpha(alpha)
	save('select_card_empty', empty)
	save('select_title', SelectTitle())
	save('status_dot', StatusDot())
	BuildLoading(save)


LOADING_ART = os.path.join(HERE, 'loading_background_source.png')
LOGIN_REF = os.path.join(HERE, 'akravon_referans_giris.png')


def BuildLoading(save):
	# AKRAVON: yukleme ekrani - sahne (1280x720, alt kisim koyulastirilir), logo, ilerleme cubugu dolgusu
	from PIL import ImageDraw
	art = Image.open(LOADING_ART).convert('RGB')
	k = max(1280.0 / art.width, 720.0 / art.height)
	art = art.resize((int(round(art.width * k)), int(round(art.height * k))), Image.LANCZOS)
	left, top = (art.width - 1280) // 2, (art.height - 720) // 2
	art = art.crop((left, top, left + 1280, top + 720)).convert('RGBA')
	shade = Image.new('RGBA', art.size, (0, 0, 0, 0))
	sd = ImageDraw.Draw(shade)
	for y in range(720):
		a = 0 if y < 470 else int(200 * ((y - 470) / 250.0) ** 1.4)
		sd.line([(0, y), (1280, y)], fill = (6, 9, 18, min(a, 210)))
	art.alpha_composite(shade)
	save('loading_bg', art.convert('RGB'))

	ref = Image.open(LOGIN_REF).convert('RGB').crop((560, 40, 1100, 285))
	logo = Image.new('RGBA', ref.size)
	s, d = ref.load(), logo.load()
	for y in range(ref.height):
		for x in range(ref.width):
			r, g, b = s[x, y]
			d[x, y] = (r, g, b, Clamp((r - b - 28) * 3.2))
	bbox = logo.getchannel('A').point(lambda v: 255 if v > 40 else 0).getbbox()
	save('akravon_logo', logo.crop(bbox))

	# altin gecisli ilerleme dolgusu (yatay esnetilir)
	fill = Image.new('RGBA', (8, 14 * HD))
	fd = ImageDraw.Draw(fill)
	for y in range(fill.height):
		t = y / float(fill.height - 1)
		c = (int(250 - 70 * t), int(214 - 70 * t), int(140 - 70 * t), 255)
		fd.line([(0, y), (8, y)], fill = c)
	save('loading_fill', fill)
	glow = Image.new('RGBA', (24 * HD, 24 * HD), (0, 0, 0, 0))
	from PIL import ImageFilter
	ImageDraw.Draw(glow).ellipse([8 * HD, 8 * HD, 16 * HD, 16 * HD], fill = (255, 232, 170, 255))
	save('loading_spark', glow.filter(ImageFilter.GaussianBlur(3 * HD)))


def StatusDot():
	# AKRAVON: yesil cevrimici noktasi - yumusak parlama, 2 kat cozunurluk
	from PIL import ImageDraw, ImageFilter
	s = 8
	size = 16 * HD
	glow = Image.new('RGBA', (size * s, size * s), (0, 0, 0, 0))
	ImageDraw.Draw(glow).ellipse([size * s * 0.1, size * s * 0.1, size * s * 0.9, size * s * 0.9], fill = (96, 220, 100, 110))
	glow = glow.filter(ImageFilter.GaussianBlur(size * s * 0.08))
	d = ImageDraw.Draw(glow)
	r0, r1 = size * s * 0.28, size * s * 0.72
	d.ellipse([r0, r0, r1, r1], fill = (102, 224, 106, 255))
	d.ellipse([r0 + size * s * 0.08, r0 + size * s * 0.06, r0 + size * s * 0.22, r0 + size * s * 0.18], fill = (220, 255, 210, 200))
	return glow.resize((size, size), Image.LANCZOS)


def SelectTitle():
	# AKRAVON: 'Karakter Sec' basligi - olusturma basligiyla ayni altin gecis ve alt sus cizgisi
	from PIL import ImageDraw, ImageFont
	ref = Image.open(REF).convert('RGB')
	ornament = GoldAlpha(ref.crop((92, 112, 468, 136)), 42, 110, (12, 16, 28))
	font = ImageFont.truetype(os.path.join(HERE, '..', '..', 'data', 'fonts', 'marcellus.ttf'), 46 * HD // 2 * 2)
	text = 'Karakter Se\u00e7'
	box = font.getbbox(text)
	tw, th = box[2] - box[0], box[3] - box[1]
	width = max(tw + 24, ornament.width * HD)
	height = th + 20 + ornament.height * HD
	mask = Image.new('L', (width, th + 16), 0)
	ImageDraw.Draw(mask).text((12 - box[0], 8 - box[1]), text, font = font, fill = 255)
	grad = Image.new('RGBA', mask.size)
	gd = ImageDraw.Draw(grad)
	for y in range(mask.height):
		k = y / float(max(1, mask.height - 1))
		gd.line([(0, y), (mask.width, y)], fill = (int(246 - 40 * k), int(222 - 52 * k), int(170 - 76 * k), 255))
	out = Image.new('RGBA', (width, height), (0, 0, 0, 0))
	shadow = Image.new('RGBA', mask.size, (0, 0, 0, 160))
	out.paste(shadow, (2, 3), mask)
	out.paste(grad, (0, 0), mask)
	orn = ornament.resize((ornament.width * HD, ornament.height * HD), Image.LANCZOS)
	out.alpha_composite(orn, (0, height - orn.height))
	return out

def Build(save):
	ref = Image.open(REF).convert('RGB')

	save('create_frame', Frame(ref))
	save('create_title', Scale(GoldAlpha(ref.crop((92, 56, 468, 136)), 42, 110, (12, 16, 28))))

	BuildGenerated(save)
	# alt serit: dugmeler/alan silinir, kenar susleri ve alt elmas kalir
	footer = ref.crop((0, 662, ref.width, ref.height)).convert('RGBA')
	for (x0, x1) in ((84, 272), (412, 942), (1008, 1288)):
		Erase(footer, (x0, 10, x1, 72), (285, 400))
	save('create_footer', Scale(footer))

if __name__ == '__main__':
	import sys
	out = sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out'
	dst = os.path.join(out, 'mobile_ui', 'akravon')
	os.makedirs(dst, exist_ok = True)

	def SaveFile(name, im):
		im.save(os.path.join(dst, name + '.png'))
		print('%-22s %dx%d' % (name, im.width, im.height))

	Build(SaveFile)
