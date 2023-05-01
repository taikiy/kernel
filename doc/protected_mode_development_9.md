# Protected Mode Development 9 - VFS (Virtual File System) and FAT16

## What is a Virtual File System?

A virtual file system (VFS) is an abstraction layer on top of a more concrete file system. The purpose of a VFS is to allow client applications to access different types of concrete file systems uniformly. A VFS can, for example, be used to access local and network storage devices transparently without the client application noticing the difference.

When a user program requests a file, i.e., `fopen("0:/foo.txt", "r")`, the kernel will parse the path and find the disk number and the file name. Then, the kernel will find the file system driver for the disk and call the driver's `fopen` function. The driver will read the file system header and find the file. The driver will then return a file descriptor to the kernel. The kernel will then return the file descriptor to the user program.

File descriptors are just integers. The kernel maintains a table of file descriptors and the corresponding file system driver. When the user program calls `fread`, the kernel will find the file system driver from the table and call the driver's `fread` function.

## VFS Implementation

[commit](https://github.com/taikiy/kernel/commit/aea4661d997a97ba64e10bb880339040fd10e390)

## FAT16 Driver

- Core functionality [commit](https://github.com/taikiy/kernel/commit/21f05d5826f3aeecdcd36bf4202a8b9cb448cad0)
- Structures [commit](https://github.com/taikiy/kernel/commit/b5d3e8cdad23451a9659ed056f7f9b2c6fd4a3c0)
- FAT16 resolver function [commit]()
