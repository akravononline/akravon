# Cynex: tek baytli windows kod sayfasi tablolari - python codec'lerinden (microsoft tanimlari)
import sys

out = sys.argv[1]
pages = [874, 1250, 1251, 1252, 1253, 1254, 1255, 1256, 1257, 1258]

lines = ['// Cynex: tools/gen_codepages.py uretti - 0x80..0xFF -> unicode (tanimsiz bayt = kendisi, windows gibi)', '#pragma once', '#include <stdint.h>', '']
for cp in pages:
	vals = []
	for b in range(128, 256):
		try:
			vals.append(ord(bytes([b]).decode('cp%d' % cp)))
		except UnicodeDecodeError:
			vals.append(b)	# Cynex: windows tanimsiz bayti ayni kod noktasina esler
	lines.append('static const uint16_t g_cp%d[128] = {' % cp)
	for i in range(0, 128, 16):
		lines.append('\t' + ', '.join('0x%04X' % v for v in vals[i:i + 16]) + ',')
	lines.append('};')
	lines.append('')
lines.append('static const struct { unsigned cp; const uint16_t* table; } g_cynexCodePages[] = {')
for cp in pages:
	lines.append('\t{ %d, g_cp%d },' % (cp, cp))
lines.append('};')
open(out, 'w', encoding='ascii', newline='\n').write('\n'.join(lines) + '\n')
print('yazildi', out, len(pages), 'kod sayfasi')
