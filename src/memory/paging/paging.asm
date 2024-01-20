[BITS 32]

section .asm

global load_directory
global enable_paging
global set_kernel_segment_registers
global set_user_segment_registers

KERNEL_DATA_SEG equ 0x10
USER_DATA_SEG   equ 0x23   ; user data segment | flags: 0x3

load_directory:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]
    mov cr3, eax

    mov esp, ebp
    pop ebp
    ret

enable_paging:
    push ebp
    mov ebp, esp

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov esp, ebp
    pop ebp
    ret

set_kernel_segment_registers:
    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret

set_user_segment_registers:
    mov ax, USER_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret
