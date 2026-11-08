# AKRAVON: beceri yuvasi yuvarlagi - kullanicinin yuvarlak cerceve referansindan (gercek saydam) uretilir
# kaynak: skin/hud_circle_reference.png ; 4 kenardaki ok susleri kutupsal acilimda temiz parcayla degistirilir
import os
import math
import numpy as np
import cv2
from PIL import Image, ImageDraw, ImageFilter
from build_akravon_kit import IvoryIcon

HERE = os.path.dirname(os.path.abspath(__file__))
CIRCLE_REFERENCE = os.path.join(HERE, 'hud_circle_reference.png')


def BuildCircle(save):
	ref = np.asarray(Image.open(CIRCLE_REFERENCE).convert('RGBA')).copy()
	alpha = ref[..., 3] > 128
	ys, xs = np.where(alpha)
	cx, cy = xs.mean(), ys.mean()
	radii = []
	for angle in (45, 135, 225, 315):
		for r in range(100, 600):
			x, y = int(round(cx + r * np.cos(np.radians(angle)))), int(round(cy + r * np.sin(np.radians(angle))))
			if not (0 <= x < ref.shape[1] and 0 <= y < ref.shape[0]) or not alpha[y, x]:
				radii.append(r)
				break
	radius = float(np.mean(radii))
	maxRadius = radius + 60
	rows = 3600
	polar = cv2.warpPolar(ref, (int(maxRadius), rows), (cx, cy), maxRadius, cv2.WARP_POLAR_LINEAR)
	fixed = polar.copy()
	half, shift = 90, 450	# 9 derece; 45 derece oteden temiz parca
	for center in (0, 900, 1800, 2700):
		for row in range(center - half, center + half + 1):
			weight = min(1.0, (half - abs(row - center)) / 10.0)
			fixed[row % rows] = (polar[row % rows] * (1 - weight) + polar[(row + shift) % rows] * weight).astype(np.uint8)
	restored = cv2.warpPolar(fixed, (ref.shape[1], ref.shape[0]), (cx, cy), maxRadius, cv2.WARP_POLAR_LINEAR | cv2.WARP_INVERSE_MAP)
	yy, xx = np.mgrid[0:ref.shape[0], 0:ref.shape[1]]
	dist = np.hypot(xx - cx, yy - cy)
	restored[..., 3] = np.where(dist < radius + 2, restored[..., 3], 0)

	side = int(radius + 6) * 2
	left, top = int(round(cx - side / 2.0)), int(round(cy - side / 2.0))
	disc = Image.fromarray(restored).crop((left, top, left + side, top + side))
	save('hud_disc', disc.resize((256, 256), Image.LANCZOS))
	save('hud_lock_disc', disc.resize((128, 128), Image.LANCZOS))

	# halka: yalnizca cerceve seridi (ikonun kenarini toplar)
	band = np.asarray(disc.resize((256, 256), Image.LANCZOS)).copy()
	y2, x2 = np.mgrid[0:256, 0:256]
	band[..., 3] = np.where(np.hypot(x2 - 127.5, y2 - 127.5) > 128 * 0.9, band[..., 3], 0)
	save('hud_ring', Image.fromarray(band))
	BuildApprovedSkillSlot(save)
	return disc.resize((256, 256), Image.LANCZOS)


def BuildApprovedSkillSlot(save):
	path = os.path.join(HERE, 'ikonlar', 'hud_approved', 'skill_slot.png')
	slot = Image.open(path).convert('RGBA')
	bounds = slot.getchannel('A').point(lambda value: 255 if value > 128 else 0).getbbox()
	if not bounds:
		raise ValueError('Approved skill slot has no visible frame')
	left, top, right, bottom = bounds
	side = max(right - left, bottom - top) + 12
	x = int(round((left + right - side) / 2.0))
	y = int(round((top + bottom - side) / 2.0))
	slot = slot.crop((x, y, x + side, y + side))
	ring = np.asarray(slot).copy()
	# Only the metal frame belongs above skill icons; the gray face stays below.
	rgb = ring[..., :3].astype(np.float32)
	gold = (rgb[..., 0] > rgb[..., 1] * 1.05) & (rgb[..., 1] > rgb[..., 2] * 1.2)
	ring[..., 3] = np.where(gold, ring[..., 3], 0)
	save('hud_disc', slot.resize((256, 256), Image.LANCZOS))
	save('hud_ring', Image.fromarray(ring).resize((256, 256), Image.LANCZOS))
	BuildSkillGradePlaque(save, slot)
	BuildSkillControls(save)


