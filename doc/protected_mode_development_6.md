# Protected Mode Development 6 - Hard Disk Driver

## PCI IDE Controller

PCI (Peripheral Component Interconnect) bus was defined to establish a high-performance and low-cost local bus that would remain through several generations of products.

IDE (Integrated Drive Electronics) is the electrical specification of the cables which connect ATA drives to another device. (I touched on ATA a bit in [Loading 32-bit kernel](./protected_mode_development_2.md))

_Note: ATA has been renamed to PATA. You'll still see the word ATA used commonly._

IDE allows up to 4 drives to be connected.

- ATA (Serial): Used for modern hard drives
- ATA (Parallel): Commonly used for hard drives
- ATAPI (Serial): Used for modern optical drives
- ATAPI (Parallel): Commonly used for optical drives

An IDE driver (the kernel) doesn't need to know whether a drive is parallel or serial. It only needs to know whether it's ATA or ATAPI. Also, accessing the SATA drive works the same way as accessing ATA drives.

On a motherboard, you'll see one or two (or possibly more) white and green IDE ports, also known as channels. Each channel can have one or two drives (master and slave) connected to one ATA cable. Possible drive types are:

- Primary master drive (primary = slot 1)
- Primary slave drive
- Secondary master drive
- Secondary slave drive

## Implementation

We communicate with ATA drives through the I/O port using `in`/`out` functions we implemented in [io.asm](../src/io/io.asm), just like writing IDT to handle calls from PIC ([PIC/IRQ](./protected_mode_development_4.md)).

Actually, we've already written instructions to read our 32-bit kernel code from the ATA drive in [ata_lba_read](../src/boot/boot.asm) function. In this section, we'll write the same logic using C.

[[commit]()]

## Debugging

Attach the debugger and see what we can read from the disk.

```shell
❯ qemu-system-i386 -hda ./bin/os.bin -s -S &
❯ i386-elf-gdb
(gdb) add-symbol-file ./build/kernelfull.o 0x100000
(gdb) target remote localhost:1234
(gdb) b kernel.c:98
Breakpoint 1 at 0x100486: file ./src/kernel.c, line 98.
(gdb) c
Continuing.

Breakpoint 1, kernel_main () at ./src/kernel.c:98
98          disk_read_sector(0, 1, buf);
(gdb) n
101         enable_interrupts();
(gdb) p buf
$1 = "\353\"\220", '\000' <repeats 33 times>, "\352)|\000\000\372\270\000\000\216\330\216\300\216м\000|\373\372\017\001\026b|\017 \300\f\001\017\"\300\352h|\b\000\000\000\000\000\000\000\000\000\377\377\000\000\000\232\317\000\377\377\000\000\000\222\317\000\027\000J|\000\000\270\001\000\000\000\271d\000\000\000\277\000\000\020\000\350\a\000\000\000\352\000\000\020\000\b\000\211\303\301\350\030\r\340\000\000\000f\272\366\001\356\211\310f\272\362\001\356\211\330f\272\363\001\356\211\330\301\350\bf\272\364\001\356\211\330\301\350\020f\272\365\001\356f\272\367\001\260 \356Qf\272\367\001\354\250\bt\367\271\000\001\000\000"...
(gdb) x/32x buf
0x1ffdec:       0xeb    0x22    0x90    0x00    0x00    0x00    0x00    0x00
0x1ffdf4:       0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0x1ffdfc:       0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0x1ffe04:       0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
```

`buf` contains `0xeb 0x22 0x90 ...` Let's see what they are.

```shell
❯ hexdump -C -n 512 ./bin/os.bin
00000000  eb 22 90 00 00 00 00 00  00 00 00 00 00 00 00 00  |."..............|
00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000020  00 00 00 00 ea 29 7c 00  00 fa b8 00 00 8e d8 8e  |.....)|.........|
00000030  c0 8e d0 bc 00 7c fb fa  0f 01 16 62 7c 0f 20 c0  |.....|.....b|. .|
00000040  0c 01 0f 22 c0 ea 68 7c  08 00 00 00 00 00 00 00  |..."..h|........|
00000050  00 00 ff ff 00 00 00 9a  cf 00 ff ff 00 00 00 92  |................|
00000060  cf 00 17 00 4a 7c 00 00  b8 01 00 00 00 b9 64 00  |....J|........d.|
00000070  00 00 bf 00 00 10 00 e8  07 00 00 00 ea 00 00 10  |................|
00000080  00 08 00 89 c3 c1 e8 18  0d e0 00 00 00 66 ba f6  |.............f..|
00000090  01 ee 89 c8 66 ba f2 01  ee 89 d8 66 ba f3 01 ee  |....f......f....|
000000a0  89 d8 c1 e8 08 66 ba f4  01 ee 89 d8 c1 e8 10 66  |.....f.........f|
000000b0  ba f5 01 ee 66 ba f7 01  b0 20 ee 51 66 ba f7 01  |....f.... .Qf...|
000000c0  ec a8 08 74 f7 b9 00 01  00 00 66 ba f0 01 f3 66  |...t......f....f|
000000d0  6d 59 e2 e7 c3 00 00 00  00 00 00 00 00 00 00 00  |mY..............|
000000e0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
000001f0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 55 aa  |..............U.|
00000200
```

That's our bootloader code!
