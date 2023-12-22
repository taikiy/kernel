section .asm

global load_tss

load_tss:
    push ebp
    mov ebp, esp
    mov ax, [ebp+8] ; TSS segment
    ltr ax ; Load Task Register - loads the segment selector in ax that points to a TSS
    pop ebp
    ret
