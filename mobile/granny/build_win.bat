@echo off
rem Cynex: cynex_granny Windows derleme - build_win.bat x86^|x64 [shared]
rem Cynex: cikti C:\CynexMobilBuild\granny\win-<arch>[-shared]
setlocal
set ARCH=%1
if "%ARCH%"=="" set ARCH=x86
set KIND=%2
set VS=C:\Program Files\Microsoft Visual Studio\18\Community
set CMAKE=C:\Android\Sdk\cmake\3.22.1\bin
call "%VS%\VC\Auxiliary\Build\vcvarsall.bat" %ARCH% >nul || exit /b 1
set PATH=%CMAKE%;%PATH%
set OUT=C:/CynexMobilBuild/granny/win-%ARCH%
set SHARED=OFF
if "%KIND%"=="shared" (
	set OUT=C:/CynexMobilBuild/granny/win-%ARCH%-shared
	set SHARED=ON
)
cmake -S C:/CynexMobil/mobile/granny -B %OUT% -G Ninja -DCMAKE_BUILD_TYPE=Release -DCYNEX_GRANNY_SHARED=%SHARED% || exit /b 1
cmake --build %OUT% || exit /b 1
