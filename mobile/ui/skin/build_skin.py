# Cynex: mobil arayuz gorselleri - YeniKlasor4 mobile_ui setinden turetilir (dugme, disk, cerceve, glif)
# kullanim: py -3 build_skin.py <cikti_klasoru>   -> <cikti>/mobile_ui/cynex/*.png + <cikti>/uimobileassets.py
import os
import sys
from PIL import Image, ImageDraw, ImageFont, ImageFilter

YK = 'C:/CynexMobilBuild/mobile_ui_kaynak/images/mobile_ui'
B2ICON = 'C:/CynexMobilBuild/b2pack/icon/icon/face'
FONTS = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'data', 'fonts')
OUT = sys.argv[1] if len(sys.argv) > 1 else 'C:/CynexMobilBuild/mobile_ui_data'
DST = os.path.join(OUT, 'mobile_ui', 'cynex')

GOLD = (240, 207, 135, 255)
GOLD_XL = (246, 223, 166, 255)
GOLD_DIM = (201, 173, 122, 255)
RIM = (133, 103, 50, 255)
TXT = (243, 230, 204, 255)
INK = (42, 22, 8, 255)

SIZES = {}

# Cynex: Panel sinifi kose resmi + duz cubuklar cizer: (yaricap, kenar, dolgu, kenar rengi)
PANEL_STYLES = {
	"panel": (12, 1, (21, 20, 17, 225), (133, 103, 50, 255)),
	"card": (10, 1, (17, 12, 12, 215), (110, 86, 44, 255)),
	"card_active": (10, 2, (42, 30, 16, 235), (231, 189, 106, 255)),
	"card_empty": (10, 1, (17, 12, 12, 130), (110, 86, 44, 150)),
	"field": (8, 1, (0, 0, 0, 150), (126, 97, 46, 255)),
	"field_focus": (8, 2, (0, 0, 0, 170), (231, 189, 106, 255)),
	"chip_active": (8, 2, (42, 30, 16, 240), (217, 169, 78, 255)),
	"pill": (20, 1, (15, 12, 10, 224), (126, 97, 46, 255)),
	"segment": (8, 1, (0, 0, 0, 150), (110, 86, 44, 255)),
	"segment_active": (6, 1, (90, 66, 30, 255), (231, 189, 106, 255)),
	"tag": (12, 1, (0, 0, 0, 0), (126, 97, 46, 255)),
	"race_warrior": (10, 2, (30, 21, 17, 240), (205, 96, 64, 255)),
	"race_assassin": (10, 2, (30, 21, 17, 240), (104, 182, 140, 255)),
	"race_sura": (10, 2, (30, 21, 17, 240), (156, 116, 206, 255)),
	"race_shaman": (10, 2, (30, 21, 17, 240), (231, 189, 106, 255)),
	"empire": (14, 1, (21, 20, 17, 225), (126, 97, 46, 255)),
	"empire_a": (14, 2, (30, 21, 17, 240), (196, 70, 56, 255)),
	"empire_b": (14, 2, (30, 21, 17, 240), (222, 180, 66, 255)),
	"empire_c": (14, 2, (30, 21, 17, 240), (80, 132, 214, 255)),
}


def Y(name):
	return Image.open(os.path.join(YK, name)).convert('RGBA')


def Save(name, im):
	im.save(os.path.join(DST, name + '.png'))
	SIZES[name] = im.size


def Fit(im, w, h):
	return im.resize((int(w), int(h)), Image.LANCZOS)


def Visible(im, threshold=40):
	box = im.getchannel('A').point(lambda a: 255 if a > threshold else 0).getbbox()
	return im.crop(box) if box else im


