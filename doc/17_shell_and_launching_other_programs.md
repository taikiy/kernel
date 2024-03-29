# Shell and Launching Other Programs

## Shell

Now that we have implemented the system calls ([Talking with the kernel from User Space](./13_calling_kernel_space_routines_from_user_space.md) and [Accessing the keyboard from the Protected Mode](./14_accessing_keyboard_in_protected_mode.md.md)), [ELF file loader](./15_elf_files.md) and [Standard Library](./16_writing_user_programs_in_C.md), we have all the tools we need to implement a shell.

To launch user programs from the shell, we need to implement a system call that will create a new process.

- sys_exec [commit](https://github.com/taikiy/kernel/commit/d508e93217b33e702c86592c5bc67e146af7166b)

## Passing Arguments to the User Program

We need to pass arguments to the user program. How we pass arguments to the user program is as follows.

1. shell calls stdlib `exec` function.
2. `exec` parses the command line arguments and makes a system call with the arguments.
3. `sys_exec` system call creates a new process with the received arguments.
4. the new process pushes the arguments to the user program's stack. [push_user_program_arguments@task.asm](../src/task/task.asm)
5. the process calls `iret`

There are other ways to pass arguments to the user program. For example, we can pass the arguments as a return value to another system call. The above method is how we implement it in this kernel. It's a bit more complicated than the system call method, but it's faster.

- [commit](https://github.com/taikiy/kernel/commit/2d8816ca275ba32c1d2d33ddb0f0660b0acef1b1)

## Terminating the Process

We also need to implement a system call that terminates the process by freeing the memory and removing the process from the process list.

- sys_exit [commit](https://github.com/taikiy/kernel/commit/155b780001d7894da542c2ff81619fdf0d75aeac)

### Exception Handling

When the user program causes an exception, we need to handle it. We can handle it by implementing an interrupt handler for each exception. The interrupt handler will terminate the process. There are many exceptions (i.e., divide by zero, page fault, etc.), but we just terminate the process when any exception occurs for now.

- exception handlers [commit](https://github.com/taikiy/kernel/commit/2b484211d6987d1616b8ea3be1d16c307872980c)

## Task Switching

In order to run multiple processes, we need to call the task switching when we receive a timer interrupt. We have all the code needed to do this, but there are a few things we need to take care of. For example, we shouldn't enable interrupts before running the first process in the system. The first process is the root process that runs indefinitely as long as the system is running.

Note that currently, all tasks are sharing the video memory. We need to implement a mechanism to switch the video memory when we switch the task. For example, we could allocate a buffer on physical memory for each task to write the output to. When the process is in the background, it can continue to write to the buffer, but the output won't be displayed on the screen. When the process is in the foreground, we can copy the buffer to the video memory.

- task switching [commit](https://github.com/taikiy/kernel/commit/cfcc0caa0be3ad35ae23108fdbb543f21070130b)

---

[Previous](./16_writing_user_programs_in_C.md) | [Home](../README.md)
