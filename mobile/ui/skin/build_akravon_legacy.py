# AKRAVON: eski (PC) pencere dokularini ayarlar penceresiyle ayni lacivert/altin gorunume getirir.
# ayni yol + ayni boyutta dosya uretilir, acik dosya olarak pack'in onune konur (kod/davranis degismez).
#  - dragonetc/offical/board_*  : pencere zemini + kose/kenar (ui.Board)
#  - ymir work/ui/pattern/titlebar_*, horizontalbar_*, thinboard_* : baslik cubugu, ara cubuk, ince kutu
#  - ymir work/ui/public/parameter_slot_*.sub, slot_base.sub, game/windows/box_face.sub : deger kutulari
#    (.sub yeni png'ye yonlendirilir; Public.dds atlasina dokunulmaz)
import os
import sys
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
NAVY = (8, 12, 26, 255)
NAVY_HI = (16, 24, 44, 255)
GOLD = (196, 156, 86, 255)
GOLD_DARK = (110, 84, 44, 255)
GOLD_HI = (236, 200, 128, 255)
SS = 4


def Panel(out, w, h):
	# ayarlar penceresinin uretilmis zemini (picker_panel, 2x) -> hedef olcekte
	src = Image.open(os.path.join(out, 'mobile_ui', 'akravon', 'picker_panel.png')).convert('RGBA')
	return src.resize((w, h), Image.LANCZOS)


def Save(out, rel, image):
	path = os.path.join(out, rel)
	os.makedirs(os.path.dirname(path), exist_ok = True)
	if rel.endswith('.tga'):
		image.convert('RGBA').save(path, 'TGA', compression = None)
	else:
		image.save(path)


def Box(w, h, fill = (5, 8, 18, 235), line = GOLD_DARK, hi = None):
	big = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	d.rounded_rectangle([0, 0, w * SS - 1, h * SS - 1], radius = 2 * SS, fill = fill, outline = line, width = SS)
	if hi:
		d.line([(3 * SS, SS * 1.5), (w * SS - 3 * SS, SS * 1.5)], fill = hi, width = SS // 2)
	return big.resize((w, h), Image.LANCZOS)


def Strip(w, h, top, bottom, line, capLeft = False, capRight = False):
	big = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	for y in range(h * SS):
		t = y / float(h * SS - 1)
		d.line([(0, y), (w * SS, y)], fill = tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3)) + (240,))
	d.line([(0, SS), (w * SS, SS)], fill = line, width = SS)
	d.line([(0, h * SS - SS * 2), (w * SS, h * SS - SS * 2)], fill = line, width = SS)
	if capLeft:
		d.line([(SS, SS), (SS, h * SS - SS * 2)], fill = line, width = SS)
	if capRight:
		d.line([(w * SS - SS * 2, SS), (w * SS - SS * 2, h * SS - SS * 2)], fill = line, width = SS)
	return big.resize((w, h), Image.LANCZOS)


