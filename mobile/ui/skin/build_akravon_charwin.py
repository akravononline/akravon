# -*- coding: utf-8 -*-
# AKRAVON: karakter penceresi (uiscript/characterwindow.py) - onayli taslak: skin/akravon_karakter_taslak.png
# solda kenar cubugu (portre, ad, bolumler), sagda ust bilgi + EXP cubugu, stat kartlari, HP/SP cubuklari, deger listesi.
# oge ADLARI orijinalle ayni (uicharacter.py GetChild ile bagliyor) -> davranis degismez.
# cubuk doluluklari ve ad konumu: patches/akravon_charwin.txt
# cikti: <out>/uiscript/characterwindow.py + <out>/ymir work/ui/akravon_legacy/cw_*.png
import os
import re
import sys
import zipfile
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT_ZIP = os.path.join(HERE, '..', '..', '..', '..', '2_Oyun_Dosyalari', 'files', 'mobile', '10_root_1.zip')
FONTS = os.path.join(HERE, '..', '..', 'data', 'fonts')
DY = 44				# AKRAVON: ust baslik (Karakter) + ayirici cizgi
W, H = 620, 476 + DY
SKILL_STEP = 54
SB = 150				# kenar cubugu genisligi
X0 = SB + 20			# sag alan
RW = W - 24 - X0		# sag alan genisligi
CW = (RW - 12) // 2		# kart / sutun genisligi
PAGE_X, PAGE_Y = X0 + (RW - 250) // 2, 60	# eski sayfalar (250x304)
IMG = 'd:/ymir work/ui/akravon_legacy/'
OUT_DIR = 'C:/AkravonTest/ui_out'
SS = 4
GOLD = (214, 172, 96, 255)
GOLD_DARK = (112, 86, 46, 255)
HEADER = (226, 194, 128, 255)
TEXTC = (232, 226, 210, 255)
DARK = (40, 24, 8, 255)

C_HEAD = (0.89, 0.76, 0.50)
C_LABEL = (0.63, 0.59, 0.51)
C_VALUE = (0.96, 0.94, 0.88)
C_WHITE = (1.0, 1.0, 1.0)

NAV = (u'Durum', u'Yetenekler', u'Hareketler', u'Görevler', u'Bonuslar')
NAV_H, NAV_STEP = 40, 44	# mobil: parmakla rahat dokunma


# ------------------------------------------------------------------ gorseller
def Font(name, px):
	return ImageFont.truetype(os.path.join(FONTS, name), px * SS)


def Canvas(w, h):
	return Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))


def Done(big, w, h):
	return big.resize((w, h), Image.LANCZOS)


def Box(w, h, line = GOLD_DARK, fill = (4, 7, 16, 235), r = 4):
	big = Canvas(w, h)
	ImageDraw.Draw(big).rounded_rectangle((0, 0, w * SS - 1, h * SS - 1), radius = r * SS, fill = fill, outline = line, width = SS)
	return Done(big, w, h)


