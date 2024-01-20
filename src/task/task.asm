[BITS 32]

section .asm

global jump_to_user_space

; void jump_to_user_space(struct registers* registers, int argc, char* argv[]);
jump_to_user_space:
    ; here, we don't push the base pointer because we call `iretd` at the end
    mov ebp, esp

    ; Push the user program arguments to the user program's ESP
    push dword [ebp + 12]       ; argv
    push dword [ebp + 8]        ; argc
    mov ebx, [ebp + 4]          ; `registers`. +4 becuase we didn't push EBP
    push dword [ebx + 40]       ; user program's ESP
    call push_user_program_arguments
    add esp, 12

    ; Push EIP, CS, EFLAGS, ESP, SS. These values will be restored by `iretd`
    ; instruction to jump to the user space. Note that the ESP has been changed
    ; by `push_user_program_arguments` function and stored in EAX.
    mov ebx, [ebp + 4]          ; `registers`. +4 becuase we didn't push EBP
    push dword [ebx + 44]       ; SS
    push dword eax              ; ESP, return value of `push_user_program_arguments`
    pushf                       ; push EFLAGS to the stack
    pop eax                     ; pop EFLAGS into EAX
    or eax, 0x200               ; set the IF flag
    push eax                    ; EFLAGS
    push dword [ebx + 32]       ; CS
    push dword [ebx + 28]       ; EIP

    ; set segment registers
    mov ax, [ebx + 44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Restore general purpose registers. This function must be called after
    ; everything else because it will overwrite registers that we are using.
    push dword [ebp + 4]
    call restore_general_purpose_registers
    add esp, 4

    iretd

; void push_user_program_arguments(void* esp, int argc, char* argv[]);
push_user_program_arguments:
    push ebp
    mov ebp, esp

    mov ebx, [ebp + 8]      ; user program's ESP
    mov edx, [ebp + 12]     ; argc
    mov ecx, [ebp + 16]     ; argv

    sub ebx, 4
    mov dword [ebx], ecx
    sub ebx, 4
    mov dword [ebx], edx

    mov eax, ebx            ; return the new ESP
    mov esp, ebp
    pop ebp
    ret

restore_general_purpose_registers:
    ; No need to push ebp because we are going to overwrite it as a part of
    ; restoring the registers. Just move the esp to ebp to access the argument.
    mov ebp, esp
    mov ebx, [ebp + 4]      ; arg `registers`. +4 becuase we didn't push EBP

    mov eax, [ebx + 0]
    mov ecx, [ebx + 4]
    mov edx, [ebx + 8]
    mov ebp, [ebx + 12]
    mov esi, [ebx + 16]
    mov edi, [ebx + 20]
    mov ebx, [ebx + 24]

    ; Do not call `mov esp, ebp` because we changed the value of ebp
    ret
