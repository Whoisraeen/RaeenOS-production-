@echo off
set "MINGW_PATH=C:\Users\woisr\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.MSVCRT_15.1.0-13.0.0-r4\mingw64\bin"
set "NASM_PATH=C:\Users\woisr\AppData\Local\bin\NASM"
set "PATH=%MINGW_PATH%;%NASM_PATH%;%PATH%"

rmdir /s /q build
mkdir build
cd build

cmake -G "MinGW Makefiles" ^
    -D CMAKE_C_COMPILER="%MINGW_PATH%\gcc.exe" ^
    -D CMAKE_ASM_NASM_COMPILER="%NASM_PATH%\nasm.exe" ^
    -D CMAKE_MAKE_PROGRAM="%MINGW_PATH%\mingw32-make.exe" ^
    ..

cmake --build .

