cmake_minimum_required(VERSION 3.10)

project(RaeenOS C ASM)

enable_language(ASM_NASM)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_FLAGS "-ffreestanding -O2 -Wall -Wextra")
set(CMAKE_ASM_NASM_FLAGS "-f elf64 -g")

file(GLOB_RECURSE KERNEL_SOURCES
    LIST_DIRECTORIES false
    "*.c" "*.asm"
)

list(FILTER KERNEL_SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/build_exclude/.*")
list(FILTER KERNEL_SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/build_exclude_drivers/.*")

# Exclude basic versions that are replaced by production components
list(FILTER KERNEL_SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/kernel_main_basic.c")
list(FILTER KERNEL_SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/kernel/pmm.c")
list(FILTER KERNEL_SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/kernel/idt_simple.c")

# Add production components explicitly
set(PRODUCTION_SOURCES
    kernel_main.c
    pmm_production.c
    vmm_production.c 
    vmm_production_impl.c
    heap_production.c
    idt_production.c
    syscall_production.c
    boot/boot_orchestrator.c
)

# Add security framework sources
file(GLOB_RECURSE SECURITY_SOURCES
    LIST_DIRECTORIES false
    "security/*.c"
)

set(SOURCES ${KERNEL_SOURCES} ${SECURITY_SOURCES} ${PRODUCTION_SOURCES}
    kernel/ipc/message_queue.c
    drivers/network/ethernet.c
    kernel/fs/ramfs.c
    kernel/shell.c
    userland/app_store.c
    libs/libc/stdio.c
    libs/libc/stdlib.c
    libs/libc/string.c
    kernel/vm/hypervisor.c
    userland/desktop/desktop.c
    kernel/swap/swap_manager.c
    kernel/ipc/shared_memory.c
    drivers/gpu/vulkan/vulkan_driver.c
    kernel/net/ipv4.c
    kernel/net/arp.c
    kernel/net/tcp.c
    kernel/net/udp.c
    kernel/net/icmp.c
    kernel/net/dhcp.c
    kernel/net/dns.c
    kernel/net/ipv6.c
    drivers/network/wifi.c
    drivers/network/bluetooth.c
    kernel/fs/raeenfs.c
    drivers/ata/ata.c
    kernel/fs/fat32.c
    drivers/nvme/nvme.c
    userland/desktop/window_manager.c
    userland/desktop/taskbar.c
    userland/desktop/start_menu.c
    userland/desktop/file_explorer.c
    kernel/graphics/3d_pipeline.c
    kernel/vm/device_emulation.c
    kernel/vm/cpu_virtualization.c
    kernel/vm/memory_virtualization.c
    kernel/ai/ai_engine.c
    kernel/ai/nlp.c
    userland/app_store/app_distribution.c
    userland/app_store/app_security.c
)

add_executable(raeenos.elf ${SOURCES})

# Add security include directory
target_include_directories(raeenos.elf PRIVATE ${CMAKE_SOURCE_DIR}/security)

# Add libc include directory
target_include_directories(raeenos.elf PRIVATE ${CMAKE_SOURCE_DIR}/libs/libc/include)

# Add RaeenUI as a subdirectory (commented out until UI CMakeLists.txt is available)
# add_subdirectory(ui)

# Build RaeShell
add_executable(raeshell userland/raeshell/raeshell.c)
set_target_properties(raeshell PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/userland")

set_target_properties(raeenos.elf PROPERTIES
    LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/kernel/linker.ld -nostdlib"
)

link_directories(${CMAKE_SOURCE_DIR}/security)
# link_directories(${CMAKE_SOURCE_DIR}/ui)
target_link_libraries(raeenos.elf security)

add_custom_command(
    TARGET raeenos.elf
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary raeenos.elf raeenos.bin
)
