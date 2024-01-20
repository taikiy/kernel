[BITS 32]

section .asm

global make_syscall:function

; int make_syscall(uint32_t syscall_id, int argc, ...)
make_syscall:
    push ebp
    mov ebp, esp

    mov edx, [ebp + 12]     ; argc
    lea ecx, [ebp + 16]     ; argv
    ; push the arguments in reverse order
    _push_args_loop:
        cmp edx, 0
        je _push_args_loop_end

        mov eax, [ecx + edx * 4 - 4]    ; argv[edx - 1]
        push eax

        dec edx
        jmp _push_args_loop
    _push_args_loop_end:

    mov eax, [ebp+8]    ; syscall_id
    int 0x80

    mov esp, ebp
    pop ebp
    ret
