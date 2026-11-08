#!/usr/bin/env bash
# Cynex: ldplayer kisayollari
# bash ld.sh kur | baslat | kapat | log [sn] | ekran <png> | dokun x y | kaydir x1 y1 x2 y2 [ms]
set -uo pipefail

ADB="C:/Android/Sdk/platform-tools/adb.exe"
DEV="${CYNEX_DEV:-emulator-5554}"
PKG="com.cynex.mobil"
APK="C:/CynexMobilBuild/apk/CynexMobil.apk"
A() { "$ADB" -s "$DEV" "$@"; }

case "${1:-}" in
	kur)     A install -r "$APK" ;;
	baslat)  A shell am force-stop "$PKG"; A logcat -c; A shell am start -n "$PKG/.CynexActivity" > /dev/null ;;
	kapat)   A shell am force-stop "$PKG" ;;
	log)     sleep "${2:-0}"; A logcat -d -v time -s CynexMobil:* CynexOyun:* AndroidRuntime:E DEBUG:* libc:F python:* ;;
	ekran)   A exec-out screencap -p > "$2" ;;
	dokun)   A shell input tap "$2" "$3" ;;
	kaydir)  A shell input swipe "$2" "$3" "$4" "$5" "${6:-300}" ;;
	pid)     A shell pidof "$PKG" ;;
	*)       echo "kullanim: ld.sh kur|baslat|kapat|log|ekran|dokun|kaydir|pid"; exit 1 ;;
esac
