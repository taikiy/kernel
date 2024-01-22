[BITS 32]

section .asm

global inject_process_arguments_to_stack

; void inject_process_arguments_to_stack(struct registers* registers, int argc, char* argv[]);
inject_process_arguments_to_stack:
    push ebp
    mov ebp, esp

    mov edx, [ebp + 16]     ; `argv`
    mov ecx, [ebp + 12]     ; `argc`
    mov eax, [ebp + 8]      ; `registers`
    mov ebx, [eax + 40]     ; user program's ESP

    ; push the arguments to the user program's stack
    sub ebx, 4
    mov dword [ebx], edx
    sub ebx, 4
    mov dword [ebx], ecx

    mov eax, ebx            ; return the new ESP
    mov esp, ebp
    pop ebp
    ret
