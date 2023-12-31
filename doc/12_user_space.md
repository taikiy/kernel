# User Space

"User Space" or "User Land" describes a state where processes run in Ring 3 (user mode) (the kernel runs in Ring 0 (kernel mode)). This is the state that most operating systems are in most of the time. User Space has lower privileges than Kernel Space, and can only access hardware and memory in a controlled way, via the kernel. If something goes wrong in User Space, the kernel can step in and prevent it from causing damage. [Memory and Hardware Protection](./2_protected_mode.md#memory-and-hardware-protection)

## How to switch to User Space

Entering or returning to User Space is basically restoring register values and setting IP to whatever user instructions we want to run. Then we execute the `iret` instruction. This instruction is similar to `ret`, but it also loads the CS, EIP, EFLAGS, SS, and ESP registers from the stack.

In a multi-tasking system, we need to save the current process state before switching to another process. This is done by saving the current process state in the TSS (Task State Segment) and then loading the new process state from the TSS.

1. Setup user code and data segments

This means rewriting the GDT to include a user code and data segment. Currently, the GDT is set up and loaded in [boot.asm](../src/boot/boot.asm) with the `lgdt` instruction.

2. Setup a TSS (Task State Segment)

The task state segment (TSS) is a structure on x86-based computers, which holds information about a task. It is used by the operating system kernel for task management. [wiki](https://en.wikipedia.org/wiki/Task_state_segment)

3. Use `iret` to switch to user mode

To make the implementation easier, we write code to control GDT and TSS in C.

- Controlling GDT in C [commit](https://github.com/taikiy/kernel/commit/179c23dbee2cf3b89304606f1ed97447f3ca5cff)
- Adding TSS to the GDT segment [commit](https://github.com/taikiy/kernel/commit/018fc3a0de9570a2f883cf320131e387d8d8b861)

## Tasks

A task is a running process. The kernel can switch between tasks to run multiple processes at the same time. This is called multitasking. The kernel saves the state of the current task in the TSS (Task State Segment) and loads the state of the next task from the TSS. A state of a task is a set of registers and the memory page a task is using.

- Task foundations: user program paging and registers structures [commit](https://github.com/taikiy/kernel/commit/df3b99f09cc1f079a175de5b9ce8b35a83aff14d)

## Mapping user program to memory

Once a user program is loaded into memory, we need to map it to the virtual address space of the process. This is done by allocating a paging chunk and mapping each physical memory block to empty virtual memory blocks.

Once the user program is mapped to the virtual address space, we can switch to user mode by setting the segment registers to the user code and data segments, the instruction pointer to the entry point of the user program, the stack pointer to the top of the stack, and other registers needed by the user program. Then we execute the `iret` instruction to switch to user mode.

- User Space functionality [commit](https://github.com/taikiy/kernel/commit/f2254c355692ae00c262040abe6ba85f22984104)

## Writing our first user program

To test the User Space functionality, we write a simple user program that goes into an infinite loop. We write it in assembly and build it with `nasm`, link it with `ld`, and copy it to the floppy disk image with `dd`. We also write a Makefile to automate these steps.

[commit](https://github.com/taikiy/kernel/commit/a1b8407c727eec8c2bc35b38432c8c67fa9102b8)

---

[Previous](./11_file_system_virtual_file_system.md) | [Next](./13_calling_kernel_space_routines_from_user_space.md) | [Home](../README.md)
