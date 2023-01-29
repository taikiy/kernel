ORG 0x7c00                      ; bootloader location address
BITS 16                         ; 16-bit (real mode)

; https://en.wikipedia.org/wiki/INT_10H
start:
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
    lodsb                       ; Load a byte from SI into AL, then increase SI

    cmp al, 0                   ; if AL contains a null-byte, stop
    je .done

    int 0x10                    ; BIOS interrupt call 0x10

    jmp .loop

.done:
    ret

message:
    db 'Hello, World!', 0

times 510 - ($ - $$) db 0       ; 510 - (current position - beginning of the current section)
dw 0xAA55                       ; Little-endian 55AA