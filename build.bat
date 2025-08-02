@echo off
set "MINGW_PATH=D:\mingw64\bin"
set "NASM_PATH=C:\Users\woisr\AppData\Local\bin\NASM"

rmdir /s /q build
mkdir build
cd build

cmake -G "MinGW Makefiles" ^
    -D CMAKE_C_COMPILER="%MINGW_PATH%\gcc.exe" ^
    -D CMAKE_ASM_NASM_COMPILER="%NASM_PATH%\nasm.exe" ^
    -D CMAKE_MAKE_PROGRAM="%MINGW_PATH%\mingw32-make.exe" ^
    ..

cmake --build .