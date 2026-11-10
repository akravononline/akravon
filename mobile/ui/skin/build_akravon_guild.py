# -*- coding: utf-8 -*-
# AKRAVON: lonca penceresi + lonca savasi ilani - sifirdan kendi tasarimimiz (karakter penceresi dili: lacivert + altin, solda dikey sekmeler)
# oge adlari ORIJINALLE AYNI (uiguild.py baglari calisir). Cikti:
#   <out>/uiscript/guildwindow.py, guildwindow_guildinfopage.py, guildwindow_guildinfopage_eu.py, declareguildwardialog.py
#   <out>/uiscript/guildwindow_{boardpage,memberpage,baseinfopage,gradepage,guildskillpage}.py (orijinal, icerik alanina tasindi)
#   <out>/ymir work/ui/akravon_legacy/gd_*.png , <out>/mobile_ui/akravon/hud/akrboard_700x520.png, akrboard_500x580.png
# kullanim: python build_akravon_guild.py [out]
import os
import re
import sys
import zipfile

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_akravon_charwin as cw
import build_akravon_inventory as inv

OUT = sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out'
cw.OUT_DIR = OUT
IMG = 'd:/ymir work/ui/akravon_legacy/'
ROOT_ZIP = os.path.join(HERE, '..', '..', '..', '..', '2_Oyun_Dosyalari', 'files', 'mobile', '10_root_1.zip')
if not os.path.exists(ROOT_ZIP):
	ROOT_ZIP = 'C:/AkravonTest/ios_dosyalar/mobile/10_root_1.zip'

W, H = 700, 520
CX, CY = 172, 66			# icerik alani (pencereye gore)
CW_, CH_ = W - CX - 16, H - CY - 16
TABS = (('GUILD_INFO', u'B\u0130LG\u0130'), ('BOARD', 'TAHTA'), ('MEMBER', u'\u00dcYELER'), ('BASE_INFO', 'ARAZ\u0130'), ('SKILL', 'BECER\u0130LER'), ('GRADE', u'R\u00dcTBELER'))
TAB_POS = (0, 1, 2, None, 3, 4)	# BASE_INFO (arazi) devre disi: gizli
TXT = (0.96, 0.94, 0.90)
DIM = (0.78, 0.74, 0.66)
GOLD = (0.89, 0.76, 0.50)


def L1(s):
	return s.encode('cp1254').decode('latin1')


def T(name, x, y, text, color = TXT, font = 'Lato:16', align = 'left'):
	return '{ "name":"%s", "type":"text", "x":%d, "y":%d, "text":"%s", "fontname":"%s", "r":%.2f, "g":%.2f, "b":%.2f, "a":1.0, "text_horizontal_align":"%s" },' % (
		name, x, y, text, font, color[0], color[1], color[2], align)


def Img(name, x, y, image, children = ()):
	s = '{ "name":"%s", "type":"image", "x":%d, "y":%d, "image":"%s%s.png"' % (name, x, y, IMG, image)
	if children:
		s += ', "children":(\n' + '\n'.join(children) + '\n)'
	return s + ' },'


def Btn(name, x, y, kind, text = '', radio = False):
	return '{ "name":"%s", "type":"%s", "x":%d, "y":%d, "text":"%s", "default_image":"%sgd_%s_up.png", "over_image":"%sgd_%s_over.png", "down_image":"%sgd_%s_down.png" },' % (
		name, 'radio_button' if radio else 'button', x, y, text, IMG, kind, IMG, kind, IMG, kind)


def Slot(name, x, y, w, h, value_name = None, value = '', font = 'Lato:16'):
	child = ()
	if value_name:
		child = (T(value_name, 0, 0, value, TXT, font, 'center').replace('"text_horizontal_align":"center" }', '"text_horizontal_align":"center", "all_align":"center" }'),)
	return Img(name, x, y, 'gd_slot_%dx%d' % (w, h), child)


def Head(name, x, y, w, text):
	return Img(name, x, y, 'gd_head_%d' % w, (T(name + '_Text', 12, 5, text, GOLD, 'Marcellus:17'),))


