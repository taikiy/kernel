[BITS 32]

global _start

extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp

    ; Enable the A20 Line
    in al, 0x92                 ; Read from the bus line 92
    or al, 2
    out 0x92, al                ; Write to the bus line 92

    ; Remap the master PIC
    mov al, 00010001b           ; Set PIC to initialization mode
    out 0x20, al                ; Send the *command* to master PIC via port 0x20
    mov al, 0x20                ; IRQ 0 mapped to INT 0x20. Following IRQs (1-7) are also mapped accordingly.
    out 0x21, al                ; Send the *data* to master PIC via port 0x21
    mov al, 00000001b           ; Set PIC to x86 mode
    out 0x21, al                ; We finish the initialization by sending the mode configuration to the *data* port.

    ; Call our kernel
    call kernel_main

    jmp $                       ; Go into an infinite loop. It lets us interact with the kernel instead of coming to a halt.
    ; cld                       ; Clears direction flag
    ; cli                       ; Disables interrupts
    ; hlt                       ; This hangs the computer

times 512 - ($ - $$) db 0       ; Pad the kernel code sector to 512 bytes
                                ; This ensures that any object files written in C and linked with this assembly
                                ; will be correctly aligned.
