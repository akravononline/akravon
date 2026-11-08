# AKRAVON: hizli slot icin HD yuvarlak beceri ikonlari
# temel kaynak: oyunun kendi 32x32 ikonlari (14_etc_1.zip, ymir work/ui/skill/**.sub + atlaslar).
# Onayli 54 beceri, build_akravon_hdskills ile HD kaynaklardan yenilenir:
# HUD icin dort kademe, beceri penceresi icin uc .sub kademesi (G/P ortak).
# cikti: <out>/ymir work/ui/akravon_skill/<klasor>/<ad>.png ve ../uiakravonskillicon.py (beceri -> ikon tablosu)
import io
import os
import sys
import zipfile
from PIL import Image, ImageDraw, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
FILES = os.path.join(HERE, '..', '..', '..', '..', '2_Oyun_Dosyalari', 'files', 'mobile')
ETC_ZIP = os.path.join(FILES, '14_etc_1.zip')
LOCALE_ZIP = os.path.join(FILES, '11_locale_1.zip')
SIZE = 64	# ekranda yuva ~67 px, ikon ~60 px -> motor neredeyse 1:1 cizer
SS = 8		# kenar yumusatma icin buyuk cizim carpani
PATHS = {'WARRIOR': 'warrior', 'ASSASSIN': 'assassin', 'SURA': 'sura', 'SHAMAN': 'shaman', 'PASSIVE': 'passive',
	'78SKILL': '78skill', 'WOLFMAN': 'wolfman', 'SUPPORT': 'common/support', 'GUILD': 'common/guild', 'HORSE': 'common/horse'}
PREFIX = 'ymir work/ui/skill/'
OUT_PREFIX = 'ymir work/ui/akravon_skill/'


def Tokens(text):
	values = {}
	for line in text.splitlines():
		parts = line.strip().split(None, 1)
		if len(parts) == 2:
			values[parts[0].lower()] = parts[1].strip().strip('"')
	return values


def Round(cell):
	big = SIZE * SS
	# 32 -> 64: once yumusak buyut, sonra hafif keskinlestir (pikseller kare kare buyumesin)
	icon = cell.convert('RGBA').resize((big, big), Image.BICUBIC).resize((SIZE, SIZE), Image.LANCZOS)
	icon = icon.filter(ImageFilter.UnsharpMask(radius = 1.2, percent = 60, threshold = 2))
	# orijinal ikonun kare cercevesi kesilsin diye hafif yakinlastir
	inset = 3
	icon = icon.crop((inset, inset, SIZE - inset, SIZE - inset)).resize((SIZE, SIZE), Image.LANCZOS)

	mask = Image.new('L', (big, big), 0)
	ImageDraw.Draw(mask).ellipse([SS * 1, SS * 1, big - SS * 1, big - SS * 1], fill = 255)
	mask = mask.resize((SIZE, SIZE), Image.LANCZOS)
	out = Image.new('RGBA', (SIZE, SIZE), (0, 0, 0, 0))
	out.paste(icon, (0, 0), mask)

	rim = Image.new('RGBA', (big, big), (0, 0, 0, 0))
	d = ImageDraw.Draw(rim)
	d.ellipse([SS * 1, SS * 1, big - SS * 1, big - SS * 1], outline = (24, 16, 8, 230), width = int(SS * 2.2))
	d.ellipse([SS * 2, SS * 2, big - SS * 2, big - SS * 2], outline = (214, 172, 96, 255), width = int(SS * 1.2))
	out.alpha_composite(rim.resize((SIZE, SIZE), Image.LANCZOS))
	return out


COOL_FRAMES = 36
COOL_SIZES = range(44, 104, 4)


