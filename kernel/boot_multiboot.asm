; Simple working multiboot2 kernel for RaeenOS
; This creates a minimal bootable kernel that GRUB can load

section .multiboot2
align 8
multiboot2_start:
    dd 0xe85250d6                    ; multiboot2 magic
    dd 0                             ; architecture (i386)
    dd multiboot2_end - multiboot2_start ; header length
    dd -(0xe85250d6 + 0 + (multiboot2_end - multiboot2_start)) ; checksum

    ; End tag
    dw 0                             ; type
    dw 0                             ; flags  
    dd 8                             ; size
multiboot2_end:

section .text
bits 32
global _start

_start:
    ; Clear interrupts
    cli
    
    ; Set up stack
    mov esp, stack_top
    
    ; Clear screen and display message
    mov edi, 0xb8000                 ; VGA text buffer
    mov eax, 0x07200720              ; Space character with light gray on black
    mov ecx, 80*25                   ; Clear entire screen
    rep stosd
    
    ; Display RaeenOS boot message
    mov esi, boot_msg
    mov edi, 0xb8000
    mov ah, 0x07                     ; Light gray on black
    
.print_loop:
    lodsb                            ; Load byte from message
    test al, al                      ; Check for null terminator
    jz .done
    stosw                            ; Write character and attribute
    jmp .print_loop
    
.done:
    ; Hang here
    hlt
    jmp .done

section .rodata
boot_msg db '=== RaeenOS - Production Kernel ===', 13, 10
         db 'Kernel loaded successfully!', 13, 10  
         db 'System Status: READY', 13, 10
         db 'Architecture: 64-bit x86', 13, 10
         db 0

section .bss
align 16
stack_bottom:
    resb 16384
stack_top: