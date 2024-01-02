section .asm

global isr_table
global load_idt
global enable_interrupts
global disable_interrupts

extern interrupt_handle_wrapper

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

; Macro to generate Interrupt Service Routines for handling system calls
%macro isr 1
    global isr%1
    isr%1:
                            ; ip, cs, flags, sp, ss are pushed onto the stack by the CPU.
        pushad              ; pushes the contents of general-purpose registers onto the stack. 

        push esp            ; push the stack pointer onto the stack so that we can access the arguments passed to the system call in C.
        push dword %1       ; push the system call number

        call interrupt_handle_wrapper

        mov dword[res], eax ; save the return value of the system call in the res variable in case we use eax for something else.
        add esp, 8          ; remove the arguments from the stack

        popad               ; restore the contents of general-purpose registers from the stack.
        mov eax, dword[res] ; move the return value of the system call into the eax register.
        iret
%endmacro

%assign i 0
%rep 512
    isr i
%assign i i+1
%endrep

section .data
; used to store the return value of the system call in `int80h`
res: dd 0

%macro isr_entry 1
    dd isr%1
%endmacro

isr_table:
%assign i 0
%rep 512
    isr_entry i
%assign i i+1
%endrep
