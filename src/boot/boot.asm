ORG 0x7c00
BITS 16                         ; 16-bit (real mode)

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; BIOS Parameter Block
; https://wiki.osdev.org/FAT#BPB_.28BIOS_Parameter_Block.29
_start:
    jmp short init
    nop
times 33 db 0

init:
    jmp 0:start

start:
    ; setup the data segment
    cli                         ; Disable interrupts. We don't want interrupts messing with registers
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti                         ; Enables interrups

.load_protected:
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp CODE_SEG:load32

; Global Descriptor Table
gdt_start:
gdt_null:                       ; 64 bits of zeros
    dd 0
    dd 0
; Offset 0x8
gdt_code:                       ;; CS should point to this
    dw 0xffff                   ; Segment Limit 0-15 bits
    dw 0                        ; Base first 0-15 bits
    db 0                        ; Base 16-23 bits
    db 0x9a                     ; Access Byte
    db 11001111b                ; High and Low 4-bit flags
    db 0                        ; Base 24-31 bits
; Offset 0x10
gdt_data:                       ;; DS, SS, ES, FS, GS
    dw 0xffff                   ; Segment Limit 0-15 bits
    dw 0                        ; Base first 0-15 bits
    db 0                        ; Base 16-23 bits
    db 0x92                     ; Access Byte
    db 11001111b                ; High and Low 4-bit flags
    db 0                        ; Base 24-31 bits
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
load32:
    mov eax, 1                  ; Beginning of the sector to read from (Kernel starts at the first sector. 0 = bootloader)
    mov ecx, 100                ; End of the sector (we added 100 sectors in Makefile)
    mov edi, 0x0100000          ; Address where the Kernel will be loaded into (linker.ld also specifies 1M)
    call ata_lba_read
    jmp CODE_SEG:0x0100000      ; Jump to our kernel code

ata_lba_read:
    mov ebx, eax                ; Backup the LBA (Linear Block Address), which is set to 1
    ; Send the highest 8 bits of the LBA to hard disk controller
    shr eax, 24
    or eax, 0xE0                ; Select the master drive
    mov dx, 0x1F6               ; Port that expects us to write LBA MSB 8 bits to
    out dx, al                  ; Write to the bus line 0x1F6
    ; Finished sending the highest 8 bits of the LBA

    ; Send the total sectors to read
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ; Finished sending the total sectors to read

    ; Send more bits of the LBA
    mov eax, ebx                ; Restore the backup LBA
    mov dx, 0x1F3
    out dx, al
    ; Finished sending more bits of the LBA

    mov eax, ebx                ; Backup LBA again just in case
    ; Send more bits of the LBA
    shr eax, 8
    mov dx, 0x1F4
    out dx, al
    ; Finished sending more bits of the LBA

    ; Send upper 16 bits of the LBA
    mov eax, ebx
    shr eax, 16
    mov dx, 0x1F5
    out dx, al
    ; Finished sending upper 16 bits of the LBA

    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

; Read all sectors into memory
.next_sector:
    push ecx

; Checking if we need to reqd
.try_again:
    mov dx, 0x1F7
    in al, dx
    test al, 8
    jz .try_again

    ; Read 256 words (512 bytes = 1 sector) at a time and store it at address specified by ES:EDI
    mov ecx, 256
    mov dx, 0x1F0
    rep insw                    ; Input word from I/O port specified in DX into memory location specified in ES:EDI
    pop ecx
    loop .next_sector
    ; End of reading sectors into memory
    ret

times 510 - ($ - $$) db 0       ; Pad the boot sector to 510 bytes
dw 0xAA55                       ; Boot signature. 55AA (2 bytes) in the little-endian

;; Everything under here is at the second sector from 0x7c00 (0x7e00)
