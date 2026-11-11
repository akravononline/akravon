# Cynex iOS: AKRAVON iOS uygulamasi (macOS uzerinde: cmake -G Xcode -DCMAKE_SYSTEM_NAME=iOS)
cmake_minimum_required(VERSION 3.22)
project(cynexmobil_ios C CXX OBJC OBJCXX)

set(CYNEX_IOS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(MOBILE ${CMAKE_CURRENT_LIST_DIR}/..)

# android/log.h, GLES3/gl3.h, EGL/egl.h yonlendirmeleri
include_directories(${CMAKE_CURRENT_LIST_DIR}/shim)
add_compile_definitions(CRYPTOPP_DISABLE_ASM)
# Xcode libc++: C++17'de kaldirilan eski STL parcalari (istemci kullaniyor) tek tek acilmali
add_compile_definitions(
	_LIBCPP_ENABLE_CXX17_REMOVED_FEATURES
	_LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION
	_LIBCPP_ENABLE_CXX17_REMOVED_BINDERS
	_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR
	_LIBCPP_ENABLE_CXX17_REMOVED_RANDOM_SHUFFLE
	_LIBCPP_ENABLE_CXX20_REMOVED_TEMPORARY_BUFFER
	_LIBCPP_DISABLE_DEPRECATION_WARNINGS)

add_subdirectory(${MOBILE}/python27port python27)
add_subdirectory(${MOBILE}/client client)
add_subdirectory(${MOBILE}/thirdparty thirdparty)
add_subdirectory(${MOBILE}/mss mss)
add_subdirectory(${MOBILE}/speedtree speedtree)
add_subdirectory(${MOBILE}/granny granny)
add_subdirectory(${MOBILE}/compat compat)
add_subdirectory(${MOBILE}/d3d8gl d3d8gl)

set(CYNEX_CLIENT_LIBS UserInterface EterPythonLib GameLib EffectLib PRTerrainLib SpeedTreeLib ScriptLib
	EterGrnLib MilesLib SphereLib EterLib EterImageLib EterPack EterLocale EterBase)

set(CYNEX_IOS_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/CynexPlatform_ios.mm
	${CMAKE_CURRENT_LIST_DIR}/main_ios.mm)
set_source_files_properties(${CYNEX_IOS_SOURCES} PROPERTIES COMPILE_FLAGS "-fobjc-arc -fms-extensions -fdeclspec -fsigned-char")

add_executable(cynexmobil MACOSX_BUNDLE ${CYNEX_IOS_SOURCES})
target_include_directories(cynexmobil PRIVATE ${MOBILE}/compat/include ${MOBILE}/platform ${MOBILE}/../client/Extern/include)
target_compile_definitions(cynexmobil PRIVATE CYNEX_MOBILE=1)
target_link_libraries(cynexmobil PRIVATE
	${CYNEX_CLIENT_LIBS} cynex_compat cynex_d3d8 cynex_mss cynex_speedtree cynex_granny cynex_lzo cynex_freetype cynex_cryptopp cynex_gif python27
	"-framework UIKit" "-framework Foundation" "-framework OpenGLES" "-framework QuartzCore"
	"-framework AudioToolbox" "-framework AVFoundation" "-framework CoreAudio" "-framework CoreGraphics" z)

set_target_properties(cynexmobil PROPERTIES
	MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/Info.plist.in
	MACOSX_BUNDLE_GUI_IDENTIFIER "com.akravon.online"
	MACOSX_BUNDLE_BUNDLE_NAME "AKRAVON"
	MACOSX_BUNDLE_BUNDLE_VERSION "1"
	MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
	XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
	XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
	XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
	XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "13.0"
	XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
	XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
	XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN "YES")
