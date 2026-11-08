#!/usr/bin/env bash
# Cynex: forum paketi - APK + oyun verisi (files/ klasoru, telefonda Android/data/com.cynex.mobil/files)
# kullanim: bash make_release.sh   (once make_mobile_packs.py ile files/mobile/*.zip uretilmis olmali)
set -euo pipefail

CLIENT="C:/Barbaros2"
ROOT="C:/CynexMobil/mobile"
REL="C:/CynexMobilBuild/release"
F="$REL/files"

mkdir -p "$F/mobile" "$F/lib" "$F/fonts" "$F/data" "$F/mark" "$F/upload" "$F/temp"

echo "== istemci ayarlari"
rm -rf "$F/bgm" "$F/config" "$F/data/svside" "$F/data/symbol" "$F/data/miles"
cp -r "$CLIENT/bgm" "$CLIENT/config" "$F/"
cp "$CLIENT/locale.cfg" "$CLIENT/locale_list.txt" "$CLIENT/channel.inf" "$F/"
cp -r "$CLIENT/data/svside" "$CLIENT/data/symbol" "$CLIENT/data/miles" "$F/data/"
sed -e 's/^WIDTH.*/WIDTH\t\t\t\t\t\t1280/' -e 's/^HEIGHT.*/HEIGHT\t\t\t\t\t\t720/' -e 's/^BPP.*/BPP\t\t\t\t\t\t32/' \
	-e 's/^WINDOWED.*/WINDOWED\t\t\t\t1/' "$CLIENT/metin2.cfg" > "$F/metin2.cfg"

echo "== python"
rm -rf "$F/lib"
mkdir -p "$F/lib"
cp -r "$ROOT/thirdparty/python27/Lib/." "$F/lib/"
cp -r "$ROOT/pylib/." "$F/lib/"
find "$F/lib" -name "*.pyc" -delete

echo "== yazi tipleri"
rm -rf "$F/fonts"
cp -r "$ROOT/data/fonts" "$F/fonts"

# Cynex: sezar2 ozel gorunumu ve test sunucusu ayari pakete girmez
if find "$F" -iname "*sezar*" | grep -q .; then
	echo "HATA: pakette sezar dosyasi var"; exit 1
fi
rm -f "$F/mobile/serverinfo.py" "$F/mobile/tamam.txt"

echo "== apk"
cp "C:/CynexMobilBuild/apk/CynexMobil.apk" "$REL/CynexMobil.apk"

du -sh "$F" "$REL/CynexMobil.apk"
