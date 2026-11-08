# AKRAVON: giris ekrani gorselleri - skin/akravon_referans_giris.png (onayli tasarim) uzerinden uretilir
# kullanim: py -3 build_akravon_login.py <cikti_klasoru>  -> <cikti>/mobile_ui/akravon/*.png|jpg + boyut listesi
# Referanstaki sahte paneller arka plandan silinir; yerlerine oyunun gercek (calisan) panelleri cizilir.
import os
import sys
from PIL import Image, ImageDraw, ImageFilter, ImageEnhance

HERE = os.path.dirname(os.path.abspath(__file__))
REF = os.path.join(HERE, 'akravon_referans_giris.png')
OUT = sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out'
DST = os.path.join(OUT, 'mobile_ui', 'akravon')

UI_W, UI_H = 1280, 720			# oyunun arayuz cozunurlugu (metin2.cfg)
GOLD = (214, 176, 104, 255)
GOLD_DARK = (122, 92, 46, 255)
BOX_FILL = (16, 18, 30, 255)

# referans (1672x941) icindeki bolgeler
REF_CARD = (1126, 108, 1660, 818)		# sag giris paneli (cerceve + susler)
REF_ACCOUNTS = (40, 515, 534, 798)		# sol hesaplar paneli
REF_LOGIN_BTN = (1150, 565, 1640, 680)	# GIRIS dugmesi arama kutusu
REF_EXIT_BTN = (1160, 685, 1625, 760)	# CIKIS dugmesi arama kutusu
REF_ICONS = {
	'icon_user': (1184, 294, 1216, 327),
	'icon_lock': (1184, 365, 1216, 401),
	'icon_pin': (1184, 440, 1216, 476),
	'icon_accounts': (80, 540, 122, 572),
	'icon_add': (95, 611, 157, 672),
}

SIZES = {}


def Save(name, im, ext = 'png'):
	path = os.path.join(DST, '%s.%s' % (name, ext))
	if ext == 'jpg':
		im.convert('RGB').save(path, quality = 92)
	else:
		im.save(path)
	SIZES[name] = im.size


def ToUI(im):
	# referans olcegi -> arayuz olcegi
	scale = UI_W / 1672.0
	return im.resize((max(1, round(im.width * scale)), max(1, round(im.height * scale))), Image.LANCZOS)


def BrightBox(im, box, threshold = 90):
	# dugmenin parlak altin/cerceve sinirlarini bul (koyu lacivert zemin disarida kalir)
	crop = im.crop(box).convert('L').point(lambda v: 255 if v > threshold else 0)
	bb = crop.getbbox()
	return (box[0] + bb[0], box[1] + bb[1], box[0] + bb[2], box[1] + bb[3])


def Erase(base, blurred, box, feather = 14):
	mask = Image.new('L', base.size, 0)
	d = ImageDraw.Draw(mask)
	d.rectangle(box, fill = 255)
	mask = mask.filter(ImageFilter.GaussianBlur(feather / 2))
	base.paste(blurred, (0, 0), mask)


