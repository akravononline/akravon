# Cynex: d3d8gl test apk kitapligi (libd3dtest.so)
add_library(d3dtest_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(d3dtest_glue PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue)

add_library(d3dtest SHARED ${CMAKE_CURRENT_LIST_DIR}/main_test.cpp ${CMAKE_CURRENT_LIST_DIR}/stb_impl.c)
target_include_directories(d3dtest PRIVATE
	${CYNEX_D3D8_MOBILE}/compat/include
	${CYNEX_D3D8_CLIENT}/Extern/include
	${CYNEX_D3D8_MOBILE}/platform
	${CYNEX_D3D8_MOBILE}/thirdparty/stb)
target_compile_definitions(d3dtest PRIVATE CYNEX_MOBILE=1)
target_compile_options(d3dtest PRIVATE -fms-extensions -fdeclspec -fsigned-char -Wno-everything)
set_target_properties(d3dtest PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(d3dtest PRIVATE d3dtest_glue cynex_d3d8 android EGL GLESv3 log m)
target_link_options(d3dtest PRIVATE -u ANativeActivity_onCreate -Wl,--no-undefined)
