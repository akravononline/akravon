#!/usr/bin/env bash
# Cynex: d3d8gl test apk (com.cynex.d3dtest) - kullanim: bash build_test_apk.sh [install]
set -euo pipefail
ROOT="C:/CynexMobil/mobile/d3d8gl"
OUT="C:/CynexMobilBuild/d3d8gl/apk"
SDK="C:/Android/Sdk"
NDK="$SDK/ndk/27.2.12479018"
BT="$SDK/build-tools/36.1.0"
JAR="$SDK/platforms/android-36/android.jar"
JDK="C:/Android/jdk-21/bin"
PACK="C:/CynexMobilBuild/b2pack"
export PATH="/c/Android/Sdk/cmake/3.22.1/bin:$PATH"

for ABI in x86_64 arm64-v8a; do
	B="C:/CynexMobilBuild/d3d8gl/test-$ABI"
	cmake -S "$ROOT" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
		-DANDROID_ABI=$ABI -DANDROID_PLATFORM=android-26 -DANDROID_STL=c++_static -DCMAKE_BUILD_TYPE=Release \
		-DCYNEX_D3D8_BUILD_TEST=ON > /dev/null
	cmake --build "$B" -j 12
	mkdir -p "$OUT/stage/lib/$ABI"
	cp "$B/test/libd3dtest.so" "$OUT/stage/lib/$ABI/"
done

# Cynex: oyun verisinden ornek dokular
rm -rf "$OUT/assets"; mkdir -p "$OUT/assets"
cp "$PACK/etc/ymir work/ui/equipment_bg_with_ring.tga" "$OUT/assets/"
cp "$PACK/etc/ymir work/bin/icon/face/warrior_m.tga" "$OUT/assets/"
cp "$PACK/etc/ymir work/ui/12zi.dds" "$OUT/assets/"
cp "$PACK/etc/ymir work/ui/936_guild.dds" "$OUT/assets/"
cp "$PACK/etc/ymir work/terrainmaps/12temple/12t_stone_tile_001.dds" "$OUT/assets/"
cp "$PACK/etc/ymir work/terrainmaps/12temple/stone_01.dds" "$OUT/assets/"
cp "$PACK/etc/ymir work/effect/etc/click/click.jpg" "$OUT/assets/"
cp "$PACK/images/banner.png" "$OUT/assets/"

"$BT/aapt2.exe" link -o "$OUT/base.apk" -I "$JAR" --manifest "$ROOT/test/AndroidManifest.xml" -A "$OUT/assets"
cp "$OUT/base.apk" "$OUT/unsigned.apk"
( cd "$OUT/stage" && python -c "
import zipfile, os, sys
z = zipfile.ZipFile(sys.argv[1], 'a')
for d, _, fs in os.walk('lib'):
	for f in fs:
		p = os.path.join(d, f).replace(os.sep, '/')
		z.write(p, p, compress_type=zipfile.ZIP_DEFLATED)
z.close()" "$OUT/unsigned.apk" )
"$BT/zipalign.exe" -f -P 16 4 "$OUT/unsigned.apk" "$OUT/aligned.apk"
"$BT/apksigner.bat" sign --ks "C:/CynexMobil/mobile/android/cynex-debug.keystore" --ks-pass pass:cynexmobil --ks-key-alias cynex \
	--out "$OUT/D3DTest.apk" "$OUT/aligned.apk"
echo "== HAZIR: $OUT/D3DTest.apk"
if [ "${1:-}" = "install" ]; then
	ADB="$SDK/platform-tools/adb.exe"
	"$ADB" -s emulator-5554 install -r "$OUT/D3DTest.apk"
fi
