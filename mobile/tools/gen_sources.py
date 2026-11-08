# Cynex: vcxproj'lardaki ClCompile listesinden mobile/client/sources.cmake uret
# PC ile mobil ayni dosyalari derlesin diye liste elle tutulmuyor
import os, re, sys

client = sys.argv[1]   # .../client
out = sys.argv[2]      # .../mobile/client/sources.cmake

libs = ['EterBase', 'EterLocale', 'EterPack', 'EterImageLib', 'EterLib', 'EterGrnLib', 'EffectLib',
	'GameLib', 'MilesLib', 'PRTerrainLib', 'ScriptLib', 'SpeedTreeLib', 'SphereLib', 'EterPythonLib',
	'UserInterface']

lines = ['# Cynex: tools/gen_sources.py uretti, elle degistirme', '']
for lib in libs:
	proj = os.path.join(client, 'vs_files', lib, lib + '.vcxproj')
	s = open(proj, encoding='utf-8-sig', errors='replace').read()
	files = []
	for m in re.finditer(r'<ClCompile Include="([^"]+)"\s*(/>|>(.*?)</ClCompile>)', s, re.S):
		inc, body = m.group(1), m.group(3) or ''
		if re.search(r"<ExcludedFromBuild[^>]*Release\|Win32[^>]*>true", body):
			continue
		p = os.path.normpath(os.path.join(os.path.dirname(proj), inc))
		if not os.path.isfile(p):
			print('YOK:', lib, inc)
			continue
		rel = os.path.relpath(p, client).replace('\\', '/')
		files.append(rel)
	lines.append('set(%s_SOURCES' % lib)
	lines += ['\t${CLIENT}/%s' % f for f in files]
	lines.append(')')
	lines.append('')
	print('%-14s %3d dosya' % (lib, len(files)))

os.makedirs(os.path.dirname(out), exist_ok=True)
open(out, 'w', encoding='ascii', newline='\n').write('\n'.join(lines))