def Fill(w, h, top, bottom):
	big = Canvas(w, h)
	d = ImageDraw.Draw(big)
	for y in range(h * SS):
		t = y / float(h * SS - 1)
		d.line([(0, y), (w * SS, y)], fill = tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3)) + (255,))
	mask = Image.new('L', big.size, 0)
	ImageDraw.Draw(mask).rounded_rectangle((0, 0, w * SS - 1, h * SS - 1), radius = h * SS // 2, fill = 255)
	out = Canvas(w, h)
	out.paste(big, (0, 0), mask)
	return Done(out, w, h)


def Plus(state):
	big = Canvas(30, 30)
	d = ImageDraw.Draw(big)
	fill = {'up': (20, 26, 44, 255), 'over': (40, 46, 70, 255), 'down': (150, 112, 56, 255)}[state]
	d.ellipse((SS, SS, 29 * SS, 29 * SS), fill = fill, outline = GOLD, width = SS + SS // 2)
	c = 15 * SS
	d.line([(c - 6 * SS, c), (c + 6 * SS, c)], fill = GOLD if state != 'down' else DARK, width = 2 * SS)
	d.line([(c, c - 6 * SS), (c, c + 6 * SS)], fill = GOLD if state != 'down' else DARK, width = 2 * SS)
	return Done(big, 30, 30)


def Minus(state):
	big = Canvas(30, 30)
	d = ImageDraw.Draw(big)
	fill = {'up': (20, 26, 44, 255), 'over': (40, 46, 70, 255), 'down': (150, 112, 56, 255)}[state]
	d.ellipse((SS, SS, 29 * SS, 29 * SS), fill = fill, outline = GOLD, width = SS + SS // 2)
	c = 15 * SS
	d.line([(c - 6 * SS, c), (c + 6 * SS, c)], fill = GOLD if state != 'down' else DARK, width = 2 * SS)
	return Done(big, 30, 30)


def Board(w, h):
	# pencere zemini birebir boyutta: lacivert doku + ince altin cerceve + kucuk kose elmaslari (olcek 1, buyutme yok)
	src = Image.open(os.path.join(OUT_DIR, 'mobile_ui', 'akravon', 'picker_panel.png')).convert('RGBA')
	tex = src.crop((60, 60, 820, 968)).resize((w * SS, h * SS), Image.LANCZOS)
	big = Canvas(w, h)
	mask = Image.new('L', big.size, 0)
	ImageDraw.Draw(mask).rounded_rectangle((0, 0, w * SS - 1, h * SS - 1), radius = 6 * SS, fill = 255)
	big.paste(tex, (0, 0), mask)
	d = ImageDraw.Draw(big)
	d.rounded_rectangle((SS // 2, SS // 2, w * SS - SS, h * SS - SS), radius = 6 * SS, outline = GOLD, width = int(SS * 1.5))
	d.rounded_rectangle((5 * SS, 5 * SS, (w - 5) * SS, (h - 5) * SS), radius = 4 * SS, outline = (112, 86, 46, 200), width = SS // 2 + 1)
	for (cx, cy) in ((5, 5), (w - 5, 5), (5, h - 5), (w - 5, h - 5)):
		r = 4.5
		d.polygon([(cx * SS, (cy - r) * SS), ((cx + r) * SS, cy * SS), (cx * SS, (cy + r) * SS), ((cx - r) * SS, cy * SS)], fill = (236, 200, 128, 255))
	return Done(big, w, h)


def Close(state):
	# ortak kapat dugmesi (ui.TitleBar): lacivert disk + altin X, 26 px
	big = Canvas(26, 26)
	d = ImageDraw.Draw(big)
	fill = {'up': (14, 20, 38, 255), 'over': (34, 42, 66, 255), 'down': (150, 112, 56, 255)}[state]
	d.ellipse((SS, SS, 25 * SS, 25 * SS), fill = fill, outline = GOLD, width = SS + SS // 2)
	c, k = 13 * SS, 5 * SS
	col = GOLD if state != 'down' else DARK
	d.line([(c - k, c - k), (c + k, c + k)], fill = col, width = 2 * SS)
	d.line([(c - k, c + k), (c + k, c - k)], fill = col, width = 2 * SS)
	return Done(big, 26, 26)


def Ring(size):
	big = Canvas(size, size)
	d = ImageDraw.Draw(big)
	d.ellipse((0, 0, size * SS - 1, size * SS - 1), fill = (8, 12, 26, 255), outline = GOLD_DARK, width = SS)
	return Done(big, size, size)


def Group(out, state):
	assets = os.path.join(out, 'mobile_ui', 'akravon')
	name = 'plate_gold_normal.png' if state == 'down' else 'picker_row_normal.png'
	return Image.open(os.path.join(assets, name)).convert('RGBA').resize((84, 30), Image.LANCZOS)


def Nav(out, active):
	big = Canvas(122, NAV_STEP * (len(NAV) - 1) + NAV_H)
	assets = os.path.join(out, 'mobile_ui', 'akravon')
	font = Font('marcellus.ttf', 17)
	d = ImageDraw.Draw(big)
	for (i, label) in enumerate(NAV):
		on = i == active
		skin = Image.open(os.path.join(assets, 'plate_gold_normal.png' if on else 'picker_row_normal.png')).convert('RGBA')
		big.alpha_composite(skin.resize((122 * SS, NAV_H * SS), Image.LANCZOS), (0, i * NAV_STEP * SS))
		d.text((61 * SS, (i * NAV_STEP + NAV_H / 2.0) * SS), label.upper(), font = font, fill = DARK if on else TEXTC, anchor = 'mm')
	return Done(big, 122, NAV_STEP * (len(NAV) - 1) + NAV_H)


def BuildImages(out):
	global OUT_DIR
	OUT_DIR = out
	folder = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy')
	os.makedirs(folder, exist_ok = True)
	images = {
		'cw_face': Box(94, 94, GOLD, (20, 18, 18, 255), 6),
		'cw_slot40': Box(40, 40, GOLD_DARK, (6, 9, 20, 240), 5),
		'cw_skillcard': Box(167, 56),	# eski boyut: hizli erisim penceresi (uiakravonskilllayout) 167 px'e gore olcekler
		'cw_skillcard2': Box(CW, 50),
		'cw_supportcard': Box(CW, 40),
		'cw_skillring': Ring(48),
		'cw_ring44': Ring(44),
		'cw_bonuscard': Box(CW, 34),
		'cw_bonusrow': Box(RW, 22, (52, 46, 34, 255), (6, 9, 20, 220), 3),
		'cw_clear32': Image.new('RGBA', (32, 32), (0, 0, 0, 0)),
		'cw_wide': Box(RW, 50),
		'cw_board': Board(540, 476),	# eski boyut: oto av / hizli erisim pencereleri
		'cw_board2': Board(W, H),
		'cw_menu': Board(312, 180),	# AKRAVON: HUD acilir menu
		'align_board': Board(380, 520), 'align_row': Box(344, 44), 'align_row_on': Box(344, 44, GOLD, (40, 30, 12, 240)),
		'close_1': Close('up'), 'close_2': Close('over'), 'close_3': Close('down'),
		'cw_card': Box(CW, 58),
		'cw_statrow': Box(RW, 50),
		'cw_aligncard': Box(122, 56),
		'cw_minus_up': Minus('up'), 'cw_minus_over': Minus('over'), 'cw_minus_down': Minus('down'),
		'cw_track': Box(RW, 14, GOLD_DARK, (4, 6, 14, 240), 7),
		'cw_fill_exp': Fill(RW - 4, 10, (236, 198, 120), (176, 132, 62)),
		'cw_bar': Box(RW - 34, 16, GOLD_DARK, (4, 6, 14, 240), 8),
		'cw_fill_hp': Fill(RW - 38, 12, (230, 86, 74), (150, 34, 30)),
		'cw_fill_sp': Fill(RW - 38, 12, (96, 150, 240), (36, 72, 160)),
		'cw_clear': Image.new('RGBA', (122, 22), (0, 0, 0, 0)),
		'cw_vline': Box(1, H - 48 - DY, (96, 74, 42, 255), (96, 74, 42, 255), 0),
		'cw_hline': Box(W - 32, 1, (96, 74, 42, 255), (96, 74, 42, 255), 0),
		'cw_line': Box(CW, 1, (40, 44, 62, 255), (40, 44, 62, 255), 0),
		'cw_plus_up': Plus('up'), 'cw_plus_over': Plus('over'), 'cw_plus_down': Plus('down'),
	}
	for i in range(len(NAV)):
		images['cw_nav_%d' % i] = Nav(out, i)
	images['cw_questcard'] = Box(RW - 20, 62)
	for s in ('normal', 'over', 'down'):
		images['cw_group_%s' % s] = Group(out, s)
		images['cw_tab_%s' % s] = Group(out, s).resize((66, 34), Image.LANCZOS)
	for (name, image) in images.items():
		image.save(os.path.join(folder, name + '.png'))
	# ortak ince cerceveli pencere zemini (secenekler vb. SkinFrame 'panel_thin'): 1.25 kat kucultulur -> kenar ~1 px
	Board(880, 1028).save(os.path.join(out, 'mobile_ui', 'akravon', 'panel_thin.png'))
	# ortak kapat dugmesi (tum pencereler): lacivert disk + altin X
	pub = os.path.join(out, 'ymir work', 'ui', 'public')
	os.makedirs(pub, exist_ok = True)
	for i in (1, 2, 3):
		sub = 'title subImage\r\nversion 1.0\r\nimage "akravon_legacy/close_%d.png"\r\nleft 0\r\ntop 0\r\nright 26\r\nbottom 26\r\n' % i
		open(os.path.join(pub, 'close_button_0%d.sub' % i), 'wb').write(sub.encode('latin1'))


# ------------------------------------------------------------------ yerlesim
def T(name, x, y, text, color, font = 'Lato:15', align = 'left', extra = ''):
	return '{ "name":"%s", "type":"text", "x":%d, "y":%d, "text":%s, "fontname":"%s", "r":%.2f, "g":%.2f, "b":%.2f, "a":1.0, "text_horizontal_align":"%s"%s },' % (
		name, x, y, text, font, color[0], color[1], color[2], align, extra)


def I(name, x, y, image, children = None, kind = 'image'):
	s = '{ "name":"%s", "type":"%s", "x":%d, "y":%d, "image":"%s"' % (name, kind, x, y, image)
	if children:
		s += ', "children" : (\n' + '\n'.join(children) + '\n)'
	return s + ' },'


def Btn(name, x, y, up, over, down):
	return '{ "name":"%s", "type":"button", "x":%d, "y":%d, "default_image":"%s", "over_image":"%s", "down_image":"%s" },' % (name, x, y, up, over, down)


def OldBtn(name, x, y, kind):
	r = 'd:/ymir work/ui/game/windows/btn_%s_' % kind
	return Btn(name, x, y, r + 'up.sub', r + 'over.sub', r + 'down.sub')


def Win(name, x, y, w, h, children):
	return '{ "name":"%s", "type":"window", "x":%d, "y":%d, "width":%d, "height":%d, "children" : (\n%s\n) },' % (name, x, y, w, h, '\n'.join(children))


def TitleBar(name, title):
	# yalniz kapat dugmesi gorunur (baslik cubugu dokusu seffaf); baslik yazisi gizli
	return '{ "name" : "%s", "type" : "titlebar", "style" : ("attach",), "x" : %d, "y" : 9, "width" : 64, "color" : "red", "children" : (\n%s\n) },' % (
		name, W - 78, T('TitleName', 0, 0, '""', C_HEAD, 'Lato:12'))


def Hidden(children):
	# oyunun SetText yaptigi ama bu tasarimda gosterilmeyen ogeler (Lv/EXP/HP/SP): ekran disinda tutulur
	return Win('Akravon_Hidden', -4000, -4000, 10, 10, children)


def CharacterPage():
	c = []

	c.append(Hidden([
		Win('Status_Lv', 0, 0, 10, 10, [T('Level_Value', 0, 0, '""', C_VALUE)]),
		Win('Status_CurExp', 0, 0, 10, 10, [T('Exp_Value', 0, 0, '""', C_VALUE)]),
		Win('Status_RestExp', 0, 0, 10, 10, [T('RestExp_Value', 0, 0, '""', C_VALUE)]),
		Win('HEL_Label', 0, 0, 10, 10, [T('HP_Value', 0, 0, '""', C_VALUE)]),
		Win('SP_Label', 0, 0, 10, 10, [T('SP_Value', 0, 0, '""', C_VALUE)]),
	]))

	# karakter durumu: 2x2 stat karti (buyuk dokunma alani)
	s = [T('Character_Bar_01_Text', 0, 0, '"Karakter Durumu"', C_HEAD, 'Marcellus:20')]
	s.append(Win('Status_Plus_Label', RW - 150, 2, 150, 18, [T('Status_Plus_Label_Text', 118, 0, '"Kalan Puan:"', C_LABEL, 'Lato:15', 'right'),
		T('Status_Plus_Value', 150, 0, '"0"', C_HEAD, 'Lato:14', 'right')]))
	cards = (('HTH', 'VIT', 'Can ve savunma'), ('INT', 'INT', 'Büyü ve SP'), ('STR', 'STR', 'Saldırı gücü'), ('DEX', 'DEX', 'İsabet, kaçınma'))
	for (i, (key, label, hint)) in enumerate(cards):
		y = 28 + i * 54
		s.append(I('%s_Card' % key, 0, y, IMG + 'cw_statrow.png', [
			T('%s_Name' % key, 16, 5, '"%s"' % label, C_HEAD, 'Marcellus:22'),
			T('%s_Hint' % key, 16, 29, '"%s"' % hint, C_LABEL, 'Lato:16'),
			Btn('%s_Minus' % key, RW - 138, 10, IMG + 'cw_minus_up.png', IMG + 'cw_minus_over.png', IMG + 'cw_minus_down.png'),
			Win('%s_Label' % key, RW - 104, 0, 60, 50, [T('%s_Value' % key, 30, 11, '"0"', C_VALUE, 'Marcellus:24', 'center')]),
			Btn('%s_Plus' % key, RW - 44, 10, IMG + 'cw_plus_up.png', IMG + 'cw_plus_over.png', IMG + 'cw_plus_down.png'),
		]))
	c.append(Win('Status_Standard', X0, 22, RW, 250, s))

	# savas degerleri
	e = [T('Status_Extent_Label', 0, 0, '"Savaş Değerleri"', C_HEAD, 'Marcellus:20')]
	values = (('ATT', 'Saldırı'), ('DEF', 'Savunma'), ('MATT', 'Büyü Hasarı'), ('MDEF', 'Büyü Savunması'),
		('MSPD', 'Hareket Hızı'), ('ASPD', 'Saldırı Hızı'), ('CSPD', 'Büyü Hızı'), ('ER', 'Savuşturma'))
	for (i, (key, label)) in enumerate(values):
		x = (i % 2) * (CW + 12)
		y = 32 + (i // 2) * 30
		e.append(Win('%s_Row' % key, x, y, CW, 28, [T('%s_Name' % key, 0, 1, '"%s"' % label, C_LABEL, 'Lato:17'),
			T('%s_Value' % key, CW, 1, '"0"', C_VALUE, 'Lato:18', 'right'), I('%s_Line' % key, 0, 26, IMG + 'cw_line.png')]))
	c.append(Win('Status_Extent', X0, 288, RW, 150, e))

	return '{ "name" : "Character_Page", "type" : "window", "style" : ("attach", "not_pick",), "x" : 0, "y" : %d, "width" : %d, "height" : %d, "children" : (\n%s\n) },' % (DY, W, H - DY, '\n'.join(c))


def SkillPage():
	# beceriler: her beceri tek kart - HD yuvarlak ikon (mevcut kademe), ad, kademe rozeti (Lv/M1/G1/P)
	# yuva 1..8 = becerinin kendisi (kademe yuvalari 21../41.. yok); uicharacter yamasi mevcut kademeyi koyar
	# kart dokulari/ad/rozet: patches/akravon_charwin.txt AkravonSkillCard
	c = []
	c.append(Win('Active_Skill_Point_Label', X0, 28, 160, 20, [T('Active_Skill_Point_Text', 0, 0, '"Kalan Puan:"', C_LABEL, 'Lato:16'),
		T('Active_Skill_Point_Value', 92, 0, '"0"', C_HEAD, 'Lato:17')]))
	grp = []
	for (i, name) in enumerate(('Skill_Group_Button_1', 'Skill_Group_Button_2')):
		grp.append('{ "name" : "%s", "type" : "radio_button", "x" : %d, "y" : 0, "text" : "", "text_color" : 0xFFE8E2D2, '
			'"default_image" : "%scw_group_normal.png", "over_image" : "%scw_group_over.png", "down_image" : "%scw_group_down.png" },' % (name, i * 90, IMG, IMG, IMG))
	grp.append(T('Active_Skill_Group_Name', 0, 6, '""', C_HEAD, 'Lato:16'))
	c.append(Win('Skill_Group_Bar', X0 + RW - 174, 22, 174, 30, grp))

	slots = []
	cards = []
	for k in range(6):
		col, row = k % 2, k // 2
		x = col * (CW + 12)
		y = row * SKILL_STEP
		cards.append(I('Skill_Card_%d' % k, x, y, IMG + 'cw_skillcard2.png', [
			I('Skill_Ring_%d' % k, 6, 4, IMG + 'cw_clear32.png'),
			T('Skill_Name_%d' % k, 58, 6, '""', C_VALUE, 'Lato:15'),
			T('Skill_Grade_%d' % k, 58, 27, '""', C_HEAD, 'Lato:16'),
		]))
		slots.append('{"index":%d, "x":%d, "y":%d, "width":44, "height":44}' % (k + 1, x + 4, y + 3))
	c.append(Win('Skill_Cards', X0, 66, RW, SKILL_STEP * 3, cards))
	c.append('{ "name" : "Skill_Active_Slot", "type" : "slot", "x" : %d, "y" : 66, "width" : %d, "height" : %d, "image" : "%scw_clear32.png", "slot" : (%s) },' % (
		X0, RW, SKILL_STEP * 3, IMG, ', '.join(slots)))
	c.append(Win('Skill_HD_Layer', X0, 66, RW, SKILL_STEP * 3, ['{ "name" : "Skill_HD_%d", "type" : "expanded_image", "style" : ("not_pick",), "x" : %d, "y" : %d, "image" : "%scw_clear32.png" },' % (
		k, (k % 2) * (CW + 12) + 4, (k // 2) * SKILL_STEP + 3, IMG) for k in range(6)]))

	# destek: ayri bolum, hep gorunur (sekmeden / gruptan bagimsiz)
	sy = 66 + SKILL_STEP * 3 + 6
	c.append(T('Support_Skill_Group_Name', X0, sy, '"Destek Yetenekleri"', C_LABEL, 'Lato:15'))
	sup = []
	supHd = []
	etc = []
	step = 58
	for k in range(6):
		col, row = k % 2, k // 2
		x = col * (CW + 12)
		y = row * step
		sup.append(Win('Support_Card_%d' % k, x, y, CW, 54, [
			T('Support_Name_%d' % k, 58, 6, '""', C_VALUE, 'Lato:16'),
			T('Support_Grade_%d' % k, 58, 28, '""', C_HEAD, 'Lato:16'),
		]))
		supHd.append('{ "name" : "Support_HD_%d", "type" : "expanded_image", "style" : ("not_pick",), "x" : %d, "y" : %d, "image" : "%scw_clear32.png" },' % (k, x + 2, y + 3, IMG))
		etc.append('{"index":%d, "x":%d, "y":%d, "width":44, "height":44}' % (101 + k, x + 4, y + 5))
	c.append(Win('Support_Cards', X0, sy + 28, RW, step * 3, sup))
	c.append('{ "name" : "Skill_ETC_Slot", "type" : "slot", "x" : %d, "y" : %d, "width" : %d, "height" : %d, "image" : "%scw_clear32.png", "slot" : (%s) },' % (
		X0, sy + 28, RW, step * 3, IMG, ', '.join(etc)))
	c.append(Win('Support_HD_Layer', X0, sy + 28, RW, step * 3, supHd))
	# sayfa dugmeleri: < 1/2 >
	c.append(Btn('Support_Prev', X0 + RW - 142, sy - 6, IMG + 'qs_lt_1.png', IMG + 'qs_lt_1.png', IMG + 'qs_lt_3.png'))
	c.append(T('Support_Page_Text', X0 + RW - 72, sy + 1, '"1/1"', C_HEAD, 'Lato:16', 'center'))
	c.append(Btn('Support_Next', X0 + RW - 46, sy - 6, IMG + 'qs_rt_1.png', IMG + 'qs_rt_1.png', IMG + 'qs_rt_3.png'))
	c.append(Win('Akravon_Hidden_Skill', -4000, -4000, 10, 10, [
		Win('Support_Skill_Point_Label', 0, 0, 10, 10, [T('Support_Skill_Point_Text', 0, 0, '""', C_LABEL, 'Lato:12'), T('Support_Skill_Point_Value', 0, 0, '"0"', C_HEAD, 'Lato:12')]),
	]))

	return '{ "name" : "Skill_Page", "type" : "window", "style" : ("attach", "not_pick",), "x" : 0, "y" : %d, "width" : %d, "height" : %d, "children" : (\n%s\n) },' % (DY, W, H - DY, '\n'.join(c))


def SideBlock():
	# kenar cubugu ust kismi (tum bolumlerde gorunur): portre + ad (+ lonca) + ayirici + kapat
	c = [T('Window_Title', W // 2, 11, '"Karakter"', C_HEAD, 'Marcellus:26', 'center'), I('Header_Line', 16, 50, IMG + 'cw_hline.png'), I('Side_Line', SB, 24 + DY, IMG + 'cw_vline.png')]
	c.append(I('Face_Slot', (SB - 94) // 2, 24 + DY, IMG + 'cw_face.png'))
	c.append('{ "name" : "Face_Image", "type" : "expanded_image", "x" : %d, "y" : %d, "x_scale" : 2.0, "y_scale" : 2.0, "image" : "d:/ymir work/ui/game/windows/face_warrior.sub" },' % ((SB - 94) // 2 + 4, 28 + DY))
	# ad kutusu image olmali: uicharacter SAFE_SetStringEvent / SetEvent(mouse_click) bagliyor
	c.append(I('Character_Name_Slot', 14, 128 + DY, IMG + 'cw_clear.png', [T('Character_Name', 61, 0, '""', C_HEAD, 'Marcellus:20', 'center')]))
	c.append(I('Guild_Name_Slot', 14, 152 + DY, IMG + 'cw_clear.png', [T('Guild_Name', 61, 0, '""', C_LABEL, 'Lato:15', 'center')]))
	c.append(I('Align_Card', 14, 216, IMG + 'cw_aligncard.png', [
		T('Align_Name', 10, 8, '"-"', C_VALUE, 'Marcellus:19', 'left'),
		T('Align_Value', 10, 32, '"Puan : 0"', C_HEAD, 'Lato:14', 'left'),
	]))
	return '\n'.join(c)


def EmotionPage():
	# hareketler: tekli (1..) + karsilikli (51..) - yuva 40 px (patches/akravon_charwin.txt)
	blank = (RW - 240) // 5
	c = [T('Action_Bar_Text', X0, 22, '"Tekli Hareketler"', C_HEAD, 'Lato:16')]
	c.append('{ "name" : "SoloEmotionSlot", "type" : "grid_table", "x" : %d, "y" : 46, "start_index" : 1, "x_count" : 6, "y_count" : 3, '
		'"x_step" : 40, "y_step" : 40, "x_blank" : %d, "y_blank" : 8, "image" : "%scw_slot40.png" },' % (X0, blank, IMG))
	c.append(T('Reaction_Bar_Text', X0, 202, '"Karşılıklı Hareketler"', C_HEAD, 'Lato:16'))
	c.append('{ "name" : "DualEmotionSlot", "type" : "grid_table", "x" : %d, "y" : 226, "start_index" : 51, "x_count" : 6, "y_count" : 3, '
		'"x_step" : 40, "y_step" : 40, "x_blank" : %d, "y_blank" : 8, "image" : "%scw_slot40.png" },' % (X0, blank, IMG))
	return '{ "name" : "Emoticon_Page", "type" : "window", "style" : ("attach", "not_pick",), "x" : 0, "y" : %d, "width" : %d, "height" : %d, "children" : (\n%s\n) },' % (DY, W, H - DY, '\n'.join(c))


def QuestPage():
	# gorevler: 5 gorev karti (ikon + ad + sure + sayac), sagda kaydirma
	c = [T('Quest_Title', X0, 22, '"Görevler"', C_HEAD, 'Marcellus:20')]
	step = 70
	for i in range(5):
		c.append(I('Quest_Card_%d' % i, X0, 54 + i * step, IMG + 'cw_questcard.png'))
	c.append('{ "name" : "Quest_Slot", "type" : "grid_table", "x" : %d, "y" : %d, "start_index" : 0, "x_count" : 1, "y_count" : 5, '
		'"x_step" : 32, "y_step" : 32, "y_blank" : %d, "image" : "d:/ymir work/ui/game/quest/slot_base.sub" },' % (X0 + 12, 54 + 15, step - 32))
	c.append('{ "name" : "Quest_ScrollBar", "type" : "scrollbar", "x" : %d, "y" : 54, "size" : %d },' % (X0 + RW - 14, step * 5 - 8))
	for i in range(5):
		y = 54 + i * step
		c.append(T('Quest_Name_0%d' % i, X0 + 56, y + 9, '""', C_VALUE, 'Lato:16'))
		c.append(T('Quest_LastTime_0%d' % i, X0 + 56, y + 30, '""', C_LABEL, 'Lato:14'))
		c.append(T('Quest_LastCount_0%d' % i, X0 + 56, y + 46, '""', C_LABEL, 'Lato:14'))
	return '{ "name" : "Quest_Page", "type" : "window", "style" : ("attach", "not_pick",), "x" : 0, "y" : %d, "width" : %d, "height" : %d, "children" : (\n%s\n) },' % (DY, W, H - DY, '\n'.join(c))


def TabControl():
	# kenar cubugu bolumleri: Tab_0N = secili bolum vurgulu resim, Tab_Button_0N = gorunmez dokunma alani
	c = ['{ "name" : "Tab_0%d", "type" : "image", "x" : 0, "y" : 0, "image" : "%scw_nav_%d.png" },' % (i + 1, IMG, i) for i in range(4)]
	c.append('{ "name" : "Akravon_Nav_Bonus", "type" : "image", "x" : 0, "y" : 0, "image" : "%scw_nav_4.png" },' % IMG)
	c += ['{ "name" : "Tab_Button_0%d", "type" : "radio_button", "x" : 0, "y" : %d, "width" : 122, "height" : %d },' % (i + 1, i * NAV_STEP, NAV_H) for i in range(5)]
	side = SideBlock() + '\n' + TitleBar('Character_TitleBar', '')
	return side + '\n' + '{ "name" : "TabControl", "type" : "window", "x" : 14, "y" : %d, "width" : 122, "height" : %d, "children" : (\n%s\n) },' % (288, NAV_STEP * (len(NAV) - 1) + NAV_H, '\n'.join(c))


def Element(text, name):
	# "name" : "X" iceren sozluk blogunun [bas, son) araligi (suslu parantez esleme)
	m = re.search(r'"name"\s*:\s*"%s"' % name, text)
	if not m:
		raise SystemExit('characterwindow: oge yok: %s' % name)
	start = text.rfind('{', 0, m.start())
	depth = 0
	for i in range(start, len(text)):
		if text[i] == '{':
			depth += 1
		elif text[i] == '}':
			depth -= 1
			if depth == 0:
				end = i + 1
				if text[end:end + 1] == ',':
					end += 1
				return (start, end)
	raise SystemExit('characterwindow: kapanmayan oge: %s' % name)


def Replace(text, name, new):
	(a, b) = Element(text, name)
	return text[:a] + new + text[b:]


def Move(text, name, x, y):
	(a, b) = Element(text, name)
	block = text[a:b]
	head = block.split('"children"')[0]
	head2 = re.sub(r'"x"\s*:\s*[^,]+,', '"x" : %d,' % x, head, count = 1)
	head2 = re.sub(r'"y"\s*:\s*[^,]+,', '"y" : %d,' % y, head2, count = 1)
	return text[:a] + head2 + block[len(head):] + text[b:]


def L1(s):
	# oyun turkceyi cp1254 tek bayt okuyor; dosyanin geri kalani latin1 olarak tasinir
	return s.encode('cp1254').decode('latin1')


def BuildScript(out):
	text = zipfile.ZipFile(ROOT_ZIP).read('uiscript/characterwindow.py').decode('latin1').replace('\r\n', '\n')
	text = Replace(text, 'Character_Page', L1(CharacterPage()))
	text = Replace(text, 'TabControl', L1(TabControl()))
	for name in ('Skill_TitleBar', 'Emoticon_TitleBar', 'Quest_TitleBar'):
		text = Replace(text, name, L1(TitleBar(name, '')))
	text = Replace(text, 'Skill_Page', L1(SkillPage()))
	text = Replace(text, 'Emoticon_Page', L1(EmotionPage()))
	text = Replace(text, 'Quest_Page', L1(QuestPage()))
	text = text.replace('"y" : (SCREEN_HEIGHT - 37 - 361) / 2,', '"y" : (SCREEN_HEIGHT - 37 - %d) / 2,' % H, 1)
	text = text.replace('"width" : 253,\n\t"height" : 361,', '"width" : %d,\n\t"height" : %d,' % (W, H), 1)
	old = '"name" : "board",\n\t\t\t"type" : "board",'
	if text.count(old) != 1:
		raise SystemExit('characterwindow: board bulunamadi')
	text = text.replace(old, '"name" : "board",\n\t\t\t"type" : "expanded_image",\n\t\t\t"image" : "%scw_board2.png",' % IMG, 1)
	text = text.replace('"width" : 253,\n\t\t\t"height" : 361,', '"width" : %d,\n\t\t\t"height" : %d,' % (W, H), 1)
	# kenar cubugu ayirici cizgi (board'un ilk cocugu)
	text = text.replace('"y_scale" : %f,' % (H / 1028.0), '"y_scale" : %f,' % (H / 1028.0), 1)
	text = '# AKRAVON: otomatik uretildi (skin/build_akravon_charwin.py)\n' + text
	dst = os.path.join(out, 'uiscript', 'characterwindow.py')
	os.makedirs(os.path.dirname(dst), exist_ok = True)
	open(dst, 'wb').write(text.replace('\n', '\r\n').encode('latin1'))


def Build(out):
	BuildImages(out)
	BuildScript(out)
	print('karakter penceresi: %dx%d (kenar cubuklu)' % (W, H))


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
