# Cynex: ikili veri yapilarinda long -> LONG, unsigned long -> DWORD, time_t -> LONG
# windows'ta ayni tipler (pc derlemesi degismez), 64bit android'de 4 bayt kalir
# kullanim: python long32.py dosya.h [--time]
import re, sys

path = sys.argv[1]
do_time = '--time' in sys.argv

raw = open(path, 'rb').read()
text = raw.decode('latin1')

# Cynex: yorum ve stringlere dokunma
tok = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[A-Za-z_][A-Za-z0-9_]*|\s+|.', re.S)
parts = tok.findall(text)

def word(i, step):
	j = i + step
	while 0 <= j < len(parts):
		p = parts[j]
		if p.isspace() or p.startswith('//') or p.startswith('/*'):
			j += step
			continue
		return j, p
	return -1, ''

out = list(parts)
stats = {'long': 0, 'unsigned long': 0, 'time_t': 0}
i = 0
while i < len(parts):
	p = parts[i]
	if p == 'long':
		pj, prev = word(i, -1)
		nj, nxt = word(i, +1)
		if nxt in ('long', 'double') or prev == 'long':
			i += 1
			continue
		if nxt == 'int':
			out[nj] = ''
			# Cynex: aradaki bosluk da gitsin
			k = i + 1
			while k < nj:
				out[k] = ''
				k += 1
		if prev == 'unsigned':
			out[pj] = ''
			k = pj + 1
			while k < i:
				out[k] = ''
				k += 1
			out[i] = 'DWORD'
			stats['unsigned long'] += 1
		elif prev == 'signed':
			out[pj] = ''
			k = pj + 1
			while k < i:
				out[k] = ''
				k += 1
			out[i] = 'LONG'
			stats['long'] += 1
		else:
			out[i] = 'LONG'
			stats['long'] += 1
	elif p == 'time_t' and do_time:
		out[i] = 'LONG'
		stats['time_t'] += 1
	i += 1

new = ''.join(out)
if new != text:
	open(path, 'wb').write(new.encode('latin1'))
print(path, stats)
