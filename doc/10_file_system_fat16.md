# File System

## Before we implement FAT16...

### Path Parser

Before implementing the FAT16, we'll implement the path parser. We don't read anything yet, but we'll use this in future implementations to read files from the disk.

[commit](https://github.com/taikiy/kernel/commit/8f6d1ebb2df287e0d7a9d79ad991489ebd0a05f2)

### Disk Stream

So far, we have implemented a low-level disk access function `disk_read_block`, which reads 1 block (512 bytes) of a given sector from the disk. We'll implement a higher-level disk access function `disk_read_stream`, which reads a stream of bytes at the given position from the disk.

[commit](https://github.com/taikiy/kernel/commit/71c17b57f5e3999464734685ffbc01f849bd5b61)

---

## What is a File System?

A file system is a structure that describes how information is laid on the disk. Disks are just a bunch of bytes, and a file system is a way to organize those bytes into files and directories. The operating system knows the file system structure and how to read/write files from the disk.

Disks are organized into sectors. A sector is a fixed-size block of bytes. The size of a sector is usually 512 bytes. A sector is the smallest unit of data that can be read/written to the disk. A sector is also the smallest unit of data that can be transferred between the disk and the CPU. Each sector is identified by LBA (Logical Block Address). The first sector has LBA 0, the second sector has LBA 1, and so on.

## File System Structure

- Contains a header that describes how many files and directories, where the root directory is, etc.
- Contains raw data for files.

File System protocols decide how the files are laid on the disk, e.g., FAT16, FAT32, NTFS, etc.

## FAT16

[FAT16 | osdev wiki](https://wiki.osdev.org/FAT16#FAT_16)

[Design of the FAT file system | wiki](https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#FAT16)

- FAT16 is a file system protocol that is used by MS-DOS and Windows 95/98/ME.
- Every file in FAT16 uses at least one cluster, and cannot store files larger than 2GB or 4GB with large file support.

### Layout

| Name             | Size                                                              |
| ---------------- | ----------------------------------------------------------------- |
| Boot sector      | 512 bytes                                                         |
| Reserved sectors | fat_headers.reserved_sectors \* 512 bytes                         |
| FAT 1            | fat_headers.sectors_per_fat \* 512 bytes                          |
| FAT 2 (optional) | fat_headers.sectors_per_fat \* 512 bytes                          |
| Root directory   | fat_headers.root_dir_entries \* sizeof(struct fat_directory_item) |
| Data region      |                                                                   |

_1 sector = 512 bytes_

#### Boot sector

The first sector of the disk is the boot sector. It contains the boot code and the file system header. The file system header contains information about the file system such as the number of sectors per cluster, the number of reserved sectors, the number of FATs, the number of root directory entries, etc.

In [`boot.asm`](../src/boot/boot.asm), the first instruction is a short jump to the label further down. Things between this jump to the next label are the file system header.

[Boot Sector | wiki](https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#Boot_Sector)

#### Reserved sectors

Then follows the reserved sectors. The number of reserved sectors is specified in the file system header. The reserved sectors are not used by the file system. They are reserved for the boot code ([`boot.asm`](../src/boot/boot.asm) and [`kernel.asm`](../src/kernel.asm)).

The reserved sectors include the boot sector, so the number of reserved sectors is at least 1.

#### FAT 1 & FAT 2

Next is the FAT (File Allocation Table). The FAT contains values that represent which clusters on the disk are taken or free. (clusters are groups of sectors)

There is the second FAT which is optional. The FAT16 header in the boot sector determines if there is a second FAT.

#### Root directory

Then follows the root directory. The root directory contains the file and directory names and the first cluster of the file. Each entry has a name and attributes such as read-only, the address of the first cluster, the size of the file, and more.

#### Data region

The last is the data region. The data region contains the actual data of the files.

### FAT entry

- A FAT entry is 16 bits (2 bytes) long.
- The first 12 bits are used to store the address of the next cluster.
- The last 4 bits are used to store the status of the cluster.
- Clusters can be chained together to form a file.

### Directory entry

```c
struct fat_directory_item
{
    uint8_t name[8];      // padded with spaces (0x20)
    uint8_t extension[3]; // padded with spaces (0x20)
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t last_modification_time;
    uint16_t last_modification_date;
    uint16_t first_cluster_low;
    uint32_t size;
} __attribute__((packed));
```

### Attributes

| Value | Name         | Description              |
| ----- | ------------ | ------------------------ |
| 0x01  | READ_ONLY    | File is read-only.       |
| 0x02  | HIDDEN       | File is hidden.          |
| 0x04  | SYSTEM       | File is a system file.   |
| 0x08  | VOLUME_LABEL | Entry is a volume label. |
| 0x10  | SUBDIRECTORY | Entry is a subdirectory. |
| 0x20  | ARCHIVE      | File is an archive file. |
| 0x40  | DEVICE       | Entry is a device.       |
| 0x80  | UNUSED       | Unused.                  |

---

### BPB Implementation

We start by implementing the BPB (BIOS Parameter Block) structure. The BPB is the file system header in the [boot sector](../src/boot/boot.asm).

[commit](https://github.com/taikiy/kernel/commit/275672e26ad1a63c43a4b41c805b47c9713e2045)

### Testing the BPB

We can test the BPB by mounting the disk image on any machine (i.e., the one you are using now to develop the kernel) and checking whether read/write works. If BPB is correctly implemented, mounting the disk image (generated by `dd` command in the [Makefile](../Makefile)) will be recognized as a FAT16 file system. On Linux, you would do this by `mount` command, but since we are on a Mac, we use an alternative command: `hdiutil`.

```shell
> make clean
> make
> hdiutil attach -imagekey diskimage-class=CRawDiskImage -mount required ./bin/os.bin
/dev/disk4          	                               	/Volumes/taiOS BOOT
> ls /Volumes/taiOS\ BOOT

❯ echo "Hello, World\!" > /Volumes/taiOS\ BOOT/hello.txt
❯ ls /Volumes/taiOS\ BOOT
hello.txt
❯ cat /Volumes/taiOS\ BOOT/hello.txt
Hello, World!
> hdiutil detach /dev/disk4
"disk4" ejected.
```

Note that the [Makefile](../Makefile) already has a target `mount` that does the same thing as the above commands.

---

[Previous](./9_file_system_disk_driver.md) | [Next](./11_file_system_virtual_file_system.md) | [Home](../README.md)
