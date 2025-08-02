; Kernel Entry Point for RaeenOS
; x86_64 64-bit assembly

bits 64

global _start
extern kernel_main

section .text

_start:
    ; Disable interrupts
    cli
    
    ; Set up stack pointer (assuming we have a stack set up by bootloader)
    ; If not, we need to set up our own stack
    
    ; Clear direction flag
    cld
    
    ; Call the kernel main function
    ; Pass multiboot info if available (in RBX from bootloader)
    mov rdi, rbx    ; First argument (multiboot info pointer)
    call kernel_main
    
    ; If kernel_main returns, halt the system
.halt:
    cli
    hlt
    jmp .halt