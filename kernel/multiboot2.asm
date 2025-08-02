; Multiboot2 header for RaeenOS kernel
section .multiboot2
align 8

multiboot2_start:
    dd 0xe85250d6                ; multiboot2 magic
    dd 0                         ; architecture (i386 - required for initial 32-bit boot)
    dd multiboot2_end - multiboot2_start ; header length
    dd -(0xe85250d6 + 0 + (multiboot2_end - multiboot2_start)) ; checksum

    ; Information request tag
    align 8
    dw 1                         ; type
    dw 0                         ; flags
    dd 20                        ; size
    dd 1                         ; mbi_tag_basic_meminfo
    dd 6                         ; mbi_tag_mmap
    dd 8                         ; mbi_tag_framebuffer

    ; End tag
    align 8
    dw 0                         ; type
    dw 0                         ; flags
    dd 8                         ; size

multiboot2_end:

[bits 32]
section .text
global _start
extern kernel_main

_start:
    ; Multiboot2 bootloader puts us in 32-bit protected mode
    ; We need to set up long mode ourselves
    
    ; Disable interrupts
    cli
    
    ; Save multiboot info
    mov edi, ebx                 ; Save multiboot info pointer
    
    ; Check if we're loaded by multiboot2 bootloader
    cmp eax, 0x36d76289
    jne .no_multiboot
    
    ; Set up temporary stack
    mov esp, temp_stack_top
    
    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    
    ; Set up initial page tables
    ; Identity map first 2MB
    mov eax, page_table_l1
    or eax, 0x03                 ; Present + writable
    mov [page_table_l2], eax
    
    mov eax, page_table_l2
    or eax, 0x03
    mov [page_table_l3], eax
    
    mov eax, page_table_l3
    or eax, 0x03
    mov [page_table_l4], eax
    
    ; Fill L1 table (identity map first 2MB)
    mov ecx, 0
    mov eax, 0x03                ; Present + writable
.fill_l1:
    mov [page_table_l1 + ecx * 8], eax
    add eax, 0x1000              ; Next page (4KB)
    inc ecx
    cmp ecx, 512                 ; 512 entries * 4KB = 2MB
    jne .fill_l1
    
    ; Load page table
    mov eax, page_table_l4
    mov cr3, eax
    
    ; Enable long mode
    mov ecx, 0xC0000080          ; EFER MSR
    rdmsr
    or eax, 1 << 8               ; Long mode enable
    wrmsr
    
    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31              ; Paging enable
    mov cr0, eax
    
    ; Load 64-bit GDT
    lgdt [gdt64_pointer]
    
    ; Far jump to 64-bit code
    jmp 0x08:long_mode_start

.no_multiboot:
    ; Halt if not loaded by multiboot
    hlt
    jmp .no_multiboot

[bits 64]
long_mode_start:
    ; Set up 64-bit segment registers
    mov ax, 0x10                 ; Data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up proper stack
    mov rsp, kernel_stack_top
    
    ; Clear direction flag
    cld
    
    ; Call kernel main with multiboot info
    xor rdi, rdi                 ; Clear rdi for kernel_main (no multiboot info needed)
    call kernel_main
    
    ; If kernel returns, halt
.halt:
    hlt
    jmp .halt

; 64-bit GDT
section .data
align 8
gdt64:
    dq 0x0000000000000000        ; Null descriptor
    dq 0x00A09A0000000000        ; Code segment (64-bit)
    dq 0x00A0920000000000        ; Data segment (64-bit)
gdt64_end:

gdt64_pointer:
    dw gdt64_end - gdt64 - 1     ; Limit
    dq gdt64                     ; Base

; Page tables (aligned to 4KB)
section .bss
align 4096
page_table_l4:
    resb 4096
page_table_l3:
    resb 4096
page_table_l2:
    resb 4096
page_table_l1:
    resb 4096

; Temporary stack for 32-bit mode
temp_stack_bottom:
    resb 4096
temp_stack_top:

; Main kernel stack
kernel_stack_bottom:
    resb 16384
kernel_stack_top: