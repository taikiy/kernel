[BITS 32]

global _start

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

    cld                         ; Clears direction flag
    cli                         ; Disables interrupts
    hlt                         ; This hangs the computer

times 512 - ($ - $$) db 0       ; Pad the kernel code sector to 512 bytes
                                ; This ensures that any object files written in C and linked with this assembly
                                ; will be correctly aligned.
