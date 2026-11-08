# AKRAVON: kullanimdaki esya gostergesi icin yumusak mavi disk (128 px) -> mobile_ui/akravon/hud/hud_active_glow.png
import os, sys, math
from PIL import Image
out = sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out'
N = 128
im = Image.new('RGBA', (N, N), (0, 0, 0, 0))
px = im.load()
c = (N - 1) / 2.0
for y in range(N):
	for x in range(N):
		d = math.hypot(x - c, y - c) / c
		if d >= 1.0:
			continue
		edge = min(1.0, max(0.0, (1.0 - d) / 0.12))		# kenarda yumusak bitis
		body = 0.55 + 0.45 * d							# kenara dogru biraz daha yogun: gri kenar kapansin
		px[x, y] = (96, 160, 255, int(255 * edge * body * 0.85))
path = os.path.join(out, 'mobile_ui', 'akravon', 'hud', 'hud_active_glow.png')
os.makedirs(os.path.dirname(path), exist_ok = True)
im.save(path)
print(path)

# dalga halkasi: yumusak ince halka (genisleyen elektrik dalgalanmasi icin)
ring = Image.new('RGBA', (N, N), (0, 0, 0, 0))
rp = ring.load()
for y in range(N):
	for x in range(N):
		d = math.hypot(x - c, y - c) / c
		k = max(0.0, 1.0 - abs(d - 0.86) / 0.16)
		if k > 0:
			rp[x, y] = (150, 205, 255, int(255 * k * k))
rpath = os.path.join(out, 'mobile_ui', 'akravon', 'hud', 'hud_active_ripple.png')
ring.save(rpath)
print(rpath)
