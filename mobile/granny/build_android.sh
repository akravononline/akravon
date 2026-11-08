#!/bin/bash
# Cynex: cynex_granny android derleme (arm64-v8a + x86_64) + --no-undefined baglama testi
# Cynex: cikti C:/CynexMobilBuild/granny/android-<abi>
set -e
NDK=/c/Android/Sdk/ndk/27.2.12479018
CMAKE=/c/Android/Sdk/cmake/3.22.1/bin/cmake.exe
NINJA=C:/Android/Sdk/cmake/3.22.1/bin/ninja.exe
for ABI in arm64-v8a x86_64; do
	OUT=C:/CynexMobilBuild/granny/android-$ABI
	"$CMAKE" -S C:/CynexMobil/mobile/granny -B $OUT -G Ninja \
		-DCMAKE_MAKE_PROGRAM=$NINJA \
		-DCMAKE_TOOLCHAIN_FILE=C:/Android/Sdk/ndk/27.2.12479018/build/cmake/android.toolchain.cmake \
		-DANDROID_ABI=$ABI -DANDROID_PLATFORM=android-26 -DANDROID_STL=c++_static \
		-DCMAKE_BUILD_TYPE=Release -DCYNEX_GRANNY_LINK_CHECK=ON
	"$CMAKE" --build $OUT
done
