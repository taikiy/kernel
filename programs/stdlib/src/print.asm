[BITS 32]

global print:function

; void print(const char* str)
print:
    push ebp
    mov ebp, esp

    push dword [ebp + 8]
    mov eax, 1  ; sys_print
    int 0x80

    mov esp, ebp
    pop ebp
    ret
