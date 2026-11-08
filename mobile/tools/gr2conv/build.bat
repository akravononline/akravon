@echo off
rem Cynex: gr2conv x86 derleme -> C:\CynexMobilBuild\granny\gr2conv
setlocal
set VS=C:\Program Files\Microsoft Visual Studio\18\Community
set CMAKE=C:\Android\Sdk\cmake\3.22.1\bin
call "%VS%\VC\Auxiliary\Build\vcvarsall.bat" x86 >nul || exit /b 1
set PATH=%CMAKE%;%PATH%
cmake -S C:/CynexMobil/mobile/tools/gr2conv -B C:/CynexMobilBuild/granny/gr2conv -G Ninja -DCMAKE_BUILD_TYPE=Release || exit /b 1
cmake --build C:/CynexMobilBuild/granny/gr2conv || exit /b 1
copy /y C:\CynexMobilBuild\granny\oracle\granny2.dll C:\CynexMobilBuild\granny\gr2conv\ >nul
