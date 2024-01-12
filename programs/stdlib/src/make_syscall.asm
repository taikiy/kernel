[BITS 32]

section .asm

global make_syscall:function

; int make_syscall(uint32_t syscall_id, int argc, uint32_t argv[])
make_syscall:
    push ebp
    mov ebp, esp

    mov edx, [ebp+12]   ; argc
    mov ecx, [ebp+16]   ; argv

read_args:
    cmp edx, 0
    je send_int80h

    mov eax, [ecx]      ; argv[i]
    push eax

    sub edx, 1
    add ecx, 4
    jmp read_args

send_int80h:
    mov eax, [ebp+8]    ; syscall_id
    int 0x80

    mov esp, ebp
    pop ebp
    ret
