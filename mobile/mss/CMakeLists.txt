# Cynex: cynex_mss - MSS32 (Miles 6.5) yerine miniaudio; android + windows test
cmake_minimum_required(VERSION 3.22)
if (CYNEX_IOS)
	project(cynex_mss C CXX OBJC)
else()
	project(cynex_mss C CXX)
endif()

option(CYNEX_MSS_BUILD_TESTS "windows test programi (mss_test)" OFF)
option(CYNEX_MSS_BUILD_LINK_CHECK "android: --no-undefined ile test .so" OFF)
option(CYNEX_MSS_MINIAUDIO_IMPL "miniaudio uygulamasini cynex_mss derlesin (baska yerde derleniyorsa OFF)" ON)

set(CYNEX_MSS_DIR ${CMAKE_CURRENT_LIST_DIR})

set(CYNEX_MSS_SOURCES
	${CYNEX_MSS_DIR}/src/mss_core.cpp
	${CYNEX_MSS_DIR}/src/mss_file.cpp
	${CYNEX_MSS_DIR}/src/mss_sample.cpp
	${CYNEX_MSS_DIR}/src/mss_stream.cpp)

if (CYNEX_MSS_MINIAUDIO_IMPL)
	if (CYNEX_IOS)
		# ios: miniaudio coreaudio arka ucu Objective-C ister
		list(APPEND CYNEX_MSS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../ios/miniaudio_impl_ios.m)
	else()
		list(APPEND CYNEX_MSS_SOURCES ${CYNEX_MSS_DIR}/src/miniaudio_impl.c)
	endif()
endif()

find_package(Threads REQUIRED)

function(cynex_mss_library name)
	add_library(${name} STATIC ${CYNEX_MSS_SOURCES})
	target_include_directories(${name} PUBLIC ${CYNEX_MSS_DIR}/include)
	set_target_properties(${name} PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
		C_STANDARD 99
		POSITION_INDEPENDENT_CODE ON)

	if (MSVC)
		target_compile_options(${name} PRIVATE /W4 /utf-8)
		target_compile_definitions(${name} PRIVATE _CRT_SECURE_NO_WARNINGS)
	else()
		target_compile_options(${name} PRIVATE -Wall -Wextra)
	endif()

	# Cynex: miniaudio AAudio/OpenSL'i dlopen ile yukler, ayrica -laaudio/-lOpenSLES gerekmez
	target_link_libraries(${name} PUBLIC Threads::Threads)
	if (ANDROID)
		target_link_libraries(${name} PUBLIC dl m)
	endif()
	if (CYNEX_IOS)
		target_link_libraries(${name} PUBLIC "-framework AudioToolbox" "-framework CoreAudio" "-framework AVFoundation" "-framework CoreFoundation")
	endif()
endfunction()

cynex_mss_library(cynex_mss)

if (CYNEX_MSS_BUILD_TESTS)
	cynex_mss_library(cynex_mss_testing)
	target_compile_definitions(cynex_mss_testing PUBLIC CYNEX_MSS_TESTING)
	target_include_directories(cynex_mss_testing PUBLIC ${CYNEX_MSS_DIR}/src)

	add_executable(mss_test ${CYNEX_MSS_DIR}/tests/mss_test.cpp)
	target_link_libraries(mss_test PRIVATE cynex_mss_testing)
	set_target_properties(mss_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
	if (MSVC)
		target_compile_options(mss_test PRIVATE /W4 /utf-8)
		target_compile_definitions(mss_test PRIVATE _CRT_SECURE_NO_WARNINGS)
	endif()
endif()

if (ANDROID AND CYNEX_MSS_BUILD_LINK_CHECK)
	add_library(cynex_mss_linkcheck SHARED ${CYNEX_MSS_DIR}/tests/link_check.cpp)
	target_link_libraries(cynex_mss_linkcheck PRIVATE cynex_mss)
	target_compile_options(cynex_mss_linkcheck PRIVATE -Wall -Wextra)
	target_link_options(cynex_mss_linkcheck PRIVATE -Wl,--no-undefined)
endif()
