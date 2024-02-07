# Overview

Developing a multi-threaded kernel from scratch

# Preparation

Documents in this repo assume macOS on an Apple M1 chip.

```shell
> brew install nasm
> brew install qemu
```

# Sections

1. [Bootloader / Real Mode](./doc/1_real_mode.md)
2. [Entering Protected Mode](./doc/2_protected_mode.md)
3. [Loading 32-bit kernel](./doc/3_32-bit_kernel.md)
4. [Writing kernel in C / Interrupt Descriptor Table](./doc/4_writing_kernel_in_C.md)
5. [Interrupt Descriptor Table](./doc/5_interrupt_descriptor_table.md)
6. [Programmable Interrupt Controller](./doc/6_programmable_interrupt_controller.md)
7. [Memory Management - Heap](./doc/7_memory_management_heap.md)
8. [Memory Management - Paging](./doc/8_memory_management_paging.md)
9. [File System - Disk Driver](./doc/9_file_system_disk_driver.md)
10. [File System - FAT16](./doc/10_file_system_fat16.md)
11. [Virtual File System](./doc/11_file_system_virtual_file_system.md)
12. [User Space](./12_user_space.md)
13. [Talking with the kernel from User Space](./doc/13_calling_kernel_space_routines_from_user_space.md)
14. [Accessing the Keyboard in Protected Mode](./doc/14_accessing_keyboard_in_protected_mode.md.md)
15. [ELF files](./doc/15_elf_files.md)
16. [Writing User Programs in C](./doc/16_writing_user_programs_in_C.md)
17. [Shell and Launching Other User Programs](./doc/17_shell_and_launching_other_programs.md)

# Resources

- https://wiki.osdev.org/
