section .asm

global inb
global inw
global outb
global outw

inb:
	push ebp
	mov ebp, esp

	xor eax, eax
	mov edx, [ebp+8]
	in al, dx

	pop ebp
	ret

inw:
	push ebp
	mov ebp, esp

	xor eax, eax
	mov edx, [ebp+8]
	in ax, dx

	pop ebp
	ret

outb:
	push ebp
	mov ebp, esp

	mov eax, [ebp+12]
	mov edx, [ebp+8]
	out dx, al

	pop ebp
	ret

outw:
	push ebp
	mov ebp, esp

	mov eax, [ebp+12]
	mov edx, [ebp+8]
	out dx, ax

	pop ebp
	ret
