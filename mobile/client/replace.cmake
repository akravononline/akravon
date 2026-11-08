# Cynex: windows'a bagli dosyalarin mobil karsiliklari
set(EterBase_REPLACE source/EterBase/error.cpp)
set(EterBase_EXTRA ${MOBILE}/port/EterBase/error_mobile.cpp)

set(EterPack_EXTRA ${MOBILE}/port/EterPack/AssetOverride_mobile.cpp)

set(EterLib_REPLACE source/EterLib/IME.cpp source/EterLib/Input.cpp source/EterLib/JpegFile.cpp)
set(EterLib_EXTRA
	${MOBILE}/port/EterLib/IME_mobile.cpp
	${MOBILE}/port/EterLib/Input_mobile.cpp
	${MOBILE}/port/EterLib/JpegFile_mobile.cpp)

set(UserInterface_REPLACE
	source/UserInterface/MovieMan.cpp
	source/UserInterface/PythonApplicationLogo.cpp
	source/UserInterface/CefWebBrowser.cpp
	source/UserInterface/CefClientApp.cpp
	source/UserInterface/CefClientHandler.cpp
	source/UserInterface/CefClientV8ExtensionHandler.cpp
	source/UserInterface/PythonrootlibManager.cpp
	source/UserInterface/PythonrootlibManagerY.cpp
	source/UserInterface/PythonuiscriptlibManager.cpp)
set(UserInterface_EXTRA
	${MOBILE}/port/UserInterface/MovieMan_mobile.cpp
	${MOBILE}/port/UserInterface/PythonApplicationLogo_mobile.cpp
	${MOBILE}/port/UserInterface/CefWebBrowser_mobile.cpp
	${MOBILE}/port/UserInterface/Discord_mobile.cpp
	${MOBILE}/port/UserInterface/CynexTouch_mobile.cpp
	${MOBILE}/port/UserInterface/CynexMobileModule_mobile.cpp)

# Cynex: dokunmatik arayuz - wndMgr eklentileri, yuva ikon olcegi
set(EterPythonLib_EXTRA ${MOBILE}/port/EterPythonLib/WndMgrMobile_mobile.cpp)
