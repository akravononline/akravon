# Cynex: cynex_speedtree - SpeedTreeRT 1.6 (CSpeedTreeRT) uyumlu temiz-oda runtime
# Cynex: agaclar sptconv ile donusturulmus olmali; include/ dizini Extern/include'dan once gelmeli
cmake_minimum_required(VERSION 3.22)
project(cynex_speedtree CXX)

add_library(cynex_speedtree STATIC
	src/SpeedTreeRT.cpp
	src/CynexSptModel.cpp
	src/CynexSptModel.h
	src/CynexSptFormat.h
	include/SpeedTreeRT.h)

target_include_directories(cynex_speedtree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(cynex_speedtree PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(cynex_speedtree PRIVATE cxx_std_11)

if(MSVC)
	target_compile_options(cynex_speedtree PRIVATE /W4 /utf-8)
	target_compile_definitions(cynex_speedtree PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
	target_compile_options(cynex_speedtree PRIVATE -Wall -Wextra -fno-strict-aliasing)
endif()
