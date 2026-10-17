@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
cd /d "%~dp0"
cl /nologo /utf-8 /std:c++17 /O2 /Oi /Ot /EHsc /bigobj /W1 /Fe:unpack.exe main.cpp unpack.cpp
if errorlevel 1 (
  echo BUILD FAILED
  exit /b 1
)
echo BUILD OK
dir unpack.exe
