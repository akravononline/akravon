# Cynex: python'a giden isaretci tutamaclari 64 bit - "int h; PyTuple_GetInteger(.., &h); (Tip*)h" -> long long
# kullanim: python handle64.py dosya.cpp ...   (bayt duzeyinde, cp1254/CRLF korunur)
import re, sys, os

ROOT = os.environ.get('CYNEX_CLIENT', 'C:/CynexMobil/client/source')
CAST = re.compile(r'\(\s*(?:const\s+)?[A-Za-z_][\w:]*\s*\*\s*\)\s*([A-Za-z_]\w*)\b')


def fix(text):
	changed = 0
	pos = 0
	while True:
		m = CAST.search(text, pos)
		if not m:
			break
		var = m.group(1)
		pos = m.end()
		# en yakin onceki bildirim
		decl = None
		for d in re.finditer(r'\bint\s+' + var + r'\s*(=\s*0\s*)?;', text[:m.start()]):
			decl = d
		if not decl:
			continue
		# bildirim ile donusum ayni fonksiyonda mi (arada fonksiyon basi yok)
		between = text[decl.end():m.start()]
		if re.search(r'\n(PyObject|bool|void|int)\s*\*?\s*\w+\s*\(', between):
			continue
		get = re.compile(r'PyTuple_GetInteger\(([^,]+),\s*([^,]+),\s*&' + var + r'\)')
		if not get.search(between):
			continue
		between2 = get.sub(r'PyTuple_GetLongLong(\1, \2, &' + var + ')', between)
		newdecl = 'long long ' + var + (' = 0' if decl.group(1) else '') + ';'
		text = text[:decl.start()] + newdecl + between2 + text[m.start():]
		pos = decl.start() + len(newdecl) + len(between2) + (m.end() - m.start())
		changed += 1
	return text, changed


for rel in sys.argv[1:]:
	p = os.path.join(ROOT, rel)
	raw = open(p, 'rb').read().decode('latin1')
	out, n = fix(raw)
	if n:
		open(p, 'wb').write(out.encode('latin1'))
	print('%s: %d' % (rel, n))
