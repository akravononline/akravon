# -*- coding: utf-8 -*-
# AKRAVON: pazar (ikashop) pencerelerinin gorselleri - ayni dosya adi / ayni boyut, bizim tasarim dili
# cikti: <out>/ikashop/lite/...   (cihazda files/mobile/ikashop/lite altina acik dosya olarak konur)
# kaynaklar: skin/src/pazar_serit.png (kullanici gorseli), skin/src/ikashop_orig/ (orijinal parcalar - olcu referansi)
import os, sys
from PIL import Image, ImageDraw, ImageEnhance, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_akravon_charwin as cw

SS = 4
GOLD = (222, 184, 110, 255)
GOLD_DIM = (112, 86, 46, 255)
NAVY = (14, 20, 36, 255)
NAVY_DEEP = (8, 11, 22, 255)
ORIG = os.path.join(HERE, 'src', 'ikashop_orig')


def Orig(name):
	return Image.open(os.path.join(ORIG, name)).convert('RGBA')


def Texture():
	src = Image.open(os.path.join(cw.OUT_DIR, 'mobile_ui', 'akravon', 'picker_panel.png')).convert('RGBA')
	return src.crop((120, 120, 700, 860))


def Save(out, name, im):
	p = os.path.join(out, 'ikashop', 'lite', name)
	os.makedirs(os.path.dirname(p), exist_ok = True)
	im.save(p)


