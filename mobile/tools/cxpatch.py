# Cynex: istemci kaynagina bayt duzeyinde yama - kodlama (cp1254) ve CRLF korunur
# kullanim: python cxpatch.py yama.txt
# yama.txt:
#   @@ dosya/yolu.cpp [adet]
#   <<<
#   eski
#   ===
#   yeni
#   >>>
import os, sys

ROOT = os.environ.get('CYNEX_CLIENT', 'C:/CynexMobil/client/source')


def apply(path, old, new, count):
	p = os.path.join(ROOT, path)
	raw = open(p, 'rb').read()
	crlf = b'\r\n' in raw
	text = raw.decode('latin1')
	if crlf:
		old = old.replace('\n', '\r\n')
		new = new.replace('\n', '\r\n')
	n = text.count(old)
	if count == 0:
		if n == 0:
			raise SystemExit('ESLESME YOK: %s\n%s' % (path, old))
	elif n != count:
		raise SystemExit('%d ESLESME (beklenen %d): %s\n%s' % (n, count, path, old))
	text = text.replace(old, new)
	open(p, 'wb').write(text.encode('latin1'))
	return n


def main(spec):
	lines = open(spec, encoding='utf-8').read().split('\n')
	i = 0
	total = 0
	while i < len(lines):
		line = lines[i]
		if not line.startswith('@@ '):
			i += 1
			continue
		parts = line[3:].split()
		path = parts[0]
		count = int(parts[1]) if len(parts) > 1 else 1
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
		n = apply(path, '\n'.join(old), '\n'.join(new), count)
		print('ok %-50s x%d' % (path, n))
		total += 1
		i = j + 1
	print('%d yama' % total)


if __name__ == '__main__':
	main(sys.argv[1])
