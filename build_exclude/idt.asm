; RaeenOS Interrupt Descriptor Table (IDT) Assembly Handlers

; External C function to be called from our ISR handler
extern isr_handler
extern syscall_handler

; Global symbols that will be used in our C code
global idt_load
global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7, isr8, isr9, isr10
global isr11, isr12, isr13, isr14, isr15, isr16, isr17, isr18, isr19, isr20
global isr21, isr22, isr23, isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
global isr32, isr33, isr34, isr35, isr36, isr37, isr38, isr39, isr40, isr41, isr42, isr43, isr44, isr45, isr46, isr47
global isr128

; Function to load the IDT pointer into the IDTR register
idt_load:
    mov eax, [esp + 4]  ; Get the pointer to the IDT structure from the stack
    lidt [eax]          ; Load the IDT
    ret

; Macro to define common ISR code (for exceptions that don't push an error code)
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli             ; Disable interrupts
    push byte 0     ; Push a dummy error code
    push byte %1    ; Push the interrupt number
    jmp isr_common_stub
%endmacro

; Macro to define ISRs that do push an error code
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli             ; Disable interrupts
    push byte %1    ; Push the interrupt number
    jmp isr_common_stub
%endmacro

; Create the first 32 ISRs (ISRs 0-31 are for CPU exceptions)
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_ERRCODE   16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE   30
ISR_NOERRCODE 31

; IRQs (Hardware Interrupts)
ISR_NOERRCODE 32
ISR_NOERRCODE 33
ISR_NOERRCODE 34
ISR_NOERRCODE 35
ISR_NOERRCODE 36
ISR_NOERRCODE 37
ISR_NOERRCODE 38
ISR_NOERRCODE 39
ISR_NOERRCODE 40
ISR_NOERRCODE 41
ISR_NOERRCODE 42
ISR_NOERRCODE 43
ISR_NOERRCODE 44
ISR_NOERRCODE 45
ISR_NOERRCODE 46
ISR_NOERRCODE 47

; ISR 128 is for system calls
ISR_NOERRCODE 128

; Common stub for all ISRs
isr_common_stub:
    ; Check if the interrupt is the syscall interrupt (128)
    cmp byte [esp + 4], 128
    je sys_call_stub
    pusha           ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
    mov ax, ds      ; Get the current data segment descriptor
    push eax        ; Push it onto the stack
    mov ax, 0x10    ; Load the kernel data segment descriptor (should be 0x10)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler ; Call our C handler

    pop ebx         ; Pop the original data segment descriptor
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popa            ; Pop edi,esi,ebp,esp,ebx,edx,ecx,eax
    add esp, 8      ; Clean up the error code and ISR number
    sti             ; Re-enable interrupts
    iret            ; Pop CS, EIP, EFLAGS, SS, and ESP

; Common stub for system calls
sys_call_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call syscall_handler

    pop ebx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popa
    add esp, 8
    sti
    iret
