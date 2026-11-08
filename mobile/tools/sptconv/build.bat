@echo off
rem Cynex: sptconv x86 derleme -> C:\CynexMobilBuild\speedtree\sptconv
setlocal
set VS=C:\Program Files\Microsoft Visual Studio\18\Community
set CMAKE=C:\Android\Sdk\cmake\3.22.1\bin
call "%VS%\VC\Auxiliary\Build\vcvarsall.bat" x86 >nul || exit /b 1
set PATH=%CMAKE%;%PATH%
cmake -S C:/CynexMobil/mobile/tools/sptconv -B C:/CynexMobilBuild/speedtree/sptconv -G Ninja -DCMAKE_BUILD_TYPE=Release || exit /b 1
cmake --build C:/CynexMobilBuild/speedtree/sptconv || exit /b 1
copy /y C:\CynexMobilBuild\speedtree\oracle\SpeedTreeRT.dll C:\CynexMobilBuild\speedtree\sptconv\ >nul
