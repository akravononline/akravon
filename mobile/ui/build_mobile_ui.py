# Cynex: mobil arayuz veri agaci - telefonda <oyun>/mobile/ altina aynen kopyalanir
# kullanim: py -3 build_mobile_ui.py [cikti_klasoru]
#  - mobile/ui/*.py      -> cp1254 + CRLF (barbaros root bicimi, coding satiri yok)
#  - mobile/ui/patches   -> barbaros root dosyalarina bayt duzeyinde yama (game.py, networkmodule.py)
#  - skin/build_skin.py  -> mobile_ui/cynex/*.png + uimobileassets.py (YeniKlasor4 mobile_ui setinden)
import os
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
B2ROOT = 'C:/CynexMobilBuild/b2pack/root'
OUT = sys.argv[1] if len(sys.argv) > 1 else 'C:/CynexMobilBuild/mobile_ui_data'


def ToRoot(text):
	data = text.replace('\r\n', '\n').encode('cp1254')
	return data.replace(b'\n', b'\r\n')


def ParseSpec(path):
	lines = open(path, encoding='utf-8').read().split('\n')
	i = 0
	while i < len(lines):
		line = lines[i]
		if not line.startswith('@@ '):
			i += 1
			continue
		target = line[3:].split()[0]
		assert lines[i + 1] == '<<<', line
		j = i + 2
		old = []
		while lines[j] != '===':
			old.append(lines[j])
			j += 1
		j += 1
		new = []
		while lines[j] != '>>>':
			new.append(lines[j])
			j += 1
		yield target, '\n'.join(old), '\n'.join(new)
		i = j + 1


def ApplyPatches():
	patched = {}
	folder = os.path.join(HERE, 'patches')
	for name in sorted(os.listdir(folder)):
		if not name.endswith('.txt'):
			continue
		for target, old, new in ParseSpec(os.path.join(folder, name)):
			if target not in patched:
				patched[target] = open(os.path.join(B2ROOT, target), 'rb').read().decode('latin1')
			text = patched[target]
			crlf = '\r\n' in text
			if crlf:
				old = old.replace('\n', '\r\n')
				new = new.replace('\n', '\r\n')
			count = text.count(old)
			if count != 1:
				raise SystemExit('%s: %d eslesme\n%s' % (target, count, old))
			patched[target] = text.replace(old, new)
	for target, text in patched.items():
		open(os.path.join(OUT, target), 'wb').write(text.encode('latin1'))
		print('yama  %s' % target)


def CopyScripts():
	for name in sorted(os.listdir(HERE)):
		if not name.endswith('.py') or name == os.path.basename(__file__):
			continue
		text = StripComments(open(os.path.join(HERE, name), encoding='utf-8').read())
		open(os.path.join(OUT, name), 'wb').write(ToRoot(text))
		print('kod   %s' % name)


def BuildSkin():
	import subprocess
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_skin.py'), OUT])


def StripComments(text):
	# Cynex: yalniz kendi notlarimiz kalsin
	lines = []
	for line in text.split(chr(10)):
		stripped = line.strip()
		if stripped.startswith('#') and not stripped.startswith('# Cynex'):
			continue
		lines.append(line)
	return chr(10).join(lines)


def CheckClean():
	# Cynex: genel surumde eski arayuzden ad / gorsel kalmasin
	bad = 0
	for root, dirs, files in os.walk(OUT):
		for name in files:
			path = os.path.join(root, name)
			if name.endswith('.py') and b'sezar' in open(path, 'rb').read().lower():
				print('HATA eski ad: %s' % path)
				bad += 1
	if os.path.isdir(os.path.join(OUT, 'mobil')):
		print('HATA eski gorsel klasoru: mobil/')
		bad += 1
	return bad == 0


def CheckSyntax():
	# Cynex: python 2.7 dilbilgisi (print deyimi dahil) - lib2to3 ayristiricisi
	import warnings
	warnings.simplefilter('ignore')
	from lib2to3 import pygram, pytree
	from lib2to3.pgen2 import driver
	grammar = pygram.python_grammar
	d = driver.Driver(grammar, convert=pytree.convert)
	bad = 0
	for name in sorted(os.listdir(OUT)):
		if not name.endswith('.py'):
			continue
		text = open(os.path.join(OUT, name), 'rb').read().decode('cp1254').replace('\r\n', '\n')
		try:
			d.parse_string(text + '\n')
		except Exception as error:
			bad += 1
			print('HATA  %s: %s' % (name, error))
	print('sozdizimi: %d hata' % bad)
	return bad == 0


def main():
	if os.path.isdir(OUT):
		for name in os.listdir(OUT):
			if name.endswith('.py'):
				os.remove(os.path.join(OUT, name))
	os.makedirs(OUT, exist_ok=True)
	if os.path.isdir(os.path.join(OUT, 'mobil')):
		shutil.rmtree(os.path.join(OUT, 'mobil'))
	CopyScripts()
	ApplyPatches()
	BuildSkin()
	if not CheckSyntax() or not CheckClean():
		raise SystemExit(1)


if __name__ == '__main__':
	main()
