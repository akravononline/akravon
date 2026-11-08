# -*- coding: utf-8 -*-
# AKRAVON: depo, nesne marketi, depo sifresi, sifre degistirme pencereleri (envanterle ayni dil)
# cikti: <out>/uiscript/{safeboxwindow,mallwindow,passworddialog,changepassworddialog}.py + ymir work/ui/akravon_legacy/sb_*.png
import os
import sys
from PIL import Image, ImageDraw, ImageEnhance, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_akravon_charwin as cw
import build_akravon_inventory as inv

IMG = 'd:/ymir work/ui/akravon_legacy/'
SS = 4
GOLD = (222, 184, 110, 255)
CELL = inv.CELL
COLS = 9
ROWS = 5
IW = 40 + COLS * CELL		# 454
GRID_W = COLS * CELL		# 414
IH = 116 + ROWS * CELL + 16 + 44 + 14
BTN_Y = 116 + ROWS * CELL + 16
PW, PH = 340, 316


def Button(w, h, state):
	# giris ekranindaki CIKIS dugmesi stili: koyu lacivert dolgu, ince cift altin cerceve, kesik koseler
	big = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	c = 7 * SS
	def Poly(inset):
		i = inset * SS
		return [(i + c, i), (w * SS - i - c, i), (w * SS - i, i + c), (w * SS - i, h * SS - i - c), (w * SS - i - c, h * SS - i), (i + c, h * SS - i), (i, h * SS - i - c), (i, i + c)]
	top, bottom = {'up': ((30, 42, 66), (14, 20, 36)), 'over': ((40, 56, 88), (20, 30, 52)), 'down': ((10, 14, 26), (20, 28, 46))}[state]
	for y in range(h * SS):
		t = y / float(h * SS)
		col = tuple(int(top[k] + (bottom[k] - top[k]) * t) for k in range(3)) + (255,)
		d.line((0, y, w * SS, y), fill = col)
	mask = Image.new('L', big.size, 0)
	ImageDraw.Draw(mask).polygon(Poly(1), fill = 255)
	body = big.copy()
	big = Image.new('RGBA', big.size, (0, 0, 0, 0))
	big.paste(body, (0, 0), mask)
	d = ImageDraw.Draw(big)
	gold = (150, 112, 56, 255) if state == 'down' else GOLD
	d.line(Poly(1) + [Poly(1)[0]], fill = gold, width = int(1.6 * SS), joint = 'curve')
	d.line(Poly(4) + [Poly(4)[0]], fill = (112, 86, 46, 190), width = max(1, SS // 2))
	return big.resize((w, h), Image.LANCZOS)


def Field(w, h):
	return inv.Slot(w, h)


def TextEl(name, text, x, y, align = 'center'):
	return '{ "name" : "%s", "type" : "text", "x" : %d, "y" : %d, "text" : "%s", "text_horizontal_align" : "%s" },' % (name, x, y, text, align)


def Btn(name, x, y, w, kind, extra = ''):
	return ('{ "name" : "%s", "type" : "%s", "x" : %d, "y" : %d, "text" : "",%s\n\t\t\t\t\t"default_image" : IMG + "sb_btn%d_1.png", "over_image" : IMG + "sb_btn%d_2.png", "down_image" : IMG + "sb_btn%d_3.png" },'
		% (name, kind, x, y, extra, w, w, w))


def Close(x, y):
	return ('{ "name" : "%s", "type" : "button", "x" : %d, "y" : %d,\n\t\t\t\t\t"default_image" : IMG + "inv_close_1.png", "over_image" : IMG + "inv_close_1.png", "down_image" : IMG + "inv_close_3.png" },' % ('%s', x, y))


HEADER = '# -*- coding: utf-8 -*-\n# AKRAVON: uretildi (skin/build_akravon_safebox.py) - elle duzenleme\nimport uiScriptLocale\nIMG = "%s"\n' % IMG


def Window(name, w, h, board, children):
	return HEADER + '''window = {
	"name" : "%s",
	"x" : 100, "y" : 60,
	"style" : ("float",),
	"width" : %d, "height" : %d,
	"children" :
	(
		{
			"name" : "board", "type" : "expanded_image", "x" : 0, "y" : 0, "image" : IMG + "%s.png",
			"children" :
			(
%s
			),
		},
	),
}
''' % (name, w, h, board, '\n'.join('\t\t\t\t' + c.replace('\n', '\n\t\t\t\t') for c in children))


def Build(out):
	folder = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy')
	os.makedirs(folder, exist_ok = True)
	os.makedirs(os.path.join(out, 'uiscript'), exist_ok = True)
	images = {}
	for n in (1, 2, 3, 4, 5):
		tw = GRID_W // n
		images['sb_strip_%d' % n] = inv.TabStrip(GRID_W, 42, n)
		images['sb_tab_on_%d' % n] = inv.TabGlow(tw, 42)
		images['sb_tab_empty_%d' % n] = Image.new('RGBA', (tw, 42), (0, 0, 0, 0))
	for w in (120, 140, 170):
		for (i, state) in ((1, 'up'), (2, 'over'), (3, 'down')):
			images['sb_btn%d_%d' % (w, i)] = Button(w, 44, state)
	images['sb_field'] = Field(150, 40)
	images['sb_board'] = cw.Board(IW, IH)
	images['sb_pw_board'] = cw.Board(PW, PH)
	images['sb_pw_board_1'] = cw.Board(PW, 200)
	images['sb_pw_board_2'] = cw.Board(PW, 256)
	images['sb_pw_board_3'] = cw.Board(PW, 278)
	images['sb_cp_board'] = cw.Board(PW, 296)
	for (name, im) in images.items():
		im.save(os.path.join(folder, name + '.png'))

	def Save(name, text):
		open(os.path.join(out, 'uiscript', name), 'w', encoding = 'utf-8', newline = '\n').write(text)

	closeBtn = lambda n, x: Close(x, 12).replace('"%s"', '"' + n + '"')
	# --- depo
	Save('safeboxwindow.py', Window('SafeboxWindow', IW, IH, 'sb_board', [
		closeBtn('CloseButton', IW - 46),
		Btn('ChangePasswordButton', (IW - 170) // 2, BTN_Y, 170, 'button'),
		Btn('ExitButton', IW - 130, BTN_Y, 120, 'button'),
	]))
	# --- nesne marketi
	Save('mallwindow.py', Window('SafeboxWindow', IW, IH, 'sb_board', [
		closeBtn('CloseButton', IW - 46),
		Btn('ExitButton', (IW - 120) // 2, BTN_Y, 120, 'button'),
	]))
	# --- depo sifresi (iki adim: secim / sifre)
	Save('passworddialog.py', Window('PasswordDialog', PW, PH, 'sb_pw_board', [
		closeBtn('titlebar', PW - 46),
		TextEl('TitleName', 'Depo Kilidi', PW // 2, 14),
		TextEl('Desc1', 'Depoyu açmak için', PW // 2, 74),
		TextEl('Desc2', 'gizli şifreni gir.', PW // 2, 94),
		'{ "name" : "password_slot", "type" : "image", "x" : 95, "y" : 122, "image" : IMG + "sb_field.png",\n\t"children" : ({ "name" : "password_value", "type" : "editline", "x" : 14, "y" : 10, "width" : 122, "height" : 22, "input_limit" : 6, "secret_flag" : 1 },) },',
		Btn('normal_button', 22, 130, 140, 'radio_button'),
		Btn('nesne_button', 178, 130, 140, 'radio_button'),
		TextEl('Desc3', "Şifreni hiç değiştirmediysen", PW // 2, 184),
		TextEl('Desc4', "ilk şifren 000000'dır.", PW // 2, 204),
		TextEl('Desc5', 'Şifreni depodan değiştirebilirsin.', PW // 2, 224),
		Btn('accept_button', (PW - 254) // 2, 252, 120, 'button'),
		Btn('cancel_button', (PW - 254) // 2 + 134, 252, 120, 'button'),
	]).replace('"x" : 100, "y" : 60,', '"x" : 0, "y" : 0,').replace('"style" : ("float",),', '"style" : ("movable", "float",),'))
	# --- sifre degistirme
	field = lambda n, vn, y: ('{ "name" : "%s", "type" : "image", "x" : 150, "y" : %d, "image" : IMG + "sb_field.png",\n\t"children" : ({ "name" : "%s", "type" : "editline", "x" : 14, "y" : 10, "width" : 122, "height" : 22, "input_limit" : 6, "secret_flag" : 1 },) },' % (n, y, vn))
	Save('changepassworddialog.py', Window('ChangePasswordDialog', PW, 296, 'sb_cp_board', [
		closeBtn('titlebar', PW - 46),
		TextEl('TitleName', 'Şifre Yenile', PW // 2, 14),
		TextEl('old_password_name', 'Mevcut şifre', 24, 82, 'left'),
		field('old_password_slot', 'old_password_value', 70),
		TextEl('new_password_name', 'Yeni şifre', 24, 128, 'left'),
		field('new_password_slot', 'new_password_value', 116),
		TextEl('new_password_check_name', 'Yeni şifre (tekrar)', 24, 174, 'left'),
		field('new_password_check_slot', 'new_password_check_value', 162),
		Btn('accept_button', (PW - 254) // 2, 232, 120, 'button'),
		Btn('cancel_button', (PW - 254) // 2 + 134, 232, 120, 'button'),
	]).replace('"x" : 100, "y" : 60,', '"x" : 0, "y" : 0,').replace('"style" : ("float",),', '"style" : ("movable", "float",),'))
	print('depo: %d gorsel + 4 uiscript' % len(images))


def uiscript_text(key):
	return key


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
