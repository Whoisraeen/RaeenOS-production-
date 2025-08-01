bits 64

extern isr_handler

%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push byte %1
    jmp isr_common_stub
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
; ... Add all 32 exception handlers

isr_common_stub:
    push rax
    push rbx
    ; ... push all other registers

    mov rdi, rsp ; Pass stack pointer to C handler
    call isr_handler

    ; ... pop all registers
    pop rbx
    pop rax

    add rsp, 16 ; Clean up error code and ISR number
    sti
    iretq