def Tint(im, color):
	r, g, b, a = color
	out = Image.new('RGBA', im.size, (r, g, b, 0))
	alpha = im.getchannel('A').point(lambda v: v * a // 255)
	out.putalpha(alpha)
	return out


def Over(base, top, x=None, y=None):
	base = base.copy()
	if x is None:
		x = (base.size[0] - top.size[0]) // 2
	if y is None:
		y = (base.size[1] - top.size[1]) // 2
	base.alpha_composite(top, (int(x), int(y)))
	return base


def Glyph(name, size, color=TXT):
	# Cynex: glif kenar boslugu atilir, kare icine oranla sigdirilir
	src = Visible(Y('glyph_%s_96.png' % name), 8)
	scale = float(size) / max(src.size)
	icon = Fit(src, max(1, round(src.size[0] * scale)), max(1, round(src.size[1] * scale)))
	return Tint(Over(Image.new('RGBA', (size, size), (0, 0, 0, 0)), icon), color)


def Font(name, size):
	files = {
		"disp": "marcellus.ttf",
		"body": "alegreyasans.ttf",
		"bold": "alegreyasansbold.ttf",
	}
	return ImageFont.truetype(os.path.join(FONTS, files[name]), size)


def Text(text, font, size, color, shadow=True, pad=2):
	f = Font(font, size)
	box = f.getbbox(text)
	w, h = box[2] - box[0] + pad * 2 + 1, box[3] - box[1] + pad * 2 + 1
	im = Image.new('RGBA', (w, h), (0, 0, 0, 0))
	d = ImageDraw.Draw(im)
	if shadow:
		d.text((pad - box[0] + 1, pad - box[1] + 1), text, font=f, fill=(0, 0, 0, 170))
	d.text((pad - box[0], pad - box[1]), text, font=f, fill=color)
	return im


def NineStretch(im, w, h, c):
	# Cynex: 9 parcali buyutme (koseler sabit)
	sw, sh = im.size
	out = Image.new('RGBA', (w, h), (0, 0, 0, 0))
	xs = [(0, c, 0, c), (c, sw - c, c, w - c), (sw - c, sw, w - c, w)]
	ys = [(0, c, 0, c), (c, sh - c, c, h - c), (sh - c, sh, h - c, h)]
	for (sx0, sx1, dx0, dx1) in xs:
		for (sy0, sy1, dy0, dy1) in ys:
			if sx1 <= sx0 or sy1 <= sy0 or dx1 <= dx0 or dy1 <= dy0:
				continue
			out.alpha_composite(Fit(im.crop((sx0, sy0, sx1, sy1)), dx1 - dx0, dy1 - dy0), (dx0, dy0))
	return out


def Slice9(prefix, im, c):
	# Cynex: NineSlicePanel parcalari (orta parcalar 2 piksel)
	w, h = im.size
	mx, my = w // 2, h // 2
	Save(prefix + '_tl', im.crop((0, 0, c, c)))
	Save(prefix + '_tr', im.crop((w - c, 0, w, c)))
	Save(prefix + '_bl', im.crop((0, h - c, c, h)))
	Save(prefix + '_br', im.crop((w - c, h - c, w, h)))
	Save(prefix + '_t', im.crop((mx - 1, 0, mx + 1, c)))
	Save(prefix + '_b', im.crop((mx - 1, h - c, mx + 1, h)))
	Save(prefix + '_l', im.crop((0, my - 1, c, my + 1)))
	Save(prefix + '_r', im.crop((w - c, my - 1, w, my + 1)))
	Save(prefix + '_c', im.crop((mx - 1, my, mx + 1, my + 1)))


def Slice3(prefix, im, cap):
	w, h = im.size
	Save(prefix + '_l', im.crop((0, 0, cap, h)))
	Save(prefix + '_m', im.crop((w // 2 - 1, 0, w // 2 + 1, h)))
	Save(prefix + '_r', im.crop((w - cap, 0, w, h)))


def RoundedCorner(radius, border, fill, line, corner):
	# Cynex: Panel kosesi - dolgu + kenar, 4x ornekleme ile yumusak
	s = 4
	R = radius * s
	big = Image.new('RGBA', (R * 2, R * 2), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	d.ellipse((0, 0, R * 2 - 1, R * 2 - 1), fill=line)
	b = border * s
	d.ellipse((b, b, R * 2 - 1 - b, R * 2 - 1 - b), fill=fill if fill[3] else (0, 0, 0, 0))
	if not fill[3]:
		# Cynex: bos dolgu - ic daireyi sil
		mask = Image.new('L', big.size, 0)
		ImageDraw.Draw(mask).ellipse((b, b, R * 2 - 1 - b, R * 2 - 1 - b), fill=255)
		big.paste((0, 0, 0, 0), (0, 0), mask)
	box = {"tl": (0, 0, R, R), "tr": (R, 0, R * 2, R), "bl": (0, R, R, R * 2), "br": (R, R, R * 2, R * 2)}[corner]
	return big.crop(box).resize((radius, radius), Image.LANCZOS)


def Ring(size, width, color):
	s = 4
	big = Image.new('RGBA', (size * s, size * s), (0, 0, 0, 0))
	d = ImageDraw.Draw(big)
	d.ellipse((0, 0, size * s - 1, size * s - 1), outline=color, width=width * s)
	return big.resize((size, size), Image.LANCZOS)


def CircleMask(im, size):
	im = Fit(im, size, size)
	mask = Image.new('L', (size * 4, size * 4), 0)
	ImageDraw.Draw(mask).ellipse((0, 0, size * 4 - 1, size * 4 - 1), fill=255)
	mask = mask.resize((size, size), Image.LANCZOS)
	out = Image.new('RGBA', (size, size), (0, 0, 0, 0))
	out.paste(im, (0, 0), mask)
	return out


def Bar(w, h, top, bottom, radius=None):
	# Cynex: dikey gecisli dolgu cubugu
	im = Image.new('RGBA', (w, h), (0, 0, 0, 0))
	grad = Image.new('RGBA', (1, h))
	for y in range(h):
		t = y / float(max(1, h - 1))
		grad.putpixel((0, y), tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(4)))
	grad = grad.resize((w, h))
	mask = Image.new('L', (w * 4, h * 4), 0)
	r = (radius if radius is not None else h // 2) * 4
	ImageDraw.Draw(mask).rounded_rectangle((0, 0, w * 4 - 1, h * 4 - 1), radius=r, fill=255)
	im.paste(grad, (0, 0), mask.resize((w, h), Image.LANCZOS))
	return im


def Trough(w, h):
	frame = Visible(Y('panes/gauge_frame_84x7.png'), 1)
	inner = Bar(w, h, (8, 6, 5, 230), (20, 16, 12, 230))
	edge = NineStretch(frame, w, h, 3)
	return Over(inner, edge, 0, 0)


# ---------------------------------------------------------------- hud
def BuildHud():
	Save('hud_joystick', Fit(Y('hud_joystick_base_260x260_normal.png'), 200, 200))
	Save('hud_knob', Fit(Y('hud_joystick_knob_110x110_normal.png'), 80, 80))
	for (state, src) in (("hud_attack", "normal"), ("hud_attack_pressed", "down")):
		disc = Fit(Y('hud_attack_230x230_%s.png' % src), 160, 160)
		Save(state, Over(disc, Glyph('swords_dark', 78, INK), None, 40))
	Save('hud_disc', Fit(Y('hud_skill_106x106_normal.png'), 64, 64))
	Save('hud_ring', Ring(64, 2, GOLD_DIM))
	Save('hud_key_plate', NineStretch(Visible(Y('panel_default_40x24.png'), 20), 26, 16, 5))
	for page in range(1, 5):
		disc = Fit(Y('hud_utility_84x84_normal.png'), 44, 44)
		Save('hud_page_%d' % page, Over(disc, Text(("I", "II", "III", "IV")[page - 1], "disp", 17, GOLD)))
	Save('hud_lock_disc', Fit(Y('hud_utility_84x84_normal.png'), 64, 64))
	Save('hud_lock_open', Glyph('lock', 22, (255, 255, 255, 170)))
	Save('hud_lock_closed', Glyph('lock', 22))

	glyphs = {
		"character": "character", "inventory": "bag", "messenger": "whisper", "itemshop": "shop",
		"system": "settings", "pickup": "pickup", "autohunt": "autohunt", "mount": "mount",
	}
	for size in (54, 60, 68):
		frames = {}
		for (state, src) in (("normal", "normal"), ("down", "down"), ("active", "over")):
			frames[state] = Fit(Visible(Y('button_icon_small_lg_48x52_%s.png' % src)), size, size)
		for (name, glyph) in glyphs.items():
			for state in ("normal", "down", "active"):
				color = GOLD_XL if state == "active" else TXT
				icon = Glyph(glyph, int(size * 0.36), color)
				Save('hudbtn_%s_%d_%s' % (name, size, state), Over(frames[state], icon, None, int(size * 0.17)))
		Save('hudbtn_alert_%d' % size, Ring(size, 3, (236, 120, 60, 255)))


# ---------------------------------------------------------------- durum cubugu
def BuildStatus():
	panel = NineStretch(Visible(Y('panel_raised_320x200.png'), 20), 332, 82, 14)
	disc = Fit(Y('hud_utility_84x84_normal.png'), 70, 70)
	Save('status_frame', Over(panel, disc, 2, 5))
	for (name, w, h, top, bottom) in (
			("gauge_hp", 220, 12, (226, 76, 60, 255), (140, 30, 24, 255)),
			("gauge_sp", 220, 10, (90, 150, 230, 255), (36, 70, 150, 255)),
			("gauge_st", 220, 8, (240, 207, 135, 255), (170, 128, 52, 255))):
		Save(name + '_trough', Trough(w, h))
		Save(name + '_fill', Bar(w - 2, h - 2, top, bottom))
	Save('bar_exp_track', Bar(220, 3, (30, 24, 18, 220), (30, 24, 18, 220), 1))
	Save('bar_exp_fill', Bar(220, 3, GOLD_XL, (190, 150, 70, 255), 1))
	for (race, src) in (("warrior_m", "warrior_m"), ("warrior_w", "warrior_w"), ("assassin_m", "assassin_m"),
			("assassin_w", "assassin_w"), ("sura_m", "sura_m"), ("sura_w", "sura_w"), ("shaman_m", "shaman_m"),
			("shaman_w", "shaman_w"), ("wolfman_m", "wolfman_m")):
		path = os.path.join(B2ICON, src + '.tga')
		if not os.path.exists(path):
			continue
		face = Image.open(path).convert('RGBA')
		Save('status_face_' + race, CircleMask(face, 52))
		Save('portrait_' + race, Over(Ring(52, 2, GOLD), CircleMask(face, 48)))


# ---------------------------------------------------------------- sohbet
def BuildChat():
	pill = Visible(Y('button_row_lg_176x52_normal.png'))
	Slice9('chat_bar', NineStretch(pill, 120, 60, 12), 20)
	Save('chat_icon', Glyph('chat', 24, GOLD))
	for (state, color) in (("normal", GOLD_DIM), ("active", GOLD_XL)):
		Save('chat_title_' + state, Text("SOHBET", "disp", 11, color, False, 0))
	line = Fit(Y('panes/divider_gold.png'), 28, 2)
	for side in ("left", "right"):
		Save('chat_ornament_' + side, Over(Image.new('RGBA', (28, 10), (0, 0, 0, 0)), line))
	Save('icon_chevron_up', Y('panes/chevron_gold_up.png'))
	Save('icon_chevron_down', Y('panes/chevron_gold_down.png'))
	Save('icon_chevron', Glyph('chevron', 14, GOLD))
	Slice9('chat_badge', NineStretch(Visible(Y('panes/btn_menu_danger_default.png'), 20), 40, 30, 6), 9)
	Slice9('chat_panel', NineStretch(Visible(Y('panel_default_320x200.png'), 20), 80, 80, 14), 14)
	Slice9('chat_tab', NineStretch(Visible(Y('button_row_88x26_over.png')), 30, 30, 6), 7)
	for (state, src) in (("normal", "normal"), ("over", "over"), ("down", "down")):
		btn = Fit(Visible(Y('button_primary_266x46_%s.png' % src)), 80, 40)
		Save('chat_send_' + state, btn)


# ---------------------------------------------------------------- pencere, alan, dugme
def BuildFrames():
	board = {}
	for (key, name) in (("tl", "board_corner_lefttop"), ("tr", "board_corner_righttop"), ("bl", "board_corner_leftbottom"),
			("br", "board_corner_rightbottom"), ("t", "board_line_top"), ("b", "board_line_bottom"),
			("l", "board_line_left"), ("r", "board_line_right"), ("c", "board_base")):
		board[key] = Y('chrome/%s.png' % name)
	for prefix in ("window", "ornate"):
		for key in ("tl", "tr", "bl", "br"):
			Save('%s_%s' % (prefix, key), board[key])
		Save(prefix + '_t', board["t"].crop((63, 0, 65, 32)))
		Save(prefix + '_b', board["b"].crop((63, 0, 65, 32)))
		Save(prefix + '_l', board["l"].crop((0, 63, 32, 65)))
		Save(prefix + '_r', board["r"].crop((0, 63, 32, 65)))
		Save(prefix + '_c', board["c"].crop((63, 63, 65, 64)))
	Save('ornate_fill', board["c"])

	Slice9('slot', NineStretch(Visible(Y('editline_266x36_normal.png'), 20), 40, 40, 8), 8)
	Slice9('slot_focus', NineStretch(Visible(Y('editline_266x36_over.png'), 20), 40, 40, 8), 8)
	Slice9('slot_dim', NineStretch(Visible(Y('editline_266x36_disabled.png'), 20), 40, 40, 8), 8)

	for state in ("normal", "over", "down"):
		src = Visible(Y('button_secondary_104x39_%s.png' % state))
		prefix = "secondary" if state == "normal" else "secondary_" + state
		Slice3(prefix, src, 12)

	for (key, w, h) in (("primary_392x58", 392, 58), ("primary_300x64", 300, 64)):
		for state in ("normal", "over", "down"):
			Save('%s_%s' % (key, state), Fit(Visible(Y('button_primary_lg_520x92_%s.png' % state)), w, h))

	for style, (radius, border, fill, line) in PANEL_STYLES.items():
		for corner in ("tl", "tr", "bl", "br"):
			Save('%s_%s' % (style, corner), RoundedCorner(radius, border, fill, line, corner))

	# Cynex: anahtar - koyu yuva, kapali koyu dugme solda, acik altin dugme sagda
	track = Bar(44, 22, (12, 9, 7, 235), (24, 18, 14, 235))
	track = Over(track, NineStretch(Visible(Y('panes/gauge_frame_84x7.png'), 1), 44, 22, 3), 0, 0)
	offKnob = Fit(Y('hud_utility_84x84_normal.png'), 26, 26)
	onKnob = Fit(Y('hud_attack_230x230_normal.png'), 26, 26)
	base = Image.new('RGBA', (48, 26), (0, 0, 0, 0))
	Save('toggle_off', Over(Over(base, track, 2, 2), offKnob, 0, 0))
	Save('toggle_on', Over(Over(base, Tint(track, (217, 169, 78, 120)), 2, 2), onKnob, 22, 0))
	for (name, src) in (("dot_normal", "pip_green"), ("dot_busy", "pip_yellow"), ("dot_full", "pip_red"), ("dot_closed", "pip_empty")):
		Save(name, Fit(Y('login/s100/%s.png' % src), 7, 7))
	Save('avatar_gold', Fit(Y('hud_icon_small_56x56_normal.png'), 44, 44))
	Save('avatar_empty', Fit(Y('hud_icon_small_56x56_disabled.png'), 44, 44))
	Save('icon_plus', Glyph('plus', 20, GOLD))
	Save('icon_close', Glyph('cancel', 18, GOLD))
	Save('icon_trash', Glyph('cancel', 18, (226, 96, 80, 255)))
	Save('icon_back', Glyph('chevron', 18, GOLD).transpose(Image.FLIP_LEFT_RIGHT))
	Save('menu_icon_grid', Glyph('menu', 20, GOLD))
	for size in (28, 30):
		for (state, src) in (("normal", "normal"), ("over", "over"), ("down", "down")):
			base = Fit(Visible(Y('button_icon_32x32_%s.png' % src), 10), size, size)
			Save('close_%d_%s' % (size, state), Over(base, Glyph('cancel', size // 2, GOLD)))


# ---------------------------------------------------------------- giris / secim / olusturma yazilari
def BuildIntro():
	for (name, text) in (("title_select", u"Karakter Se\u00e7"), ("title_create", u"Yeni Karakter"), ("title_empire", u"\u0130mparatorluk")):
		Save(name, Text(text, "disp", 30, GOLD_XL))
	for (name, text) in (("label_login", u"G\u0130R\u0130\u015e"), ("label_start", u"OYUNA G\u0130R"), ("label_create", u"OLU\u015eTUR"),
			("label_select_empire", u"BU \u0130MPARATORLU\u011eU SE\u00c7")):
		Save(name, Text(text, "bold", 22, INK, False))
	races = (("warrior", u"Sava\u015f\u00e7\u0131", "swords"), ("assassin", u"Ninja", "sword"),
		("sura", u"Sura", "boss"), ("shaman", u"\u015eaman", "potion"))
	for (race, name, glyph) in races:
		Save('race_' + race, Text(name, "disp", 18, GOLD))
		Save('race_%s_large' % race, Text(name, "disp", 28, GOLD_XL))
		Save('class_' + race, Glyph(glyph, 40, GOLD))
		Save('icon_race_%s_28' % race, Glyph(glyph, 24, GOLD))
		Save('icon_race_%s_34' % race, Glyph(glyph, 30, GOLD))
		disc = Fit(Y('hud_utility_84x84_normal.png'), 52, 52)
		Save('emblem_' + race, Over(disc, Glyph(glyph, 28, GOLD)))
	Save('emblem_empty', Fit(Y('hud_utility_84x84_disabled.png'), 52, 52))
	for (key, text) in (("a", "SHINSOO"), ("b", "CHUNJO"), ("c", "JINNO")):
		Save('empire_' + key, Text(text, "disp", 26, GOLD_XL))


def WriteAssets():
	lines = ["# Cynex: build_skin.py uretir - elle duzenleme", "ASSET_SIZES = {"]
	for name in sorted(SIZES):
		lines.append('\t"%s" : (%d, %d),' % (name, SIZES[name][0], SIZES[name][1]))
	lines.append("}")
	lines.append("")
	lines.append("PANEL_STYLES = {")
	for name in sorted(PANEL_STYLES):
		lines.append('\t"%s" : %r,' % (name, PANEL_STYLES[name]))
	lines.append("}")
	lines.append("")
	open(os.path.join(OUT, 'uimobileassets.py'), 'wb').write(('\r\n'.join(lines)).encode('ascii'))


def main():
	os.makedirs(DST, exist_ok=True)
	for name in os.listdir(DST):
		os.remove(os.path.join(DST, name))
	BuildHud()
	BuildStatus()
	BuildChat()
	BuildFrames()
	BuildIntro()
	WriteAssets()
	print('gorsel %d' % len(SIZES))


if __name__ == '__main__':
	main()
