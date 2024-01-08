[BITS 32]

section .asm

global free:function

; void free(void *ptr)
free:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]    ; `ptr` argument
    push eax
    mov eax, 5          ; sys_free
    int 0x80

    mov esp, ebp
    pop ebp
    ret
