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
    src\multidore64\utilslib.c

if exist dist\main.prg (
    echo Build successful!
    exit /b 0
) else (
    echo Build failed!
    exit /b 1
)