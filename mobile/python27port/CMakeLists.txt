# Cynex: python 2.7.18 (static) - kaynak thirdparty/python27, orijinal tarball
cmake_minimum_required(VERSION 3.22)

set(PY ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/python27)

set(PY_PARSER
	acceler.c grammar1.c listnode.c node.c parser.c parsetok.c bitset.c
	metagrammar.c firstsets.c grammar.c pgen.c myreadline.c tokenizer.c)
list(TRANSFORM PY_PARSER PREPEND ${PY}/Parser/)

file(GLOB PY_OBJECTS ${PY}/Objects/*.c)

file(GLOB PY_PYTHON ${PY}/Python/*.c)
# Cynex: platforma ozel dosyalar haric
list(FILTER PY_PYTHON EXCLUDE REGEX "/(dynload_[a-z0-9]+|mactoolboxglue|sigcheck|frozenmain|atof|dup2|getcwd|strdup|strtod)\\.c$")

set(PY_MODULES
	gcmodule.c signalmodule.c posixmodule.c threadmodule.c zipimport.c
	_sre.c _codecsmodule.c _weakref.c errnomodule.c symtablemodule.c xxsubtype.c
	getbuildinfo.c
	arraymodule.c mathmodule.c _math.c cmathmodule.c timemodule.c datetimemodule.c
	operator.c _struct.c itertoolsmodule.c _collectionsmodule.c _functoolsmodule.c
	_randommodule.c _bisectmodule.c _heapqmodule.c cStringIO.c cPickle.c binascii.c
	md5module.c md5.c shamodule.c sha256module.c sha512module.c stropmodule.c
	_csv.c _json.c future_builtins.c zlibmodule.c unicodedata.c
	_io/_iomodule.c _io/bufferedio.c _io/bytesio.c _io/fileio.c _io/iobase.c
	_io/stringio.c _io/textio.c
	cjkcodecs/multibytecodec.c cjkcodecs/_codecs_kr.c cjkcodecs/_codecs_cn.c cjkcodecs/_codecs_jp.c
	cjkcodecs/_codecs_tw.c cjkcodecs/_codecs_hk.c cjkcodecs/_codecs_iso2022.c)
list(TRANSFORM PY_MODULES PREPEND ${PY}/Modules/)

add_library(python27 STATIC
	${PY_PARSER} ${PY_OBJECTS} ${PY_PYTHON} ${PY_MODULES}
	${CMAKE_CURRENT_LIST_DIR}/config.c
	${CMAKE_CURRENT_LIST_DIR}/getpath_cynex.c)

# Cynex: pyconfig.h buradan
target_include_directories(python27 PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${PY}/Include)
target_include_directories(python27 PRIVATE ${PY}/Python ${PY}/Modules ${PY}/Modules/_io)
target_compile_definitions(python27 PRIVATE Py_BUILD_CORE NDEBUG PLATFORM="linux2")
target_compile_options(python27 PRIVATE
	-std=gnu99 -fwrapv -fno-strict-aliasing -O2
	-Wno-unused-value -Wno-unused-function -Wno-sign-compare -Wno-deprecated-declarations
	-Wno-parentheses-equality -Wno-incompatible-pointer-types-discards-qualifiers)
target_link_libraries(python27 PUBLIC z m)
# Cynex: py27 headerlarinda register var, c++17 hata veriyor
target_compile_options(python27 INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wno-register>)