def BuildSkillControls(save):
	folder = os.path.join(HERE, 'ikonlar', 'hud_approved')
	base = Image.open(os.path.join(folder, 'base.png')).convert('RGBA')
	left, top, right, bottom = base.getchannel('A').point(lambda a: 255 if a > 128 else 0).getbbox()
	side = max(right - left, bottom - top) + 12
	x, y = int(round((left + right - side) / 2.0)), int(round((top + bottom - side) / 2.0))
	base = base.crop((x, y, x + side, y + side))
	pixels = np.asarray(base).copy()
	rgb = pixels[..., :3].astype(np.float32)
	gold = (rgb[..., 0] > rgb[..., 1] * 1.05) & (rgb[..., 1] > rgb[..., 2] * 1.2) & (rgb[..., 0] > 25)
	pixels[..., :3] = np.where(gold[..., None], np.minimum(rgb * 1.18, 255), rgb).astype(np.uint8)
	base = Image.fromarray(pixels).resize((1024, 1024), Image.LANCZOS)
	# Recolor only the symbol texture; the approved gold rim stays unchanged.
	symbol = Image.open(os.path.join(folder, 'less.png')).convert('RGBA')
	depth = cv2.distanceTransform((np.asarray(symbol)[..., 3] > 240).astype(np.uint8),
		cv2.DIST_L2, 5)
	sy, sx = np.unravel_index(depth.argmax(), depth.shape)
	half = max(1, int(depth[sy, sx] / math.sqrt(2)) - 1)
	sample = symbol.crop((sx - half, sy - half, sx + half, sy + half)).resize(
		(128, 128), Image.BICUBIC)
	metalTexture = np.tile(np.asarray(IvoryIcon(sample))[..., :3], (8, 8, 1)).astype(np.float32)
	for name in ('pages', 'edit'):
		mask = Image.new('L', (1024, 1024), 0)
		d = ImageDraw.Draw(mask)
		if name == 'pages':
			for start in (205, 25):
				d.arc((240, 240, 784, 784), start=start, end=start + 130, fill=255, width=70)
				a = math.radians(start + 130)
				px, py = 512 + 272 * math.cos(a), 512 + 272 * math.sin(a)
				tx, ty = -math.sin(a), math.cos(a)
				nx, ny = math.cos(a), math.sin(a)
				d.polygon([(px + tx * 75, py + ty * 75),
					(px - tx * 60 + nx * 90, py - ty * 60 + ny * 90),
					(px - tx * 60 - nx * 90, py - ty * 60 - ny * 90)], fill=255)
		else:
			d.line((344, 716, 640, 420), fill=255, width=112)
			d.ellipse((288, 660, 400, 772), fill=255)
			d.ellipse((536, 252, 780, 496), fill=255)
			d.polygon([(616, 234), (816, 234), (816, 406), (704, 430), (610, 336)], fill=0)
			d.ellipse((322, 694, 364, 736), fill=0)
		canvas = base.copy()
		distance = cv2.distanceTransform(np.asarray(mask), cv2.DIST_L2, 5)
		bevel = np.minimum(distance / 32.0, 1.0)
		gy, gx = np.gradient(distance)
		light = 0.94 + (gx + gy) * (1 - bevel) * 0.30
		texture = Image.fromarray(np.clip(metalTexture * light[..., None], 0, 255).astype(np.uint8))
		shadow = Image.new('RGBA', canvas.size, (25, 13, 3, 0))
		shadow.putalpha(mask.filter(ImageFilter.GaussianBlur(5)))
		canvas.alpha_composite(shadow, (4, 8))
		canvas.paste(texture, (0, 0), mask)
		save('hud_skill_' + name, canvas.resize((256, 256), Image.LANCZOS))


def BuildSkillGradePlaque(save, slot):
	# Reuse the approved frame's actual metal pixels instead of a new gold palette.
	side = 1024
	source = np.asarray(slot.resize((side, side), Image.LANCZOS))
	yy, xx = np.mgrid[0:448, 0:side]
	dx = xx - np.clip(xx, 220, 803)
	dy = yy - 223.5
	dist = np.hypot(dx, dy)
	angle = np.arctan2(dy, dx)
	radius = 480 + np.clip((dist - 196) / 22.0, 0, 1) * 22
	mapx = (511.5 + radius * np.cos(angle)).astype(np.float32)
	mapy = (511.5 + radius * np.sin(angle)).astype(np.float32)
	metal = cv2.remap(source, mapx, mapy, cv2.INTER_LINEAR)
	canvas = Image.new('RGBA', (1024, 448), (0, 0, 0, 0))
	draw = ImageDraw.Draw(canvas)
	draw.rounded_rectangle((4, 4, 1019, 443), radius=220, fill=(92, 54, 17, 255))
	draw.rounded_rectangle((12, 12, 1011, 435), radius=212, fill=(220, 166, 64, 255))
	mask = Image.new('L', canvas.size, 0)
	md = ImageDraw.Draw(mask)
	md.rounded_rectangle((16, 16, 1007, 431), radius=208, fill=255)
	md.rounded_rectangle((36, 36, 987, 411), radius=188, fill=0)
	canvas.paste(Image.fromarray(metal), (0, 0), mask)
	draw = ImageDraw.Draw(canvas)
	draw.rounded_rectangle((36, 36, 987, 411), radius=188, fill=(42, 19, 17, 255),
		outline=(123, 80, 31, 255), width=5)
	save('hud_skill_grade', canvas.resize((256, 112), Image.LANCZOS))
