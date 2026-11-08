# Cynex: gr2conv - .gr2 dosyalarini mobil (sikistirmasiz, 64bit pointer) duzene ceviren PC araci
# Cynex: gercek granny2.dll ile calisir, sadece Win32 (x86) derlenir
cmake_minimum_required(VERSION 3.22)
project(gr2conv CXX)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(FATAL_ERROR "gr2conv x86 derlenmeli (granny2.dll 32bit)")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GR2CONV_CLIENT_DIR "C:/CynexMobil/client" CACHE PATH "istemci kok dizini (Extern/include, Extern/lib)")

add_executable(gr2conv gr2conv.cpp)
target_include_directories(gr2conv PRIVATE ${GR2CONV_CLIENT_DIR}/Extern/include)
target_link_libraries(gr2conv PRIVATE ${GR2CONV_CLIENT_DIR}/Extern/lib/granny2.11.8.0.lib)
target_compile_definitions(gr2conv PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN UNICODE _UNICODE)
target_compile_options(gr2conv PRIVATE /W4 /utf-8 /EHsc)
