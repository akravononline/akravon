# -*- coding: utf-8 -*-
# AKRAVON: ios_gonder.py ile ayni dosya listesini / donusturmeyi ANDROID cihaza (LDPlayer emulatoru veya USB telefon) gonderir.
#   python araclar/android_gonder.py --ui uimobilechrome.py --orijinal --gorsel --baslat
#   ortam degiskeni CYNEX_DEV: adb seri no (varsayilan emulator-5554; OnePlus: 9cdc6a10)
# Dosyalar once yerel bir klasore dizilir, tek zip olarak cihaza gonderilir ve cihazda acilir (tek tek adb push cok yavas / dizinleri dusuruyor).
import asyncio
import os
import shutil
import subprocess
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import ios_gonder as g

ADB = os.environ.get('CYNEX_ADB', 'C:/LDPlayer/LDPlayer9/adb.exe')
DEVICE = os.environ.get('CYNEX_DEV', 'emulator-5554')
APP = 'com.cynex.mobil'
ROOT = '/sdcard/Android/data/%s/files' % APP
STAGE = 'C:/AkravonTest/android_stage'
ZIP = 'C:/AkravonTest/android_stage.zip'
TMP = '/data/local/tmp/akr_gonder.zip'


def Adb(*args, **kwargs):
	return subprocess.run([ADB, '-s', DEVICE] + list(args), capture_output = True, text = True, **kwargs)


class AndroidDevice(object):
	def __init__(self, dry):
		self.dry = dry
		self.count = 0

	async def Open(self):
		if os.path.isdir(STAGE):
			shutil.rmtree(STAGE)
		os.makedirs(STAGE)
		state = Adb('get-state').stdout.strip()
		if not self.dry and state != 'device':
			print('HATA: adb cihaz yok (%s) - seri no: %s' % (state or 'baglanti yok', DEVICE))
			sys.exit(1)

	async def Put(self, path, data):
		# iOS yollari '/Documents/...' ile baslar: Android'de ayni yer uygulamanin files/ klasoru
		rel = path[len('/Documents'):] if path.startswith('/Documents') else path
		target = os.path.join(STAGE, rel.lstrip('/'))
		os.makedirs(os.path.dirname(target), exist_ok = True)
		open(target, 'wb').write(data)
		self.count += 1

	async def MakeDirs(self, path):
		pass

	def Flush(self):
		if self.dry or self.count == 0:
			return
		with zipfile.ZipFile(ZIP, 'w', zipfile.ZIP_STORED) as archive:
			for (root, dirs, files) in os.walk(STAGE):
				for name in files:
					full = os.path.join(root, name)
					archive.write(full, os.path.relpath(full, STAGE).replace(os.sep, '/'))
		print('zip: %d dosya, %.1f MB' % (self.count, os.path.getsize(ZIP) / 1048576.0))
		print(Adb('push', ZIP, TMP).stdout.strip().split('\n')[-1])
		result = Adb('shell', 'cd %s && unzip -o %s > /dev/null && echo TAMAM' % (ROOT, TMP))
		print(result.stdout.strip() or result.stderr.strip())
		Adb('shell', 'rm -f %s' % TMP)


def Launch():
	Adb('shell', 'am', 'force-stop', APP)
	result = Adb('shell', 'monkey', '-p', APP, '-c', 'android.intent.category.LAUNCHER', '1')
	print(result.stdout.strip().split('\n')[-1] if result.stdout.strip() else result.stderr.strip())


def Main(arguments):
	launch = '--baslat' in arguments
	arguments = [a for a in arguments if a != '--baslat']
	device = AndroidDevice('--kontrol' in arguments)
	g.Device = lambda dry: device
	asyncio.run(g.Main(arguments))
	device.Flush()
	if launch:
		Launch()


if __name__ == '__main__':
	if len(sys.argv) < 2:
		print(__doc__)
		sys.exit(0)
	Main(sys.argv[1:])
