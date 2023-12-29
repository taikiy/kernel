 [BITS 32]

 section .asm

 global _start

_start:
   mov eax, 0     ; sys_print
   int 0x80       ; call kernel
   jmp $