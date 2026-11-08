# Cynex: istemci kutuphaneleri (android) - kaynak listesi sources.cmake (vcxproj'dan)
cmake_minimum_required(VERSION 3.22)

set(CLIENT ${CMAKE_CURRENT_LIST_DIR}/../../client)
set(MOBILE ${CMAKE_CURRENT_LIST_DIR}/..)
include(${CMAKE_CURRENT_LIST_DIR}/sources.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/replace.cmake OPTIONAL)

set(CYNEX_CLIENT_INCLUDES
	${MOBILE}/compat/include
	${MOBILE}/granny/include
	${MOBILE}/speedtree/include
	${MOBILE}/mss/include
	${MOBILE}/python27port
	${MOBILE}/thirdparty/python27/Include
	${MOBILE}/thirdparty/lzo/include
	${MOBILE}/thirdparty/lzo/include/lzo
	${MOBILE}/thirdparty/stb
	${MOBILE}/thirdparty/freetype/include
	${MOBILE}/thirdparty/giflib
	${CLIENT}/Extern/include
	${CLIENT}/source)

set(CYNEX_CLIENT_DEFINES
	CYNEX_MOBILE=1 NDEBUG _LIB USE_LOD DUNGEON_WORK _CRT_SECURE_NO_WARNINGS
	_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS FMT_HEADER_ONLY
	_LIBCPP_ENABLE_CXX17_REMOVED_FEATURES)	# Cynex: unary_function, auto_ptr, random_shuffle

# Cynex: msvc'nin hos gordugu seyler clang'da uyari kalsin, hata olmasin
set(CYNEX_CLIENT_FLAGS
	-fms-extensions -fdeclspec -fno-strict-aliasing -fwrapv -fsigned-char
	-Wno-register -Wno-everything)
# Cynex: 64 bit denetimi - isaretci kirpan donusumleri goster
if (CYNEX_AUDIT64)
	list(APPEND CYNEX_CLIENT_FLAGS -Wmicrosoft-cast -Wpointer-to-int-cast -Wvoid-pointer-to-int-cast -Wint-to-pointer-cast -Wformat -Wcall-to-pure-virtual-from-ctor-dtor)
endif()

function(cynex_client_lib name)
	set(src ${${name}_SOURCES})
	if (DEFINED ${name}_REPLACE)
		foreach(r ${${name}_REPLACE})
			list(REMOVE_ITEM src ${CLIENT}/${r})
		endforeach()
	endif()
	if (DEFINED ${name}_EXTRA)
		list(APPEND src ${${name}_EXTRA})
	endif()
	add_library(${name} STATIC ${src})
	target_include_directories(${name} PRIVATE ${CYNEX_CLIENT_INCLUDES})
	target_compile_definitions(${name} PRIVATE ${CYNEX_CLIENT_DEFINES} ${ARGN})
	target_compile_options(${name} PRIVATE ${CYNEX_CLIENT_FLAGS})
	set_target_properties(${name} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
endfunction()

cynex_client_lib(EterBase)
cynex_client_lib(EterLocale)
cynex_client_lib(EterPack)
cynex_client_lib(EterImageLib)
cynex_client_lib(EterLib)
cynex_client_lib(EterGrnLib)
cynex_client_lib(EffectLib)
cynex_client_lib(GameLib BACKGROUND_LOADING)
cynex_client_lib(MilesLib)
cynex_client_lib(PRTerrainLib)
cynex_client_lib(ScriptLib)
cynex_client_lib(SpeedTreeLib)
cynex_client_lib(SphereLib)
cynex_client_lib(EterPythonLib)
cynex_client_lib(UserInterface)
