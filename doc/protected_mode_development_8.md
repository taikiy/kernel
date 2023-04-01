# Protected Mode Development 8 - File System

## What is a File System?

A file system is a structure that describes how information is laid on the disk. Disks are just a bunch of bytes, and a file system is a way to organize those bytes into files and directories. The operating system knows the file system structure and how to read/write files from the disk.

Disks are organized into sectors. A sector is a fixed-size block of bytes. The size of a sector is usually 512 bytes. A sector is the smallest unit of data that can be read/written to the disk. A sector is also the smallest unit of data that can be transferred between the disk and the CPU. Each sector is identified by LBA (Logical Block Address). The first sector has LBA 0, the second sector has LBA 1, and so on.

## File System Structure

- Contains a header that describes how many files and directories, where the root directory is, etc.
- Contains raw data for files.

File System protocols decide how the files are laid on the disk, e.g., FAT16, FAT32, NTFS, etc.

## FAT16

- FAT16 is a file system protocol that is used by MS-DOS and Windows 95/98/ME.
- The first sector of the disk is the boot sector. It contains the boot code and the file system header. The file system header contains information about the file system such as the number of sectors per cluster, the number of reserved sectors, the number of FATs, the number of root directory entries, etc.
  - In `[boot.asm](../src/boot/boot.asm)`, the first instruction is a short jump to the label further down. Things between this jump to the next label are the file system header.
- Then follows the reserved sectors. The number of reserved sectors is specified in the file system header.
- Next is the FAT (File Allocation Table). The FAT contains values that represent which clusters on the disk are taken or free. (clusters are groups of sectors)
- There is the second FAT which is optional. The FAT16 header in the boot sector determines if there is a second FAT.
- Then follows the root directory. The root directory contains the file and directory names and the first cluster of the file. Each entry has a name and attributes such as read-only, the address of the first cluster, the size of the file, and more.
- The last is the data region. The data region contains the actual data of the files.

## Path Parser

Before implementing the FAT16, we'll implement the path parser. We don't actually read anything yet, but we'll use this in future implementations to read files from the disk.

[commit]()

---

[previous](./protected_mode_development_7.md) | [next](./protected_mode_development_9.md)
