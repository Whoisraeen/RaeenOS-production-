; RaeenOS Paging Assembly Functions

global load_page_directory
global enable_paging

; Loads the page directory address into CR3
load_page_directory:
    mov eax, [esp + 4]  ; Get the address of the page directory from the stack
    mov cr3, eax        ; Load it into the CR3 register
    ret

; Enables the paging bit in the CR0 register
enable_paging:
    mov eax, cr0
    or eax, 0x80000000 ; Set the paging bit (bit 31)
    mov cr0, eax
    ret
