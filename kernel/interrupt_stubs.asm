; interrupt_stubs.asm
; Assembly interrupt stubs for RaeenOS
; These stubs save processor state and call the common handler

section .text
bits 64

; External common handler
extern idt_common_handler

; Macro for exception handlers without error code
%macro ISR_NOERRCODE 1
    global isr%1
    isr%1:
        push 0          ; Push dummy error code
        push %1         ; Push interrupt number
        jmp isr_common_stub
%endmacro

; Macro for exception handlers with error code
%macro ISR_ERRCODE 1
    global isr%1
    isr%1:
        push %1         ; Push interrupt number
        jmp isr_common_stub
%endmacro

; Macro for IRQ handlers
%macro IRQ 2
    global irq%1
    irq%1:
        push 0          ; Push dummy error code
        push %2         ; Push interrupt number
        jmp isr_common_stub
%endmacro

; Exception handlers
ISR_NOERRCODE 0     ; Divide Error
ISR_NOERRCODE 1     ; Debug Exception
ISR_NOERRCODE 2     ; Non-Maskable Interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Overflow
ISR_NOERRCODE 5     ; Bound Range Exceeded
ISR_NOERRCODE 6     ; Invalid Opcode
ISR_NOERRCODE 7     ; Device Not Available
ISR_ERRCODE   8     ; Double Fault
ISR_NOERRCODE 9     ; Coprocessor Segment Overrun (legacy)
ISR_ERRCODE   10    ; Invalid TSS
ISR_ERRCODE   11    ; Segment Not Present
ISR_ERRCODE   12    ; Stack Fault
ISR_ERRCODE   13    ; General Protection Fault
ISR_ERRCODE   14    ; Page Fault
ISR_NOERRCODE 15    ; Reserved
ISR_NOERRCODE 16    ; x87 FPU Error
ISR_ERRCODE   17    ; Alignment Check
ISR_NOERRCODE 18    ; Machine Check
ISR_NOERRCODE 19    ; SIMD Exception
ISR_NOERRCODE 20    ; Virtualization Exception
ISR_NOERRCODE 21    ; Control Protection Exception

; IRQ handlers (32-47 for legacy PIC)
IRQ 0, 32    ; Timer
IRQ 1, 33    ; Keyboard
IRQ 2, 34    ; Cascade
IRQ 3, 35    ; COM2
IRQ 4, 36    ; COM1
IRQ 5, 37    ; LPT2
IRQ 6, 38    ; Floppy
IRQ 7, 39    ; LPT1
IRQ 8, 40    ; CMOS Clock
IRQ 9, 41    ; Free
IRQ 10, 42   ; Free
IRQ 11, 43   ; Free
IRQ 12, 44   ; PS2 Mouse
IRQ 13, 45   ; FPU
IRQ 14, 46   ; Primary ATA
IRQ 15, 47   ; Secondary ATA

; Common stub that saves all registers and calls the handler
isr_common_stub:
    ; Save all general-purpose registers
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
    
    ; Save segment registers
    mov ax, ds
    push rax
    
    ; Load kernel data segment
    mov ax, 0x10   ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call the common interrupt handler
    ; RSP points to the exception frame
    mov rdi, rsp
    call idt_common_handler
    
    ; Restore segment registers
    pop rax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Restore all general-purpose registers
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
    
    ; Clean up error code and interrupt number
    add rsp, 16
    
    ; Return from interrupt
    iretq