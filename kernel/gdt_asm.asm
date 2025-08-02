; GDT Assembly Functions for RaeenOS
; x86_64 64-bit assembly

bits 64

global gdt_flush
global tss_flush

section .text

; Load the GDT
; Expects GDT pointer as first argument (RDI in x86_64 System V ABI)
gdt_flush:
    lgdt [rdi]          ; Load GDT pointer
    
    ; Reload segment registers
    mov ax, 0x10        ; Data segment selector (offset 0x10 in GDT)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Far jump to reload CS with code segment selector
    push 0x08           ; Code segment selector (offset 0x08 in GDT)
    lea rax, [rel .reload_cs]
    push rax
    retfq               ; Far return to reload CS
    
.reload_cs:
    ret

; Load the TSS
; Expects TSS selector as first argument (RDI in x86_64 System V ABI)
tss_flush:
    ltr di              ; Load Task Register with TSS selector
    ret