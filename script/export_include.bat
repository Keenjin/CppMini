@echo off

echo "made by Keen."
echo "run: %0"

set cur_dir=%~dp0

set proj_dir=%1
set include_dir=%2

rem cd "%proj_dir%"

rem setlocal enabledelayedexpansion
rem for %%i in (*.h) do (
	rem set target=%proj_dir%%%i
rem	xcopy "%proj_dir%%%i" "%include_dir%" /Y /E /S
rem )
rem setlocal disabledelayedexpansion

rem cd %cur_dir%

if exist "%proj_dir%include" (
	xcopy "%proj_dir%include\*" "%include_dir%" /Y /E /S
)
