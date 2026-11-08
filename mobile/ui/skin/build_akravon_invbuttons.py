# AKRAVON: kullanicinin verdigi depo / kostum buton gorselleri -> inv_depo_*.png / inv_kost_*.png (50x50, seffaf zemin)
import os, sys
from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageFilter
HERE = os.path.dirname(os.path.abspath(__file__))
out = sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out'
src = Image.open(os.path.join(HERE, 'src', 'depo_kostum_butonlari.png')).convert('RGB')
bg = Image.new('RGB', src.size, src.getpixel((5, 5)))
diff = ImageChops.difference(src, bg).convert('L').point(lambda p: 255 if p > 14 else 0)
diff = diff.filter(ImageFilter.MaxFilter(5)).filter(ImageFilter.MinFilter(5))
W, H = src.size
def Box(x0, x1):
	part = diff.crop((x0, 0, x1, H))
	return part.getbbox()
half = W // 2
folder = os.path.join(out, 'ymir work', 'ui', 'akravon_legacy')
for (name, x0, x1) in (('depo', 0, half), ('kost', half, W)):
	b = Box(x0, x1)
	crop = src.crop((x0 + b[0], b[1], x0 + b[2], b[3]))
	w, h = crop.size
	c = int(min(w, h) * 0.17)
	mask = Image.new('L', (w * 4, h * 4), 0)
	s = 4
	ImageDraw.Draw(mask).polygon([(c*s, 0), ((w-c)*s, 0), (w*s, c*s), (w*s, (h-c)*s), ((w-c)*s, h*s), (c*s, h*s), (0, (h-c)*s), (0, c*s)], fill = 255)
	mask = mask.resize((w, h), Image.LANCZOS)
	im = crop.convert('RGBA'); im.putalpha(mask)
	im = im.resize((46, 46), Image.LANCZOS)
	over = ImageEnhance.Brightness(im.convert('RGB')).enhance(1.2).convert('RGBA'); over.putalpha(im.getchannel('A'))
	down = ImageEnhance.Brightness(im.convert('RGB')).enhance(0.72).convert('RGBA'); down.putalpha(im.getchannel('A'))
	for (suffix, img) in (('up', im), ('over', over), ('down', down)):
		img.save(os.path.join(folder, 'inv_%s_%s.png' % (name, suffix)))
print('inv butonlari: depo + kostum')
