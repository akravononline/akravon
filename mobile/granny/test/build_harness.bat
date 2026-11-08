@echo off
rem Cynex: dogrulama araci derleme - build_harness.bat x86^|x64
setlocal
set ARCH=%1
if "%ARCH%"=="" set ARCH=x86
set VS=C:\Program Files\Microsoft Visual Studio\18\Community
set CMAKE=C:\Android\Sdk\cmake\3.22.1\bin
call "%VS%\VC\Auxiliary\Build\vcvarsall.bat" %ARCH% >nul || exit /b 1
set PATH=%CMAKE%;%PATH%
cmake -S C:/CynexMobil/mobile/granny/test -B C:/CynexMobilBuild/granny/harness-%ARCH% -G Ninja -DCMAKE_BUILD_TYPE=Release || exit /b 1
cmake --build C:/CynexMobilBuild/granny/harness-%ARCH% || exit /b 1