def Build(out):
	# --- ui.Board: 32 kose, 128 kenar, 777x486 zemin
	p = Panel(out, 352, 412)
	(W, H) = p.size
	c = 32
	Save(out, 'dragonetc/offical/board_corner_lefttop.tga', p.crop((0, 0, c, c)))
	Save(out, 'dragonetc/offical/board_corner_righttop.tga', p.crop((W - c, 0, W, c)))
	Save(out, 'dragonetc/offical/board_corner_leftbottom.tga', p.crop((0, H - c, c, H)))
	Save(out, 'dragonetc/offical/board_corner_rightbottom.tga', p.crop((W - c, H - c, W, H)))
	Save(out, 'dragonetc/offical/board_line_top.tga', p.crop((c + 40, 0, c + 168, c)))
	Save(out, 'dragonetc/offical/board_line_bottom.tga', p.crop((c + 40, H - c, c + 168, H)))
	Save(out, 'dragonetc/offical/board_line_left.tga', p.crop((0, c + 60, c, c + 188)))
	Save(out, 'dragonetc/offical/board_line_right.tga', p.crop((W - c, c + 60, W, c + 188)))
	base = Panel(out, 880, 1028).crop((60, 60, 820, 968)).resize((777, 486), Image.LANCZOS).convert('RGB')
	Save(out, 'dragonetc/offical/board_base.tga', base)

	# --- baslik cubugu (32x32 x3): koyu lacivert serit, altin alt/ust cizgi
	# ayarlar penceresi gibi: zemin yok, yalniz basligin altinda ince altin cizgi
	for side in ('left', 'center', 'right'):
		bar = Image.new('RGBA', (32 * SS, 32 * SS), (0, 0, 0, 0))
		d = ImageDraw.Draw(bar)
		x0 = 6 * SS if side == 'left' else 0
		x1 = 26 * SS if side == 'right' else 32 * SS
		Save(out, 'ymir work/ui/pattern/titlebar_%s.tga' % side, bar.resize((32, 32), Image.LANCZOS))
	# ara bolum cubugu
	Save(out, 'ymir work/ui/pattern/horizontalbar_left.tga', Strip(32, 32, (34, 30, 22), (16, 14, 12), GOLD_DARK, capLeft = True))
	Save(out, 'ymir work/ui/pattern/horizontalbar_center.tga', Strip(32, 32, (34, 30, 22), (16, 14, 12), GOLD_DARK))
	Save(out, 'ymir work/ui/pattern/horizontalbar_right.tga', Strip(32, 32, (34, 30, 22), (16, 14, 12), GOLD_DARK, capRight = True))

	# ince kutu (16x16): koyu zemin + ince altin cizgi
	t = Box(48, 48, fill = (4, 6, 14, 200), line = GOLD_DARK)
	Save(out, 'ymir work/ui/pattern/thinboard_corner_lefttop.tga', t.crop((0, 0, 16, 16)))
	Save(out, 'ymir work/ui/pattern/thinboard_corner_righttop.tga', t.crop((32, 0, 48, 16)))
	Save(out, 'ymir work/ui/pattern/thinboard_corner_leftbottom.tga', t.crop((0, 32, 16, 48)))
	Save(out, 'ymir work/ui/pattern/thinboard_corner_rightbottom.tga', t.crop((32, 32, 48, 48)))
	Save(out, 'ymir work/ui/pattern/thinboard_line_top.tga', t.crop((16, 0, 32, 16)))
	Save(out, 'ymir work/ui/pattern/thinboard_line_bottom.tga', t.crop((16, 32, 32, 48)))
	Save(out, 'ymir work/ui/pattern/thinboard_line_left.tga', t.crop((0, 16, 16, 32)))
	Save(out, 'ymir work/ui/pattern/thinboard_line_right.tga', t.crop((32, 16, 48, 32)))

	# --- deger kutulari: .sub -> akravon_legacy/*.png (ayni boyut)
	subs = {'public/parameter_slot_00': (39, 18), 'public/parameter_slot_01': (52, 18), 'public/parameter_slot_02': (61, 18),
		'public/parameter_slot_03': (90, 18), 'public/parameter_slot_04': (115, 18), 'public/parameter_slot_05': (130, 18),
		'public/parameter_slot_06': (220, 18), 'public/parameter_slot_07': (34, 18), 'public/parameter_slot_100x18': (100, 18),
		'public/parameter_slot_77x18': (77, 18), 'public/slot_base': (32, 32), 'game/windows/box_face': (53, 53)}
	for (rel, (w, h)) in subs.items():
		name = rel.replace('/', '_')
		Save(out, 'ymir work/ui/akravon_legacy/%s.png' % name, Box(w, h, hi = (60, 70, 100, 160)))
		text = 'title subImage\r\nversion 1.0\r\nimage "akravon_legacy/%s.png"\r\nleft 0\r\ntop 0\r\nright %d\r\nbottom %d\r\n' % (name, w, h)
		path = os.path.join(out, 'ymir work/ui/%s.sub' % rel)
		os.makedirs(os.path.dirname(path), exist_ok = True)
		open(path, 'wb').write(text.encode('latin1'))
	print('eski pencere dokulari: tamam')


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