def Background(ref):
	# AKRAVON: temiz sahne (skin/login_background_source.png - referanstaki tum paneller kaldirilmis), 16:9 kesilir
	src = Image.open(os.path.join(HERE, 'login_background_source.png')).convert('RGB')
	cropH = int(src.width * UI_H / float(UI_W))
	top = min(max(0, (src.height - cropH) // 2 - 40), src.height - cropH)
	base = src.crop((0, top, src.width, top + cropH))
	Save('login_bg', base.resize((UI_W, UI_H), Image.LANCZOS), 'jpg')


def Buttons(ref):
	for (name, box) in (('btn_login', REF_LOGIN_BTN), ('btn_exit', REF_EXIT_BTN)):
		crop = ref.crop(BrightBox(ref, box, 70 if name == 'btn_exit' else 90))
		normal = ToUI(crop).convert('RGBA')
		Save(name + '_normal', normal)
		Save(name + '_over', ImageEnhance.Brightness(normal).enhance(1.12))
		Save(name + '_down', ImageEnhance.Brightness(normal).enhance(0.72))


def Icons(ref):
	for (name, box) in REF_ICONS.items():
		Save(name, ToUI(ref.crop(box)).convert('RGBA'))


def CreationBackground():
	source = Image.open(os.path.join(HERE, 'create_character_background_source.png')).convert('RGB')
	source = source.resize((UI_W, UI_H), Image.LANCZOS)
	source = ImageEnhance.Brightness(source).enhance(0.82)
	tint = Image.new('RGB', source.size, (8, 13, 30))
	source = Image.blend(source, tint, 0.10)
	Save('create_background', source)


def Diamond():
	# panel ust/alt orta susu - ince eskitilmis altin elmas (saydam zemin)
	w, h = 40, 26
	scale = 4
	im = Image.new('RGBA', (w * scale, h * scale), (0, 0, 0, 0))
	d = ImageDraw.Draw(im)
	cx, cy = w * scale // 2, h * scale // 2
	def Rhomb(rx, ry, fill, outline = None, width = 1):
		d.polygon([(cx, cy - ry), (cx + rx, cy), (cx, cy + ry), (cx - rx, cy)], fill = fill, outline = outline, width = width)
	# kanat cizgileri
	d.line([(0, cy), (w * scale, cy)], fill = GOLD_DARK, width = scale)
	Rhomb(int(11 * scale), int(11 * scale), BOX_FILL, GOLD, 2 * scale)
	Rhomb(int(6 * scale), int(6 * scale), (232, 196, 120, 255))
	Rhomb(int(3 * scale), int(3 * scale), (255, 236, 182, 255))
	Save('diamond', im.resize((w, h), Image.LANCZOS))


def CheckBoxes():
	s = 4
	size = 26
	for checked in (False, True):
		im = Image.new('RGBA', (size * s, size * s), (0, 0, 0, 0))
		d = ImageDraw.Draw(im)
		d.rounded_rectangle([s, s, size * s - s - 1, size * s - s - 1], radius = 3 * s, fill = BOX_FILL, outline = GOLD, width = 2 * s)
		if checked:
			d.line([(7 * s, 13 * s), (11 * s, 18 * s), (20 * s, 7 * s)], fill = (236, 200, 120, 255), width = 3 * s, joint = 'curve')
		Save('check_on' if checked else 'check_off', im.resize((size, size), Image.LANCZOS))


def Chevron():
	s = 4
	w, h = 10, 16
	im = Image.new('RGBA', (w * s, h * s), (0, 0, 0, 0))
	d = ImageDraw.Draw(im)
	d.line([(2 * s, 2 * s), (8 * s, 8 * s), (2 * s, 14 * s)], fill = GOLD, width = 2 * s, joint = 'curve')
	Save('chevron', im.resize((w, h), Image.LANCZOS))



# AKRAVON: lacivert "plaka" dugmeler (CIKIS dugmesi diliyle): dikey gecisli zemin, ince altin kenar
PLATES = {'chip': (104, 56), 'close': (180, 48), 'wide': (240, 56), 'back': (130, 44)}
GOLD_PLATES = {'gold': (300, 58)}


def Plate(width, height, active, warm = False):
	s = 4
	W, H = width * s, height * s
	im = Image.new('RGBA', (W, H), (0, 0, 0, 0))
	grad = Image.new('RGBA', (W, H))
	if warm:
		top, bottom = ((54, 43, 29), (28, 20, 13)) if active else ((43, 34, 24), (20, 15, 11))
	else:
		top, bottom = ((44, 40, 34), (22, 18, 14)) if active else ((30, 37, 60), (9, 12, 24))
	gd = ImageDraw.Draw(grad)
	for y in range(H):
		k = y / float(H - 1)
		gd.line([(0, y), (W, y)], fill = tuple(int(top[i] + (bottom[i] - top[i]) * k) for i in range(3)) + (240,))
	mask = Image.new('L', (W, H), 0)
	ImageDraw.Draw(mask).rounded_rectangle([0, 0, W - 1, H - 1], radius = 6 * s, fill = 255)
	im.paste(grad, (0, 0), mask)
	d = ImageDraw.Draw(im)
	edge = (236, 196, 116, 255) if active else (150, 118, 64, 255)
	if warm and not active:
		edge = (226, 188, 112, 255)
	d.rounded_rectangle([s // 2, s // 2, W - 1 - s // 2, H - 1 - s // 2], radius = 6 * s, outline = edge, width = (2 if active else 1) * s + s // 2)
	# ust ic parlama cizgisi
	d.line([(8 * s, 3 * s), (W - 8 * s, 3 * s)], fill = (255, 230, 170, 70 if active else 40), width = s)
	return im.resize((width, height), Image.LANCZOS)


def GoldPlate(width, height, active):
	# eskitilmis altin dugme: mat altin gecis, koyu dis cizgi, ince parlak ic kenar
	s = 4
	W, H = width * s, height * s
	im = Image.new('RGBA', (W, H), (0, 0, 0, 0))
	grad = Image.new('RGBA', (W, H))
	top, bottom = ((236, 200, 124), (160, 116, 52)) if active else ((214, 176, 102), (138, 98, 42))
	gd = ImageDraw.Draw(grad)
	for y in range(H):
		k = y / float(H - 1)
		gd.line([(0, y), (W, y)], fill = tuple(int(top[i] + (bottom[i] - top[i]) * k) for i in range(3)) + (255,))
	mask = Image.new('L', (W, H), 0)
	ImageDraw.Draw(mask).rounded_rectangle([0, 0, W - 1, H - 1], radius = 5 * s, fill = 255)
	im.paste(grad, (0, 0), mask)
	d = ImageDraw.Draw(im)
	d.rounded_rectangle([s // 2, s // 2, W - 1 - s // 2, H - 1 - s // 2], radius = 5 * s, outline = (92, 64, 26, 255), width = 2 * s)
	d.rounded_rectangle([3 * s, 3 * s, W - 1 - 3 * s, H - 1 - 3 * s], radius = 3 * s, outline = (250, 228, 170, 120), width = s)
	return im.resize((width, height), Image.LANCZOS)


def Plates():
	for (name, (w, h)) in GOLD_PLATES.items():
		Save('plate_%s_normal' % name, GoldPlate(w, h, False))
		Save('plate_%s_active' % name, GoldPlate(w, h, True))
	for (name, (w, h)) in PLATES.items():
		Save('plate_%s_normal' % name, Plate(w, h, False, name == 'back'))
		Save('plate_%s_active' % name, Plate(w, h, True, name == 'back'))


def main():
	os.makedirs(DST, exist_ok = True)
	ref = Image.open(REF).convert('RGBA')
	Background(ref)
	Buttons(ref)
	Icons(ref)
	CreationBackground()
	Diamond()
	CheckBoxes()
	Chevron()
	Plates()
	sys.path.insert(0, HERE)
	import build_akravon_create
	build_akravon_create.Build(Save)
	for (name, size) in sorted(SIZES.items()):
		print('%-18s %dx%d' % (name, size[0], size[1]))


if __name__ == '__main__':
	main()
