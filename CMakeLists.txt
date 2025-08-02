cmake_minimum_required(VERSION 3.10)

project(RaeenOS C ASM)

enable_language(ASM_NASM)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_FLAGS "-ffreestanding -O2 -Wall -Wextra")
set(CMAKE_ASM_NASM_FLAGS "-f elf64 -g")

file(GLOB_RECURSE SOURCES
    LIST_DIRECTORIES false
    "*.c" "*.asm"
)

list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/build_exclude/.*")
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/build_exclude_drivers/.*")

add_executable(raeenos.elf ${SOURCES})

# Build RaeShell
add_executable(raeshell userland/raeshell/raeshell.c)
set_target_properties(raeshell PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/userland")

set_target_properties(raeenos.elf PROPERTIES
    LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/kernel/linker.ld -nostdlib"
)

add_custom_command(
    TARGET raeenos.elf
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary raeenos.elf raeenos.bin
)
