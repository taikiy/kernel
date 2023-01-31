ORG 0
BITS 16                         ; 16-bit (real mode)

jmp 0x7c0:start

start:
    ; setup the data segment
    cli                         ; Disable interrupts. We don't want interrupts messing with registers
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    mov ax, 0x00
    mov ss, ax
    mov sp, 0x7c00
    sti                         ; Enable interrups

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

times 510 - ($ - $$) db 0       ; 510 - (current position - beginning of the current section)
dw 0xAA55                       ; Little-endian 55AA