# Cynex: mobil veri paketi - acilmis pack'ler + 64bit gr2 + cynxspt + mobil arayuz -> files/mobile/*.zip
# Cynex: eski pack'lerdeki 32bit modeller ayrica gitmesin diye tum veri yeniden paketlenir
# kullanim: python make_mobile_packs.py [cikis_klasoru]
import os, sys, zipfile, time

B2PACK = r'C:/CynexMobilBuild/b2pack'
GR2 = r'C:/CynexMobilBuild/granny/b2pack64'
SPT = r'C:/CynexMobilBuild/speedtree/converted/b2pack'
UI = r'C:/CynexMobilBuild/mobile_ui_data'
OUT = [a for a in sys.argv[1:] if not a.startswith('--')][0] if [a for a in sys.argv[1:] if not a.startswith('--')] else r'C:/CynexMobilBuild/release/files/mobile'

MAX_BYTES = 1800 * 1024 * 1024		# Cynex: istemci ofseti int - arsiv 2GB altinda kalsin
MAX_FILES = 60000					# Cynex: zip64 olmasin
STORED = ('.jpg', '.jpeg', '.png', '.mp3', '.ogg', '.zip')


def key(rel):
	# Cynex: istemci yalniz ascii harfleri kucultur (korece/turkce ham baytlar oldugu gibi)
	k = ''.join(c.lower() if c < '\x80' else c for c in rel.replace('\\', '/'))
	if len(k) > 2 and k[1] == ':' and k[2] == '/':
		k = k[3:]
	return k.lstrip('/')


def walk(root):
	out = {}
	if not os.path.isdir(root):
		return out
	for d, _, fs in os.walk(root):
		for f in fs:
			p = os.path.join(d, f)
			out[key(os.path.relpath(p, root))] = p
	return out


def pack_order():
	# Cynex: Index sirasi - istemcide ilk kayitli pack kazanir
	lines = [l.strip() for l in open(os.path.join(B2PACK, 'Index'), encoding='latin1') if l.strip()]
	names = [l for l in lines[1:] if l != '*']
	order = ['root', 'locale']
	for n in names:
		if n not in order:
			order.append(n)
	return order


class Writer:
	def __init__(self, prefix):
		self.prefix = prefix
		self.part = 0
		self.z = None
		self.bytes = 0
		self.files = 0

	def _open(self):
		self.part += 1
		path = os.path.join(OUT, '%s_%d.zip' % (self.prefix, self.part))
		self.z = zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED, allowZip64=False)
		self.bytes = self.files = 0

	def add(self, name, src):
		size = os.path.getsize(src)
		if self.z is None or self.files >= MAX_FILES or self.bytes + size > MAX_BYTES:
			self.close()
			self._open()
		ct = zipfile.ZIP_STORED if name.endswith(STORED) else zipfile.ZIP_DEFLATED
		self.z.write(src, name, compress_type=ct, compresslevel=6)
		self.files += 1
		self.bytes += size	# Cynex: sikismasa bile 2GB'i asmasin

	def close(self):
		if self.z:
			self.z.close()
			self.z = None


def main():
	os.makedirs(OUT, exist_ok=True)
	# Cynex: --devam = tamamlanmis pack'leri atla (yarida kalan yeniden yazilir)
	resume = '--devam' in sys.argv
	done = set()
	if resume and os.path.exists(os.path.join(OUT, 'tamam.txt')):
		done = set(open(os.path.join(OUT, 'tamam.txt')).read().split())
	else:
		for f in os.listdir(OUT):
			if f.endswith('.zip') or f == 'tamam.txt':
				os.remove(os.path.join(OUT, f))

	taken = set()
	t0 = time.time()

	# Cynex: mobil arayuz en once (game.py vb. root dosyalarini ezer)
	ui = walk(UI)
	for n in ui:
		if 'sezar' in n:
			raise SystemExit('genel pakette sezar dosyasi: ' + n)
	if '00_cynexui' not in done:
		for f in os.listdir(OUT):
			if f.startswith('00_cynexui_'):
				os.remove(os.path.join(OUT, f))
		w = Writer('00_cynexui')
		for n in sorted(ui):
			w.add(n, ui[n])
		w.close()
		open(os.path.join(OUT, 'tamam.txt'), 'a').write('00_cynexui\n')
	taken.update(ui)
	print('cynexui: %d dosya' % len(ui))

	for i, pack in enumerate(pack_order()):
		files = walk(os.path.join(B2PACK, pack))
		files.update(walk(os.path.join(GR2, pack)))		# Cynex: 64bit modeller
		files.update(walk(os.path.join(SPT, pack)))		# Cynex: cynxspt agaclar
		names = [n for n in sorted(files) if n not in taken and not n.endswith('.txt.filepart')]
		if not names:
			continue
		prefix = '%02d_%s' % (10 + i, pack)
		if prefix in done:
			taken.update(names)
			print('%-18s atlandi' % pack)
			continue
		for f in os.listdir(OUT):
			if f.startswith(prefix + '_'):
				os.remove(os.path.join(OUT, f))
		w = Writer(prefix)
		for n in names:
			w.add(n, files[n])
			taken.add(n)
		w.close()
		open(os.path.join(OUT, 'tamam.txt'), 'a').write(prefix + '\n')
		print('%-18s %6d dosya (%ds)' % (pack, len(names), time.time() - t0))

	total = sum(os.path.getsize(os.path.join(OUT, f)) for f in os.listdir(OUT) if f.endswith('.zip'))
	print('toplam %d dosya, %.1f MB' % (len(taken), total / 1048576.0))


if __name__ == '__main__':
	main()
