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

    ; Disk interrupt preparation
    mov ah, 0x02                ; READ SECTOR command
    mov al, 0x01                ; Read 1 sector
    mov ch, 0x00                ; Cylinder number 0
    mov cl, 0x02                ; Sector number 2
    mov dh, 0x00                ; Head number 0
                                ; We don't set DL. BIOS sets it to the booted disk
    mov bx, buffer              ; Data read will be buffered at ES:BX
    int 0x13

    jc error                    ; if the carry flag is set (error), jump

    mov si, buffer
    call print

    cld                         ; Clears direction flag
    cli                         ; Disables interrupts
    hlt                         ; This hangs the computer


error: 
    mov si, error_message
    call print

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

error_message:
    db 'Failed to load sector', 0

times 510 - ($ - $$) db 0       ; Pad the boot sector to 510 bytes
dw 0xAA55                       ; Boot signature. 55AA (2 bytes) in the little-endian

;; Everything under here is at the second sector from 0x7c00 (0x7e00)

buffer: