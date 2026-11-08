#!/usr/bin/env bash
# Cynex: test icin oyun verisini ldplayer/telefona gonder
# kullanim: bash push_data.sh [istemci_klasoru]
set -uo pipefail
export MSYS_NO_PATHCONV=1

ADB="C:/Android/Sdk/platform-tools/adb.exe"
DEV="${CYNEX_DEV:-emulator-5554}"
CLIENT="${1:-C:/Barbaros2}"
GR2="C:/CynexMobilBuild/granny/b2pack64"
SPT="C:/CynexMobilBuild/speedtree/converted/b2pack"
DST="/sdcard/Android/data/com.cynex.mobil/files"
A() { "$ADB" -s "$DEV" "$@"; }

A shell mkdir -p "$DST/mobile" "$DST/fonts" "$DST/mark" "$DST/upload" "$DST/temp" "$DST/data"

echo "== istemci dosyalari"
for f in pack lib bgm config locale.cfg locale_list.txt channel.inf data/svside data/symbol data/miles; do
	[ -e "$CLIENT/$f" ] && A push "$CLIENT/$f" "$DST/$(dirname $f)/" > /dev/null && echo "  $f"
done

# Cynex: python standart kutuphanesi (pc lib'inde posixpath yok)
A push "C:/CynexMobil/mobile/thirdparty/python27/Lib/." "$DST/lib/" > /dev/null && echo "  python lib"
A push "C:/CynexMobil/mobile/pylib/." "$DST/lib/" > /dev/null && echo "  mobil python eklentileri"
A shell rm -f "$DST/lib/_locale.pyc"	# Cynex: pc'deki sahte _locale python'u bozuyor

# Cynex: mobilde 1280x720 cizim, ekrana olceklenir
TMP="C:/CynexMobilBuild/metin2_mobile.cfg"
sed -e 's/^WIDTH.*/WIDTH\t\t\t\t\t\t1280/' -e 's/^HEIGHT.*/HEIGHT\t\t\t\t\t\t720/' -e 's/^BPP.*/BPP\t\t\t\t\t\t32/' \
	-e 's/^WINDOWED.*/WINDOWED\t\t\t\t1/' "$CLIENT/metin2.cfg" > "$TMP"
A push "$TMP" "$DST/metin2.cfg" > /dev/null
rm -f "$TMP"

echo "== donusturulmus modeller/agaclar (mobile/)"
for root in "$GR2" "$SPT"; do
	for pack in "$root"/*/; do
		for child in "$pack"*; do
			[ -d "$child" ] && A push "$child" "$DST/mobile/" > /dev/null && echo "  $(basename "$pack")/$(basename "$child")"
		done
	done
done

# Cynex: serbest lisansli yazi tipleri (wine tahoma, liberation sans = arial)
echo "== yazi tipleri"
A push "C:/CynexMobil/mobile/data/fonts/." "$DST/fonts/" > /dev/null && echo "  fonts"

# Cynex: dokunmatik arayuz (mobile/ui/build_mobile_ui.py ciktisi) - pack'teki ayni adli dosyalarin yerine gecer
UIDATA="C:/CynexMobilBuild/mobile_ui_data"
if [ -d "$UIDATA" ]; then
	echo "== mobil arayuz"
	A push "$UIDATA/." "$DST/mobile/" > /dev/null && echo "  mobile_ui_data"
fi

A shell du -sh "$DST"
