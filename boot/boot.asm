; RaeenOS Bootloader
; ------------------

org 0x7c00      ; BIOS loads us at this address
bits 16         ; We start in 16-bit real mode

; --- Multiboot Header ---
; This tells the bootloader (e.g., GRUB) that this is a Multiboot kernel.
; It must be within the first 8KB of the kernel image.
; For now, we place it directly in the bootloader for simplicity, assuming
; the kernel is loaded immediately after the bootloader.

MULTIBOOT_PAGE_ALIGN equ 1<<0
MULTIBOOT_MEMORY_INFO equ 1<<1
MULTIBOOT_AOUT_KLUDGE equ 1<<16 ; Required for a.out kernels, but good practice

MULTIBOOT_HEADER_MAGIC equ 0x1BADB002 ; Multiboot 1 magic number
MULTIBOOT_HEADER_FLAGS equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

section .multiboot_header
    align 4
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM

; --- Entry point for the bootloader ---
start:
    ; --- Setup segment registers ---
    ; BIOS loads us with CS:IP = 0x07C0:0x0000. We want DS, ES, SS to point to 0.
    ; We also need a stack. Also save boot drive number from DL.
    mov [BOOT_DRIVE], dl ; Save boot drive number that BIOS provides in DL
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00 ; Stack grows downwards from bootloader start

    ; --- Clear the screen ---
    mov ah, 0x06    ; Scroll up function
    mov al, 0x00    ; Clear entire window
    mov bh, 0x07    ; White on black text
    mov cx, 0x0000  ; Top-left corner (0,0)
    mov dx, 0x184f  ; Bottom-right corner (80,25)
    int 0x10

    ; --- Print welcome message ---
    mov si, welcome_msg
    call print_string

    ; --- Get Memory Map (using BIOS INT 0xE820) ---
    ; This is crucial for the kernel to understand available memory.
    ; The Multiboot spec expects the memory map to be passed in EBX and EAX.
    ; EAX = magic number (0x2BADB002)
    ; EBX = address of the Multiboot information structure

    mov edi, MEMORY_MAP_BUFFER ; Buffer to store memory map entries
    mov ebx, 0                 ; Continuation value (must be 0 for first call)
    mov edx, 0x534D4150        ; 'SMAP' magic number
    mov ecx, 24                ; Size of buffer for each entry
    mov dword [mmap_length], 0 ; Initialize entry count

.get_mmap_loop:
    mov eax, 0xE820            ; Function code for memory map
    mov [edi + 20], dword 1    ; Set ACPI attributes (always 1)
    int 0x15                   ; Call BIOS interrupt
    jc .get_mmap_error         ; If carry flag set, error occurred
    cmp eax, 0x534D4150        ; Check if EAX contains 'SMAP' (success)
    jne .get_mmap_error        ; If not, error occurred
    add edi, 24                ; Move to next buffer position
    inc dword [mmap_length]    ; Increment entry count
    cmp ebx, 0                 ; Check if more entries exist
    jne .get_mmap_loop         ; Loop if EBX is not 0 (more entries)

    jmp .mmap_done

.get_mmap_error:
    mov si, mmap_error_msg
    call print_string
    ; For now, we'll just proceed without a memory map, but a real OS would panic.

.mmap_done:
    ; --- Load kernel from disk ---
    mov si, loading_msg
    call print_string

    mov bx, 0x1000              ; Set memory address to load kernel (0x1000:0x0000 = 0x10000)
    mov dh, 16                  ; Number of sectors to read (16 sectors * 512 bytes = 8KB)
    mov dl, [BOOT_DRIVE]        ; Get boot drive number from BIOS
    call load_disk              ; Call our disk loading function

    jc load_failed              ; If carry flag is set, loading failed

    ; --- Prepare for kernel jump (32-bit protected mode) ---
    ; Enable A20 line (required for >1MB memory access)
    call enable_a20

    ; Load GDT (Global Descriptor Table)
    lgdt [gdt_ptr]

    ; Set CR0 bit 0 (PE - Protection Enable) to enter protected mode
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; Far jump to 32-bit code segment
    jmp dword 0x08:protected_mode_start

load_failed:
    mov si, load_failed_msg
    call print_string

halt:
    cli
    hlt
    jmp halt

; --- Function to load sectors from disk using BIOS int 13h ---
; IN: bx = load address, dh = number of sectors, dl = drive number
; OUT: carry flag set on error
load_disk:
    pusha
    mov ah, 0x02                ; BIOS read sector function
    mov al, dh                  ; Number of sectors to read
    mov ch, 0                   ; Cylinder 0
    mov cl, 2                   ; Start at sector 2 (sector 1 is the bootloader)
    mov dh, 0                   ; Head 0
    ; es:bx is the buffer address. BIOS sets es=ds=0 for us.
    int 0x13
    popa
    ret

