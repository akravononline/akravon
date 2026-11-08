# Cynex: libcynexmobil.so
cmake_minimum_required(VERSION 3.22)
project(cynexmobil C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Cynex: cryptopp kutuphanesi CRYPTOPP_DISABLE_ASM ile derleniyor; baslik kullanan istemci kodu da ayni ayarla derlenmeli
# (aksi halde sinif duzeni uyusmaz - ARM64'te anahtar anlasmasi bozuluyordu)
add_compile_definitions(CRYPTOPP_DISABLE_ASM)

add_subdirectory(../python27port python27)
add_subdirectory(../client client)
add_subdirectory(../thirdparty thirdparty)
add_subdirectory(../mss mss)
add_subdirectory(../speedtree speedtree)
add_subdirectory(../granny granny)
add_subdirectory(../compat compat)
add_subdirectory(../d3d8gl d3d8gl)

add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue)

add_library(cynexmobil SHARED main_android.cpp)
set(CYNEX_CLIENT_LIBS UserInterface EterPythonLib GameLib EffectLib PRTerrainLib SpeedTreeLib ScriptLib
	EterGrnLib MilesLib SphereLib EterLib EterImageLib EterPack EterLocale EterBase)
target_link_libraries(cynexmobil PRIVATE native_app_glue
	-Wl,--start-group ${CYNEX_CLIENT_LIBS} cynex_compat cynex_d3d8 cynex_mss cynex_speedtree cynex_granny cynex_lzo cynex_freetype cynex_cryptopp cynex_gif python27 -Wl,--end-group
	android EGL GLESv3 log z)
# Cynex: ANativeActivity_onCreate export edilmezse uygulama acilmiyor
# Cynex: #pragma comment(lib, "...") windows kutuphanelerini aratmasin
target_link_options(cynexmobil PRIVATE -u ANativeActivity_onCreate -Wl,--no-undefined -Wl,--no-dependent-libraries -Wl,--error-limit=0)
