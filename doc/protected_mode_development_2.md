# Protected Mode Development - Part 2

## 1. Cross-Compiler

Installing the i686-elf toolchain on M1 Mac is very easy.

```
> brew install i686-elf-binutils i686-elf-gcc i386-elf-gdb
```

## 2. Loading the 32-bit kernel into memory

1. Kernel code

Create [`kernel.asm`](../src/kernel.asm), copy all 32-bit code to it. We'll change [`boot.asm`](../src/boot/boot.asm) later to load the 32-bit kernel code. At this point, `load32` label is no longer available because these two files are not linked. What we'll do is load the kernel to some known address, and specify that address from the bootloader.

2. Linker script

We'll also need [`linker.ld`](../src/linker.ld) to link the object files.

```
SECTIONS
{
    . = 1M;
}
```

This means that the kernel will be loaded into 1M into memory (1024 \* 1024 = 0x100000).

3. Makefile to build object files and link them

We'll also add bunch of lines in [Makefile](../Makefile) to build and link the object files into a single binary.

4. Modify boot.asm

Now we'll write a disk read/write driver. Since we are now in Protected Mode, we cannot (easily) call INT 13h BIOS interrupts. Instead, we'll access the disk through ports (CPU IO port bus) by communicating with motherboard's hard disk controller following the ATA disk spec.

[ATA PIO Mode](https://wiki.osdev.org/ATA_PIO_Mode)

> The ATA disk specification is built around an older specification called ST506. With ST506, each disk drive was connected to a controller board by two cables -- a data cable, and a command cable. The controller board was plugged into a motherboard bus. The CPU communicated with the controller board through the CPU's IO ports, which were directly connected to the motherboard bus.

5. Build, run, and debug

Now, we can use `i386-elf-gdb` instead of `lldb`. Load the symbol file `kernelfull.o` using `add-symbol-file` command so that we can set breakpoints. You must specify the address of the code being loaded:

`add-symbol-file ./build/kernelfull.o 0x100000`

Now you can debug the kernel just like you would do with any executables!

```
> make clean
> make
❯ qemu-system-x86_64 -hda ./bin/os.bin -s -S &
❯ i386-elf-gdb
GNU gdb (GDB) 12.1
(gdb) add-symbol-file ./build/kernelfull.o 0x100000
add symbol table from file "./build/kernelfull.o" at
	.text_addr = 0x100000
(y or n) y
Reading symbols from ./build/kernelfull.o...
(gdb) b _start
Breakpoint 1 at 0x100000: file ./src/kernel.asm, line 9.
(gdb) target remote localhost:1234
Remote debugging using localhost:1234
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x000000000000fff0 in ?? ()
(gdb) c
Continuing.

Breakpoint 1, _start () at ./src/kernel.asm:9
9	    mov ax, DATA_SEG
(gdb) x/20i 0x100000
=> 0x100000 <_start>:	mov    $0x10,%ax
   0x100004 <_start+4>:	mov    %eax,%ds
   0x100006 <_start+6>:	mov    %eax,%es
   0x100008 <_start+8>:	mov    %eax,%fs
   0x10000a <_start+10>:	mov    %eax,%gs
   0x10000c <_start+12>:	mov    %eax,%ss
   0x10000e <_start+14>:	mov    $0x200000,%ebp
   0x100013 <_start+19>:	mov    %ebp,%esp
   0x100015 <_start+21>:	in     $0x92,%al
   0x100017 <_start+23>:	or     $0x2,%al
   0x100019 <_start+25>:	out    %al,$0x92
   0x10001b <_start+27>:	cld
   0x10001c <_start+28>:	cli
   0x10001d <_start+29>:	hlt
   0x10001e:	add    %al,(%rax)
   0x100020:	add    %al,(%rax)
   0x100022:	add    %al,(%rax)
   0x100024:	add    %al,(%rax)
   0x100026:	add    %al,(%rax)
   0x100028:	add    %al,(%rax)
```

a
