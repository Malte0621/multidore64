@echo off
setlocal enabledelayedexpansion
rmdir /s /q build
rmdir /s /q dist
mkdir build 2>nul
cd src
:: Loop through all .c files in the current directory
for %%f in (*.c) do (
    :: Compile each .c file into a .s file
    cc65 -O -o ../build/%%f.s -t c64 %%f
)
for %%F in (*.s) do (
    :: Copy each .s file into the build directory
    copy %%F ..\build\%%F
)
for %%F in (*.bin) do (
    :: Copy each .bin file into the build directory
    copy %%F ..\build\%%F
)
for %%F in (*.cfg) do (
    :: Copy each .cfg file into the build directory
    copy %%F ..\build\%%F
)
cd ..\build
:: Loop through all .s files in the build directory
for %%f in (*.s) do (
    :: Compile each .s file into a .o file
    ca65 -o %%f.o %%f
)
mkdir ..\dist 2>nul
:: Link all .o files into a .prg file
:: ld65 -o ../dist/main -t c64 main.o c64.lib
:: Loop through all .o files in the build directory and add them to the link command
set files=
for %%f in (*.o) do (
    set files=!files! %%f
)

: Check if c64.cfg exist in the build directory
if exist c64.cfg (
    :: Link all .o files into a .prg file
    ld65 -C c64.cfg -o ../dist/main.d64 %files% c64.lib
) else (
    :: Link all .o files into a .prg file
    ld65 -o ../dist/main.d64 -t c64 %files% c64.lib
)

:: Check if any "Error" were printed
if not exist ../dist/main.d64 (
    :: Print the error message
    echo Build failed.
    :: Exit with error code
    exit /b 1
)
:: Print the success message
echo Build succeeded.

pause