def Win(name, x, y, w, h, children):
	return '{ "name":"%s", "type":"window", "x":%d, "y":%d, "width":%d, "height":%d, "children":(\n%s\n) },' % (name, x, y, w, h, '\n'.join(children))


def Images():
	folder = os.path.join(OUT, 'ymir work', 'ui', 'akravon_legacy')
	hud = os.path.join(OUT, 'mobile_ui', 'akravon', 'hud')
	os.makedirs(folder, exist_ok = True)
	os.makedirs(hud, exist_ok = True)
	cw.Board(W, H).save(os.path.join(hud, 'akrboard_%dx%d.png' % (W, H)))
	cw.Board(720, 480).save(os.path.join(hud, 'akrboard_720x480.png'))
	cw.Board(460, 230).save(os.path.join(hud, 'akrboard_460x230.png'))
	img = {'gd_clear': Image.new('RGBA', (1, 1), (0, 0, 0, 0)), 'gd_vline': cw.Box(1, H - 90, (96, 74, 42, 255), (96, 74, 42, 255), 0), 'gd_vline_s': cw.Box(1, 230, (96, 74, 42, 255), (96, 74, 42, 255), 0), 'gd_hline_s': cw.Box(664, 1, (96, 74, 42, 255), (96, 74, 42, 255), 0)}
	assets = os.path.join(OUT, 'mobile_ui', 'akravon')
	ss = 4
	for (i, (key, label)) in enumerate(TABS):
		for on in (False, True):
			skin = Image.open(os.path.join(assets, 'plate_gold_normal.png' if on else 'picker_row_normal.png')).convert('RGBA')
			big = Image.new('RGBA', (140 * ss, 40 * ss), (0, 0, 0, 0))
			big.alpha_composite(skin.resize((140 * ss, 40 * ss), Image.LANCZOS))
			ImageDraw.Draw(big).text((70 * ss, 20 * ss), label, font = cw.Font('marcellus.ttf', 16), fill = cw.DARK if on else cw.TEXTC, anchor = 'mm')
			img['gd_tab%d_%s' % (i, 'on' if on else 'off')] = big.resize((140, 40), Image.LANCZOS)
	for (w, h) in ((170, 36), (100, 36), (212, 34), (74, 36), (96, 84), (270, 36), (186, 36), (150, 36), (160, 36), (70, 36), (64, 36), (200, 36), (60, 36), (96, 88)):
		img['gd_slot_%dx%d' % (w, h)] = inv.Slot(w, h)
	for w in (212, 240, 512):
		img['gd_head_%d' % w] = cw.Box(w, 30, cw.GOLD_DARK, (34, 26, 12, 235), 4)
	for (kind, w, h) in (('btn108', 108, 40), ('btn212', 212, 40), ('btn100', 100, 40), ('btn170', 170, 44), ('btn130', 130, 40), ('btn96', 96, 40), ('btn104t', 104, 32), ('btn90t', 90, 32), ('btn100h', 100, 36), ('btn120h', 120, 36), ('btn150', 150, 44)):
		for (state, fill, line) in (('up', (10, 14, 30, 240), cw.GOLD_DARK), ('over', (22, 30, 56, 240), cw.GOLD), ('down', (44, 36, 16, 245), cw.GOLD)):
			img['gd_%s_%s' % (kind, state)] = cw.Box(w, h, line, fill, 6)
	for (w, h) in ((120, 44), (300, 44), (130, 46), (76, 46), (50, 46), (56, 46), (44, 46), (44, 48), (200, 48), (52, 48)):
		img['gd_slot_%dx%d' % (w, h)] = inv.Slot(w, h)
	# kucuk simgeli dugmeler: sil (X), gonder, yenile
	def GlyphBtn(w, h, state, draw):
		fill = {'up': (10, 14, 30, 240), 'over': (22, 30, 56, 240), 'down': (44, 36, 16, 245)}[state]
		line = cw.GOLD_DARK if state == 'up' else cw.GOLD
		big = cw.Box(w, h, line, fill, 5)
		draw(ImageDraw.Draw(big), w, h)
		return big
	def DrawX(d, w, h):
		cx, cy = w // 2, h // 2
		d.line((cx - 6, cy - 6, cx + 6, cy + 6), fill = (222, 184, 110, 255), width = 3)
		d.line((cx - 6, cy + 6, cx + 6, cy - 6), fill = (222, 184, 110, 255), width = 3)
	def Label(text):
		def draw(d, w, h):
			d.text((w // 2, h // 2), text, font = cw.Font('marcellus.ttf', 14).font_variant(size = 14), fill = (236, 226, 204, 255), anchor = 'mm')
		return draw
	for state in ('up', 'over', 'down'):
		img['gd_x_%s' % state] = GlyphBtn(44, 44, state, DrawX)
		img['gd_send_%s' % state] = GlyphBtn(100, 44, state, Label(u'G\u00d6NDER'))
		img['gd_refresh_%s' % state] = GlyphBtn(100, 44, state, Label(u'YEN\u0130LE'))
	img['gd_cell64'] = inv.Slot(64, 64)
	dim = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
	ImageDraw.Draw(dim).rounded_rectangle((6, 6, 57, 57), 14, fill = (4, 6, 14, 150))
	img['gd_dim64'] = dim
	img['gd_lvbox'] = inv.Slot(40, 28)
	def DrawPlus(d, w, h):
		d.line((w // 2 - 7, h // 2, w // 2 + 7, h // 2), fill = (222, 184, 110, 255), width = 3)
		d.line((w // 2, h // 2 - 7, w // 2, h // 2 + 7), fill = (222, 184, 110, 255), width = 3)
	for state in ('up', 'over', 'down'):
		img['gd_plus_%s' % state] = GlyphBtn(30, 30, state, DrawPlus)
		img['gd_plusb_%s' % state] = GlyphBtn(36, 28, state, DrawPlus)
	check = Image.new('RGBA', (18, 18), (0, 0, 0, 0))
	cd = ImageDraw.Draw(check)
	cd.ellipse((0, 0, 17, 17), fill = (176, 136, 70, 255), outline = (255, 232, 170, 255))
	cd.line([(4, 9), (8, 13), (14, 5)], fill = (24, 16, 6, 255), width = 2)
	img['gd_check'] = check
	# imza / simge dugmeleri: eski 'LONCA IMZASI' basliginin rengi (kahverengi zemin, koyu altin cerceve)
	for (state, fill, line) in (('up', (34, 26, 12, 235), cw.GOLD_DARK), ('over', (52, 40, 18, 240), cw.GOLD), ('down', (22, 16, 6, 245), cw.GOLD)):
		img['gd_hd108_%s' % state] = cw.Box(108, 40, line, fill, 4)
	# SAVAS ILANI: belirgin - koyu kirmizi zemin, parlak altin cift cerceve
	for (state, fill) in (('up', (112, 28, 22, 245)), ('over', (140, 38, 30, 250)), ('down', (82, 18, 14, 250))):
		big = cw.Box(212, 44, (236, 198, 120, 255), fill, 7)
		ImageDraw.Draw(big).rounded_rectangle((3, 3, 208, 40), 5, outline = (150, 112, 52, 255), width = 1)
		img['gd_btn212w_%s' % state] = big
	for (name, im) in img.items():
		im.save(os.path.join(folder, name + '.png'))
	return len(img)


def InfoPage():
	c = []
	rows = (('GuildName', u'Lonca \u0130smi', 'GuildNameSlot', 'GuildNameValue', 'AKRAVON', 160),
		('GuildMaster', 'Lider', 'GuildMasterNameSlot', 'GuildMasterNameValue', 'Lider', 160),
		('GuildLevel', 'Seviye', 'GuildLevelSlot', 'GuildLevelValue', '1', 60),
		('CurrentExperience', u'Tecr\u00fcbe', 'CurrentExperienceSlot', 'CurrentExperienceValue', '0', 160),
		('LastExperience', u'Gereken Tecr\u00fcbe', 'LastExperienceSlot', 'LastExperienceValue', '0', 160),
		('GuildMemberCount', u'\u00dcye Say\u0131s\u0131', 'GuildMemberCountSlot', 'GuildMemberCountValue', '1 / 32', 160),
		('GuildMemberLevelAverage', 'Ortalama Seviye', 'GuildMemberLevelAverageSlot', 'GuildMemberLevelAverageValue', '1', 160),
		('GuildMoney', u'Lonca Yang\u0131', 'GuildMoneySlot', 'GuildMoneyValue', '0', 160))
	for (i, (name, label, slot, value, sample, sw)) in enumerate(rows):
		y = i * 52
		c.append(T(name, 0, y + 8, L1(label), DIM, 'Lato:16'))
		c.append(Slot(slot, 122, y, sw, 36, value, sample))
	c.append(Btn('OfferButton', 188, 2 * 52 + 2, 'btn90t', L1(u'YATIRIM')))
	# sag sutun
	c.append(Img('LargeGuildMarkSlot', 300, 0, 'gd_slot_96x88', ('{ "name":"LargeGuildMark", "type":"mark", "x":24, "y":26 },',)))
	c.append(Btn('UploadGuildMarkButton', 404, 0, 'hd108', L1(u'\u0130MZA')))
	c.append(Btn('UploadGuildSymbolButton', 404, 48, 'hd108', L1(u'S\u0130MGE')))
	c.append(Head('Guild_Enemy_Title_Bar', 300, 106, 212, L1(u'D\u00dc\u015eMAN LONCA')))
	for i in range(6):
		y = 148 + i * 38
		c.append(Img('EnemyGuildSlot%d' % (i + 1), 300, y, 'gd_slot_212x34', (T('EnemyGuildName%d' % (i + 1), 0, 0, L1(u'Bo\u015f'), TXT, 'Lato:15', 'center').replace('"text_horizontal_align":"center" }', '"text_horizontal_align":"center", "all_align":"center" }'),)))
		c.append(Btn('EnemyGuildCancel%d' % (i + 1), 474, y - 3, 'btn96', L1('X')))
	c.append(Btn('DeclareWarButton', 300, 386, 'btn212w', L1(u'SAVA\u015e \u0130LANI')))
	return c


def Script(name, window_children, w, h, x = CX, y = CY, extra = ''):
	return '# -*- coding: utf-8 -*-\n# AKRAVON: otomatik uretildi (skin/build_akravon_guild.py)\nimport uiScriptLocale\n\nwindow = {\n\t"name" : "%s",\n\t"x" : %d,\n\t"y" : %d,\n\t"width" : %d,\n\t"height" : %d,\n%s\t"children" : (\n%s\n\t),\n}\n' % (name, x, y, w, h, extra, '\n'.join(window_children))


def Write(path, text):
	os.makedirs(os.path.dirname(path), exist_ok = True)
	open(path, 'wb').write(text.replace('\n', '\r\n').encode('latin1'))


def MainWindow():
	tabs = []
	for (i, (key, label)) in enumerate(TABS):
		pos = TAB_POS[i]
		x, y = (14, 66 + pos * 46) if pos is not None else (-3000, 0)
		tabs.append('{ "name":"Tab_Button_%02d", "type":"radio_button", "x":%d, "y":%d, "default_image":"%sgd_tab%d_off.png", "over_image":"%sgd_tab%d_off.png", "down_image":"%sgd_tab%d_on.png" },' % (i + 1, x, y, IMG, i, IMG, i, IMG, i))
	tabs.append('{ "name":"Sidebar_Divider", "type":"image", "x":160, "y":64, "image":"%sgd_vline.png" },' % IMG)
	for i in range(6):
		tabs.append('{ "name":"Tab_%02d", "type":"image", "x":0, "y":0, "image":"%sgd_clear.png" },' % (i + 1, IMG))
	board = '{ "name":"Board", "type":"board_with_titlebar", "x":0, "y":0, "width":%d, "height":%d, "title":uiScriptLocale.GUILD_NAME, "children":(\n%s\n) },' % (W, H, '\n'.join(tabs))
	return '# -*- coding: utf-8 -*-\n# AKRAVON: otomatik uretildi (skin/build_akravon_guild.py)\nimport uiScriptLocale\n\nwindow = {\n\t"name" : "GuildWindow",\n\t"style" : ("movable", "float",),\n\t"x" : (SCREEN_WIDTH - %d) / 2,\n\t"y" : (SCREEN_HEIGHT - %d) / 2,\n\t"width" : %d,\n\t"height" : %d,\n\t"children" : (\n%s\n\t),\n}\n' % (W, H, W, H, board)


def DeclareDialog():
	Wd, Hd = 720, 480
	c = []
	fields = ((u'Düşman Lonca', 'InputName', 'InputSlot', 'InputValue', 16), (u'Maks. Skor', 'InputNameScore', 'InputSlotScore', 'InputValueScore', 3),
		(u'Maks. Üye', 'InputNameUser', 'InputSlotUser', 'InputValueUser', 3), (u'Min. Seviye', 'InputNameMinSeviye', 'InputSlotMinSeviye', 'InputValueMinSeviye', 2))
	for (i, (label, tname, slot, value, limit)) in enumerate(fields):
		y = 70 + i * 54
		c.append(T(tname, 28, y + 8, L1(label), DIM))
		c.append(Img(slot, 150, y, 'gd_slot_200x36', ('{ "name":"%s", "type":"editline", "x":14, "y":9, "width":172, "height":20, "input_limit":%d },' % (value, limit),)))
	c.append('{ "name":"Declare_Divider", "type":"image", "x":372, "y":66, "image":"%sgd_vline_s.png" },' % IMG)
	for (i, (label, prefix)) in enumerate(((u'Savaşçı', 'warrior'), ('Ninja', 'assassin'), ('Sura', 'sura'), (u'Şaman', 'shaman'))):
		y = 70 + i * 54
		c.append(T('%sStatusText' % prefix.capitalize(), 396, y + 8, L1(label), DIM))
		c.append(Btn('%s_on_button' % prefix, 500, y, 'btn100h', L1(u'AKTİF'), True))
		c.append(Btn('%s_off_button' % prefix, 608, y, 'btn100h', L1(u'PASİF'), True))
	gt = [T('GameTypeLabel', 0, 8, L1(u'Savaş Türü'), DIM),
		Btn('NormalButton', 122, 0, 'btn120h', L1('ALAN'), True),
		Btn('WarpButton', 252, 0, 'btn120h', L1('ARENA'), True),
		Btn('CTFButton', 382, 0, 'btn120h', L1('BAYRAK'), True)]
	c.append('{ "name":"GameType", "x":28, "y":318, "width":520, "height":40, "children":(\n%s\n) },' % '\n'.join(gt))
	c.append('{ "name":"Declare_Divider2", "type":"image", "x":28, "y":376, "image":"%sgd_hline_s.png" },' % IMG)
	c.append(Btn('AcceptButton', 190, 396, 'btn150', L1('ONAYLA')))
	c.append(Btn('CancelButton', 380, 396, 'btn150', L1(u'VAZGEÇ')))
	board = '{ "name":"Board", "type":"board_with_titlebar", "x":0, "y":0, "width":%d, "height":%d, "title":uiScriptLocale.GUILD_WAR_DECLARE, "children":(\n%s\n) },' % (Wd, Hd, '\n'.join(c))
	return '# -*- coding: utf-8 -*-\n# AKRAVON: otomatik uretildi (skin/build_akravon_guild.py)\nimport uiScriptLocale\n\nwindow = {\n\t"name" : "InputDialog",\n\t"style" : ("movable", "float",),\n\t"x" : (SCREEN_WIDTH - %d) / 2,\n\t"y" : (SCREEN_HEIGHT - %d) / 2,\n\t"width" : %d,\n\t"height" : %d,\n\t"children" : (\n%s\n\t),\n}\n' % (Wd, Hd, Wd, Hd, board)


def GradeNameDialog():
	Wd, Hd = 460, 230
	c = []
	c.append(T('GradeNameLabel', 28, 80, L1(u'R\u00fctbe ad\u0131'), DIM))
	c.append(Img('GradeNameSlot', 150, 72, 'gd_slot_270x36', ('{ "name":"GradeNameValue", "type":"editline", "x":14, "y":9, "width":242, "height":20, "input_limit":8 },',)))
	c.append(Btn('AcceptButton', 60, 150, 'btn150', L1('TAMAM')))
	c.append(Btn('CancelButton', 250, 150, 'btn150', L1(u'VAZGE\u00c7')))
	board = '{ "name":"Board", "type":"board_with_titlebar", "x":0, "y":0, "width":%d, "height":%d, "title":uiScriptLocale.GUILD_GRADE_CHANGE_GRADE_NAME, "children":(\n%s\n) },' % (Wd, Hd, '\n'.join(c))
	return '# -*- coding: utf-8 -*-\n# AKRAVON: otomatik uretildi (skin/build_akravon_guild.py)\nimport uiScriptLocale\n\nwindow = {\n\t"name" : "ChangeGradeNameDialog",\n\t"style" : ("movable", "float",),\n\t"x" : (SCREEN_WIDTH - %d) / 2,\n\t"y" : (SCREEN_HEIGHT - %d) / 2,\n\t"width" : %d,\n\t"height" : %d,\n\t"children" : (\n%s\n\t),\n}\n' % (Wd, Hd, Wd, Hd, board)


def BoardPage():
	c = [Img('CommentSlot', 0, 386, 'gd_slot_300x44', ('{ "name":"CommentValue", "type":"editline", "x":12, "y":12, "width":276, "height":20, "input_limit":49, "text":"" },',))]
	c.append(Btn('RefreshButton', 412, 386, 'refresh'))
	return c


def MemberPage():
	c = []
	for (name, x, text) in (('IndexName', 10, u'\u00dcYE'), ('IndexGrade', 150, u'R\u00dcTBE'), ('IndexJob', 258, 'MESLEK'), ('IndexLevel', 332, 'SEV.'), ('IndexOffer', 386, 'KATKI'), ('IndexGeneral', 448, 'GEN.')):
		c.append(T(name, x, 5, L1(text), GOLD, 'Marcellus:15'))
	c.append('{ "name":"ScrollBar", "type":"scrollbar", "x":494, "y":30, "scrollbar_type":"normal", "size":362 },')
	return c


def GradePage():
	c = []
	for (name, x, text) in (('GradeNumber', 12, 'NO'), ('GradeName', 62, u'R\u00dcTBE ADI'), ('InviteAuthority', 266, 'DAVET'), ('DriveOutAuthority', 332, 'ATMA'), ('NoticeAuthority', 392, 'DUYURU'), ('SkillAuthority', 456, u'BECER\u0130')):
		c.append(T(name, x, 5, L1(text), GOLD, 'Marcellus:13'))
	return c


def SkillPage():
	c = []
	c.append(Img('Skill_Head_Active', 0, 0, 'gd_head_512', (T('Active_Skill_Title', 12, 5, L1(u'AKT\u0130F BECER\u0130LER'), GOLD, 'Marcellus:17'), T('Skill_Plus_Label', 372, 6, L1('Puan'), DIM, 'Lato:16'), T('Skill_Plus_Value', 436, 6, '0', GOLD, 'LatoBd:17'))))
	c.append('{ "name":"Active_Skill_Slot_Table", "type":"grid_table", "x":16, "y":44, "start_index":210, "x_count":6, "y_count":1, "x_step":80, "y_step":72, "image":"%sgd_clear.png" },' % IMG)
	c.append(Img('Skill_Head_Affect', 0, 150, 'gd_head_512', (T('Affect_Title', 12, 5, L1(u'DURUM'), GOLD, 'Marcellus:17'),)))
	c.append('{ "name":"Affect_Slot_Table", "type":"grid_table", "x":16, "y":192, "start_index":0, "x_count":9, "y_count":2, "x_step":54, "y_step":54, "image":"%sinv_cell.png" },' % IMG)
	c.append(Img('Skill_Head_Power', 0, 318, 'gd_head_512', (T('Dragon_God_Power_Title', 12, 5, L1(u'EJDER TANRI G\u00dcC\u00dc'), GOLD, 'Marcellus:17'),)))
	c.append('{ "name":"Dragon_God_Power_Gauge_Slot", "type":"image", "x":8, "y":372, "image":"d:/ymir work/ui/game/guild/gauge.sub" },')
	c.append('{ "name":"Dragon_God_Power_Gauge", "type":"ani_image", "x":12, "y":372, "delay":6, "images":("D:/Ymir Work/UI/Pattern/SPGauge/01.tga","D:/Ymir Work/UI/Pattern/SPGauge/02.tga","D:/Ymir Work/UI/Pattern/SPGauge/03.tga","D:/Ymir Work/UI/Pattern/SPGauge/04.tga","D:/Ymir Work/UI/Pattern/SPGauge/05.tga","D:/Ymir Work/UI/Pattern/SPGauge/06.tga","D:/Ymir Work/UI/Pattern/SPGauge/07.tga") },')
	c.append(Img('Dragon_God_Power_Slot', 204, 358, 'gd_slot_160x36', (T('Dragon_God_Power_Value', 0, 0, '0 / 0', TXT, 'Lato:16', 'center').replace('"text_horizontal_align":"center" }', '"text_horizontal_align":"center", "all_align":"center" }'),)))
	c.append(Btn('Heal_GSP_Button', 382, 356, 'btn130', L1('\u0130Y\u0130LE\u015eT\u0130R')))
	# kullanilmayan (devre disi) pasif beceri izgarasi: ekran disi, kodun bagi kopmasin
	c.append('{ "name":"Passive_Skill_Slot_Table", "type":"grid_table", "x":-3000, "y":0, "start_index":200, "x_count":9, "y_count":1, "x_step":32, "y_step":32, "image":"d:/ymir work/ui/public/Slot_Base.sub" },')
	return c


def OtherPages():
	z = zipfile.ZipFile(ROOT_ZIP)
	for n in ('baseinfopage',):
		t = z.read('uiscript/guildwindow_%s.py' % n).decode('latin1')
		t2 = re.sub(r'("x"\s*:\s*)8(\s*,\s*"y"\s*:\s*)30', lambda m: '%s%d%s%d' % (m.group(1), CX, m.group(2), CY), t, count = 1)
		assert t2 != t, n
		Write(os.path.join(OUT, 'uiscript', 'guildwindow_%s.py' % n), t2.replace('\r\n', '\n'))


def Build():
	count = Images()
	Write(os.path.join(OUT, 'uiscript', 'guildwindow.py'), MainWindow())
	page = Script('GuildWindow_GuildInfoPage', InfoPage(), CW_, CH_)
	for name in ('guildwindow_guildinfopage.py', 'guildwindow_guildinfopage_eu.py'):
		Write(os.path.join(OUT, 'uiscript', name), page)
	Write(os.path.join(OUT, 'uiscript', 'declareguildwardialog.py'), DeclareDialog())
	Write(os.path.join(OUT, 'uiscript', 'changegradenamedialog.py'), GradeNameDialog())
	for (n, children) in (('boardpage', BoardPage()), ('memberpage', MemberPage()), ('gradepage', GradePage())):
		Write(os.path.join(OUT, 'uiscript', 'guildwindow_%s.py' % n), Script('GuildWindow_%sPage' % n.capitalize(), children, CW_, CH_))
	Write(os.path.join(OUT, 'uiscript', 'guildwindow_guildskillpage.py'), Script('GuildWindow_GuildSkillPage', SkillPage(), CW_, CH_))
	OtherPages()
	print('lonca: %d gorsel + 4 yeni betik + 5 sayfa tasindi' % count)


if __name__ == '__main__':
	Build()
