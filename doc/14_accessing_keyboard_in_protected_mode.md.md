# Accessing the keyboard from the Protected Mode

## Keyboard Drivers

- Keyboard access is interrupt-driven.
- Each process has its own keyboard buffer. The keyboard buffer is a circular buffer.
- The keyboard buffer can be pushed and popped by the process.
- We parse the Scan Code and convert it to ASCII code.
- When we push a key, we get the current active _process_ and push the key to the keyboard buffer of the process. We push to the process because the input comes from the active process that's running on the screen.
- When we pop a key, we get the current active _task_ and pop the key from the keyboard buffer of the process. We pop from the buffer of the active task's process because the active task could be different from the active process.

The standard IRQ number for the keyboard is 1. Because we mapped the PIC to 0x20, the IRQ number for the keyboard is 0x21. (Refer to [PIC (Programmable Interrupt Controller)](./6_programmable_interrupt_controller.md))

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

---

[Previous](./13_calling_kernel_space_routines_from_user_space.md) | [Next]() | [Home](../README.md)
