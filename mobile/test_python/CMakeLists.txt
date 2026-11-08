# Cynex: python link testi
cmake_minimum_required(VERSION 3.22)
project(cynex_pytest C)
add_subdirectory(../python27port python27)
add_library(cynex_pytest SHARED pytest.c)
target_link_libraries(cynex_pytest PRIVATE python27 log)
target_link_options(cynex_pytest PRIVATE -Wl,--no-undefined)
