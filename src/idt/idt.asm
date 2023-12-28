section .asm

global load_idt
global int_noop
global int21h
global isr80h
global enable_interrupts
global disable_interrupts

extern int21h_handler
extern int_noop_handler
extern isr80h_handler

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

load_idt:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; first argument passed to this function
    lidt [ebx]

    pop ebp
    ret

int21h:
    pushad          ; pushes the contents of general-purpose registers (EAX, EBX, ECX, EDX, ESP, EBP, ESI and EDI) onto the stack.

    call int21h_handler

    popad
    iret

int_noop:
    pushad          ; pushes the contents of general-purpose registers (EAX, EBX, ECX, EDX, ESP, EBP, ESI and EDI) onto the stack.

    call int_noop_handler

    popad
    iret

; Interrupt Service Routines for handling system calls
isr80h:
                        ; ip, cs, flags, sp, ss are pushed onto the stack by the CPU.
    pushad              ; pushes the contents of general-purpose registers onto the stack. 

    push esp            ; push the stack pointer onto the stack so that we can access the arguments passed to the system call in C.
    push eax            ; push the system call number

    call isr80h_handler

    mov dword[res], eax ; save the return value of the system call in the res variable in case we use eax for something else.
    add esp, 8          ; remove the arguments from the stack

    popad               ; restore the contents of general-purpose registers from the stack.
    mov eax, dword[res] ; move the return value of the system call into the eax register.
    iret

section .data
; used to store the return value of the system call in `isr80h`
res: dd 0