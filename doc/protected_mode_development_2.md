# Protected Mode Development 2 - Loading 32-bit kernel

## 1. Cross-Compiler

Installing the i686-elf toolchain on M1 Mac is very easy.

```shell
> brew install i686-elf-binutils i686-elf-gcc i386-elf-gdb
```

## 2. Loading the 32-bit kernel into memory

([git commit](https://github.com/taikiy/kernel/commit/9ada2f4b0606508ff5c1a163e83da0f5d0e57527))

1. Kernel code

Create [`kernel.asm`](../src/kernel.asm), and copy all 32-bit code to it. We'll change [`boot.asm`](../src/boot/boot.asm) later to load the 32-bit kernel code. At this point, `load32` label is no longer available because these two files are not linked. What we'll do is load the kernel to some known address, and specify that address from the bootloader.

2. Linker script

We'll also need [`linker.ld`](../src/linker.ld) to describe how to link the object files.

```
SECTIONS
{
    . = 1M;
}
```

This means that the kernel will be loaded into 1M into memory (1024 \* 1024 = 0x100000).

3. Makefile to build object files and link them

We'll also add a bunch of lines in [Makefile](../Makefile) to build and link the object files into a single binary.

4. Modify boot.asm

Now we'll write a disk read/write driver. Since we are now in Protected Mode, we cannot (easily) call INT 13h BIOS interrupts. Instead, we'll access the disk through ports (CPU IO port bus) by communicating with the motherboard's hard disk controller following the ATA disk spec.

[ATA PIO Mode](https://wiki.osdev.org/ATA_PIO_Mode)

> The ATA disk specification is built around an older specification called ST506. With ST506, each disk drive was connected to a controller board by two cables -- a data cable, and a command cable. The controller board was plugged into a motherboard bus. The CPU communicated with the controller board through the CPU's IO ports, which were directly connected to the motherboard bus.

5. Build, run, and debug

Now, we can use `i386-elf-gdb` instead of `lldb`. Load the symbol file `kernelfull.o` using `add-symbol-file` command so that we can set breakpoints. We must specify the address of the code is loaded:

`add-symbol-file ./build/kernelfull.o 0x100000`

Now we can debug the kernel just like we would do with any executables!

```shell
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
(gdb) layout asm
```

https://asciinema.org/a/n3qfuN4AV8u3GWKHAqFxtGR15

## 3. Memory alignment

Quick summary of how our code works with memory.

- Bootloader is loaded at address 0x7c00 and must be 512 bytes (boot.asm)
- Before entering Protected Mode, we define GDT that defines the memory segments - `CODE_SEG` and `DATA_SEG`.
- In `boot.asm`, we load our kernel code from the disk and place it at address `CODE_SEG:0x100000`.
- Kernel code is written in `kernel.asm` which is assembled into an ELF file (Makefile `kernel.asm.o`). All other kernel code (not written yet at this point, but in the future) will be linked to output `kernelfull.o` and compiled into `kernel.bin`.
- The C compiler aligns stack frames, data, etc., by a multiple of 4 bytes, because memory access of 32-bit processors is a lot faster when aligned. If the content of an object file is misaligned, it may cause unexpected errors. But our `kernel.asm` is not a C program, thus not aligned by default.
- To properly align the kernelfull.o, we do:
  - Add the padding instruction at the end of `kernel.asm` so that it becomes 1-sector (512 bytes) long.
    - Note that aligning `boot.asm` to 512 bytes is unrelated to the memory alignment issue we talk about here. The bootloader must have the boot signature 0x55AA at 511 and 512 bytes.
  - Make sure that `kernel.asm.o` is the first file to be linked. That ensures `kernel.asm.o` is located in the `.text` section ([`linker.ld`](../src/linker.ld)) when linked, and always starts at 0x100000. `kernel.asm.o` is 512 bytes long, so any other C object files linked after that are automatically aligned.
  - In other kernel assembly files, specify `.asm` section so that they are linked at the end of the object file. If the assembled code is not a multiple of 4 bytes, that's okay because those files are at the end.

---

[next](./protected_mode_development_3.md)
