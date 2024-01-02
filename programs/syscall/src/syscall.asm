 [BITS 32]

section .asm

global _start

_start:
   ; arguments to sys_sum
   push 20
   push 30

   mov eax, 0     ; sys_sum
   int 0x80       ; call kernel
   add esp, 8     ; clean up stack

   call get_key   ; wait for a key press

   push message
   mov eax, 1
   int 0x80
   add esp, 4

   jmp $

get_key:
   mov eax, 2     ; sys_getchar
   int 0x80
   cmp eax, 0x00  ; if sys_getchar() == 0x00
   je get_key     ; loop until sys_getchar() returns a key
   ret

section .data
message: db "Hello, World!", 0
