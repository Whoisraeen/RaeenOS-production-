; Multiboot2 header for RaeenOS - Compatible with GRUB
section .multiboot2
align 8
multiboot2_start:
    dd 0xe85250d6                    ; multiboot2 magic
    dd 0                             ; architecture 0 (i386)
    dd multiboot2_end - multiboot2_start ; header length
    dd -(0xe85250d6 + 0 + (multiboot2_end - multiboot2_start)) ; checksum

    ; Information request tag
    dw 1                             ; type (information request)
    dw 0                             ; flags
    dd 20                            ; size
    dd 1                             ; basic memory info
    dd 6                             ; memory map
    dd 8                             ; framebuffer

    ; Entry address tag
    dw 3                             ; type (entry address)
    dw 0                             ; flags
    dd 12                            ; size
    dd _start                        ; entry point address

    ; End tag
    dw 0                             ; type
    dw 0                             ; flags
    dd 8                             ; size

multiboot2_end:

section .text
bits 32
global _start
extern kernel_main

_start:
    ; We're in 32-bit protected mode with paging disabled
    cli                              ; Disable interrupts
    
    ; Save multiboot2 information
    mov edi, eax                     ; Magic value
    mov esi, ebx                     ; Multiboot info structure
    
    ; Set up stack
    mov esp, stack_top
    
    ; Clear EFLAGS
    push 0
    popf
    
    ; Initialize basic systems that need 32-bit mode
    ; (We'll transition to 64-bit in C code if needed)
    
    ; Call C kernel main function
    ; Push multiboot info as parameters (cdecl calling convention)
    push esi                         ; multiboot info pointer
    push edi                         ; magic value
    call kernel_main
    
    ; If kernel returns, halt
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384                       ; 16KB stack
stack_top: