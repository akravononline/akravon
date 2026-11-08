# -*- coding: utf-8 -*-
# AKRAVON: Hava Kilici kilic aurasi - uzaktan da renkli gorunsun. Orijinal "geom_sword_loop / geom_spear_loop" uzakta mavi bir seride
# donuyordu (mavi serit + mavi parilti baskin). Burada: parilti turkuaz'a, ince serit turkuaz-yesil-mor'a cekilir, parilti %40 buyur.
# Yeni dosyalar orijinalin ustunu ortter (gevsek dosya); eski hal icin iki dosyayi silmek yeterli.
import os
import re
import sys
import zipfile

OUT = sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out'
PACK = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..', '2_Oyun_Dosyalari', 'files', 'mobile', '23_pc_1.zip')
BASE = 'ymir work/pc/warrior/effect/'
DEST = os.path.join(OUT, 'ymir work', 'pc', 'warrior', 'effect')
GROW = 1.15

# ParticleProperty sirasi: 0 buyuk parilti, 1 ates, 2 ince serit
COLORS = {
	0: ([(0.0, 0.15)], [(0.0, 0.85)], [(0.0, 0.75)]),
	2: ([(0.0, 0.2), (0.35, 0.1), (0.7, 0.75), (1.0, 0.95)], [(0.0, 1.0), (0.35, 1.0), (0.7, 0.30), (1.0, 0.45)], [(0.0, 0.75), (0.35, 0.25), (0.7, 1.0), (1.0, 0.85)]),
}


def SetList(group, name, keys):
	body = '\n        '.join('%.6f %.6f' % key for key in keys)
	return re.sub(r'(List %s\s*\{)[^}]*(\})' % name, lambda m: m.group(1) + '\n        ' + body + '\n    ' + m.group(2), group)


def ScaleSizes(group, factor):
	def repl(m):
		lines = []
		for line in m.group(2).split('\n'):
			tokens = line.split()
			if len(tokens) == 2:
				line = line.replace(tokens[1], '%.6f' % (float(tokens[1]) * factor), 1)
			lines.append(line)
		return m.group(1) + '\n'.join(lines) + m.group(3)
	for name in ('TimeEventSizeX', 'TimeEventSizeY'):
		group = re.sub(r'(List %s\s*\{)([^}]*)(\})' % name, repl, group)
	return group


def Process(text):
	# her "Group Particle" blogu emitter + property birlikte icerir; sira: 0 buyuk parilti, 1 ates, 2 ince serit
	pieces = re.split('(Group Particle' + chr(10) + ')', text)
	out = [pieces[0]]
	for index in range(1, len(pieces), 2):
		body = pieces[index + 1]
		layer = index // 2
		if layer in COLORS:
			(r, g, b) = COLORS[layer]
			for (name, keys) in (('TimeEventColorRed', r), ('TimeEventColorGreen', g), ('TimeEventColorBlue', b)):
				body = SetList(body, name, keys)
		if layer == 0:
			body = ScaleSizes(body, GROW)
		out.append(pieces[index] + body)
	return ''.join(out)


zf = zipfile.ZipFile(PACK)
os.makedirs(DEST, exist_ok = True)
for f in os.listdir(DEST):
	if f.startswith('geom_'):
		os.remove(os.path.join(DEST, f))
for name in ('geom_sword_loop', 'geom_spear_loop'):
	text = zf.read(BASE + name + '.mse').decode('cp1254')
	text = text.replace('\r\n', '\n')
	open(os.path.join(DEST, name + '.mse'), 'wb').write(Process(text).replace('\n', '\r\n').encode('cp1254'))
	print('yazildi', name)
