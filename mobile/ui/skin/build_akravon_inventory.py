# -*- coding: utf-8 -*-
# AKRAVON: envanter + kayan ekipman penceresi (onayli taslak: envanter_taslak7)
# cikti: <out>/uiscript/inventorywindowex.py (oge adlari ayni -> uiinventory.py baglari calisir)
#        <out>/ymir work/ui/akravon_legacy/inv_*.png
import io
import os
import sys
import zipfile
from PIL import Image, ImageDraw, ImageEnhance, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_akravon_charwin as cw

ICON_ZIP = os.path.join(HERE, '..', '..', '..', '..', '2_Oyun_Dosyalari', 'files', 'mobile', '15_icon_1.zip')
IMG = 'd:/ymir work/ui/akravon_legacy/'
SS = 4
GOLD = (222, 184, 110, 255)

# olculer (ui px) - uiakravoninventory.py ile ayni
CELL = 50
ROWS = 7						# AKRAVON: 5 sayfa x (5 x 7) goz; sunucu 180 yuva (175 gorunur)
PAGES = 5
IW = 5 * CELL + 40			# envanter penceresi
EU = 50						# ekipman birimi
ES = 60						# ekipman adimi
EW = 4 * ES - 10 + 40		# ekipman penceresi
EH = 330
TAB = 30					# cekmece kulpu
GAP = 10
W = TAB + EW + GAP + IW
GRID_Y = 116
TABS_Y = 62
MONEY_Y = GRID_Y + ROWS * CELL + 10
IH = MONEY_Y + 60

# (yuva, x, y, w, h, bos yuva gorseli)
EQUIP = (
	('EQUIPMENT_START_INDEX+4', 0, 0, EU, 3 * EU, '00010'),		# silah
	('EQUIPMENT_START_INDEX+1', ES, 0, EU, EU, '12200'),			# kask
	('EQUIPMENT_START_INDEX+0', ES, ES, EU, 2 * EU + 10, '11200'),	# zirh
	('EQUIPMENT_START_INDEX+2', ES, 3 * ES, EU, EU, '15000'),		# ayakkabi
	('EQUIPMENT_START_INDEX+10', 2 * ES, 0, EU, EU, '13000'),		# kalkan
	('EQUIPMENT_START_INDEX+9', 3 * ES, 0, EU, EU, '08000'),		# ok
	('EQUIPMENT_START_INDEX+6', 2 * ES, ES, EU, EU, '17000'),		# kupe
	('EQUIPMENT_START_INDEX+5', 3 * ES, ES, EU, EU, '16000'),		# kolye
	('EQUIPMENT_START_INDEX+3', 2 * ES, 2 * ES, EU, EU, '14000'),	# bilezik
	('item.EQUIPMENT_BELT', 3 * ES, 2 * ES, EU, EU, '18000'),		# kusak
	('EQUIPMENT_START_INDEX+7', 2 * ES, 3 * ES, EU, EU, '71136'),	# ozel 1
	('EQUIPMENT_START_INDEX+8', 3 * ES, 3 * ES, EU, EU, '71136'),	# ozel 2
)


def Slot(w, h):
	# oyuk yuva: koyu ic, ic golge, bronz kenar, altta ince isik
	big = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	d.rounded_rectangle((0, 0, w * SS - 1, h * SS - 1), 6 * SS, fill = (8, 11, 22, 255), outline = (92, 74, 44, 255), width = SS)
	shade = Image.new('L', big.size, 0)
	ImageDraw.Draw(shade).rounded_rectangle((SS, SS, w * SS - SS, h * SS - SS), 5 * SS, outline = 255, width = 5 * SS)
	shade = shade.filter(ImageFilter.GaussianBlur(4 * SS))
	big.alpha_composite(Image.composite(Image.new('RGBA', big.size, (0, 0, 0, 170)), Image.new('RGBA', big.size, (0, 0, 0, 0)), shade))
	d = ImageDraw.Draw(big)
	d.rounded_rectangle((2 * SS, 2 * SS, w * SS - 2 * SS, h * SS - 2 * SS), 5 * SS, outline = (40, 40, 52, 255), width = SS)
	d.line((8 * SS, h * SS - 2 * SS, w * SS - 8 * SS, h * SS - 2 * SS), fill = (150, 120, 70, 160), width = SS)
	return big.resize((w, h), Image.LANCZOS)


