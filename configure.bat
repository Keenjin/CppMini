@echo off
echo "made by Keen."

echo "run: %0"

set cur_dir=%~dp0
set build_dir=%cur_dir%\build.dir
set quit_mode=%1
set tools_dir=%cur_dir%tools\

echo "makesure you have installed QT and Visual Studio 2017!"

rem ========================================================================================
rem 检查vs2017是否安装
rem ========================================================================================
echo ">>>>>>>>>>>>>>>> check Visual Studio 2017 installed!"

for /f "delims=" %%i in ('%tools_dir%\QuerySoftDir.exe -DisplayName="Visual Studio Community 2017"') do set "vs2017_dir=%%i"
if exist "%vs2017_dir%" (
	goto findedvs
)

echo "Visual Studio 2017 not founded!"
goto failed

:findedvs
echo "Visual Studio 2017 have installed! path: %vs2017_dir%"

rem ========================================================================================
rem 编译Solution
rem ========================================================================================
echo ">>>>>>>>>>>>>>>> build sln mt"
set main_sln=%cur_dir%\CppMini.sln
"%vs2017_dir%\Common7\IDE\devenv" %main_sln% /Rebuild "DebugMT|x86" /Out %build_dir%\build_debug_mt_x86.log
"%vs2017_dir%\Common7\IDE\devenv" %main_sln% /Rebuild "DebugMT|x64" /Out %build_dir%\build_debug_mt_x64.log
"%vs2017_dir%\Common7\IDE\devenv" %main_sln% /Rebuild "ReleaseMT|x86" /Out %build_dir%\build_release_mt_x86.log
"%vs2017_dir%\Common7\IDE\devenv" %main_sln% /Rebuild "ReleaseMT|x64" /Out %build_dir%\build_release_mt_x64.log
"%vs2017_dir%\Common7\IDE\devenv" %main_sln% /Rebuild "Debug|x86" /Out %build_dir%\build_debug_x86.log
"%vs2017_dir%\Common7\IDE\devenv" %main_sln% /Rebuild "Debug|x64" /Out %build_dir%\build_debug_x64.log
"%vs2017_dir%\Common7\IDE\devenv" %main_sln% /Rebuild "Release|x86" /Out %build_dir%\build_release_x86.log
"%vs2017_dir%\Common7\IDE\devenv" %main_sln% /Rebuild "Release|x64" /Out %build_dir%\build_release_x64.log

:successed
echo ">>>>>>>>>>>>>>>> run %0 successed!"
if "%quit_mode%" NEQ "/s" (
	timeout /T 30
)
goto finished

:failed
echo ">>>>>>>>>>>>>>>> run %0 failed!"
if "%quit_mode%" NEQ "/s" (
	pause
)

:finished
