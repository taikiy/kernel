# Accessing the keyboard from the Protected Mode

## Keyboard Drivers

- Keyboard access is interrupt-driven.
- Each process has its own keyboard buffer. The keyboard buffer is a circular buffer.
- The keyboard buffer can be pushed and popped by the process.
- We parse the Scan Code and convert it to ASCII code.
- When we push a key, we get the current active _process_ and push the key to the keyboard buffer of the process. We push to the process because the input comes from the active process that's running on the screen.
- When we pop a key, we get the current active _task_ and pop the key from the keyboard buffer of the process. We pop from the buffer of the active task's process because the active task could be different from the active process.

The standard IRQ number for the keyboard is 1. Because we mapped the PIC to 0x20, the IRQ number for the keyboard is 0x21. (Refer to [PIC (Programmable Interrupt Controller)](./6_programmable_interrupt_controller.md))

In this section, we will implement the virtual keyboard driver and the PS/2 keyboard driver.

[PS/2 Keyboard](https://wiki.osdev.org/PS/2_Keyboard)

```c
struct keyboard_buffer {
    char buffer[KEYBOARD_BUFFER_SIZE];
    int head;
    int tail;
};
```

- Virtual keyboard driver [commit](https://github.com/taikiy/kernel/commit/1db81a07a2d5e855504dcbc827bea0fc9913b9dd)
- PS/2 keyboard driver foundation [commit](https://github.com/taikiy/kernel/commit/d802c1ef99b515d0118798f50104e68d97c5b435)
- In between, I did IDT refactoring, terminal improvement, and adding switching processes functionality.
- PS/2 keyboard driver [commit](https://github.com/taikiy/kernel/commit/87deb951b281b4315bbebd870386b7844ae4e267)

## Keyboard Scan Code

PS2 keyboard sends 1 byte of data at a time. The data is called _Scan Code_. The scan code is a byte that represents the key pressed or released. The scan code is different from the ASCII code. The scan code is a hardware-specific code. The ASCII code is a software-specific code. Keyboard drivers are responsible for converting, taking into account the SHIFT key, CAPS LOCK key, etc., to produce the lowercase or uppercase ASCII code.

In the PS2 keyboard driver, we read the scan code using the `inb` instruction. The `inb` instruction reads a byte from the port. The port number is 0x60. The `inb` instruction is an x86 instruction. We pass the converted ASCII character back to the virtual keyboard driver. The virtual keyboard driver pushes the ASCII character to the keyboard buffer of the active process.

## Accessing the keyboard buffer from the user program

We can access the keyboard buffer from the user program by making a syscall. The syscall will pop the keyboard buffer of the active task's parent process.

- sys_getchar() [commit](https://github.com/taikiy/kernel/commit/612c3606e9bd9d37cf78f8b53cb43e592e7f8e92)
- sys_putchar() [commit](https://github.com/taikiy/kernel/commit/d9872c35651b458927dfca92217d60543ad5713e)
- Backspace [commit](https://github.com/taikiy/kernel/commit/477508b756f92c614f20f3bd4127e81167fc0a7e)

---

[Previous](./13_calling_kernel_space_routines_from_user_space.md) | [Next]() | [Home](../README.md)