def Tile(w, h):
	# kesintisiz lacivert doku: ayna katlama
	t = Texture().crop((0, 0, (w + 1) // 2, (h + 1) // 2))
	big = Image.new('RGBA', (t.width * 2, t.height * 2))
	big.paste(t, (0, 0))
	big.paste(t.transpose(Image.FLIP_LEFT_RIGHT), (t.width, 0))
	big.paste(t.transpose(Image.FLIP_TOP_BOTTOM), (0, t.height))
	big.paste(t.transpose(Image.ROTATE_180), (t.width, t.height))
	return big.crop((0, 0, w, h))


def Plate(w, h, r, state = 'up', fill = None):
	# yuvarlak kose yuva/dugme zemini
	big = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	base = {'up': (18, 26, 46), 'over': (30, 42, 70), 'down': (9, 13, 26)}[state]
	if fill:
		base = fill
	d.rounded_rectangle((0, 0, w * SS - 1, h * SS - 1), r * SS, fill = base + (255,), outline = GOLD if state != 'down' else GOLD_DIM, width = max(1, SS))
	d.rounded_rectangle((2 * SS, 2 * SS, w * SS - 2 * SS, h * SS - 2 * SS), max(1, r - 1) * SS, outline = (60, 52, 40, 255), width = SS // 2)
	return big.resize((w, h), Image.LANCZOS)


def Glyph(kind, w, h, col = (255, 226, 160, 255)):
	big = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	cx, cy = w * SS // 2, h * SS // 2
	k = SS
	lw = int(1.6 * SS)
	if kind == 'pin':
		d.ellipse((cx - 5 * k, cy - 8 * k, cx + 5 * k, cy + 2 * k), outline = col, width = lw)
		d.polygon([(cx - 4 * k, cy), (cx + 4 * k, cy), (cx, cy + 8 * k)], outline = col)
		d.ellipse((cx - 2 * k, cy - 5 * k, cx + 2 * k, cy - k), fill = col)
	elif kind == 'trash':
		d.rectangle((cx - 5 * k, cy - 4 * k, cx + 5 * k, cy + 7 * k), outline = col, width = lw)
		d.line((cx - 7 * k, cy - 5 * k, cx + 7 * k, cy - 5 * k), fill = col, width = lw)
		d.rectangle((cx - 2 * k, cy - 8 * k, cx + 2 * k, cy - 5 * k), outline = col, width = lw)
		d.line((cx - 2 * k, cy - 2 * k, cx - 2 * k, cy + 5 * k), fill = col, width = k)
		d.line((cx + 2 * k, cy - 2 * k, cx + 2 * k, cy + 5 * k), fill = col, width = k)
	elif kind == 'safe':
		d.rounded_rectangle((cx - 8 * k, cy - 7 * k, cx + 8 * k, cy + 7 * k), 2 * k, outline = col, width = lw)
		d.ellipse((cx - 4 * k, cy - 4 * k, cx + 4 * k, cy + 4 * k), outline = col, width = lw)
		d.line((cx, cy, cx + 3 * k, cy - 3 * k), fill = col, width = lw)
	elif kind == 'offer':
		d.line((cx - 8 * k, cy - 3 * k, cx + 6 * k, cy - 3 * k), fill = col, width = lw)
		d.polygon([(cx + 8 * k, cy - 3 * k), (cx + 3 * k, cy - 7 * k), (cx + 3 * k, cy + k)], fill = col)
		d.line((cx + 8 * k, cy + 4 * k, cx - 6 * k, cy + 4 * k), fill = col, width = lw)
		d.polygon([(cx - 8 * k, cy + 4 * k), (cx - 3 * k, cy), (cx - 3 * k, cy + 8 * k)], fill = col)
	elif kind == 'gavel':
		d.rounded_rectangle((cx - 7 * k, cy - 8 * k, cx + 3 * k, cy - 2 * k), k, fill = col)
		d.line((cx - k, cy - k, cx + 7 * k, cy + 7 * k), fill = col, width = int(2.4 * SS))
		d.line((cx - 8 * k, cy + 8 * k, cx + 2 * k, cy + 8 * k), fill = col, width = lw)
	elif kind == 'plus':
		d.line((cx - 5 * k, cy, cx + 5 * k, cy), fill = col, width = lw)
		d.line((cx, cy - 5 * k, cx, cy + 5 * k), fill = col, width = lw)
	elif kind == 'clock':
		d.ellipse((cx - 6 * k, cy - 6 * k, cx + 6 * k, cy + 6 * k), outline = col, width = lw)
		d.line((cx, cy, cx, cy - 4 * k), fill = col, width = lw)
		d.line((cx, cy, cx + 3 * k, cy + k), fill = col, width = lw)
	elif kind == 'search':
		d.ellipse((cx - 7 * k, cy - 7 * k, cx + 3 * k, cy + 3 * k), outline = col, width = lw)
		d.line((cx + 2 * k, cy + 2 * k, cx + 8 * k, cy + 8 * k), fill = col, width = int(2.2 * SS))
	elif kind == 'reset':
		d.arc((cx - 7 * k, cy - 7 * k, cx + 7 * k, cy + 7 * k), 40, 330, fill = col, width = lw)
		d.polygon([(cx + 6 * k, cy - 7 * k), (cx + 9 * k, cy - 1 * k), (cx + 2 * k, cy - 2 * k)], fill = col)
	elif kind == 'info':
		d.ellipse((cx - 8 * k, cy - 8 * k, cx + 8 * k, cy + 8 * k), outline = col, width = lw)
		d.line((cx, cy - k, cx, cy + 5 * k), fill = col, width = lw)
		d.ellipse((cx - k, cy - 5 * k, cx + k, cy - 3 * k), fill = col)
	return big.resize((w, h), Image.LANCZOS)


def IconButton(out, folder, w, h, glyph, r = 5):
	for (state, fn) in (('up', 'default'), ('over', 'hover'), ('down', 'down')):
		im = Plate(w, h, r, state)
		g = Glyph(glyph, w, h, (255, 226, 160, 255) if state != 'down' else (190, 150, 90, 255))
		im.alpha_composite(g)
		Save(out, '%s/%s.png' % (folder, fn), im)


def TextButton(out, folder, w, h, r = 5):
	# yazisiz dugme zemini (yazi kodda ustune yazilir)
	for (state, fn) in (('up', 'default'), ('over', 'hover'), ('down', 'down')):
		Save(out, '%s/%s.png' % (folder, fn), Plate(w, h, r, state))


def Board(out):
	# --- orta: kesintisiz lacivert doku
	Save(out, 'common/board/center.png', Tile(116, 99))
	# --- kenarlar
	def Vline(w, h, gold_x):
		im = Image.new('RGBA', (w, h), (0, 0, 0, 0))
		d = ImageDraw.Draw(im)
		d.rectangle((0, 0, w - 1, h - 1), fill = NAVY_DEEP)
		d.rectangle((gold_x, 0, gold_x + 1, h - 1), fill = GOLD)
		d.rectangle((gold_x + 3 if gold_x < w // 2 else gold_x - 3, 0, gold_x + 3 if gold_x < w // 2 else gold_x - 2, h - 1), fill = GOLD_DIM)
		return im
	Save(out, 'common/board/left.png', Vline(7, 35, 1))
	Save(out, 'common/board/right.png', Vline(8, 89, 5))
	b = Image.new('RGBA', (89, 8), (0, 0, 0, 0))
	d = ImageDraw.Draw(b)
	d.rectangle((0, 0, 88, 7), fill = NAVY_DEEP)
	d.rectangle((0, 5, 88, 6), fill = GOLD)
	d.rectangle((0, 2, 88, 2), fill = GOLD_DIM)
	Save(out, 'common/board/bottom.png', b)
	# --- alt koseler (yuvarlak)
	def Corner(w, h, left):
		big = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
		d = ImageDraw.Draw(big)
		R = 14 * SS
		if left:
			d.pieslice((0, (h - 1) * SS - 2 * R, 2 * R, (h - 1) * SS + 2 * R - 2 * R + 2 * R), 90, 180, fill = NAVY_DEEP)
		# basit: dikdortgen + yuvarlak dis kose maskesi
		rect = Image.new('RGBA', big.size, NAVY_DEEP)
		mask = Image.new('L', big.size, 0)
		md = ImageDraw.Draw(mask)
		if left:
			md.rounded_rectangle((0, -4 * R, w * SS + 4 * R, h * SS - 1), R, fill = 255)
		else:
			md.rounded_rectangle((-4 * R, -4 * R, w * SS - 1, h * SS - 1), R, fill = 255)
		out_im = Image.new('RGBA', big.size, (0, 0, 0, 0))
		out_im.paste(rect, (0, 0), mask)
		d2 = ImageDraw.Draw(out_im)
		if left:
			d2.arc((1, -4 * R, 2 * R + 1, h * SS - 1), 90, 180, fill = GOLD, width = int(1.8 * SS))
			d2.line((R, h * SS - 2 * SS, w * SS, h * SS - 2 * SS), fill = GOLD, width = int(1.8 * SS))
			d2.line((SS + SS // 2, 0, SS + SS // 2, h * SS - R), fill = GOLD, width = int(1.8 * SS))
		else:
			d2.arc((w * SS - 2 * R - 1, -4 * R, w * SS - 1, h * SS - 1), 0, 90, fill = GOLD, width = int(1.8 * SS))
			d2.line((0, h * SS - 2 * SS, w * SS - R, h * SS - 2 * SS), fill = GOLD, width = int(1.8 * SS))
			d2.line((w * SS - SS - SS // 2, 0, w * SS - SS - SS // 2, h * SS - R), fill = GOLD, width = int(1.8 * SS))
		return out_im.resize((w, h), Image.LANCZOS)
	Save(out, 'common/board/lb_corner.png', Corner(21, 20, True))
	Save(out, 'common/board/rb_corner.png', Corner(18, 20, False))
	# --- baslik cubugu
	def Title(w, cap):
		big = Image.new('RGBA', (w * SS, 46 * SS), (0, 0, 0, 0))
		d = ImageDraw.Draw(big)
		d.rectangle((0, 0, w * SS, 31 * SS), fill = (20, 30, 52, 255))
		for i in range(31 * SS):
			t = i / float(31 * SS)
			d.line((0, i, w * SS, i), fill = (int(26 - 10 * t), int(38 - 14 * t), int(64 - 22 * t), 255))
		d.line((0, 1 * SS, w * SS, 1 * SS), fill = GOLD, width = int(1.8 * SS))
		d.line((0, 30 * SS, w * SS, 30 * SS), fill = GOLD, width = int(1.4 * SS))
		d.line((0, 27 * SS, w * SS, 27 * SS), fill = GOLD_DIM, width = SS // 2)
		return big
	left = Title(12, True)
	ImageDraw.Draw(left).line((SS + SS // 2, SS, SS + SS // 2, 46 * SS), fill = GOLD, width = int(1.8 * SS))
	right = Title(11, True)
	ImageDraw.Draw(right).line((11 * SS - SS - SS // 2, SS, 11 * SS - SS - SS // 2, 46 * SS), fill = GOLD, width = int(1.8 * SS))
	Save(out, 'common/board/titlebar/left.png', left.resize((12, 46), Image.LANCZOS))
	Save(out, 'common/board/titlebar/right.png', right.resize((11, 46), Image.LANCZOS))
	Save(out, 'common/board/titlebar/center.png', Title(616, False).resize((616, 46), Image.LANCZOS))
	# --- kapat dugmesi (kare X - envanterle ayni)
	import build_akravon_inventory as inv
	for (state, fn) in (('up', 'default'), ('down', 'hover')):
		Save(out, 'common/board/titlebar/close/%s.png' % fn, inv.Close(state))	# 32x32 - parmakla basilabilir


def Slots(out):
	# 33x33 hucre: yuvarlak oyuk (envanterle ayni dil)
	import build_akravon_inventory as inv
	base = inv.Slot(33, 33)
	Save(out, 'common/slot/default.png', base)
	Save(out, 'common/slot/cell56.png', inv.Slot(56, 56))
	Save(out, 'business/slot_box.png', Plate(348, 348, 8, 'down', fill = (7, 10, 20)))
	Save(out, 'common/slot/hover.png', ImageEnhance.Brightness(base).enhance(1.55))
	# genis alan kutulari
	Save(out, 'safebox/slot_box.png', Plate(348, 348, 8, 'down', fill = (7, 10, 20)))
	Save(out, 'list_box.png', Plate(459, 474, 8, 'down', fill = (9, 13, 26)))
	Save(out, 'search_shop/result_box.png', Plate(378, 494, 8, 'down', fill = (9, 13, 26)))
	# sure kutusu / alan kutulari
	Save(out, 'business/duration_box.png', Plate(138, 40, 7, 'down', fill = (9, 13, 26)))
	Save(out, 'safebox/money_box.png', Plate(110, 25, 6, 'down', fill = (9, 13, 26)))
	Save(out, 'search_shop/input_box.png', Plate(139, 22, 5, 'down', fill = (9, 13, 26)))
	Save(out, 'search_shop/input_name.png', Plate(165, 26, 6, 'down', fill = (9, 13, 26)))
	Save(out, 'search_shop/attribute_value_box.png', Plate(55, 22, 5, 'down', fill = (9, 13, 26)))
	Save(out, 'search_shop/filter_box.png', Plate(208, 63, 7, 'down', fill = (9, 13, 26)))
	Save(out, 'shopbuilder/field_name.png', Plate(348, 40, 8, 'down', fill = (9, 13, 26)))
	for n in ('result_item_box', 'result_item_auction_box'):
		Save(out, 'search_shop/%s.png' % n, Plate(339, 111, 8, 'up', fill = (14, 20, 36)))
	for n in ('incoming_offer', 'outcoming_offer'):
		Save(out, 'offer_list/%s.png' % n, Plate(339, 111, 8, 'up', fill = (14, 20, 36)))
	Save(out, 'auction/info_box.png', Plate(502, 205, 8, 'down', fill = (9, 13, 26)))
	Save(out, 'auction/offer_box.png', Plate(366, 25, 6, 'down', fill = (9, 13, 26)))
	Save(out, 'history_search/box.png', Plate(438, 25, 6, 'down', fill = (9, 13, 26)))


def Banner(out):
	src = Image.open(os.path.join(HERE, 'src', 'pazar_serit.png')).convert('RGB')
	w, h = 348, 67
	# genislik tam, dikey olarak tezgah bolgesi (ust ve alt kirpilir)
	ratio = w / float(h)
	ch = int(src.width / ratio)
	top = int((src.height - ch) * 0.55)
	crop = src.crop((0, top, src.width, top + ch)).resize((w * 3, h * 3), Image.LANCZOS).resize((w, h), Image.LANCZOS)
	Save(out, 'business/bg_img.png', crop.convert('RGBA'))


def Buttons(out):
	IconButton(out, 'business/buttons/move_shop', 40, 24, 'pin')
	IconButton(out, 'business/buttons/cancel_shop', 40, 24, 'trash')
	IconButton(out, 'business/buttons/safebox', 45, 30, 'safe')
	IconButton(out, 'business/buttons/offer', 45, 30, 'offer')
	IconButton(out, 'business/buttons/auction', 45, 30, 'gavel')
	IconButton(out, 'business/buttons/duration_up', 20, 20, 'plus', 4)
	for (n, w, h) in (('claim_items', 33, 24), ('claim_money', 33, 24)):
		TextButton(out, 'safebox/buttons/' + n, w, h)
	TextButton(out, 'safebox/buttons/select_page', 40, 24)
	IconButton(out, 'search_shop/mini_search_button', 24, 24, 'search', 5)
	IconButton(out, 'search_shop/reset_filter_button', 24, 24, 'reset', 5)
	IconButton(out, 'history_search/info_button', 24, 24, 'info', 5)
	for (n, w, h) in (('buy_button', 66, 19), ('offer_button', 66, 19)):
		TextButton(out, 'search_shop/' + n, w, h, 4)
	for (n, w, h) in (('attribute_select_button', 137, 21), ('combo_box_button', 117, 21)):
		TextButton(out, 'search_shop/' + n, w, h, 4)
	TextButton(out, 'search_shop/auction_list_button', 100, 30)
	TextButton(out, 'search_shop/history_button', 100, 30)
	TextButton(out, 'auction/cancel_button', 105, 24, 5)
	TextButton(out, 'auction/raise_button', 105, 24, 5)
	from PIL import ImageFont
	font = ImageFont.truetype(os.path.join(HERE, '..', '..', 'data', 'fonts', 'marcellus.ttf'), 18)
	labels = {'depo' : 'DEPO', 'kapat' : 'KAPAT', 'pazar' : 'SÜRE +'}
	for n in ('depo', 'kapat', 'pazar'):
		for (st, fn) in (('up', 'n'), ('over', 'h'), ('down', 'd')):
			im = Plate(84, 40, 7, st)
			d = ImageDraw.Draw(im)
			tw = d.textlength(labels[n], font = font)
			d.text(((84 - tw) / 2, 9), labels[n], font = font, fill = (250, 236, 210, 255) if st != 'down' else (190, 170, 130, 255))
			Save(out, 'shopbuilder/%s_%s.png' % (n, fn), im)
	for (n, w, h) in (('attribute_element', 186, 14), ('combo_box_element', 121, 14)):
		for (s, fn) in (('up', 'default'), ('over', 'hover')):
			Save(out, 'search_shop/%s/%s.png' % (n, fn), Plate(w, h, 3, s))


def Dialog(out):
	# eski "ne kadar yang'a satmak istiyorsun" penceresi (uicommon.MoneyInputDialogNew) - ayni boyutlar
	folder = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy')
	os.makedirs(folder, exist_ok = True)
	cw.Board(242, 387).save(os.path.join(folder, 'ikm_board.png'))
	Plate(107, 110, 8, 'down', fill = (9, 13, 26)).save(os.path.join(folder, 'ikm_item.png'))
	Plate(187, 26, 6, 'down', fill = (9, 13, 26)).save(os.path.join(folder, 'ikm_price.png'))
	for (i, st) in ((1, 'up'), (2, 'over'), (3, 'down')):
		Plate(89, 23, 5, st).save(os.path.join(folder, 'ikm_btn_%d.png' % i))


def Public(out):
	# eski pencerelerin ortak dugme ve slot gorselleri (public/*.sub) - bizim cerceve. .sub dosyasi png'yi gosterir.
	import build_akravon_inventory as inv
	legacy = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy')
	folder = os.path.join(out, 'ymir work', 'ui', 'public')
	os.makedirs(legacy, exist_ok = True)
	os.makedirs(folder, exist_ok = True)
	NL = chr(13) + chr(10)

	def Sub(name, png, w, h):
		text = 'title subImage' + NL + 'version 1.0' + NL + 'image "akravon_legacy/%s"' % png + NL + 'left 0' + NL + 'top 0' + NL + 'right %d' % w + NL + 'bottom %d' % h + NL
		open(os.path.join(folder, name), 'wb').write(text.encode('latin1'))

	inv.Slot(32, 32).save(os.path.join(legacy, 'public_slot_base.png'))
	Sub('slot_base.sub', 'public_slot_base.png', 32, 32)
	for (prefix, w, h) in (('small_button', 43, 21), ('middle_button', 61, 21), ('large_button', 88, 21), ('xlarge_button', 180, 25)):
		for (i, st) in ((1, 'up'), (2, 'over'), (3, 'down')):
			png = 'public_%s_%d.png' % (prefix, i)
			Plate(w, h, 4, st).save(os.path.join(legacy, png))
			Sub('%s_%02d.sub' % (prefix, i), png, w, h)


def CutButton(w, h, state):
	# Depo Sifresi penceresindeki dugme dili: koyu lacivert, ince cift altin cerceve, kesik koseler
	import build_akravon_safebox as sbx
	im = sbx.Button(w, h, 'up' if state in ('up', 'sel') else state)
	if state == 'sel':
		tint = Image.new('RGBA', im.size, (120, 90, 30, 255))
		im = Image.blend(im.convert('RGBA'), tint, 0.38)
		alpha = Image.open(os.path.join(HERE, 'src', 'dummy.png')).convert('L') if False else None
	return im


def CloseSquare(state):
	# kare kapat (envanterle ayni): 44x44, altin cerceve, X
	N = 44
	big = Image.new('RGBA', (N * SS, N * SS), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	fill = (150, 112, 56, 255) if state == 'down' else (8, 11, 22, 255)
	d.rounded_rectangle((0, 0, N * SS - 1, N * SS - 1), 8 * SS, fill = fill, outline = GOLD, width = int(1.5 * SS))
	col = (24, 16, 6, 255) if state == 'down' else GOLD
	a, b = 13 * SS, 31 * SS
	d.line((a, a, b, b), fill = col, width = 4 * SS)
	d.line((a, b, b, a), fill = col, width = 4 * SS)
	return big.resize((N, N), Image.LANCZOS)


def Pazar(out):
	# kendi pazar pencerelerimiz (uiakravonpazar.py) icin gorseller
	import build_akravon_inventory as inv
	folder = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy')
	os.makedirs(folder, exist_ok = True)
	for h in (556, 556):
		cw.Board(372, h).save(os.path.join(folder, 'pz_board_%d.png' % h))
	inv.Close('up').resize((40, 40), Image.LANCZOS).save(os.path.join(folder, 'pz_close_1.png'))
	inv.Close('down').resize((40, 40), Image.LANCZOS).save(os.path.join(folder, 'pz_close_3.png'))
	for (w, h) in ((348, 52), (168, 46)):
		inv.Slot(w, h).save(os.path.join(folder, 'pz_field_%d.png' % w))
	# sayfa/sekme seridi: envanterdeki sekmelerle ayni dil (cukur serit + secili kapsul)
	for n in (5,):
		inv.TabStrip(348, 46, n).save(os.path.join(folder, 'pz_tabs_strip.png'))
		inv.TabGlow(348 // n, 46).save(os.path.join(folder, 'pz_tabs_on.png'))
	Plate(348, 348, 8, 'down', fill = (7, 10, 20)).save(os.path.join(folder, 'pz_gridbox.png'))
	Plate(130, 130, 9, 'down', fill = (9, 13, 26)).save(os.path.join(folder, 'pz_itemplate.png'))
	Plate(320, 484, 6).save(os.path.join(folder, 'pz_dialog_unused.png'))
	cw.Board(560, 384).save(os.path.join(folder, 'pz_dialog.png'))
	inv.Slot(360, 56).save(os.path.join(folder, 'pz_field_wide.png'))
	Plate(360, 96, 8, 'down', fill = (9, 13, 26)).save(os.path.join(folder, 'pz_info.png'))
	inv.Slot(280, 56).save(os.path.join(folder, 'pz_field.png'))
	inv.Slot(200, 44).save(os.path.join(folder, 'pz_money.png'))
	for (w, h) in ((168, 52), (64, 46), (74, 46), (88, 46), (280, 48), (134, 56), (111, 44), (360, 44), (248, 52)):
		CutButton(w, h, 'up').save(os.path.join(folder, 'pz_btn%d_1.png' % w))
		CutButton(w, h, 'sel').save(os.path.join(folder, 'pz_btn%d_2.png' % w))
		CutButton(w, h, 'down').save(os.path.join(folder, 'pz_btn%d_3.png' % w))


def Popups(out):
	# onay / mesaj kutulari: uiscript dosyalarini bizim yerlesimle degistirir (ayni oge adlari)
	folder = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy')
	os.makedirs(folder, exist_ok = True)
	os.makedirs(os.path.join(out, 'uiscript'), exist_ok = True)
	cw.Board(460, 226).save(os.path.join(folder, 'pp_board_q.png'))
	cw.Board(460, 206).save(os.path.join(folder, 'pp_board_p.png'))
	for (i, st) in ((1, 'up'), (2, 'sel'), (3, 'down')):
		CutButton(190, 52, st).save(os.path.join(folder, 'pp_btn_%d.png' % i))

	def Script(name, win, height, board, msgs, buttons):
		body = ''
		for m in msgs:
			body += '\t\t\t\t{ "name" : "%s", "type" : "text", "x" : 230, "y" : 30, "text" : "", "text_horizontal_align" : "center" },\n' % m
		for (n, x) in buttons:
			body += ('\t\t\t\t{ "name" : "%s", "type" : "button", "x" : %d, "y" : %d, "text" : "",\n'
				'\t\t\t\t\t"default_image" : IMG + "pp_btn_1.png", "over_image" : IMG + "pp_btn_1.png", "down_image" : IMG + "pp_btn_3.png" },\n' % (n, x, height - 76))
		text = ('# -*- coding: utf-8 -*-\n# AKRAVON: uretildi (skin/build_akravon_ikashop.py)\nimport uiScriptLocale\nIMG = "d:/ymir work/ui/akravon_legacy/"\n'
			'window = {\n\t"name" : "%s",\n\t"style" : ("float",),\n\t"x" : SCREEN_WIDTH / 2 - 230, "y" : SCREEN_HEIGHT / 2 - %d,\n\t"width" : 460, "height" : %d,\n\t"children" :\n\t(\n'
			'\t\t{\n\t\t\t"name" : "board", "type" : "expanded_image", "x" : 0, "y" : 0, "image" : IMG + "%s",\n\t\t\t"children" :\n\t\t\t(\n%s\t\t\t),\n\t\t},\n\t),\n}\n') % (win, height // 2, height, board, body)
		open(os.path.join(out, 'uiscript', name), 'w', encoding = 'utf-8', newline = '\n').write(text)
	Script('questiondialog.py', 'QuestionDialog', 226, 'pp_board_q.png', ['message'], [('accept', 25), ('cancel', 245)])
	Script('questiondialog2.py', 'QuestionDialog2', 226, 'pp_board_q.png', ['message1', 'message2'], [('accept', 25), ('cancel', 245)])
	Script('questiondialog3.py', 'QuestionDialog3', 226, 'pp_board_q.png', ['message1', 'message2', 'message3'], [('accept', 25), ('cancel', 245)])
	Script('popupdialog.py', 'PopupDialog', 206, 'pp_board_p.png', ['message'], [('accept', 135)])
	Script('popupdialog2.py', 'PopupDialog2', 206, 'pp_board_p.png', ['message1', 'message2'], [('accept', 135)])


def Build(out):
	Popups(out)
	Pazar(out)
	Public(out)
	Dialog(out)
	Board(out)
	Slots(out)
	Buttons(out)
	Banner(out)
	print('pazar: gorseller hazir')


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
