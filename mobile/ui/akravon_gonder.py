# AKRAVON: degisen arayuz dosyalarini ldplayer'a acik dosya olarak gonderir (zip/apk yeniden derlenmez)
# cihazda <oyun>/mobile/ altindaki acik dosyalar mobile/*.zip arsivlerinden once okunur.
# kullanim: py -3 akravon_gonder.py            (gonder)
#           py -3 akravon_gonder.py --geri-al  (gonderilenleri cihazdan sil -> zip'teki eski surume doner)
import os
import subprocess
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_mobile_ui as builder

ADB = 'C:/LDPlayer/LDPlayer9/adb.exe'
DEVICE = os.environ.get('CYNEX_DEV', 'emulator-5554')
DST = '/sdcard/Android/data/com.cynex.mobil/files/mobile'
OUT = 'C:/AkravonTest/ui_out'

# AKRAVON asamalarinda degisen betikler
SCRIPTS = ['uiakravon.py', 'uimobilelogin.py', 'uimobilestyle.py', 'uimobileselect.py', 'uimobilecreate.py', 'uimobilelocale.py', 'uimobileloading.py', 'uimobilechrome.py', 'uimobilechat.py', 'uimobilehud.py', 'uimobilehudedit.py', 'uiakravonatlas.py', 'uimobilehudcontroller.py', 'uimobilesystem.py', 'uimobileoptions.py', 'uiattackbutton.py', 'uivirtualjoystick.py', 'uiakravonskillicon.py', 'uiakravonbonus.py', 'uiakravonautohunt.py', 'uiakravoninventory.py', 'uiakravonsafebox.py', 'uimobileempire.py', 'uiakravonpazar.py', 'uiakravonpopup.py']
ASSET_DIRS = ['ikashop/lite', 'mobile_ui/akravon', 'ymir work/ui/akravon_skill', 'ymir work/ui/akravon_skill_square', 'ymir work/ui/skill', 'ymir work/ui/akravon_legacy', 'dragonetc/offical', 'ymir work/ui/pattern', 'ymir work/ui/public', 'ymir work/ui/game/windows', 'ymir work/ui/game/bio', 'locale/tr/ui/windows', 'uiscript', 'characterdetails']
UI_ZIP = os.path.join(HERE, '..', '..', '..', '2_Oyun_Dosyalari', 'files', 'mobile', '00_cynexui_1.zip')
ROOT_ZIP = os.path.join(HERE, '..', '..', '..', '2_Oyun_Dosyalari', 'files', 'mobile', '10_root_1.zip')
PATCHED = []	# zip'teki (barbaros yamali) dosya + patches/akravon_*.txt
CRLF = chr(13) + chr(10)
LF = chr(10)


def Adb(*args):
	subprocess.check_call([ADB, '-s', DEVICE] + list(args))


def BuildPatched():
	# zip'teki calisan surum + yalniz AKRAVON yamalari (orijinal derleyici de patches/ altindan uygular)
	folder = os.path.join(HERE, 'patches')
	archive = zipfile.ZipFile(UI_ZIP)
	rootArchive = zipfile.ZipFile(ROOT_ZIP)
	texts = {}
	for name in sorted(os.listdir(folder)):
		if not (name.startswith('akravon_') and name.endswith('.txt')):
			continue
		for (target, old, new) in builder.ParseSpec(os.path.join(folder, name)):
			if target not in texts:
				source = archive if target in archive.namelist() else rootArchive
				texts[target] = source.read(target).decode('latin1')
			text = texts[target]
			if CRLF in text:
				old = old.replace(LF, CRLF)
				new = new.replace(LF, CRLF)
			if text.count(old) != 1:
				raise SystemExit('%s: yama eslesmedi: %r' % (target, old[:80]))
			texts[target] = text.replace(old, new)
	del PATCHED[:]
	for (target, text) in texts.items():
		open(os.path.join(OUT, target), 'wb').write(text.encode('latin1'))
		PATCHED.append(target)


def Build():
	os.makedirs(OUT, exist_ok = True)
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_login.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_hud.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_skillicons.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_usericons.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_legacy.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_charlabels.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_charwin.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_bonus.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_autohunt.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_inventory.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_safebox.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_invbuttons.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_empire.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_atlas.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_menuicons.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_ikashop.py'), OUT])
	subprocess.check_call([sys.executable, os.path.join(HERE, 'skin', 'build_akravon_bio.py'), OUT])
	# betikler uretimden SONRA kopyalanir (uiakravonskillicon.py uretilen tabloyu icerir)
	for name in SCRIPTS:
		text = builder.StripComments(open(os.path.join(HERE, name), encoding = 'utf-8').read())
		open(os.path.join(OUT, name), 'wb').write(builder.ToRoot(text))
	BuildPatched()
	builder.OUT = OUT
	if not builder.CheckSyntax():
		raise SystemExit('python 2.7 sozdizimi hatasi - gonderilmedi')


def Push():
	for name in SCRIPTS + PATCHED:
		Adb('push', os.path.join(OUT, name), DST + '/' + name)
	for folder in ASSET_DIRS:
		Adb('shell', 'mkdir -p "%s/%s"' % (DST, folder))
		Adb('push', os.path.join(OUT, folder) + '/.', DST + '/' + folder + '/')


def Revert():
	for name in SCRIPTS + ['networkmodule.py', 'uicharacter.py']:
		Adb('shell', 'rm -f %s/%s' % (DST, name))
	for folder in ASSET_DIRS:
		Adb('shell', 'rm -rf "%s/%s"' % (DST, folder))


if __name__ == '__main__':
	if '--geri-al' in sys.argv:
		Revert()
	else:
		Build()
		Push()
	print('tamam')
