# Cynex: dogrulama araci (Windows) - granny DLL'leri calisma aninda yuklenir
cmake_minimum_required(VERSION 3.22)
project(granny_harness CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CYNEX_GRANNY_HEADER_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../client/Extern/include" CACHE PATH "granny2.11.8.0.h dizini")

add_executable(granny_harness granny_harness.cpp)
target_include_directories(granny_harness PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include ${CYNEX_GRANNY_HEADER_DIR})
target_compile_definitions(granny_harness PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
target_compile_options(granny_harness PRIVATE /W3 /utf-8 /EHsc)
