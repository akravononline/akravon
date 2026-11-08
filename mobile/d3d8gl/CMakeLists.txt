# Cynex: cynex_d3d8 - Direct3D 8 + D3DX8 -> OpenGL ES 3.0
cmake_minimum_required(VERSION 3.22)
if (CYNEX_IOS)
	project(cynex_d3d8 C CXX OBJC OBJCXX)
else()
	project(cynex_d3d8 C CXX)
endif()

option(CYNEX_D3D8_BUILD_TEST "test apk icin libd3dtest.so" OFF)

set(CYNEX_D3D8_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CYNEX_D3D8_MOBILE ${CMAKE_CURRENT_LIST_DIR}/..)
set(CYNEX_D3D8_CLIENT ${CMAKE_CURRENT_LIST_DIR}/../../client)

add_library(cynex_d3d8 STATIC
	${CYNEX_D3D8_DIR}/src/cd3d_format.cpp
	${CYNEX_D3D8_DIR}/src/cd3d_resource.cpp
	${CYNEX_D3D8_DIR}/src/cd3d_ffp.cpp
	${CYNEX_D3D8_DIR}/src/cd3d_device.cpp
	${CYNEX_D3D8_DIR}/src/cd3d_draw.cpp
	${CYNEX_D3D8_DIR}/src/cd3d_d3d8.cpp
	${CYNEX_D3D8_DIR}/src/d3dx_math.cpp
	${CYNEX_D3D8_DIR}/src/d3dx_tex.cpp
	${CYNEX_D3D8_DIR}/src/d3dx_mesh.cpp)
if (CYNEX_IOS)
	target_sources(cynex_d3d8 PRIVATE ${CYNEX_D3D8_DIR}/src/cd3d_eagl.mm)
	set_source_files_properties(${CYNEX_D3D8_DIR}/src/cd3d_eagl.mm PROPERTIES LANGUAGE OBJCXX COMPILE_FLAGS "-fobjc-arc -fms-extensions -fdeclspec")
else()
	target_sources(cynex_d3d8 PRIVATE ${CYNEX_D3D8_DIR}/src/cd3d_egl.cpp)
endif()

# Cynex: istemciyle ayni basliklar ve bayraklar - vtable duzeni birebir olsun
target_include_directories(cynex_d3d8 PRIVATE
	${CYNEX_D3D8_MOBILE}/compat/include
	${CYNEX_D3D8_CLIENT}/Extern/include
	${CYNEX_D3D8_MOBILE}/platform
	${CYNEX_D3D8_MOBILE}/thirdparty/stb)
target_compile_definitions(cynex_d3d8 PRIVATE CYNEX_MOBILE=1 NDEBUG)
target_compile_options(cynex_d3d8 PRIVATE
	-fms-extensions -fdeclspec -fno-strict-aliasing -fsigned-char
	-Wall -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-microsoft -Wno-unknown-pragmas
	-Wno-pragma-pack -Wno-missing-braces -Wno-unused-private-field -Wno-deprecated-declarations)
set_target_properties(cynex_d3d8 PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON POSITION_INDEPENDENT_CODE ON)
if (CYNEX_IOS)
	target_link_libraries(cynex_d3d8 PUBLIC "-framework OpenGLES" "-framework UIKit" "-framework QuartzCore" "-framework Foundation")
else()
	target_link_libraries(cynex_d3d8 PUBLIC EGL GLESv3 log android)
endif()

if (CYNEX_D3D8_BUILD_TEST)
	add_subdirectory(${CYNEX_D3D8_DIR}/test ${CMAKE_CURRENT_BINARY_DIR}/test)
endif()