def BuildCooldown(outDir):
	# dolan daire: kalan sure koyu dilim (saat yonunde azalir) + ucunda altin yay
	import math
	folder = os.path.join(outDir, 'mobile_ui', 'akravon', 'cool')
	os.makedirs(folder, exist_ok = True)
	for px in COOL_SIZES:
		big = px * SS
		for f in range(COOL_FRAMES + 1):
			frac = f / float(COOL_FRAMES)
			im = Image.new('RGBA', (big, big), (0, 0, 0, 0))
			d = ImageDraw.Draw(im)
			if frac > 0:
				# AKRAVON: kalan sure koyu kirmizi dilim (yari saydam) + kenarinda parlayan kor cizgi
				start = -90 + 360 * (1 - frac)
				d.pieslice([SS, SS, big - SS, big - SS], start, 270, fill = (110, 8, 8, 165))
				glow = Image.new('RGBA', (big, big), (0, 0, 0, 0))
				gd = ImageDraw.Draw(glow)
				gd.arc([SS * 2, SS * 2, big - SS * 2, big - SS * 2], -90, start, fill = (255, 120, 60, 255), width = int(SS * 4))
				glow = glow.filter(ImageFilter.GaussianBlur(SS * 1.5))
				im.alpha_composite(glow)
				d.arc([SS * 2, SS * 2, big - SS * 2, big - SS * 2], -90, start, fill = (255, 214, 150, 255), width = int(SS * 1.6))
			im.resize((px, px), Image.LANCZOS).save(os.path.join(folder, 'cool_%d_%02d.png' % (px, f)))


def Build(outDir):
	BuildCooldown(outDir)
	etc = zipfile.ZipFile(ETC_ZIP)
	names = dict((n.lower(), n) for n in etc.namelist())
	atlases = {}
	made = set()
	for key in sorted(names):
		if not (key.startswith(PREFIX) and key.endswith('.sub')):
			continue
		tok = Tokens(etc.read(names[key]).decode('latin1'))
		image = 'ymir work/ui/' + tok.get('image', '').lower()
		if image not in names:
			continue
		if image not in atlases:
			atlases[image] = Image.open(io.BytesIO(etc.read(names[image]))).convert('RGBA')
		l, t, r, b = [int(tok.get(k, 0)) for k in ('left', 'top', 'right', 'bottom')]
		if r - l < 16 or b - t < 16:
			continue
		rel = key[len(PREFIX):-4]
		dst = os.path.join(outDir, OUT_PREFIX + rel + '.png')
		os.makedirs(os.path.dirname(dst), exist_ok = True)
		Round(atlases[image].crop((l, t, r, b))).save(dst)
		made.add(rel)

	# beceri numarasi -> ikon tabani (skilldesc.txt: 0 vnum, 1 meslek, 12 ikon adi)
	table = {}
	desc = zipfile.ZipFile(LOCALE_ZIP).read('locale/tr/skilldesc.txt').decode('latin1')
	for line in desc.splitlines():
		v = line.split('\t')
		if len(v) < 13 or not v[0].strip().isdigit():
			continue
		folder = PATHS.get(v[1].strip().upper())
		icon = v[12].strip().lower()
		if folder and icon:
			table[int(v[0])] = folder + '/' + icon
	from build_akravon_hdskills import BuildApprovedSkills
	approvedMade, _ = BuildApprovedSkills(outDir, table)
	made.update(approvedMade)
	lines = ['# AKRAVON: otomatik uretildi (skin/build_akravon_skillicons.py) - elle duzenleme', 'MADE = set([']
	lines += ['\t"%s",' % m for m in sorted(made)]
	lines += ['])', 'TABLE = {']
	lines += ['\t%d: "%s",' % (k, table[k]) for k in sorted(table)]
	lines += ['}', '', '',
		'def GetIcon(skillIndex, grade):',
		'\timport uiAkravonActionIcon',
		'\tsupportIcon = uiAkravonActionIcon.GetSupportIcon(skillIndex)',
		'\tif supportIcon:',
		'\t\treturn supportIcon',
		'\tbase = TABLE.get(skillIndex)',
		'\tif not base:',
		'\t\treturn None',
		'\tfor name in (base + "_%02d" % (min(max(grade, 0), 3) + 1), base + "_01", base):',
		'\t\tif name in MADE:',
		'\t\t\treturn "d:/' + OUT_PREFIX + '%s.png" % name',
		'\treturn None', '']
	open(os.path.join(HERE, '..', 'uiakravonskillicon.py'), 'w', newline = '\n').write('\n'.join(lines))
	print('beceri ikonu: %d png, %d beceri' % (len(made), len(table)))


if __name__ == '__main__':
	Build(sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out')
