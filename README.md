# Overview

Developing a multi-threaded kernel from scratch

# Preparation

Documents in this repo assumes macOS on Apple M1 chip.

```shell
> brew install nasm
> brew install qemu
```

# Sections

Real Mode Development

1. [Bootloader](./doc/real_mode_development.md)

Protected Mode Development

2. [Entering Protected Mode](./doc/protected_mode_development_1.md)
3. [Loading 32-bit kernel](./doc/protected_mode_development_2.md)
4. [Writing kernel in C](./doc/protected_mode_development_3.md)
5. [Memory Management - Heap](./doc/protected_mode_development_4.md)
6. [Memory Management - Paging](./doc/protected_mode_development_5.md)

# Resources

- https://wiki.osdev.org/
