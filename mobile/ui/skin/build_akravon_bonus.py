# -*- coding: utf-8 -*-
# AKRAVON: karakter bonuslari (uicharacterdetails) - karakter penceresinin "Bonuslar" bolumune gomulur.
# characterdetails/ dokulari ayni boyutta AKRAVON gorunumunde yeniden cizilir; pencere zemini kaldirilir.
# cikti: <out>/characterdetails/*.tga|png + <out>/uiscript/characterdetailswindow.py
import os
import sys
import zipfile
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT_ZIP = os.path.join(HERE, '..', '..', '..', '..', '2_Oyun_Dosyalari', 'files', 'mobile', '10_root_1.zip')
SS = 4
GOLD = (214, 172, 96, 255)
GOLD_DARK = (112, 86, 46, 255)
NAVY = (6, 9, 20, 235)


def Canvas(w, h):
	return Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))


def Done(big, w, h):
	return big.resize((w, h), Image.LANCZOS)


def Box(w, h, fill = NAVY, line = GOLD_DARK, r = 4):
	big = Canvas(w, h)
	ImageDraw.Draw(big).rounded_rectangle((0, 0, w * SS - 1, h * SS - 1), radius = r * SS, fill = fill, outline = line, width = SS)
	return Done(big, w, h)


def Asset(out, name, w, h):
	return Image.open(os.path.join(out, 'mobile_ui', 'akravon', name)).convert('RGBA').resize((w, h), Image.LANCZOS)


def Row(w, h):
	big = Canvas(w, h)
	ImageDraw.Draw(big).line([(4 * SS, (h - 1) * SS), ((w - 4) * SS, (h - 1) * SS)], fill = (44, 48, 68, 255), width = SS)
	return Done(big, w, h)


def Search(state):
	big = Canvas(25, 19)
	d = ImageDraw.Draw(big)
	fill = {0: (14, 20, 38, 255), 1: (34, 42, 66, 255), 2: (150, 112, 56, 255)}[state]
	d.rounded_rectangle((0, 0, 25 * SS - 1, 19 * SS - 1), radius = 4 * SS, fill = fill, outline = GOLD_DARK, width = SS)
	d.ellipse((7 * SS, 4 * SS, 15 * SS, 12 * SS), outline = GOLD, width = int(SS * 1.5))
	d.line([(14 * SS, 11 * SS), (18 * SS, 15 * SS)], fill = GOLD, width = 2 * SS)
	return Done(big, 25, 19)


def Save(out, rel, image):
	path = os.path.join(out, rel)
	os.makedirs(os.path.dirname(path), exist_ok = True)
	if rel.endswith('.tga'):
		image.convert('RGBA').save(path, 'TGA', compression = None)
	else:
		image.save(path)


def Build(out):
	D = 'characterdetails/'
	Save(out, D + 'bonus_bg.tga', Box(227, 320))
	Save(out, D + 'log_bg.tga', Box(227, 361))
	Save(out, D + 'category_item.tga', Asset(out, 'picker_row_normal.png', 212, 23))
	Save(out, D + 'bonus_item.tga', Row(194, 22))
	Save(out, D + 'btn_0.tga', Asset(out, 'picker_row_normal.png', 102, 25))
	Save(out, D + 'btn_1.tga', Asset(out, 'plate_gold_normal.png', 102, 25))
	Save(out, D + 'btn_select_3.png', Asset(out, 'picker_row_normal.png', 102, 24))
	Save(out, D + 'btn_select_1.png', Asset(out, 'plate_gold_normal.png', 102, 24))
	for i in range(3):
		Save(out, D + 'search_btn_%d.tga' % i, Search(i))
	# kaydirma cubugu: ince lacivert ray + altin tutamak
	for (name, size, color) in (('scrollbar_top', (9, 7), (10, 14, 28, 255)), ('scrollbar_mid', (9, 7), (10, 14, 28, 255)),
			('scrollbar_bottom', (9, 6), (10, 14, 28, 255)), ('scrollbar_scale', (9, 1), (10, 14, 28, 255)),
			('scroll_top', (9, 2), GOLD), ('scroll_mid', (9, 1), GOLD), ('scroll_bottom', (9, 2), GOLD)):
		Save(out, D + 'scrollbar/%s.tga' % name, Image.new('RGBA', size, color))
	for name in ('base', 'bottom', 'top', 'left', 'right', 'corner_left_bottom', 'corner_left_top', 'corner_right_bottom', 'corner_right_top'):
		Save(out, D + 'board/%s.tga' % name, Image.new('RGBA', (7, 7), (0, 0, 0, 0)))

	# pencere zemini yok (karakter penceresinin icine oturur), baslik dugmesi "Karakter Bonuslari"
	text = zipfile.ZipFile(ROOT_ZIP).read('uiscript/characterdetailswindow.py').decode('latin1')
	for (a, b) in (('"type" : "board",', '"type" : "window",'), ('"text" : "Bonus Tablosu",', '"text" : "Karakter Bonuslar\xfd",')):
		if text.count(a) != 1:
			raise SystemExit('characterdetailswindow: %s yok' % a)
		text = text.replace(a, b)
	text = '# AKRAVON: otomatik uretildi (skin/build_akravon_bonus.py)\r\n' + text
	dst = os.path.join(out, 'uiscript', 'characterdetailswindow.py')
	os.makedirs(os.path.dirname(dst), exist_ok = True)
	open(dst, 'wb').write(text.encode('latin1'))
	print('karakter bonuslari: tamam')


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
