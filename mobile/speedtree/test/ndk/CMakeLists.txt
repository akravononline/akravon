# Cynex: NDK baglama testi: cynex_speedtree tanimsiz sembol birakmamali (-Wl,--no-undefined)
cmake_minimum_required(VERSION 3.22)
project(cynex_speedtree_ndktest CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. cynex_speedtree)

add_library(cynex_speedtree_linktest SHARED SptLinkTest.cpp)
target_link_libraries(cynex_speedtree_linktest PRIVATE cynex_speedtree log)
target_compile_options(cynex_speedtree_linktest PRIVATE -Wall -Wextra)
target_link_options(cynex_speedtree_linktest PRIVATE -Wl,--no-undefined)
