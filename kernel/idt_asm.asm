; RaeenOS 64-bit IDT Assembly Functions
; Basic IDT support for x86_64

[bits 64]

section .text

; Global symbols
global idt_load

; Function to load the IDT pointer into the IDTR register
; void idt_load(uint64_t idt_ptr)
idt_load:
    ; RDI contains the pointer to the IDT descriptor
    lidt [rdi]
    ret

; Simple interrupt stubs (we'll keep this minimal for now)
global isr_stub_0
global isr_stub_1
global isr_stub_33  ; Keyboard interrupt

; Exception handler stub (divide by zero)
isr_stub_0:
    cli
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    ; Call C handler (if we had one)
    ; call exception_handler
    
    ; For now, just halt
    hlt
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    sti
    iretq

; Debug exception stub
isr_stub_1:
    cli
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    ; For now, just halt
    hlt
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    sti
    iretq

; Keyboard interrupt handler stub (IRQ1)
isr_stub_33:
    cli
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    ; Call our keyboard handler
    extern keyboard_handler
    call keyboard_handler
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    sti
    iretq