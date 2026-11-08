# Cynex: uygulama simgesi
import os, sys
from PIL import Image, ImageDraw, ImageFont

out_root = sys.argv[1]
sizes = {"mdpi": 48, "hdpi": 72, "xhdpi": 96, "xxhdpi": 144, "xxxhdpi": 192}
font_path = r"C:\Windows\Fonts\georgiab.ttf"

for dpi, px in sizes.items():
	s = px * 4  # Cynex: 4x ciz, kucult
	img = Image.new("RGBA", (s, s), (0, 0, 0, 0))
	d = ImageDraw.Draw(img)
	d.rounded_rectangle((0, 0, s - 1, s - 1), radius=s // 5, fill=(14, 16, 30, 255))
	d.rounded_rectangle((s // 24, s // 24, s - 1 - s // 24, s - 1 - s // 24), radius=s // 6,
		outline=(212, 168, 74, 255), width=max(2, s // 40))
	f = ImageFont.truetype(font_path, int(s * 0.62))
	box = d.textbbox((0, 0), "C", font=f)
	w, h = box[2] - box[0], box[3] - box[1]
	d.text(((s - w) / 2 - box[0], (s - h) / 2 - box[1]), "C", font=f, fill=(232, 190, 96, 255))
	img = img.resize((px, px), Image.LANCZOS)
	path = os.path.join(out_root, "mipmap-" + dpi)
	os.makedirs(path, exist_ok=True)
	img.save(os.path.join(path, "ic_launcher.png"))
print("simge uretildi:", ", ".join(sizes))
