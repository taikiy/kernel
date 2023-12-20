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
4. [Writing kernel in C / Interrupt Descriptor Table](./doc/4_interrupt_descriptor_table.md)
5. [Programmable Interrupt Controller](./doc/5_programmable_interrupt_controller.md)
6. [Memory Management - Heap](./doc/6_memory_management_heap.md)
7. [Memory Management - Paging](./doc/7_memory_management_paging.md)
8. [File System - Disk Driver](./doc/8_file_system_disk_driver.md)
9. [File System - FAT16](./doc/9_file_system_fat16.md)
10. [Virtual File System](./doc/10_file_system_virtual_file_system.md)
11. [User Land](./11_user_land.md)

# Resources

- https://wiki.osdev.org/
