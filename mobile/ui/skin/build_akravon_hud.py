# AKRAVON: oyun ici HUD gorselleri - uretilmis kaynaklardan (yesil zemin) 2 kat cozunurlukte
# kaynak: skin/hud_parts_source.png (dugme/disk/plaka), skin/hud_icons_source.png (menu simgeleri)
# kullanim: py -3 build_akravon_hud.py <cikti>  ->  <cikti>/mobile_ui/akravon/hud/*.png
import math
import os
import sys
import numpy as np
from PIL import Image, ImageDraw, ImageEnhance, ImageFilter

from build_akravon_circle import BuildCircle
from build_akravon_kit import BuildKit, CutRect, FitIcon, IvoryIcon
from build_akravon_player import BuildStatus
from build_akravon_actions import Build as BuildActions

HERE = os.path.dirname(os.path.abspath(__file__))
PARTS = os.path.join(HERE, 'hud_parts_source.png')
ICONS = os.path.join(HERE, 'hud_icons_source.png')

NAVY = (12, 18, 32)
MENU_ICONS = ('character', 'inventory', 'messenger', 'itemshop', 'system', 'pickup', 'autohunt', 'mount', 'chat')


def Key(im):
	# yesil zemini saydam yap; yari saydam kenarlarda yesil tasmasini gider
	a = np.asarray(im.convert('RGB')).astype(np.int32)
	r, g, b = a[..., 0], a[..., 1], a[..., 2]
	d = g - np.maximum(r, b)
	alpha = np.clip((90 - d) * 255 // 70, 0, 255)
	alpha[d <= 20] = 255
	g2 = np.minimum(g, np.maximum(r, b) + 8)
	out = np.dstack([r, g2, b, alpha]).astype(np.uint8)
	return Image.fromarray(out, 'RGBA')


def Components(im, minArea = 2000):
	import cv2
	a = np.asarray(im.convert('RGB')).astype(np.int32)
	mask = ((a[..., 1] - np.maximum(a[..., 0], a[..., 2])) < 60).astype(np.uint8)
	n, lab, stats, cent = cv2.connectedComponentsWithStats(mask, 8)
	boxes = [tuple(int(v) for v in s[:4]) for s in stats[1:] if s[4] > minArea]
	boxes = [(x, y, x + w, y + h) for (x, y, w, h) in boxes]
	boxes.sort(key = lambda b: (0 if b[3] < 420 else (1 if b[1] < 720 else 2), b[0]))
	return boxes


def Square(im, size):
	side = max(im.width, im.height)
	cell = Image.new('RGBA', (side, side), (0, 0, 0, 0))
	cell.alpha_composite(im, ((side - im.width) // 2, (side - im.height) // 2))
	return cell.resize((size, size), Image.LANCZOS)


def RefillDisc(im, radiusRatio, fill, alpha):
	# yari saydam ic alani lacivert yap (yesil zemin icine sizmasin); altin cizgiler kalir
	a = np.asarray(im).astype(np.int32).copy()
	h, w = a.shape[:2]
	yy, xx = np.mgrid[0:h, 0:w]
	r = np.hypot(xx - w / 2.0, yy - h / 2.0) / (w / 2.0)
	gold = (a[..., 0] - a[..., 2]) > 45
	inner = (r < radiusRatio) & ~gold
	a[inner, 0], a[inner, 1], a[inner, 2] = fill
	a[inner, 3] = alpha
	return Image.fromarray(a.astype(np.uint8), 'RGBA')


def HStretch(src, capLeft, capRight, width, height):
	k = height / float(src.height)
	src = src.resize((max(1, int(round(src.width * k))), height), Image.LANCZOS)
	l, r = int(round(capLeft * k)), int(round(capRight * k))
	out = Image.new('RGBA', (width, height), (0, 0, 0, 0))
	out.paste(src.crop((0, 0, l, height)), (0, 0))
	mid = src.crop((l, 0, src.width - r, height)).resize((max(1, width - l - r), height), Image.LANCZOS)
	out.paste(mid, (l, 0))
	out.paste(src.crop((src.width - r, 0, src.width, height)), (width - r, 0))
	return out


def Nine(src, inset, width, height):
	k = min(1.0, height / float(src.height))
	i = inset
	d = max(1, int(round(inset * k)))
	out = Image.new('RGBA', (width, height), (0, 0, 0, 0))
	sx, sy = (0, i, src.width - i, src.width), (0, i, src.height - i, src.height)
	dx, dy = (0, d, width - d, width), (0, d, height - d, height)
	for a in range(3):
		for b in range(3):
			box = (sx[a], sy[b], sx[a + 1], sy[b + 1])
			size = (dx[a + 1] - dx[a], dy[b + 1] - dy[b])
			if size[0] > 0 and size[1] > 0:
				out.paste(src.crop(box).resize(size, Image.LANCZOS), (dx[a], dy[b]))
	return out


ATTACK_SOURCE = os.path.join(HERE, 'hud_attack_source.png')
JOYSTICK_REFERENCE = os.path.join(HERE, 'hud_joystick_reference.png')


def BuildControls(save):
	import cv2
	# joystick: kullanicinin referansi (gercek saydam) - ortadaki topuz ayri parca, taban orta bosluklu
	ref = Image.open(JOYSTICK_REFERENCE).convert('RGBA')
	alpha = ref.getchannel('A').point(lambda v: 255 if v > 12 else 0)
	bbox = alpha.getbbox()
	base = ref.crop(bbox)
	cx, cy = base.width / 2.0, base.height / 2.0
	knobR = int(base.width * 0.168)
	knob = base.crop((int(cx - knobR), int(cy - knobR), int(cx + knobR), int(cy + knobR)))
	mask = Image.new('L', knob.size, 0)
	ImageDraw.Draw(mask).ellipse([int(knobR * 0.08), int(knobR * 0.08), knob.width - 1 - int(knobR * 0.08), knob.height - 1 - int(knobR * 0.08)], fill = 255)
	knob.putalpha(ImageChops_multiply(knob.getchannel('A'), mask))
	rgb = cv2.cvtColor(np.asarray(base.convert('RGB')), cv2.COLOR_RGB2BGR)
	hole = np.zeros(rgb.shape[:2], np.uint8)
	cv2.circle(hole, (int(cx), int(cy)), int(knobR * 1.06), 255, -1)
	filled = cv2.inpaint(rgb, hole, 9, cv2.INPAINT_TELEA)
	blurred = cv2.GaussianBlur(filled, (0, 0), knobR * 0.35)
	soft = cv2.GaussianBlur(hole, (0, 0), knobR * 0.12).astype(np.float32)[..., None] / 255.0
	filled = (filled * (1 - soft) + blurred * soft).astype(np.uint8)
	filled = Image.fromarray(cv2.cvtColor(filled, cv2.COLOR_BGR2RGB)).convert('RGBA')
	filled.putalpha(base.getchannel('A'))
	save('hud_joystick', Square(filled, 400))
	save('hud_knob', Square(knob, 160))

	# saldiri: referansa bakilarak uretilen dugme (yesil zemin)
	attack = Key(Image.open(ATTACK_SOURCE))
	attack = attack.crop(attack.getchannel('A').point(lambda v: 255 if v > 40 else 0).getbbox())
	attack = Square(attack, 320)
	save('hud_attack', attack)
	save('hud_attack_pressed', ImageEnhance.Brightness(attack).enhance(1.22))


def ImageChops_multiply(a, b):
	from PIL import ImageChops
	return ImageChops.multiply(a, b)


def Build(out):
	BuildActions(out)
	dst = os.path.join(out, 'mobile_ui', 'akravon', 'hud')
	os.makedirs(dst, exist_ok = True)

	def save(name, im):
		im.save(os.path.join(dst, name + '.png'))
		print('%-26s %dx%d' % (name, im.width, im.height))

	src = Image.open(PARTS)
	keyed = Key(src)
	boxes = Components(src)
	(attackOld, joystickOld, knobOld, disc, ring, frame, frameActive, status) = [keyed.crop(b) for b in boxes[:8]]

	BuildControls(save)
	disc256 = BuildCircle(save)

	# ortak kit: menu / yardimci butonlar, kilit ikonlari (altin yok - lacivert tas + eskitilmis bronz + krem cizgi ikon)
	icons = BuildKit(save, disc256)
	plus = disc256.copy()
	plusIcon = FitIcon(icons['plus'], 112)
	plus.alpha_composite(plusIcon, ((plus.width - plusIcon.width) // 2, (plus.height - plusIcon.height) // 2))
	save('hud_plus', plus)
	from PIL import ImageFont
	font = ImageFont.truetype(os.path.join(HERE, '..', '..', 'data', 'fonts', 'tahomabd.ttf'), 120)
	for page in range(1, 5):
		face = disc256.copy()
		draw = ImageDraw.Draw(face)
		box = draw.textbbox((0, 0), str(page), font = font)
		draw.text(((face.width - (box[2] - box[0])) // 2 - box[0], (face.height - (box[3] - box[1])) // 2 - box[1] - 4), str(page), font = font, fill = (234, 223, 196, 255))
		save('hud_page_%d' % page, face)
	# harita filtresi: ic kare opak, kenar 2 px saydam; doku kenari uzatildigi icin harita kare pencerenin disina tasmaz
	filterImage = Image.new('RGBA', (128, 128), (255, 255, 255, 0))
	filterImage.paste(Image.new('RGBA', (124, 124), (255, 255, 255, 255)), (2, 2))
	save('minimap_filter', filterImage)
	# AKRAVON: beceri yuvasi altindaki ayar (anahtar) ve sayfa cevir dugmeleri diger HUD dugmeleriyle ayni plakadan (256 px)
	for (target, source) in (('hud_skill_edit', 'btn_wrench_normal'), ('hud_skill_pages', 'btn_refresh_normal')):
		save(target, Image.open(os.path.join(dst, source + '.png')).convert('RGBA').resize((256, 256), Image.LANCZOS))
	# AKRAVON: ic + dis yumusak isilti; resim dugmeden 40 px (136 olcekte) buyuk, kenar halkasi dugme kenarinda
	(size, pad) = (176, 20)
	big = size * 4
	alert = Image.new('RGBA', (big, big), (0, 0, 0, 0))
	edge = [(pad + 1) * 4, (pad + 1) * 4, big - (pad + 1) * 4, big - (pad + 1) * 4]	# dugme kenari
	halo = Image.new('RGBA', alert.size, (0, 0, 0, 0))
	ImageDraw.Draw(halo).ellipse(edge, outline = (255, 255, 255, 255), width = 30)
	alert.alpha_composite(halo.filter(ImageFilter.GaussianBlur(30)))	# ice ve disa esit yayilir
	alert.alpha_composite(halo.filter(ImageFilter.GaussianBlur(12)))
	ImageDraw.Draw(alert).ellipse(edge, outline = (255, 255, 255, 90), width = 5)
	save('btn_alert', alert.resize((size, size), Image.LANCZOS))
	save('chat_icon', IvoryIcon(FitIcon(icons['chat'], 48)))

	# durum plakasi: kullanicinin referansindan (portre halkasi, Lv plakasi, HP/SP/EXP yuvalari)
	BuildStatus(save)

	# sohbet cubugu: ince altin kenarli lacivert serit (2x, oyunda yatay olceklenir)
	bar = Nine(frame, 40, 1040, 84)
	save('chat_bar', bar)

	# hp / sp / st / exp dolgulari (yatay gecis, 2x yukseklik)
	for (name, top, bottom, h) in (('gauge_hp', (226, 74, 66), (140, 30, 30), 24), ('gauge_sp', (88, 150, 236), (34, 70, 150), 20),
			('gauge_st', (228, 196, 96), (150, 112, 40), 14), ('gauge_exp', (250, 214, 140), (180, 140, 70), 8)):
		fill = Image.new('RGBA', (8, h))
		d = ImageDraw.Draw(fill)
		for y in range(h):
			t = y / float(max(1, h - 1))
			d.line([(0, y), (8, y)], fill = tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3)) + (255,))
		d.line([(0, 0), (8, 0)], fill = (255, 255, 255, 70))
		save(name, fill)
	track = Image.new('RGBA', (8, 8), (4, 6, 12, 220))
	save('gauge_track', track)

	# beceri ikonu maskesi: kare ikonun koselerini disk rengiyle orter (ortasi yuvarlak delik).
	# istemcideki SetSlotRoundIcons bayragi henuz cizimde kullanilmiyor; APK derlenmeden yuvarlak gorunum
	big = 1024
	hole = Image.new('L', (big, big), 0)
	ImageDraw.Draw(hole).ellipse([big * (0.5 - HOLE), big * (0.5 - HOLE), big * (0.5 + HOLE), big * (0.5 + HOLE)], fill = 255)
	hole = hole.resize((256, 256), Image.LANCZOS)
	mask = disc256.copy()
	alpha = mask.getchannel('A')
	from PIL import ImageChops
	mask.putalpha(ImageChops.multiply(alpha, ImageChops.invert(hole)))
	save('hud_icon_mask', mask)
	# ekrandaki boyutla birebir kopyalar (motor buyuk dokuyu yumusatmadan kucultuyor -> kenar kirilir)
	for px in range(40, 162, 2):
		save('hud_icon_mask_%d' % px, mask.resize((px, px), Image.LANCZOS))


HOLE = 0.315	# delik yaricapi / yuva capi (ikon 0.66 x yuva -> yaricap 0.33)


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
