@echo off
setlocal
cd /d "%~dp0"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo ERROR: Visual Studio Build Tools or Visual Studio is required to compile.
  exit /b 1
)
set "VSROOT="
for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%I"
if not defined VSROOT (
  echo ERROR: MSVC C++ compiler not found.
  exit /b 1
)
call "%VSROOT%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
if not exist dist mkdir dist
rc.exe /nologo app.rc
if errorlevel 1 exit /b 1
cl.exe /nologo /O1 /MT /GS /W4 /DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN /D_WIN32_WINNT=0x0601 /Fe:dist\NVContainerFuckOff.exe main.c app.res /link /SUBSYSTEM:WINDOWS /MACHINE:X64 user32.lib gdi32.lib advapi32.lib
if errorlevel 1 exit /b 1
if not exist "dist\NVContainerFuckOff.exe" (
  echo ERROR: Executable was not produced.
  exit /b 1
)
echo Native executable built: dist\NVContainerFuckOff.exe
exit /b 0
