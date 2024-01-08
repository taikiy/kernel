[BITS 32]

global getchar:function

; int getchar(void)
getchar:
    push ebp
    mov ebp, esp

l1:
    mov eax, 2      ; sys_getchar
    int 0x80
    cmp eax, 0x00   ; if sys_getchar() == 0x00
    je l1           ; loop until sys_getchar() returns a key

    mov esp, ebp
    pop ebp
    ret
