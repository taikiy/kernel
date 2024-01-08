[BITS 32]

section .asm

global malloc:function

; void* malloc(size_t size)
malloc:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]    ; `size` argument
    push eax
    mov eax, 4          ; sys_malloc
    int 0x80

    mov esp, ebp
    pop ebp
    ret
