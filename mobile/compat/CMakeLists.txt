# Cynex: cynex_compat - win32 karsiliklari + android platform katmani
set(COMPAT ${CMAKE_CURRENT_LIST_DIR})
set(MOBILE ${COMPAT}/..)

add_library(cynex_compat STATIC
	${COMPAT}/src/crt.cpp
	${COMPAT}/src/winapi_file.cpp
	${COMPAT}/src/winapi_misc.cpp
	${COMPAT}/src/winapi_sync.cpp
	${COMPAT}/src/winuser.cpp
	${COMPAT}/src/wingdi.cpp
	${COMPAT}/src/il_mobile.cpp
	${COMPAT}/src/stb_impl.c
	${COMPAT}/src/pyfix.cpp
	${MOBILE}/platform/CynexInput.cpp)
if (NOT CYNEX_IOS)
	target_sources(cynex_compat PRIVATE ${MOBILE}/platform/CynexPlatform.cpp)
endif()

target_include_directories(cynex_compat PRIVATE
	${COMPAT}/include
	${MOBILE}/platform
	${MOBILE}/thirdparty/stb
	${MOBILE}/../client/Extern/include
	${ANDROID_NDK}/sources/android/native_app_glue)
target_compile_definitions(cynex_compat PRIVATE CYNEX_MOBILE=1 _CRT_SECURE_NO_WARNINGS)
target_compile_options(cynex_compat PRIVATE -fms-extensions -fdeclspec -fsigned-char -Wno-everything)
set_target_properties(cynex_compat PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
if (CYNEX_IOS)
	target_link_libraries(cynex_compat PRIVATE cynex_freetype)
else()
	target_link_libraries(cynex_compat PRIVATE cynex_freetype native_app_glue android log)
endif()
