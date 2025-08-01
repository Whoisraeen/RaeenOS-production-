bits 64

section .text
global _start
extern kernel_main

_start:
    ; The bootloader has already put us in 64-bit long mode.
    ; The PML4, GDT, and a temporary stack are already set up.
    ; We just need to call the C kernel.

    ; Clear interrupts before setting up our own IDT
    cli

    ; Call the kernel's main function
    ; The multiboot info pointer is passed in RDI by the bootloader
    call kernel_main

    ; If kernel_main returns, hang the system
    hlt
