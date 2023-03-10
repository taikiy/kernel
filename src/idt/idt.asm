section .asm

global idt_load
global int21h
global int_noop
global enable_interrupts
global disable_interrupts

extern int21h_handler
extern int_noop_handler

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

idt_load:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; first argument passed to this function
    lidt [ebx]

    pop ebp
    ret

int21h:
    cli
    pushad          ; pushes the contents of general-purpose registers (EAX, EBX, ECX, EDX, ESP, EBP, ESI and EDI) onto the stack.

    call int21h_handler

    popad
    sti
    iret

int_noop:
    cli
    pushad          ; pushes the contents of general-purpose registers (EAX, EBX, ECX, EDX, ESP, EBP, ESI and EDI) onto the stack.

    call int_noop_handler

    popad
    sti
    iret
