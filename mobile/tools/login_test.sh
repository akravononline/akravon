#!/usr/bin/env bash
# Cynex: test sunucusuna otomatik giris (ldplayer, mobil giris ekrani; ilk dokunusta panel yukari kayar)
export MSYS_NO_PATHCONV=1
A="C:/Android/Sdk/platform-tools/adb.exe -s ${CYNEX_DEV:-emulator-5554}"
D="$(dirname "$0")"
bash $D/ld.sh baslat; sleep 22
bash $D/ld.sh dokun 1517 375; sleep 1.5; $A shell input text "${1:-cynex}"; sleep 1
bash $D/ld.sh dokun 1517 303; sleep 1; $A shell input text "${2:-cynex123}"; sleep 1
bash $D/ld.sh dokun 1517 400; sleep 1; $A shell input text "${3:-1234}"; sleep 1
bash $D/ld.sh dokun 1368 100; sleep 2
bash $D/ld.sh dokun 708 514; sleep 1
bash $D/ld.sh dokun 960 814; sleep 1.5
bash $D/ld.sh dokun 1517 564
