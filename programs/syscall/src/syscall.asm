 [BITS 32]

section .asm

global _start

_start:
   ; arguments to sys_sum
   push 20
   push 30

   mov eax, 0    ; sys_sum
   int 0x80      ; call kernel
   add esp, 8    ; clean up stack

   push message
   mov eax, 1
   int 0x80
   add esp, 4

   jmp $

section .data
message: db "Hello, World!", 0
