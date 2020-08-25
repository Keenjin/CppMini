@echo off

echo "made by Keen."
echo "run: %0"

set cur_dir=%~dp0
set src_dir=%cur_dir%utils\
set dst_dir=%cur_dir%include\

cd "%src_dir%"

rem setlocal enabledelayedexpansion
for %%i in (*.h) do (
	rem set target=%src_dir%%%i
	if "%%i" NEQ "icu_utf.h" (
		xcopy "%src_dir%%%i" "%dst_dir%" /Y /E /S
	)
)
rem setlocal disabledelayedexpansion

cd %cur_dir%