def IconButton(kind, state):
	# kendi tasarimimiz: slot cukuru + ince cizgili altin ikon (sandik = depo, elbise = kostum)
	N = 50
	im = Slot(N, N)
	if state == 'down':
		im = ImageEnhance.Brightness(im).enhance(0.7)
	elif state == 'over':
		im = ImageEnhance.Brightness(im).enhance(1.25)
	big = Image.new('RGBA', (N * SS, N * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	col = (255, 226, 160, 255) if state != 'down' else (190, 150, 90, 255)
	w = int(1.8 * SS)
	k = SS
	if kind == 'depo':
		d.rounded_rectangle((11 * k, 22 * k, 39 * k, 40 * k), 3 * k, outline = col, width = w)
		d.arc((12 * k, 12 * k, 38 * k, 32 * k), 180, 360, fill = col, width = w)
		d.line((12 * k, 27 * k, 38 * k, 27 * k), fill = col, width = w)
		d.rounded_rectangle((22 * k, 25 * k, 28 * k, 31 * k), k, fill = col)
	else:
		d.arc((20 * k, 10 * k, 30 * k, 20 * k), 200, 450, fill = col, width = w)
		d.line((25 * k, 19 * k, 25 * k, 23 * k), fill = col, width = w)
		d.line((25 * k, 23 * k, 10 * k, 35 * k), fill = col, width = w)
		d.line((25 * k, 23 * k, 40 * k, 35 * k), fill = col, width = w)
		d.line((10 * k, 35 * k, 40 * k, 35 * k), fill = col, width = w)
	im.alpha_composite(big.resize((N, N), Image.LANCZOS))
	return im


def Ghost(icons, vnum, size):
	data = icons.read(icons.names['icon/item/%s.tga' % vnum])
	im = Image.open(io.BytesIO(data)).convert('RGBA')
	im = im.resize((size, size * im.height // im.width), Image.LANCZOS)
	im = ImageEnhance.Brightness(ImageEnhance.Color(im).enhance(0)).enhance(0.6)
	im.putalpha(im.getchannel('A').point(lambda p: int(p * 0.35)))
	return im


def EquipBase(icons):
	base = Image.new('RGBA', (4 * ES - 10, 4 * ES - 10), (0, 0, 0, 0))
	for (index, x, y, w, h, vnum) in EQUIP:
		base.alpha_composite(Slot(w, h), (x, y))
		ghost = Ghost(icons, vnum, min(w, h) - 14)
		base.alpha_composite(ghost, (x + (w - ghost.width) // 2, y + (h - ghost.height) // 2))
	return base


def TabGlow(w, h):
	# secili sayfa: dolu slotla ayni oyuk derinlik (serin lacivert taban, ust/sol ic golge, alt/sag ince isik)
	R = 8
	im = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(im)
	box = (4 * SS, 4 * SS, (w - 4) * SS, (h - 4) * SS)
	d.rounded_rectangle(box, R * SS, fill = (30, 40, 62, 255))
	shade = Image.new('L', im.size, 0)
	ImageDraw.Draw(shade).rounded_rectangle(box, R * SS, outline = 255, width = 5 * SS)
	shade = shade.filter(ImageFilter.GaussianBlur(3 * SS))
	mask = Image.new('L', im.size, 0)
	ImageDraw.Draw(mask).rounded_rectangle(box, R * SS, fill = 255)
	dark = Image.new('RGBA', im.size, (0, 0, 0, 150))
	im.alpha_composite(Image.composite(dark, Image.new('RGBA', im.size, (0, 0, 0, 0)), Image.composite(shade, Image.new('L', im.size, 0), mask)))
	d = ImageDraw.Draw(im)
	d.line((10 * SS, (h - 5) * SS, (w - 10) * SS, (h - 5) * SS), fill = (165, 198, 255, 46), width = SS)
	return im.resize((w, h), Image.LANCZOS)


def TabStrip(w, h, count):
	im = Slot(w, h)
	d = ImageDraw.Draw(im)
	for i in range(1, count):
		x = int(w * i / float(count))
		d.line((x, 8, x, h - 8), fill = (70, 60, 40, 255), width = 1)
	return im


def PullTab(opened):
	(w, h) = (TAB, 110)
	tex = cw.Board(60, h).crop((0, 0, w, h))
	mask = Image.new('L', (w * SS, h * SS), 0)
	ImageDraw.Draw(mask).rounded_rectangle((0, 0, (w + 14) * SS, h * SS - 1), 12 * SS, fill = 255)
	mask = mask.resize((w, h), Image.LANCZOS)
	tab = Image.new('RGBA', (w, h), (0, 0, 0, 0))
	tab.paste(tex, (0, 0), mask)
	line = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	ImageDraw.Draw(line).rounded_rectangle((SS, SS, (w + 14) * SS, h * SS - SS), 12 * SS, outline = GOLD, width = int(1.5 * SS))
	tab.alpha_composite(line.resize((w, h), Image.LANCZOS))
	return tab


def Chevron(out, opened):
	# tek cizgili, ince ok: acik -> '>' (kapatir), kapali -> '<' (acar)
	(w, h) = (14, 22)
	big = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	a = (3 * SS, 3 * SS) if opened else (w * SS - 3 * SS, 3 * SS)
	b = (w * SS - 3 * SS, h * SS // 2) if opened else (3 * SS, h * SS // 2)
	c = (3 * SS, h * SS - 3 * SS) if opened else (w * SS - 3 * SS, h * SS - 3 * SS)
	d.line((a, b, c), fill = GOLD, width = int(1.6 * SS), joint = 'curve')
	return big.resize((w, h), Image.LANCZOS)


def Close(state):
	big = Image.new('RGBA', (32 * SS, 32 * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	fill = (150, 112, 56, 255) if state == 'down' else (8, 11, 22, 255)
	d.rounded_rectangle((0, 0, 32 * SS - 1, 32 * SS - 1), 5 * SS, fill = fill, outline = GOLD, width = SS)
	col = (24, 16, 6, 255) if state == 'down' else GOLD
	d.line((8 * SS, 8 * SS, 24 * SS, 24 * SS), fill = col, width = 3 * SS)
	d.line((8 * SS, 24 * SS, 24 * SS, 8 * SS), fill = col, width = 3 * SS)
	return big.resize((32, 32), Image.LANCZOS)


def Coin():
	big = Image.new('RGBA', (26 * SS, 26 * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	d.ellipse((SS, SS, 25 * SS, 25 * SS), fill = (226, 182, 64, 255), outline = (120, 86, 24, 255), width = int(1.5 * SS))
	d.ellipse((6 * SS, 6 * SS, 20 * SS, 20 * SS), outline = (255, 232, 150, 255), width = int(1.5 * SS))
	d.text((13 * SS, 13 * SS), 'Y', fill = (120, 86, 24, 255), anchor = 'mm', font = __import__('PIL.ImageFont', fromlist = ['x']).truetype(os.path.join(HERE, '..', '..', 'data', 'fonts', 'latobd.ttf'), 12 * SS))
	return big.resize((26, 26), Image.LANCZOS)


def Sort(state):
	im = Slot(44, 40)
	if state == 'down':
		im = ImageEnhance.Brightness(im).enhance(1.6)
	d = ImageDraw.Draw(im)
	for (x, up) in ((16, True), (28, False)):
		(y0, y1) = (12, 28)
		d.line((x, y0, x, y1), fill = GOLD, width = 2)
		tip = y0 if up else y1
		k = 5 if up else -5
		d.polygon([(x - 5, tip + k), (x + 5, tip + k), (x, tip)], fill = GOLD)
	return im


def Layout():
	eq = []
	for (index, x, y, w, h, vnum) in EQUIP:
		eq.append('\t\t\t\t\t\t{"index":%s, "x":%d, "y":%d, "width":%d, "height":%d},' % (index, x, y, w, h))
	tabs = []
	tw = (IW - 40) // PAGES
	names = ('I', 'II', 'III', 'IV', 'V')
	for i in range(PAGES):
		tabs.append('''			{
				"name" : "Inventory_Tab_%02d", "type" : "radio_button", "x" : %d, "y" : %d,
				"tooltip_text" : "Sayfa %d",
				"default_image" : IMG + "inv_tab_empty.png", "over_image" : IMG + "inv_tab_empty.png", "down_image" : IMG + "inv_tab_on.png",
				"children" : ({ "name" : "Inventory_Tab_%02d_Print", "type" : "text", "x" : 0, "y" : -2, "all_align" : "center", "text" : "%s" },),
			},''' % (i + 1, 20 + i * tw, TABS_Y, i + 1, i + 1, names[i]))
	return '''# -*- coding: utf-8 -*-
# AKRAVON: uretildi (skin/build_akravon_inventory.py) - elle duzenleme
import uiScriptLocale
import item
import app
import player
EQUIPMENT_START_INDEX = player.EQUIPMENT_SLOT_START
IMG = "%(img)s"

window = {
	"name" : "InventoryWindow",
	"x" : SCREEN_WIDTH - %(w)d - 100,
	"y" : 70,
	"style" : ("float",),
	"width" : %(w)d,
	"height" : %(ih)d,
	"children" :
	(
		{
			"name" : "EquipBoard", "type" : "expanded_image", "x" : %(ex)d, "y" : 0, "image" : IMG + "inv_equip_board.png",
			"children" :
			(
				{
					"name" : "Equipment_Base", "type" : "expanded_image", "x" : 20, "y" : 62, "image" : IMG + "inv_equip_base.png",
					"children" :
					(
						{
							"name" : "EquipmentSlot", "type" : "slot", "x" : 0, "y" : 0, "width" : %(eg)d, "height" : %(eg)d,
							"slot" : (
%(eq)s
							),
						},
						{ "name" : "DSSButton", "type" : "button", "x" : 0, "y" : 0 },
						{ "name" : "Equipment_Tab_01", "type" : "radio_button", "x" : 0, "y" : 0,
							"default_image" : "d:/ymir work/ui/game/windows/tab_button_small_01.sub",
							"over_image" : "d:/ymir work/ui/game/windows/tab_button_small_02.sub",
							"down_image" : "d:/ymir work/ui/game/windows/tab_button_small_03.sub" },
						{ "name" : "Equipment_Tab_02", "type" : "radio_button", "x" : 0, "y" : 0,
							"default_image" : "d:/ymir work/ui/game/windows/tab_button_small_01.sub",
							"over_image" : "d:/ymir work/ui/game/windows/tab_button_small_02.sub",
							"down_image" : "d:/ymir work/ui/game/windows/tab_button_small_03.sub" },
					),
				},
				{ "name" : "MallButton", "type" : "button", "x" : 142, "y" : %(my)d, "tooltip_text" : uiScriptLocale.MALL_TITLE,
					"default_image" : IMG + "inv_depo_up.png", "over_image" : IMG + "inv_depo_over.png", "down_image" : IMG + "inv_depo_down.png" },
				{ "name" : "CostumeButton", "type" : "button", "x" : 202, "y" : %(my)d, "tooltip_text" : uiScriptLocale.COSTUME_TITLE,
					"default_image" : IMG + "inv_kost_up.png", "over_image" : IMG + "inv_kost_over.png", "down_image" : IMG + "inv_kost_down.png" },
			),
		},
		{
			"name" : "board", "type" : "expanded_image", "x" : %(ix)d, "y" : 0, "image" : IMG + "inv_board.png",
			"children" :
			(
				{ "name" : "TitleBar", "type" : "titlebar", "style" : ("attach",), "x" : 0, "y" : -100, "width" : 60, "color" : "yellow",
					"children" : ({ "name" : "TitleName", "type" : "text", "x" : 0, "y" : 3, "text" : "", "text_horizontal_align" : "center", "horizontal_align" : "center" },) },
				{ "name" : "Inventory_Tab_Strip", "type" : "expanded_image", "x" : 20, "y" : %(ty)d, "image" : IMG + "inv_tab_strip.png" },
%(tabs)s
				{ "name" : "ItemSlot", "type" : "grid_table", "x" : 20, "y" : %(gy)d, "start_index" : 0, "x_count" : 5, "y_count" : %(rows)d,
					"x_step" : %(cell)d, "y_step" : %(cell)d, "image" : IMG + "inv_cell.png" },
				{ "name" : "Money_Slot", "type" : "button", "x" : 20, "y" : %(mny)d,
					"default_image" : IMG + "inv_money.png", "over_image" : IMG + "inv_money.png", "down_image" : IMG + "inv_money.png",
					"children" :
					(
						{ "name" : "Money_Icon", "type" : "image", "x" : 10, "y" : 7, "image" : IMG + "inv_coin.png" },
						{ "name" : "Money", "type" : "text", "x" : 14, "y" : 11, "horizontal_align" : "right", "text_horizontal_align" : "right", "text" : "0" },
					),
				},
				{ "name" : "EditButton", "type" : "button", "x" : %(sx)d, "y" : %(mny)d, "tooltip_text" : uiScriptLocale.ENVANTER_DUZENLE,
					"default_image" : IMG + "inv_sort_up.png", "over_image" : IMG + "inv_sort_up.png", "down_image" : IMG + "inv_sort_down.png" },
			),
		},
	),
}
''' % {'img': IMG, 'w': W, 'ih': IH, 'ex': TAB, 'ix': TAB + EW + GAP, 'eg': 4 * ES - 10, 'eq': '\n'.join(eq),
		'cy': 3 * ES + 9, 'my': 62 + 4 * ES - 10 + 12, 'ty': TABS_Y, 'tabs': '\n'.join(tabs), 'gy': GRID_Y, 'cell': CELL,
		'mny': MONEY_Y, 'sx': IW - 20 - 44, 'rows': ROWS}


def Build(out):
	class Icons(object):
		pass
	icons = zipfile.ZipFile(ICON_ZIP)
	icons.names = dict((n.lower(), n) for n in icons.namelist())
	folder = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy')
	os.makedirs(folder, exist_ok = True)
	tw = (IW - 40) // PAGES
	images = {
		'inv_board': cw.Board(IW, IH),
		'inv_equip_board': cw.Board(EW, EH),
		'inv_equip_base': EquipBase(icons),
		'inv_cell': Slot(CELL, CELL),
		'inv_tab_strip': TabStrip(IW - 40, 42, PAGES),
		'inv_tab_empty': Image.new('RGBA', (tw, 42), (0, 0, 0, 0)),
		'inv_tab_on': TabGlow(tw, 42),
		'inv_money': Slot(IW - 40, 40),
		'inv_coin': Coin(),
		'inv_depo_up': IconButton('depo', 'up'), 'inv_depo_over': IconButton('depo', 'over'), 'inv_depo_down': IconButton('depo', 'down'),
		'inv_kost_up': IconButton('kost', 'up'), 'inv_kost_over': IconButton('kost', 'over'), 'inv_kost_down': IconButton('kost', 'down'),
		'inv_sort_up': Sort('up'), 'inv_sort_down': Sort('down'),
		'inv_close_1': Close('up'), 'inv_close_2': Close('up'), 'inv_close_3': Close('down'),
		'inv_pull': PullTab(False),
		'inv_chev_open': Chevron(out, False), 'inv_chev_close': Chevron(out, True),
	}
	for (name, im) in images.items():
		im.save(os.path.join(folder, name + '.png'))
	dst = os.path.join(out, 'uiscript', 'inventorywindowex.py')
	os.makedirs(os.path.dirname(dst), exist_ok = True)
	open(dst, 'w', encoding = 'utf-8', newline = '\n').write(Layout())
	BuildCostume(out, icons)
	print('envanter: %d gorsel + uiscript/inventorywindowex.py + costumewindow.py' % len(images))


# --- AKRAVON: kostum penceresi (ekipmanla ayni dil) ---
COSTUME = (
	('COSTUME_START_INDEX+4', 0, 0, EU, 3 * EU, '00010'),		# kostum silah
	('COSTUME_START_INDEX+1', ES, 0, EU, EU, '45001'),		# sac
	('COSTUME_START_INDEX+0', ES, ES, EU, 2 * EU + 10, '41001'),	# kostum govde
	('item.EQUIPMENT_RING4', 2 * ES, 0, EU, EU, '71136'),
	('item.EQUIPMENT_RING5', 3 * ES, 0, EU, EU, '71136'),
	('item.EQUIPMENT_RING1', 2 * ES, ES, EU, EU, '71136'),
	('item.EQUIPMENT_RING2', 3 * ES, ES, EU, EU, '71136'),
	('item.EQUIPMENT_RING3', 2 * ES, 2 * ES, EU, EU, '71136'),
	('item.EQUIPMENT_RING6', 3 * ES, 2 * ES, EU, EU, '71136'),
	('COSTUME_START_INDEX+2', 0, 3 * ES + 10, EU, EU, '71124'),	# binek
	('COSTUME_START_INDEX+5', ES, 3 * ES + 10, EU, EU, '53001'),	# pet
)
CEH = 62 + 4 * ES + 14


def GhostSafe(icons, vnum, size):
	try:
		return Ghost(icons, vnum, size)
	except KeyError:
		return Ghost(icons, '71136', size)


def CostumeBase(icons):
	base = Image.new('RGBA', (4 * ES - 10, 4 * ES), (0, 0, 0, 0))
	for (index, x, y, w, h, vnum) in COSTUME:
		base.alpha_composite(Slot(w, h), (x, y))
		ghost = GhostSafe(icons, vnum, min(w, h) - 14)
		base.alpha_composite(ghost, (x + (w - ghost.width) // 2, y + (h - ghost.height) // 2))
	return base


def Eye(shown):
	big = Image.new('RGBA', (20 * SS, 20 * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	d.ellipse((SS, SS, 19 * SS, 19 * SS), fill = (14, 20, 38, 235), outline = GOLD, width = SS)
	col = (255, 214, 140, 255) if shown else (150, 140, 120, 255)
	d.ellipse((4 * SS, 7 * SS, 16 * SS, 13 * SS), outline = col, width = int(1.5 * SS))
	d.ellipse((8 * SS, 8 * SS, 12 * SS, 12 * SS), fill = col)
	if not shown:
		d.line((4 * SS, 15 * SS, 16 * SS, 5 * SS), fill = col, width = int(1.6 * SS))
	return big.resize((20, 20), Image.LANCZOS)


def CostumeLayout():
	slots = '\n'.join('\t\t\t\t\t\t{"index":%s, "x":%d, "y":%d, "width":%d, "height":%d},' % (i, x, y, w, h) for (i, x, y, w, h, v) in COSTUME)
	return """# -*- coding: utf-8 -*-
# AKRAVON: uretildi (skin/build_akravon_inventory.py) - elle duzenleme
import uiScriptLocale
import item
import app
COSTUME_START_INDEX = item.COSTUME_SLOT_START
IMG = "%(img)s"
window = {
	"name" : "CostumeWindow",
	"x" : SCREEN_WIDTH - 700,
	"y" : 80,
	"style" : ("float",),
	"width" : %(w)d,
	"height" : %(h)d,
	"children" :
	(
		{
			"name" : "board", "type" : "expanded_image", "style" : ("attach",), "x" : 0, "y" : 0, "image" : IMG + "inv_costume_board.png",
			"children" :
			(
				{ "name" : "TitleBar", "type" : "titlebar", "style" : ("attach",), "x" : 0, "y" : -100, "width" : 60, "color" : "yellow",
					"children" : ({ "name" : "TitleName", "type" : "text", "x" : 0, "y" : 3, "text" : "", "text_horizontal_align" : "center" },) },
				{
					"name" : "Costume_Base", "type" : "expanded_image", "x" : 20, "y" : 62, "image" : IMG + "inv_costume_base.png",
					"children" :
					(
						{
							"name" : "CostumeSlot", "type" : "slot", "x" : 0, "y" : 0, "width" : %(bw)d, "height" : %(bh)d,
							"slot" : (
%(slots)s
							),
						},
					),
				},
			),
		},
	),
}
""" % {'img': IMG, 'w': EW, 'h': CEH, 'lw': EW - 32, 'bw': 4 * ES - 10, 'bh': 4 * ES, 'slots': slots}


def BuildCostume(out, icons):
	folder = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy')
	cw.Board(EW, CEH).save(os.path.join(folder, 'inv_costume_board.png'))
	CostumeBase(icons).save(os.path.join(folder, 'inv_costume_base.png'))
	hide = os.path.join(out, 'costume_hide')
	os.makedirs(hide, exist_ok = True)
	Eye(False).save(os.path.join(hide, 'button_hide_0.tga'), format = 'TGA')
	Eye(True).save(os.path.join(hide, 'button_show_0.tga'), format = 'TGA')
	open(os.path.join(out, 'uiscript', 'costumewindow.py'), 'w', encoding = 'utf-8', newline = '\n').write(CostumeLayout())


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
