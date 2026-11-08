# -*- coding: utf-8 -*-
# AKRAVON: Uzaktan Biyolog penceresi - ayni boyut/konumlarda lacivert-altin gorseller (uibiyolog.py degismez)
# cikti: <out>/ymir work/ui/game/bio/*.png|tga (zip'tekilerin uzerine gecer)
import io, os, sys, zipfile
from PIL import Image, ImageDraw, ImageFont, ImageEnhance
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_akravon_charwin as cw
import build_akravon_inventory as inv
ZIP = os.path.join(HERE, '..', '..', '..', '..', '2_Oyun_Dosyalari', 'files', 'mobile', '16_images_1.zip')
FONTS = os.path.join(HERE, '..', '..', 'data', 'fonts')
GOLD = (222, 184, 110, 255)
TEXT = (232, 226, 210, 255)
LINE = (96, 74, 42, 255)
SS = 4


def F(size, bold = False):
	return ImageFont.truetype(os.path.join(FONTS, 'latobd.ttf' if bold else 'lato.ttf'), size)


def M(size):
	return ImageFont.truetype(os.path.join(FONTS, 'marcellus.ttf'), size)


def Box(w, h, fill = (8, 11, 22, 235), line = LINE, r = 5):
	b = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	ImageDraw.Draw(b).rounded_rectangle((0, 0, w * SS - 1, h * SS - 1), r * SS, fill = fill, outline = line, width = SS)
	return b.resize((w, h), Image.LANCZOS)


