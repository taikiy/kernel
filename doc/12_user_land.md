# User-Land

"User-Land" describes a state where processes run in Ring 3 (user mode) (the kernel runs in Ring 0 (kernel mode)). This is the state that most operating systems are in most of the time. User-Land has lower privileges than Kernel-Land, and can only access hardware and memory in a controlled way, via the kernel. If something goes wrong in User-Land, the kernel can step in and prevent it from causing damage. [Memory and Hardware Protection](./2_protected_mode.md#memory-and-hardware-protection)

## How to switch to User-Land

Entering or returning to User-Land is basically restoring register values and setting IP to whatever user instructions we want to run. Then we execute the `iret` instruction. This instruction is similar to `ret`, but it also loads the CS, EIP, EFLAGS, SS, and ESP registers from the stack.

In a multi-tasking system, we need to save the current process state before switching to another process. This is done by saving the current process state in the TSS (Task State Segment) and then loading the new process state from the TSS.

1. Setup user code and data segments

This means rewriting the GDT to include a user code and data segment. Currently, the GDT is set up and loaded in [boot.asm](../src/boot/boot.asm) with the `lgdt` instruction.

2. Setup a TSS (Task State Segment)

The task state segment (TSS) is a structure on x86-based computers, which holds information about a task. It is used by the operating system kernel for task management. [wiki](https://en.wikipedia.org/wiki/Task_state_segment)

3. Use `iret` to switch to user mode

To make the implementation easier, we write code to control GDT and TSS in C.

- Controlling GDT in C [commit](https://github.com/taikiy/kernel/commit/179c23dbee2cf3b89304606f1ed97447f3ca5cff)
- Adding TSS to the GDT segment [commit](https://github.com/taikiy/kernel/commit/018fc3a0de9570a2f883cf320131e387d8d8b861)
