@echo off
rem Cynex: d3dx8 matematik kahin testi (x86, d3dx8.lib)
set VS=C:\Program Files\Microsoft Visual Studio\18\Community
call "%VS%\VC\Auxiliary\Build\vcvarsall.bat" x86 >nul || exit /b 1
set OUT=C:\CynexMobilBuild\d3d8gl\oracle
if not exist %OUT% mkdir %OUT%
cl /nologo /O2 /EHsc /MT /DCYNEX_D3DX_ORACLE /I C:\CynexMobil\client\Extern\include /I %~dp0 ^
	%~dp0d3dx_oracle.cpp C:\CynexMobil\mobile\d3d8gl\src\d3dx_math.cpp ^
	/Fo%OUT%\ /Fe%OUT%\d3dx_oracle.exe /link /LIBPATH:C:\CynexMobil\client\Extern\lib d3dx8.lib d3d8.lib legacy_stdio_definitions.lib user32.lib advapi32.lib /NODEFAULTLIB:libci.lib || exit /b 1
%OUT%\d3dx_oracle.exe