def Background(src):
	bg = src.convert('RGBA').copy()
	(W, H) = bg.size
	X = 182
	bg.paste(cw.Board(W - X + 80, H + 80).crop((40, 40, W - X + 40, H + 40)), (X, 0))	# yalniz doku, cerceve/elmas yok
	d = ImageDraw.Draw(bg)
	d.line((X, 6, X, H - 6), fill = LINE, width = 1)
	d.rounded_rectangle((0, 0, X - 8, H - 1), 6, outline = GOLD, width = 2)	# portre karti
	bg.alpha_composite(Box(W - X - 16, 54), (X + 8, 22))
	d = ImageDraw.Draw(bg)
	d.text((X + 12, 85), u'Teslim Edilen Nesneler', font = M(14), fill = GOLD, anchor = 'lm')
	d.line((X + 12, 94, W - 12, 94), fill = LINE)
	bg.alpha_composite(Box(108, 19), (189, 100))
	bg.alpha_composite(Box(108, 19), (189, 123))
	d = ImageDraw.Draw(bg)
	d.text((243, 109), u'İlerleme Durumu', font = F(12), fill = TEXT, anchor = 'mm')
	bg.alpha_composite(Box(W - X - 8, 22, (6, 9, 20, 245)), (X + 4, 147))
	d = ImageDraw.Draw(bg)
	d.ellipse((186, 151, 200, 165), outline = GOLD, width = 1)
	d.text((193, 158), '?', font = F(11, True), fill = GOLD, anchor = 'mm')
	d.text((X + 12, 180), u'Görev Ödülleri', font = M(14), fill = GOLD, anchor = 'lm')
	d.line((X + 12, 190, W - 12, 190), fill = LINE)
	# orijinal ok ve saat simgeleri
	ch = Image.open('C:/AkravonTest/ui_out/mobile_ui/akravon/hud/chat_arrow.png').convert('RGBA').rotate(-90, expand = True)
	ch.thumbnail((26, 26), Image.LANCZOS)
	bg.alpha_composite(ch, (270 - ch.width // 2, 46 - ch.height // 2))
	bg.alpha_composite(src.convert('RGBA').crop((382, 149, 402, 168)), (382, 149))
	bg.alpha_composite(Box(W - X - 18, 100), (X + 6, 192))
	return bg


def Slot(size, gold):
	im = inv.Slot(size, size)
	if gold:
		ImageDraw.Draw(im).rounded_rectangle((0, 0, size - 1, size - 1), 6, outline = GOLD, width = 2)
	return im


def Plate(w, h, state):
	src = Image.open('C:/AkravonTest/ui_out/mobile_ui/akravon/create_button_%s.png' % ('down' if state == 2 else 'normal')).convert('RGBA')
	im = src.resize((w, h), Image.LANCZOS)
	return ImageEnhance.Brightness(im).enhance(1.15) if state == 1 else im


def Row(w, h, state):
	fill = {0: (8, 11, 22, 235), 1: (24, 30, 52, 245), 2: (60, 46, 22, 245)}[state]
	return Box(w, h, fill, GOLD if state == 2 else LINE, 3)


def Build(out):
	zf = zipfile.ZipFile(ZIP)
	read = lambda n: Image.open(io.BytesIO(zf.read('ymir work/ui/game/bio/' + n)))
	folder = os.path.join(out, 'ymir work', 'ui', 'game', 'bio')
	os.makedirs(folder, exist_ok = True)
	images = {'../../akravon_legacy/bio_board.png': cw.Board(435, 394), 'bg.png': Background(read('bg.png')), 'gold_slot.tga': Slot(42, True), 'grey_slot.tga': Slot(42, False), 'slot.png': Slot(42, False)}
	for i in range(3):
		images['button_%d.png' % i] = Plate(88, 42, i)
		images['teslim_btn_%s.png' % 'nhd'[i]] = Plate(121, 27, i)
		images['large_button_%d.png' % i] = Row(214, 21, i)
	for (name, im) in images.items():
		im.convert('RGBA').save(os.path.join(folder, name), format = 'TGA' if name.endswith('.tga') else 'PNG')
	Build2(out)
	print('biyolog: %d gorsel' % len(images))


# --- AKRAVON v2: sifirdan yerlesim (uiakravonbio koordinatlariyla ayni) ---
CW, CH = 528, 340
X0, RW = 200, 328


def Background2(src):
	bg = Image.new('RGBA', (CW, CH), (0, 0, 0, 0))
	portrait = src.convert('RGBA').crop((0, 0, 180, 326)).resize((184, 333), Image.LANCZOS)
	card = Image.new('RGBA', (184, 333), (0, 0, 0, 0))
	m = Image.new('L', (184 * SS, 333 * SS), 0)
	ImageDraw.Draw(m).rounded_rectangle((0, 0, 184 * SS - 1, 333 * SS - 1), 8 * SS, fill = 255)
	card.paste(portrait, (0, 0), m.resize((184, 333), Image.LANCZOS))
	bg.alpha_composite(card, (0, 3))
	d = ImageDraw.Draw(bg)
	d.rounded_rectangle((0, 3, 183, 335), 8, outline = GOLD, width = 2)
	bg.alpha_composite(Box(RW, 72), (X0, 24))
	ch = Image.open('C:/AkravonTest/ui_out/mobile_ui/akravon/hud/chat_arrow.png').convert('RGBA').rotate(-90, expand = True)
	ch.thumbnail((24, 24), Image.LANCZOS)
	bg.alpha_composite(ch, (X0 + 84 - ch.width // 2, 55 - ch.height // 2))
	ImageDraw.Draw(bg).text((X0 + 39, 85), u'Görev Eşyası', font = F(13, True), fill = (200, 176, 120, 255), anchor = 'mm')
	d = ImageDraw.Draw(bg)
	d.text((X0 + RW - 105, 85), u'Öz', font = F(14, True), fill = (200, 176, 120, 255), anchor = 'mm')
	d.text((X0 + RW - 41, 85), u'Unutkanlık', font = F(14, True), fill = (200, 176, 120, 255), anchor = 'mm')
	bg.alpha_composite(Box(206, 22), (X0, 106))
	bg.alpha_composite(Box(206, 24), (X0, 132))
	d = ImageDraw.Draw(bg)
	d.text((X0 + 103, 117), u'İlerleme Durumu', font = F(15, True), fill = GOLD, anchor = 'mm')
	bg.alpha_composite(Box(RW, 28, (6, 9, 20, 245)), (X0, 168))
	d = ImageDraw.Draw(bg)
	Disc(bg, X0 + 16, 182, '?')
	Clock(bg, X0 + RW - 16, 182)
	d = ImageDraw.Draw(bg)
	d.text((X0 + 2, 214), u'Görev Ödülleri', font = M(18), fill = GOLD, anchor = 'lm')
	d.line((X0, 226, X0 + RW, 226), fill = LINE)
	bg.alpha_composite(Box(RW, 106), (X0, 232))
	return bg


def Disc(bg, cx, cy, ch):
	r = 9
	big = Image.new('RGBA', (r * 2 * SS + SS * 2, r * 2 * SS + SS * 2), (0, 0, 0, 0))
	ImageDraw.Draw(big).ellipse((SS, SS, r * 2 * SS, r * 2 * SS), fill = (20, 28, 56, 255), outline = GOLD, width = int(1.5 * SS))
	im = big.resize((r * 2 + 2, r * 2 + 2), Image.LANCZOS)
	ImageDraw.Draw(im).text((r + 1, r + 1), ch, font = F(13, True), fill = GOLD, anchor = 'mm')
	bg.alpha_composite(im, (cx - r - 1, cy - r - 1))


def Clock(bg, cx, cy):
	import math
	r = 9
	big = Image.new('RGBA', (r * 2 * SS + SS * 2, r * 2 * SS + SS * 2), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	c = r * SS + SS
	d.ellipse((SS, SS, r * 2 * SS, r * 2 * SS), fill = (20, 28, 56, 255), outline = GOLD, width = int(1.5 * SS))
	d.line((c, c, c, c - 5 * SS), fill = GOLD, width = int(1.6 * SS))
	d.line((c, c, c + 4 * SS, c + 2 * SS), fill = GOLD, width = int(1.6 * SS))
	bg.alpha_composite(big.resize((r * 2 + 2, r * 2 + 2), Image.LANCZOS), (cx - r - 1, cy - r - 1))


def Check(on):
	big = Image.new('RGBA', (20 * SS, 20 * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	d.rounded_rectangle((SS, SS, 19 * SS, 19 * SS), 4 * SS, fill = (8, 11, 22, 255), outline = GOLD, width = int(1.5 * SS))
	if on:
		d.line([(5 * SS, 10 * SS), (9 * SS, 14 * SS), (15 * SS, 6 * SS)], fill = (255, 214, 140, 255), width = int(2.4 * SS))
	return big.resize((20, 20), Image.LANCZOS)


def Build2(out):
	zf = zipfile.ZipFile(ZIP)
	src = Image.open(io.BytesIO(zf.read('ymir work/ui/game/bio/bg.png')))
	folder = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy')
	Background2(src).save(os.path.join(folder, 'bio_bg2.png'))
	cw.Board(CW + 32, CH + 68).save(os.path.join(folder, 'bio_board.png'))
	Check(False).save(os.path.join(folder, 'bio_check_off.png'))
	Check(True).save(os.path.join(folder, 'bio_check_on.png'))


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
