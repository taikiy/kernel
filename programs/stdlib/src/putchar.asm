[BITS 32]

section .asm

global putchar:function

; int putchar(int c)
putchar:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]    ; get the character to print
    push eax
    mov eax, 3          ; sys_putchar
    int 0x80

    mov esp, ebp
    pop ebp
    ret
