# Cynex: cynex_granny - EterGrnLib'in kullandigi Granny 2.11.8 fonksiyonlarinin kendi uygulamamiz
# Cynex: dosyalar gr2conv ile cevrilmis olmali (sikistirmasiz, little-endian, yerel pointer boyu)
cmake_minimum_required(VERSION 3.22)
project(cynex_granny CXX)

set(CYNEX_GRANNY_HEADER_DIR "${CMAKE_CURRENT_LIST_DIR}/../../client/Extern/include" CACHE PATH "granny2.11.8.0.h dizini")
option(CYNEX_GRANNY_SHARED "Windows test icin granny2.dll ile ayni ABI'de DLL derle" OFF)

set(CYNEX_GRANNY_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/src/granny_animation.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/granny_core.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/granny_curve.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/granny_file.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/granny_mesh.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/granny_pose.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/granny_types.cpp)

if(CYNEX_GRANNY_SHARED)
	add_library(cynex_granny SHARED ${CYNEX_GRANNY_SOURCES})
	# Cynex: dllexport + __stdcall, gercek granny2.dll ile birebir ayni isimler
	target_compile_definitions(cynex_granny PRIVATE BUILDING_GRANNY)
else()
	add_library(cynex_granny STATIC ${CYNEX_GRANNY_SOURCES})
	if(WIN32)
		target_compile_definitions(cynex_granny PUBLIC BUILDING_GRANNY_STATIC)
	endif()
endif()

target_include_directories(cynex_granny PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/include
	${CYNEX_GRANNY_HEADER_DIR})

set_target_properties(cynex_granny PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	POSITION_INDEPENDENT_CODE ON)

if(MSVC)
	target_compile_options(cynex_granny PRIVATE /W4 /utf-8 /wd4100)
	target_compile_definitions(cynex_granny PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
	target_compile_options(cynex_granny PRIVATE -Wall -Wextra -fno-exceptions -fno-rtti -fno-strict-aliasing)
endif()

# Cynex: tum semboller cozuluyor mu - test .so (sadece android/linux)
option(CYNEX_GRANNY_LINK_CHECK "cynex_granny_link_check.so derle (-Wl,--no-undefined)" OFF)
if(CYNEX_GRANNY_LINK_CHECK AND NOT MSVC)
	add_library(cynex_granny_link_check SHARED ${CMAKE_CURRENT_LIST_DIR}/test/link_check.cpp)
	target_link_libraries(cynex_granny_link_check PRIVATE cynex_granny)
	target_compile_options(cynex_granny_link_check PRIVATE -Wall -Wextra)
	target_link_options(cynex_granny_link_check PRIVATE -Wl,--no-undefined)
endif()
