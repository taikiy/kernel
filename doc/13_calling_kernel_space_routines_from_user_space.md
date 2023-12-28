# Talking with the kernel from User Space

We can use the `int` instruction to call a kernel function from User Space. The `int` instruction takes a single byte as an argument. This byte is the interrupt number. We can use this interrupt number to call a kernel function. The kernel can then use the interrupt number to determine which function to call. Once the kernel function is done, it returns to the user program.

Below is an example of a user program calling a kernel function.

1. Let's assume that the kernel function we want to call is `print`. We can assign a code number to this function. Let's say the code number is 0x1.
2. The user program sets `0x1` to EAX.
3. The user program pushes the address of a message to be printed to the stack.
4. The user program executes the `int` instruction. We can assign any number to the interrupt number. Let's say the interrupt number is 0x80.
5. The kernel function `print` is called.
6. The kernel returns from the call normally. The CPU will execute instructions equivalent to `switch_to_task` and `return_to_task`. We don't have to do this manually.

```asm
; void print(const char* message)
print:
    push ebp
    mov ebp, esp
    mov eax, 1          ; 1 is the code number for print
    mov ebx, [ebp + 8]  ; address of the string to print
    push dword ebx      ; push the address to the stack
    int 0x80            ; invoke the the kernel call
    add esp, 4          ; restore the stack pointer (pop the argument)
    pop ebp
    ret
```

- Interrupt 0x80 handler [commit]()
