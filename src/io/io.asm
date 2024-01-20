section .asm

global inb
global inw
global outb
global outw

; write a byte to the specified port
inb:
	push ebp
	mov ebp, esp

	xor eax, eax
	mov edx, [ebp+8]
	in al, dx

	mov esp, ebp
	pop ebp
	ret

; write a word to the specified port
inw:
	push ebp
	mov ebp, esp

	xor eax, eax
	mov edx, [ebp+8]
	in ax, dx

	mov esp, ebp
	pop ebp
	ret

; read a byte from the specified port
outb:
	push ebp
	mov ebp, esp

	mov eax, [ebp+12]
	mov edx, [ebp+8]
	out dx, al

	mov esp, ebp
	pop ebp
	ret

; read a word from the specified port
outw:
	push ebp
	mov ebp, esp

	mov eax, [ebp+12]
	mov edx, [ebp+8]
	out dx, ax

	mov esp, ebp
	pop ebp
	ret
