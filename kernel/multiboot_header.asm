; Multiboot2 Header for 64-bit RaeenOS Kernel
; This makes the kernel bootable by GRUB2

section .multiboot_header
align 8
multiboot_start:
    dd 0xE85250D6                ; Multiboot2 magic number
    dd 0                         ; Architecture (0 = i386)
    dd multiboot_end - multiboot_start  ; Header length
    
    ; Checksum
    dd 0x100000000 - (0xE85250D6 + 0 + (multiboot_end - multiboot_start))
    
    ; End tag
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
multiboot_end: