; RaeenOS Paging Assembly Functions (64-bit)

global load_page_directory
global enable_paging

; Loads the page directory address into CR3
load_page_directory:
    mov rax, rdi        ; First argument in RDI (System V ABI)
    mov cr3, rax        ; Load it into the CR3 register
    ret

; Enables the paging bit in the CR0 register
enable_paging:
    mov rax, cr0
    or rax, 0x80000000  ; Set the paging bit (bit 31)
    mov cr0, rax
    ret
