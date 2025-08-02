; Multiboot2 Header for RaeenOS
; This makes the kernel bootable by GRUB2

MBALIGN  equ  1 << 0              ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1              ; provide memory map
FLAGS    equ  MBALIGN | MEMINFO   ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002           ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)      ; checksum of above, to prove we are multiboot

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384  ; 16 KiB stack
stack_top:

section .text
global _start
extern kernel_main

_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Push multiboot info for kernel_main
    push ebx    ; Multiboot info structure
    push eax    ; Multiboot magic number
    
    ; Call the kernel
    call kernel_main
    
    ; Disable interrupts and halt if kernel returns
    cli
.hang:
    hlt
    jmp .hang