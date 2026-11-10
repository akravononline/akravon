# -*- coding: utf-8 -*-
# AKRAVON iOS gonderici: kaynak = PROJE klasoru (3_Kaynak_Kod/mobile). Cihaza acik dosya olarak yazar (zip'ler degismez).
#
#   python ios_gonder.py --ui uimobileoptions.py uiakravonquest.py   proje/ui altindaki UTF-8 betikleri cevirip gonderir
#   python ios_gonder.py --orijinal                                  ui/orijinalden_degisen/* (karakter, biyolog, mesajlasma ...) gonderir
#   python ios_gonder.py --gorsel                                    gorseller/ agacini (yeni ikon / tahta gorselleri) gonderir
#   python ios_gonder.py --karakter                                  karakter penceresini yeniden uretir (skin/build_akravon_charwin.py) ve gonderir
#   python ios_gonder.py --baslat                                    oyunu yeniden baslatir (her gonderimden sonra gerekli)
#   python ios_gonder.py --kontrol ...                               hicbir sey gondermez, ne gidecegini listeler
# Birlesik kullanim: python ios_gonder.py --ui a.py --orijinal --karakter --baslat
import asyncio
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
MOBILE = os.path.abspath(os.path.join(HERE, '..'))
UI = os.path.join(MOBILE, 'ui')
ORIJINAL = os.path.join(UI, 'orijinalden_degisen')
GORSEL = os.path.join(MOBILE, 'gorseller')
BUILD = os.path.join(MOBILE, 'derleme')
UI_OUT = 'C:/AkravonTest/ui_out'		# karakter penceresi derleyicisinin okudugu / yazdigi klasor (picker_panel.png vb.)
BID = 'com.akravon.online.5UDV2DSXPV'
DEVICE_MOBILE = '/Documents/mobile'
ESCAPE_NAMES = set(('uiakravonbulksell.py', 'uirank.py'))		# cp949 okuyan betikler: yuksek baytlar \xNN kacisi
# orijinalden_degisen icinde cihazda alt klasore giden dosyalar
SUBFOLDER = {'characterwindow.py': 'uiscript', 'inventorywindowex.py': 'uiscript'}


def Convert(name):
	# UTF-8 kaynak -> cihazin okudugu cp1254 betik (python 2.7 sozdizimi kontrollu)
	sys.path.insert(0, UI)
	import build_mobile_ui as builder
	text = open(os.path.join(UI, name), encoding='utf-8').read()
	text = builder.ToRoot(builder.StripComments(text))
	data = text.encode('cp1254', errors='replace') if isinstance(text, str) else text
	if name in ESCAPE_NAMES:
		data = re.sub(rb'[\x80-\xff]', lambda m: b'\\x%02x' % m.group(0)[0], data)
	import warnings
	warnings.simplefilter('ignore')
	from lib2to3 import pygram, pytree
	from lib2to3.pgen2 import driver
	checked = data.decode('cp1254').replace('\r\n', '\n')
	driver.Driver(pygram.python_grammar, convert=pytree.convert).parse_string(checked + '\n')
	return data


class Device(object):
	def __init__(self, dry):
		self.dry = dry
		self.ha = None
		self.count = 0

	async def Open(self):
		if self.dry:
			return
		from pymobiledevice3.lockdown import create_using_usbmux
		from pymobiledevice3.services.house_arrest import HouseArrestService
		lockdown = await create_using_usbmux()
		self.ha = await HouseArrestService.create(lockdown, BID)

	async def Put(self, path, data):
		self.count += 1
		print(('[kontrol] ' if self.dry else 'gonder ') + path + ' (%d bayt)' % len(data))
		if not self.dry:
			await self.ha.set_file_contents(path, data)

	async def MakeDirs(self, path):
		if not self.dry:
			try:
				await self.ha.makedirs(path)
			except Exception:
				pass


async def PushTree(device, folder, target):
	made = set()
	for (root, dirs, files) in os.walk(folder):
		rel = os.path.relpath(root, folder).replace(os.sep, '/')
		here = target.rstrip('/') + ('' if rel == '.' else '/' + rel)
		if here not in made:
			await device.MakeDirs(here)
			made.add(here)
		for name in sorted(files):
			await device.Put(here + '/' + name, open(os.path.join(root, name), 'rb').read())


async def Main(arguments):
	dry = '--kontrol' in arguments
	device = Device(dry)
	await device.Open()
	index = 0
	while index < len(arguments):
		flag = arguments[index]
		index += 1
		if flag == '--ui':
			while index < len(arguments) and not arguments[index].startswith('--'):
				name = arguments[index]
				index += 1
				await device.Put(DEVICE_MOBILE + '/' + name, Convert(name))
		elif flag == '--orijinal':
			for name in sorted(os.listdir(ORIJINAL)):
				sub = SUBFOLDER.get(name)
				await device.Put(DEVICE_MOBILE + ('/' + sub if sub else '') + '/' + name, open(os.path.join(ORIJINAL, name), 'rb').read())
		elif flag == '--gorsel':
			await PushTree(device, GORSEL, DEVICE_MOBILE)
		elif flag == '--karakter':
			# yeniden uret -> proje icindeki kopyalara yaz -> gonder
			subprocess.check_call([sys.executable, os.path.join(UI, 'skin', 'build_akravon_charwin.py'), UI_OUT])
			import shutil
			shutil.copy(UI_OUT + '/uiscript/characterwindow.py', os.path.join(ORIJINAL, 'characterwindow.py'))
			legacy = os.path.join(GORSEL, 'ymir work', 'ui', 'akravon_legacy')
			source = UI_OUT + '/ymir work/ui/akravon_legacy/'
			for name in os.listdir(legacy):
				if name.startswith('cw_') and os.path.exists(source + name):
					shutil.copy(source + name, os.path.join(legacy, name))
					await device.Put(DEVICE_MOBILE + '/ymir work/ui/akravon_legacy/' + name, open(os.path.join(legacy, name), 'rb').read())
			await device.Put(DEVICE_MOBILE + '/uiscript/characterwindow.py', open(os.path.join(ORIJINAL, 'characterwindow.py'), 'rb').read())
		elif flag == '--baslat':
			if not dry:
				subprocess.call([sys.executable, '-m', 'pymobiledevice3', 'developer', 'dvt', 'launch', BID, '--kill-existing'])
			else:
				print('[kontrol] oyun yeniden baslatilir')
	print('toplam', device.count, 'dosya', '(kontrol - gonderilmedi)' if dry else 'gonderildi')


if __name__ == '__main__':
	if len(sys.argv) < 2:
		print(__doc__)
		sys.exit(0)
	asyncio.run(Main(sys.argv[1:]))
