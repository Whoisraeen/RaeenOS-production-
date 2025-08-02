; Simple Multiboot2 header for RaeenOS kernel
section .multiboot2
align 8

multiboot2_start:
    dd 0xe85250d6                ; multiboot2 magic
    dd 0                         ; architecture (i386)
    dd multiboot2_end - multiboot2_start ; header length
    dd -(0xe85250d6 + 0 + (multiboot2_end - multiboot2_start)) ; checksum

    ; End tag
    dw 0                         ; type
    dw 0                         ; flags
    dd 8                         ; size

multiboot2_end:

section .text
bits 32
global _start
extern kernel_main

_start:
    ; Disable interrupts
    cli
    
    ; Set up stack
    mov esp, stack_top
    
    ; Reset EFLAGS
    push 0
    popf
    
    ; Call kernel main
    call kernel_main
    
    ; Hang if kernel returns
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top: