# Cynex: sptconv - .spt agaclarini cynex_speedtree formatina ceviren PC araci
# Cynex: gercek SpeedTreeRT.dll (1.6, 32bit) ile calisir, sadece Win32 (x86) derlenir
cmake_minimum_required(VERSION 3.22)
project(sptconv CXX)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(FATAL_ERROR "sptconv x86 derlenmeli (SpeedTreeRT.dll 32bit)")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SPTCONV_CLIENT_DIR "C:/CynexMobil/client" CACHE PATH "istemci kok dizini (Extern/include, Extern/lib)")

add_executable(sptconv sptconv.cpp)
target_include_directories(sptconv PRIVATE ${SPTCONV_CLIENT_DIR}/Extern/include)
target_link_libraries(sptconv PRIVATE ${SPTCONV_CLIENT_DIR}/Extern/lib/SpeedTreeRT.lib)
target_compile_definitions(sptconv PRIVATE _CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES NOMINMAX WIN32_LEAN_AND_MEAN UNICODE _UNICODE)
target_compile_options(sptconv PRIVATE /W4 /utf-8 /EHsc)
