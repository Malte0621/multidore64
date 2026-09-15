@echo off
setlocal enabledelayedexpansion

rmdir /s /q dist 2>nul
mkdir dist 2>nul

oscar64 -O3 -Ox -Op ^
    -tm=c64 ^
    -tf=prg ^
    -o=dist\main.prg ^
    src\main.c ^
    src\multidore64\renderlib.c ^
    src\multidore64\soundlib.c ^
    src\multidore64\soundlib_asm.c ^
    src\multidore64\controllerlib.c ^
    src\multidore64\utilslib.c ^
    src\multidore64\filelib.c

if not exist dist\main.prg (
    echo Build failed!
    exit /b 1
)

REM Build standard 1541 disk image (dist\main.d64)
REM Default bundle: main PRG and song.bin. Pass extra files via script arguments:
REM   compile.bat extra.prg:extra level1.dat:level1
python tools\make_d64.py dist\main.d64 multidore64 dist\main.prg:main src\song.bin:song.bin %*

if exist dist\main.d64 (
    echo Build successful! Generated dist\main.prg and dist\main.d64
    exit /b 0
) else (
    echo Failed to create disk image!
    exit /b 1
)