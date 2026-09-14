@echo off
setlocal enabledelayedexpansion
rem MultiDore 64 build script (oscar64)
rem oscar64 is a whole-program optimizing C compiler for the 6502.
rem It compiles, assembles, and links in a single pass.
rem
rem Usage:
rem   compile.bat          - build .prg only (default)
rem   compile.bat prg      - build .prg
rem   compile.bat crt      - build cartridge (.crt)
rem   compile.bat tap      - build tape image (.tap)
rem   compile.bat all      - build all formats

rem Cartridge settings
set CARTRIDGE_NAME=MULTIDORE64
set CARTRIDGE_ID=1
set CARTRIDGE_SUB=0

rem Clean previous build artifacts
rmdir /s /q build 2>nul
rmdir /s /q dist 2>nul
mkdir dist 2>nul

cd src

rem Collect all source files (main.c + multidore64\*.c)
set sources=main.c
for %%F in (multidore64\*.c) do (
    set sources=!sources! %%F
)

rem oscar64 common flags
set FLAGS=-O3 -Oo -tm=c64

rem Determine what to build
set target=%1
if "%target%"=="" set target=prg

if /i "%target%"=="prg" goto build_prg
if /i "%target%"=="crt" goto build_crt
if /i "%target%"=="tap" goto build_tap
if /i "%target%"=="all" goto build_all
echo Unknown target: %target%
echo Usage: %~nx0 [prg^|crt^|tap^|all]
exit /b 1

:build_prg
echo Building PRG...
oscar64 %FLAGS% -tf=prg -o=..\dist\main.prg !sources!
if exist ..\dist\main.prg (
    echo   -^> dist\main.prg
    echo Build successful!
    exit /b 0
) else (
    echo Build failed!
    exit /b 1
)

:build_crt
echo Building cartridge...
oscar64 %FLAGS% -tf=crt -cname=%CARTRIDGE_NAME% -cid=%CARTRIDGE_ID% -csub=%CARTRIDGE_SUB% -o=..\dist\main.crt !sources!
if exist ..\dist\main.crt (
    echo   -^> dist\main.crt
    echo Build successful!
    exit /b 0
) else (
    echo Build failed!
    exit /b 1
)

:build_tap
echo Building tape image...
oscar64 %FLAGS% -tf=bin -o=..\dist\main.bin !sources!
if not exist ..\dist\main.bin (
    echo Build failed!
    exit /b 1
)
rem Create .tap file from the binary using Python
python -c "data=open(r'..\dist\main.bin','rb').read(); name=b'MAIN          '; addr=0x0801; msg=b'\x00\x00\x00\x00'+b'\x00\x00\x00\x00'+b'\x00'+bytes([len(name)])+name+b'\x00'; hdr=b'\x00\x00\x00\x00'+b'\x00\x00\x00\x00'+b'\x10'+bytes([addr&0xFF,(addr>>8)&0xFF])+b'\x00'; f=open(r'..\dist\main.tap','wb'); f.write(msg); f.write(hdr); f.write(data); f.close()"
if %errorlevel% neq 0 (
    rem Fallback: just copy the bin to tap
    copy ..\dist\main.bin ..\dist\main.tap >nul
)
del ..\dist\main.bin 2>nul
if exist ..\dist\main.tap (
    echo   -^> dist\main.tap
    echo Build successful!
    exit /b 0
) else (
    echo Build failed!
    exit /b 1
)

:build_all
echo Building all formats...
oscar64 %FLAGS% -tf=prg -o=..\dist\main.prg !sources!
echo   -^> dist\main.prg
oscar64 %FLAGS% -tf=crt -cname=%CARTRIDGE_NAME% -cid=%CARTRIDGE_ID% -csub=%CARTRIDGE_SUB% -o=..\dist\main.crt !sources!
echo   -^> dist\main.crt
oscar64 %FLAGS% -tf=bin -o=..\dist\main.bin !sources!
python -c "data=open(r'..\dist\main.bin','rb').read(); name=b'MAIN          '; addr=0x0801; msg=b'\x00\x00\x00\x00'+b'\x00\x00\x00\x00'+b'\x00'+bytes([len(name)])+name+b'\x00'; hdr=b'\x00\x00\x00\x00'+b'\x00\x00\x00\x00'+b'\x10'+bytes([addr&0xFF,(addr>>8)&0xFF])+b'\x00'; f=open(r'..\dist\main.tap','wb'); f.write(msg); f.write(hdr); f.write(data); f.close()"
del ..\dist\main.bin 2>nul
echo   -^> dist\main.tap
echo Build successful!
exit /b 0