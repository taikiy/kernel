[BITS 32]

section .asm

global return_to_task
global set_user_segment_registers

; void return_to_task(struct registers* registers);
return_to_task:
    ; here, we don't push the base pointer because we call `iretd` at the end
    mov ebp, esp
    mov ebx, [ebp + 4] ; +4 becuase we didn't push EBP

    ; push the data/stack selectors SS
    push dword [ebx + 44]
    ; push the stack pointer ESP
    push dword [ebx + 40]
    ; push the flags
    pushf
    pop eax
    or eax, 0x200
    push eax
    ; push the code selector CS
    push dword [ebx + 32]
    ; push the instruction pointer IP
    push dword [ebx + 28]

    ; setup segment registers
    mov ax, [ebx + 44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword [ebp + 4]
    call restore_general_purpose_registers
    add esp, 4

    iretd

; void restore_general_purpose_registers(struct registers* registers);
restore_general_purpose_registers:
    push ebp
    mov ebp, esp
    mov ebx, [ebp + 8] ; ebx = registers
    mov eax, [ebx + 0]
    mov ecx, [ebx + 4]
    mov edx, [ebx + 8]
    mov ebp, [ebx + 12]
    mov esi, [ebx + 14]
    mov edi, [ebx + 16]
    mov ebx, [ebx + 20]
    pop ebp
    ret

set_user_segment_registers:
    mov ax, 0x23 ; user data segment | flags: 0x3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret