ORG 0x7c00                      ; bootloader location address
BITS 16                         ; 16-bit (real mode)

; https://en.wikipedia.org/wiki/INT_10H
start:
    mov ah, 0eh                 ; Teletype output
    mov al, 'A'
    int 0x10                    ; BIOS interrupt call 0x10

    jmp $                       ; Halts; keeps jumping to itself

times 510 - ($ - $$) db 0       ; 510 - (current position - beginning of the current section)
dw 0xAA55                       ; Little-endian 55AA