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

- Interrupt 0x80 handler [commit](https://github.com/taikiy/kernel/commit/42c0b6374e21e096060d27e3255a2e007c55b0cd)
- Register 0x80 handler in IDT [commit](https://github.com/taikiy/kernel/commit/2020d58d9047f2584ab03d95ccfab2b221ff2ced)
- `sys_print` function [commit](https://github.com/taikiy/kernel/commit/459b80a2d26d9ff61d42f026305b79abbe5acf4f)

## Passing arguments

We can pass arguments from the user space programs to the kernel space (syscall) by pushing them to the stack before making `int 0x80` call. The kernel can then read the arguments from the task's stack.

`get_arg_from_task() @ syscall.c:18`

---

`--- User Space ---`

1. The user program pushes the arguments to the stack.
2. The user program sets the kernel command ID (`SYSCALL_COMMAND` in our source) to EAX.
3. The user program executes the `int 0x80` instruction.

`--- Kernel Space ---`

4. The kernel saves the user program's registers to the task instance.
5. The kernel calls the syscall handler.
6. The syscall handler reads the saved registers from the task instance. ESP points to the top of the stack.
7. The syscall handler switches to the User Space to read the arguments from the stack.

`--- User Space ---`

8. For each argument, the syscall reads the value at `$esp + index`. The value is `uint32_t` which is the address of the argument value at `index`. Then, we cast it to `uint32_t*` and dereference it to get the value.
9. The syscall handler switches to the Kernel Space.

`--- Kernel Space ---`

10. The syscall casts the value to `void*` because the value could be of any type. The caller is responsible for casting the value to the correct type. If it's an address to some data (string, struct, etc.), the caller must call `copy_data_from_user_space()` to copy the user space data to the kernel space.

---

- Implement `copy_data_from_user_space()` [commit](https://github.com/taikiy/kernel/commit/ff3f410d753d25828b2af2442c19f19d95245d29)
- Reading the user program stack (syscall arguments) [commit](https://github.com/taikiy/kernel/commit/c0c471c50d87aa256e21db74e8891221360f412b)

There was an issue with the `linker.ld` files. The `.asm` section was defined at the end of the file, below `.data` section. I'm not sure the exact reason,
but when I defined a constant string in the `.data` section in the `syscall.asm` file, `int 0x80` call didn't work. I'm guessing it's because of the memory alignment issue. I moved the `.asm` section closer to the top just below the `.text` section, and it worked.

---

[Previous](./12_user_space.md) | [Next](./14_accessing_keyboard_in_protected_mode.md) | [Home](../README.md)
