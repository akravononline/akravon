@echo off
rem Cynex: tek dosyalik granny deneme programi derle (x86): build_probe.bat probe_xxx
setlocal
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x86 >nul || exit /b 1
if not exist C:\CynexMobilBuild\granny\probe mkdir C:\CynexMobilBuild\granny\probe
cd /d C:\CynexMobilBuild\granny\probe
copy /y C:\CynexMobilBuild\granny\oracle\granny2.dll . >nul
cl /nologo /O2 /EHsc /std:c++17 /utf-8 /I C:\CynexMobil\client\Extern\include C:\CynexMobil\mobile\granny\test\probe\%1.cpp C:\CynexMobil\client\Extern\lib\granny2.11.8.0.lib /Fe%1.exe || exit /b 1
