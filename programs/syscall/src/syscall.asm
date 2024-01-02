 [BITS 32]

section .asm

global _start

_start:
   call getchar   ; wait for a key press

   cmp eax, 0x5a  ; if key press == 'Z'
   je exit        ; exit the program

   push eax       ; save the key press
   call putchar   ; print the key press

   jmp _start     ; loop

exit:
   ; mov eax, 1     ; sys_exit
   ; mov ebx, 0     ; exit code
   ; int 0x80
   jmp $

; Returns a key press. Blocks until a key is pressed.
getchar:
   mov eax, 2     ; sys_getchar
   int 0x80
   cmp eax, 0x00  ; if sys_getchar() == 0x00
   je getchar     ; loop until sys_getchar() returns a key
   ret

; Prints a character to the screen.
putchar:
   push ebp
   mov ebp, esp

   mov eax, [ebp+8]     ; get the character to print
   push eax
   mov eax, 3           ; sys_putchar
   int 0x80

   mov esp, ebp
   pop ebp
   ret


section .data
message: db "Hello, World!", 0
