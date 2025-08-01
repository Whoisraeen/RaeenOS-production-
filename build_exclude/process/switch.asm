; RaeenOS Context Switching Implementation
; --------------------------------------
; This file contains the low-level assembly routine for switching between processes.
; It's the core of the multitasking scheduler.

[bits 32]

global context_switch

; void context_switch(uint32_t* old_esp, uint32_t new_esp);
; C-callable function to switch the CPU context.
; It saves the old process's registers and loads the new one's.
;
; The magic happens on the 'ret' instruction, which will pop the new process's
; EIP off the stack, effectively jumping to the new process's code.

context_switch:
    ; 1. Save the current process's context
    ;    The stack looks like this when we enter context_switch:
    ;    [old_esp] (return address from caller)
    ;    [new_esp]
    ;    [old_esp_ptr]

    push ebp
    mov ebp, esp

    ; Save general purpose registers
    push eax
    push ecx
    push edx
    push ebx
    push esi
    push edi

    ; Save the current stack pointer (ESP) into the old_esp_ptr
    mov eax, [ebp + 8] ; old_esp_ptr
    mov [eax], esp

    ; 2. Load the new process's context
    mov esp, [ebp + 12] ; new_esp

    ; Restore general purpose registers
    pop edi
    pop esi
    pop ebx
    pop edx
    pop ecx
    pop eax

    ; Restore EBP and return
    pop ebp
    ret