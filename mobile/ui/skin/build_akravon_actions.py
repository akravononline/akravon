import os

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.join(HERE, "ikonlar", "action_hd")
EMOTION_IDS = tuple(range(1, 19)) + (51, 52, 53)
SUPPORT_IDS = (122, 131, 130, 121, 163, 167, 180, 179, 164, 165, 166, 178)
EMOTION_BOXES = (
	(42, 52, 199, 192), (291, 52, 198, 192), (543, 52, 200, 192),
	(793, 52, 200, 192), (1049, 52, 196, 191), (1296, 52, 195, 191),
	(41, 274, 198, 195), (291, 275, 198, 194), (542, 274, 198, 196),
	(793, 274, 199, 195), (1047, 275, 198, 195), (1296, 275, 195, 194),
	(41, 500, 200, 195), (291, 501, 198, 193), (542, 500, 199, 195),
	(793, 500, 200, 194), (1047, 501, 198, 194), (1295, 501, 197, 193),
	(40, 732, 199, 194), (286, 731, 204, 198), (538, 731, 207, 198),
)


def RoundIcon(source, box):
	x, y, width, height = box
	image = source.crop((x - 2, y - 2, x + width + 2, y + height + 2)).resize(
		(256, 256), Image.LANCZOS)
	mask = Image.new("L", (1024, 1024), 0)
	ImageDraw.Draw(mask).ellipse((7, 7, 1016, 1016), fill=255)
	image.putalpha(mask.resize((256, 256), Image.LANCZOS))
	return image


def SquareEmotionIcon(source, box):
	from build_akravon_charwin import Box, GOLD_DARK

	x, y, width, height = box
	image = source.crop((x, y, x + width, y + height)).convert("RGB")
	background = (8, 13, 19)
	pixels = []
	for offset, (red, green, blue) in enumerate(image.getdata()):
		px, py = offset % width, offset // width
		radius = ((px + 0.5 - width / 2.0) / width) ** 2 + (
			(py + 0.5 - height / 2.0) / height) ** 2
		if radius > 0.45 ** 2 or (
				red > 70 and red > green * 1.12 and green > blue * 1.3):
			pixels.append(background)
		else:
			weight = min(1.0, max(0.0, (max(red, green, blue) - 25) / 35.0))
			pixels.append(tuple(int(base + (value - base) * weight)
				for value, base in zip((red, green, blue), background)))
	image.putdata(pixels)
	image = image.crop((int(width * 0.05), int(height * 0.05),
		width - int(width * 0.05), height - int(height * 0.05)))
	image = image.resize((40, 40), Image.LANCZOS).convert("RGBA")
	image.alpha_composite(Box(40, 40, GOLD_DARK, (0, 0, 0, 0), 5))
	return image


def Build(out):
	emotions = Image.open(os.path.join(SOURCE, "emotions.png")).convert("RGBA")
	support = Image.open(os.path.join(SOURCE, "support_sheet.png")).convert("RGBA")
	if emotions.size != (1536, 1024):
		raise ValueError("Unexpected approved emotion atlas dimensions")
	if support.size != (1155, 1362):
		raise ValueError("Unexpected approved support atlas dimensions")
	folder = os.path.join(out, "mobile_ui", "akravon", "actions")
	os.makedirs(folder, exist_ok=True)
	for index, box in zip(EMOTION_IDS, EMOTION_BOXES):
		icon = RoundIcon(emotions, box)
		SquareEmotionIcon(emotions, box).save(
			os.path.join(folder, "emotion_%d_square_40.png" % index))
		for size in (32, 64):
			icon.resize((size, size), Image.LANCZOS).save(
				os.path.join(folder, "emotion_%d_%d.png" % (index, size)))
	for i, index in enumerate(SUPPORT_IDS):
		x = (34, 222, 406, 590, 776, 963)[i % 6]
		y = 893 if i < 6 else 1077
		icon = RoundIcon(support, (x, y, 156, 154))
		for size in (64,):
			icon.resize((size, size), Image.LANCZOS).save(
				os.path.join(folder, "support_%d_%d.png" % (index, size)))


if __name__ == "__main__":
	import sys
	Build(sys.argv[1])
