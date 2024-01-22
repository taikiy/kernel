[BITS 32]

section .asm

global _start
extern main
extern exit

_start:
    call main
    call exit
    ret
