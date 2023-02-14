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
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp 
    cld                         ; Clears direction flag
    cli                         ; Disables interrupts
    hlt                         ; This hangs the computer

times 510 - ($ - $$) db 0       ; Pad the boot sector to 510 bytes
dw 0xAA55                       ; Boot signature. 55AA (2 bytes) in the little-endian

;; Everything under here is at the second sector from 0x7c00 (0x7e00)
