# AKRAVON: oyunun varsayilan yazi tipi Tahoma -> Lato (locale/tr/locale_game.txt, gevsek dosya)
import zipfile,os
zf=zipfile.ZipFile('C:/Users/YUSUF/Downloads/CynexMobil_v1/CynexMobil_v1/2_Oyun_Dosyalari/files/mobile/11_locale_1.zip')
raw=zf.read('locale/tr/locale_game.txt')
new={b'UI_DEF_FONT\t':b'Lato:15',b'UI_DEF_FONT_LARGE\t':b'Lato:17',b'UI_DEF_FONT_SMALL\t':b'Lato:13'}
lines=raw.split(b'\n'); n=0
for i,l in enumerate(lines):
    for k,v in new.items():
        if l.startswith(k):
            lines[i]=k+v+(b'\r' if l.endswith(b'\r') else b''); n+=1
out='C:/AkravonTest/ui_out/locale/tr/locale_game.txt'
os.makedirs(os.path.dirname(out),exist_ok=True)
open(out,'wb').write(b'\n'.join(lines)); print('degisen',n)
