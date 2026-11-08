import json
import os

from PIL import Image, ImageDraw


HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.join(HERE, 'ikonlar', 'skill_hd')
STAGES = ('1lv', 'm1', 'g1', 'p')


def BorderlessRound(image, size):
	# The source artwork includes a square metal rim within its outer 24 pixels.
	content = image.crop((24, 24, 232, 232))
	mask = Image.new('L', (832, 832), 0)
	ImageDraw.Draw(mask).ellipse((4, 4, 827, 827), fill=255)
	mask = mask.resize(content.size, Image.LANCZOS)
	from PIL import ImageChops
	content.putalpha(ImageChops.multiply(content.getchannel('A'), mask))
	return content.resize((size, size), Image.LANCZOS)


def BuildApprovedSkills(outDir, table):
	with open(os.path.join(SOURCE, 'manifest.json'), encoding='utf-8') as stream:
		manifest = json.load(stream)
	records = manifest['icons']
	if len(records) != 216:
		raise ValueError('Expected 216 approved skill images')
	made = set()
	approved = set()
	for record in records:
		skillId = record['id']
		base = record['class'] + '/' + record['stem']
		if table.get(skillId) != base:
			raise ValueError('Skill mapping mismatch: %s' % skillId)
		stage = STAGES.index(record['stage']) + 1
		name = base + '_%02d' % stage
		image = Image.open(os.path.join(SOURCE, record['files']['256'])).convert('RGBA')
		if image.size != (256, 256):
			raise ValueError('Expected 256px source: %s' % name)
		roundIcon = BorderlessRound(image, 64)
		roundPath = os.path.join(outDir, 'ymir work', 'ui', 'akravon_skill', name + '.png')
		os.makedirs(os.path.dirname(roundPath), exist_ok=True)
		roundIcon.save(roundPath)
		square = BorderlessRound(image, 32)
		squarePath = os.path.join(outDir, 'ymir work', 'ui', 'akravon_skill_square', name + '.png')
		os.makedirs(os.path.dirname(squarePath), exist_ok=True)
		square.save(squarePath)
		if stage <= 3:
			subPath = os.path.join(outDir, 'ymir work', 'ui', 'skill', name + '.sub')
			os.makedirs(os.path.dirname(subPath), exist_ok=True)
			with open(subPath, 'w', encoding='ascii', newline='\r\n') as stream:
				stream.write('title subImage\nversion 1.0\nimage "akravon_skill_square/%s.png"\n'
							 'left 0\ntop 0\nright 32\nbottom 32\n' % name)
		made.add(name)
		approved.add(skillId)
	if len(made) != 216 or len(approved) != 54:
		raise ValueError('Incomplete approved skill set')
	return made, approved
