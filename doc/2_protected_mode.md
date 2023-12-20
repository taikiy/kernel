# Entering Protected Mode

Protected mode is an operation mode of x86 architectures. It gives access to 4GB of address space, memory protection, and much more. ([osdev wiki](https://wiki.osdev.org/Protected_mode))

## Memory and Hardware Protection

There are different protection levels in the processor. Levels are called _Rings_. The kernel runs in _Ring 0_. This is the most privileged mode that can talk with hardware and write data to any memory address. _Ring 1_ and _2_ are generally not used, but can be used for device drivers. _Ring 3_ is the least privileged level which is used to run user code. This prevents user applications from overwriting kernel memory, talking with hardware directly, accessing other processes' memory, and using privileged instructions (i.e., `sti`, `cli`).

## Memory access

### Selector Memory Scheme

What we know as _segmentation_ registers in Real Mode become _selector_ registers in Protected Mode. _Selectors_ point to data structures that describe memory ranges and the permissions (ring level) required to access a given range.

### Paging Memory Scheme

Paging memory scheme maps virtual memory addresses to physical memory addresses somewhere entirely different in memory. This allows user processes to believe that they are loaded into the same memory address and makes it impossible for a user program to see the address space of other running programs. As far as a program is concerned, it is the only process running. Note that all virtual and physical addresses need to be divisible by 4096.

### 32-bit instructions

In Protected Mode, we gain access to 32-bit instructions, which enable easy access to 32-bit registers, thus 4GB memory.

# Development

## 1. Entering Protected Mode

We create entries for the Global Descriptor Table (GDT) and load its address into the GDT register by `lgdt` instruction, with additional parameters to enter Protected Mode ([osdev wiki](https://wiki.osdev.org/GDT)). We will use the GDT default values since we'll be using the paging memory scheme. ([git commit](https://github.com/taikiy/kernel/commit/82f6dae884e016ec3045f76bd52ebc4f91b886aa))

## 2. Verify using LLDB

This repo assumes the development is done on an M1 Mac. For that, we need to use LLDB instead of GDB. Here's how:

```shell
# Launch QEMU with gdb server enabled
# '-s'   shorthand for -gdb tcp::1234
# '-S'   freeze CPU at startup (use 'c' to start execution)
> qemu-x86-64 -hda ./boot.bin -s -S &
> lldb
(lldb) gdb-remote 1234    # connects to gdb server on localhost:1234
Process 1 stopped
* thread #1, stop reason = signal SIGTRAP
    frame #0: 0x000000000000fff0
->  0xfff0: addb   %al, (%rax)
    0xfff2: addb   %al, (%rax)
    0xfff4: addb   %al, (%rax)
    0xfff6: addb   %al, (%rax)
Target 0: (No executable module.) stopped.
(lldb) c                  # continue
Process 1 resuming
(lldb) process interrupt  # (C-c also works) now the bootloader should come to a halt
Process 1 stopped
* thread #1, stop reason = signal SIGINT
    frame #0: 0x0000000000007c68
->  0x7c68: jmp    0x7c68
    0x7c6a: addb   %al, (%rax)
    0x7c6c: addb   %al, (%rax)
    0x7c6e: addb   %al, (%rax)
Target 0: (No executable module.) stopped.
(lldb) register read
general:
       rax = 0x0000000000000011
       ...
       rbp = 0x0000000000200000
       rsp = 0x0000000000200000
       ...
       cs = 0x00000008    # Code Segment = 8 means Protected Mode (?)
       ss = 0x00000010
       ds = 0x00000010
       es = 0x00000010
       fs = 0x00000010
       gs = 0x00000010
```

## 3. Enabling A20 Line

[A20 Line](https://wiki.osdev.org/A20_Line) needs to be enabled to access all memories.

---

[previous](./1_real_mode.md) | [next](./3_32-bit_kernel.md)

---

## References

- [GDB to LLDB command map](https://lldb.llvm.org/use/map.html#examining-thread-state)
- [Entering Protected Mode tutorial](http://www.osdever.net/tutorials/view/the-world-of-protected-mode)
