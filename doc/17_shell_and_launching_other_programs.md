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

- [commit]()

We also need to implement a system call that terminates the process.

- sys_exit [commit]()