; --- Function to print a null-terminated string ---
print_string:
    mov ah, 0x0e    ; Teletype output function
.loop:
    lodsb           ; Load byte from [si] into al, and increment si
    cmp al, 0       ; Check for null terminator
    je .done
    int 0x10        ; Print character
    jmp .loop
.done:
    ret

; --- Enable A20 line (Gate A20) ---
; This allows access to memory above 1MB.
enable_a20:
    in al, 0x92      ; Read keyboard controller port
    or al, 2         ; Set A20 bit
    out 0x92, al     ; Write back
    ret

; --- Global Descriptor Table (GDT) ---
; Defines memory segments for protected mode.

gdt_start:
; Null descriptor
    dq 0x0

; Code segment descriptor (0x08)
; Base = 0, Limit = 0xFFFFF (4GB), Present, DPL=0 (kernel), Executable, Read/Write, 32-bit
gdt_code:
CODE_SEG equ gdt_code - gdt_start
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10011010b    ; Access byte (Present, DPL=0, Code, Read/Write)
    db 11001111b    ; Flags (Granularity=4KB, 32-bit) + Limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)

; Data segment descriptor (0x10)
; Base = 0, Limit = 0xFFFFF (4GB), Present, DPL=0 (kernel), Read/Write, 32-bit
gdt_data:
DATA_SEG equ gdt_data - gdt_start
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b    ; Access byte (Present, DPL=0, Data, Read/Write)
    db 11001111b    ; Flags (Granularity=4KB, 32-bit) + Limit (bits 16-19)
    db 0x00
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1 ; Limit (size of GDT - 1)
    dd gdt_start               ; Base address of GDT

; --- 32-bit Protected Mode Entry ---
[bits 32]
protected_mode_start:
    ; Reload segment registers with protected mode selectors
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, KERNEL_STACK_TOP ; Set up a temporary kernel stack

    ; Copy kernel from 16-bit load location to final 32-bit location
    call copy_kernel_to_1mb

    ; Pass Multiboot info to kernel_main
    ; EAX = Multiboot magic (0x2BADB002)
    ; EBX = Address of Multiboot info structure
    mov eax, 0x2BADB002
    mov ebx, MEMORY_MAP_BUFFER ; This is where the Multiboot info structure will be

    ; Call the kernel's main function
    ; The kernel expects mmap_addr (EBX) and mmap_length (EAX) as arguments.
    ; We need to adjust this to match the C function signature.
    ; For now, we'll pass the memory map buffer address and the count of entries.
    ; The kernel will then parse the Multiboot info structure.
    push dword [mmap_length] ; Arg 2: mmap_length
    push dword MEMORY_MAP_BUFFER ; Arg 1: mmap_addr
    call 0x100000  ; Call kernel at physical address 0x100000

    ; If kernel_main returns (it shouldn't in a real OS), halt.
    cli
    hlt

; --- Copy kernel from 16-bit load location to 1MB ---
copy_kernel_to_1mb:
    ; Copy 8KB (16 sectors * 512 bytes) from 0x10000 to 0x100000
    mov esi, 0x10000        ; Source: where we loaded the kernel in 16-bit mode
    mov edi, KERNEL_LOAD_ADDR ; Destination: 1MB
    mov ecx, 2048           ; Copy 8KB (2048 dwords)
    rep movsd               ; Copy ECX dwords from ESI to EDI
    ret

; --- Constants ---
KERNEL_LOAD_SEGMENT equ 0x1000    ; Load kernel at segment 0x1000 (0x10000 physical)
KERNEL_LOAD_ADDR equ 0x100000     ; Final kernel address at 1MB
MEMORY_MAP_BUFFER equ 0x8000      ; Buffer for memory map entries (below 1MB)
KERNEL_STACK_TOP equ 0x70000      ; Temporary stack for protected mode

; --- Data ---
BOOT_DRIVE db 0
welcome_msg db 'Welcome to RaeenOS!', 0
loading_msg db 'Loading kernel...', 0
load_failed_msg db 'Error: Kernel load failed!', 0
mmap_error_msg db 'Error: Failed to get memory map!', 0

mmap_length dd 0 ; Stores the number of memory map entries

; --- Bootloader Padding and Signature ---
times 510 - ($ - $$) db 0   ; Pad the rest of the sector with 0s
dw 0xaa55                   ; BIOS boot signature