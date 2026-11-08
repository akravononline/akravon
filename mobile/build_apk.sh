#!/usr/bin/env bash
# Cynex Mobil - apk build
# kullanim: bash build_apk.sh [Release|Debug]
set -euo pipefail

CONFIG="${1:-Release}"
ROOT="${CYNEX_ROOT:-C:/CynexMobil/mobile}"   # Cynex: turkce karakterli yolda llvm-ar patliyor
OUT="C:/CynexMobilBuild/apk"
SDK="C:/Android/Sdk"
NDK="$SDK/ndk/27.2.12479018"
BT="$SDK/build-tools/36.1.0"
JAR="$SDK/platforms/android-36/android.jar"
JDK="C:/Android/jdk-21/bin"
CMAKE_BIN="$SDK/cmake/3.22.1/bin"
ABIS="${CYNEX_ABIS:-arm64-v8a x86_64}"      # Cynex: telefon + emulator

export PATH="/c/Android/Sdk/cmake/3.22.1/bin:$PATH"

echo "== 1/6 yerel kitaplik ($CONFIG)"
for ABI in $ABIS; do
	B="C:/CynexMobilBuild/native-$ABI-$CONFIG"
	cmake -S "$ROOT/native" -B "$B" -G Ninja \
		-DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
		-DANDROID_ABI="$ABI" -DANDROID_PLATFORM=android-26 -DANDROID_STL=c++_static \
		-DCMAKE_BUILD_TYPE="$CONFIG" > /dev/null
	cmake --build "$B" -j 12
	mkdir -p "$OUT/stage/lib/$ABI"
	cp "$B/libcynexmobil.so" "$OUT/stage/lib/$ABI/"
	if [ "$CONFIG" = "Release" ]; then
		"$NDK/toolchains/llvm/prebuilt/windows-x86_64/bin/llvm-strip.exe" --strip-unneeded "$OUT/stage/lib/$ABI/libcynexmobil.so"
	fi
done

echo "== 2/6 kaynaklar"
rm -rf "$OUT/res" "$OUT/gen" "$OUT/classes" "$OUT/dex"
mkdir -p "$OUT/res" "$OUT/gen" "$OUT/classes" "$OUT/dex"
python "$ROOT/tools/gen_icon.py" "$OUT/res_src"
"$BT/aapt2.exe" compile --dir "$OUT/res_src" -o "$OUT/res/res.zip"
"$BT/aapt2.exe" link -o "$OUT/base.apk" -I "$JAR" --manifest "$ROOT/android/AndroidManifest.xml" \
	--java "$OUT/gen" "$OUT/res/res.zip" --auto-add-overlay

echo "== 3/6 java"
"$JDK/javac.exe" -source 11 -target 11 -nowarn -encoding UTF-8 -classpath "$JAR" -d "$OUT/classes" \
	$(find "$ROOT/android/java" "$OUT/gen" -name '*.java')
"$BT/d8.bat" --release --min-api 26 --lib "$JAR" --output "$OUT/dex" $(find "$OUT/classes" -name '*.class')

echo "== 4/6 paket"
cp "$OUT/base.apk" "$OUT/unsigned.apk"
( cd "$OUT/dex" && "$JDK/jar.exe" uf "$OUT/unsigned.apk" classes.dex )
# Cynex: extractNativeLibs=false -> .so stored olmali
( cd "$OUT/stage" && python -c "
import zipfile, os, sys
z = zipfile.ZipFile(sys.argv[1], 'a')
for d, _, fs in os.walk('lib'):
	for f in fs:
		p = os.path.join(d, f).replace(os.sep, '/')
		z.write(p, p, compress_type=zipfile.ZIP_STORED)
z.close()" "$OUT/unsigned.apk" )

echo "== 5/6 hizalama"
"$BT/zipalign.exe" -f -P 16 4 "$OUT/unsigned.apk" "$OUT/aligned.apk"

echo "== 6/6 imza"
KS="$ROOT/android/cynex-debug.keystore"
if [ ! -f "$KS" ]; then
	"$JDK/keytool.exe" -genkeypair -keystore "$KS" -storepass cynexmobil -keypass cynexmobil \
		-alias cynex -keyalg RSA -keysize 2048 -validity 10000 -dname "CN=Cynex, O=Cynex, C=TR" > /dev/null
fi
"$BT/apksigner.bat" sign --ks "$KS" --ks-pass pass:cynexmobil --ks-key-alias cynex \
	--out "$OUT/CynexMobil.apk" "$OUT/aligned.apk"
"$BT/apksigner.bat" verify "$OUT/CynexMobil.apk"

ls -la "$OUT/CynexMobil.apk"
echo "== HAZIR: $OUT/CynexMobil.apk"
