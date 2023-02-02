ORG 0
BITS 16                         ; 16-bit (real mode)

; BIOS Parameter Block
; https://wiki.osdev.org/FAT#BPB_.28BIOS_Parameter_Block.29
_start:
    jmp short init
    nop
times 33 db 0

init:
    jmp 0x7c0:start

handle_zero:
    mov ah, 0eh
    mov al, 'A'
    mov bx, 0x00
    int 0x10
    iret

handle_one:
    mov ah, 0eh
    mov al, 'B'
    mov bx, 0x00
    int 0x10
    iret

start:
    ; setup the data segment
    cli                         ; Disable interrupts. We don't want interrupts messing with registers
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    mov ax, 0x00
    mov ss, ax
    mov sp, 0x7c00
    sti                         ; Enables interrups

    ; Add a custom interrupt handler for Int0h (address 0x00).
    ; word[0x00] will use `ds` as the segment by default, which at this point of the code points at
    ; 0x7c0. We explicitly specify [ss:0x00], which points at 0x00, set in the code above.
    mov word[ss:0x00], handle_zero  ; First two bytes of RAM - offset
    mov word[ss:0x02], 0x7c0        ; Second two bytes of RAM - segment

    mov word[ss:0x04], handle_one
    mov word[ss:0x06], 0x7c0

    int 0                       ; same as mov ax, 0x00; div ax
    int 1

    mov si, message             ; SI = Source Index
    call print
    
    cld                         ; Clears direction flag

    ; this hangs the computer
    cli
    hlt

print:
    ; setup INT10h params
    mov ah, 0eh                 ; Teletype output
    mov bx, 0                   ; bh = page number, bl = color

.loop:
    lodsb                       ; Load a byte from DS:SI into AL, then increase SI (see "Notes" in ./doc/real_mode_development.md)

    cmp al, 0                   ; if AL contains a null-byte, stop
    je .done

    int 0x10                    ; BIOS interrupt call 0x10. https://en.wikipedia.org/wiki/INT_10H

    jmp .loop

.done:
    ret

message:
    db 'Hello, World!', 0

times 510 - ($ - $$) db 0       ; Pad the boot sector to 510 bytes
dw 0xAA55                       ; Boot signature. 55AA (2 bytes) in the little-endian