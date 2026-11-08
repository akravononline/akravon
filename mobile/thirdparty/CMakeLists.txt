# Cynex: ucuncu parti kutuphaneler (lzo, freetype, cryptopp)
set(TP ${CMAKE_CURRENT_LIST_DIR})
set(EXTERN ${CMAKE_CURRENT_LIST_DIR}/../../client/Extern)

file(GLOB LZO_SRC ${TP}/lzo/src/*.c)
add_library(cynex_lzo STATIC ${LZO_SRC})
target_include_directories(cynex_lzo PUBLIC ${TP}/lzo/include PRIVATE ${TP}/lzo/src)
target_compile_options(cynex_lzo PRIVATE -O2 -w)

set(FT ${TP}/freetype/src)
add_library(cynex_freetype STATIC
	${FT}/base/ftsystem.c ${FT}/base/ftinit.c ${FT}/base/ftdebug.c ${FT}/base/ftbase.c
	${FT}/base/ftbbox.c ${FT}/base/ftglyph.c ${FT}/base/ftbitmap.c ${FT}/base/ftmm.c
	${FT}/truetype/truetype.c ${FT}/sfnt/sfnt.c ${FT}/smooth/smooth.c ${FT}/raster/raster.c
	${FT}/autofit/autofit.c ${FT}/psnames/psnames.c
	${FT}/base/ftsynth.c ${FT}/gzip/ftgzip.c)
target_include_directories(cynex_freetype PUBLIC ${TP}/freetype/include PRIVATE ${TP}/freetype_config)
target_compile_definitions(cynex_freetype PRIVATE FT2_BUILD_LIBRARY "FT_CONFIG_MODULES_H=<ftmodule.h>")
target_compile_options(cynex_freetype PRIVATE -O2 -w)

file(GLOB CRYPTOPP_SRC ${EXTERN}/cryptopp/*.cpp)
list(FILTER CRYPTOPP_SRC EXCLUDE REGEX "/(test|bench[0-9]*|validat[0-9]*|datatest|regtest[0-9]*|fipsalgt|fipstest|dlltest|adhoc|cryptlib_bds)\.cpp$")
if (CYNEX_IOS)
	add_library(cynex_cryptopp STATIC ${CRYPTOPP_SRC})
	target_include_directories(cynex_cryptopp PUBLIC ${EXTERN}/include)
else()
	add_library(cynex_cryptopp STATIC ${CRYPTOPP_SRC} ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
	target_include_directories(cynex_cryptopp PUBLIC ${EXTERN}/include PRIVATE ${ANDROID_NDK}/sources/android/cpufeatures)
endif()
target_compile_definitions(cynex_cryptopp PRIVATE CRYPTOPP_DISABLE_ASM NDEBUG)
target_compile_options(cynex_cryptopp PRIVATE -O2 -w -fexceptions -frtti)
set_target_properties(cynex_cryptopp PROPERTIES CXX_STANDARD 17)

add_library(cynex_gif STATIC ${TP}/giflib/dgif_lib.c ${TP}/giflib/egif_lib.c ${TP}/giflib/gif_err.c ${TP}/giflib/gif_hash.c
	${TP}/giflib/gifalloc.c ${TP}/giflib/gif_font.c ${TP}/giflib/openbsd-reallocarray.c ${TP}/giflib/quantize.c)
target_include_directories(cynex_gif PUBLIC ${TP}/giflib)
target_compile_options(cynex_gif PRIVATE -O2 -w)
