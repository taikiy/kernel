# Protected Mode Development Part 4 - Memory Management

## Implement `in()` and `out()`

We create wrapper functions in C for assembler instructions to perform input from/output to a specified I/O port. This is a typical method to write a set of low-level instructions in assembly, export it, and use it from C.

([commit])(https://github.com/taikiy/kernel/commit/99a23c79d93c2fb05d8003ce4d91ab0957b91b2f)

## Programmable Interrupt Controller

### What are PIC and IRQ?

Programmable Interrupt Controller (PIC) allows different types of hardware, such as hard disk and keyboard, to interrupt the processor state. This is one of the most important chips that make up the x86 architecture, and without it, the system would not be an interrupt-driven architecture.

For example, ISA (Industry Standard Architecture) standard defines IRQ 0 as a timer interrupt, IRQ 1 as a keyboard interrupt, and so on. IRQs will communicate with PIC, and PIC will raise the interrupt line which is connected to the processor.

The system has two PICs - one for master ports and the other for slave ports. The master ports handle IRQ 0-7, and the slave ports handle IRQ 8-15. The master PIC control ports are 0x20 (command) and 0x21 (data), and the slave PIC control ports are 0xA0 (command) and 0xA1 (data).

In protected mode, some IRQs are mapped to interrupts 8-15, but this is a problem as these interrupts are reserved in protected mode for exceptions. For that reason, We need to remap the PIC. Without remapping, it would be difficult to distinguish between software errors and IRQs.

A common choice is to remap IRQs to the beginning of the available INT range, which is INT 0x20 for the master and INT 0x28 for the slave.

### Interrupt acknowledgment

Whenever a PIC calls us (the kernel) that an interrupt has happened, we must acknowledge that we have handled the interrupt. If we don't respond, PIC will never call us again for other interrupts. For this reason, we implement `int_noop_handler` which acknowledges the interrupts without doing anything.

### Handling the interrupts

Assuming we've remapped IRQ 0 to 0x20, that means we'll receive timer interrupts (IRQ 0) as INT 0x20, keyboard interrupts (IRQ 1) as 0x21, and so on.

Previously, we implemented IDT (Interrupt Descriptor Table) where we define the mapping between interrupts and their handlers. This is the time we start using IDT. We can define IDT for INT 0x21, for example, to handle the keyboard interrupts.

However, before handling the interrupt, we'll need to disable interrupts while in the handler block, store the register contents before handling the interrupt so that we can go back to the previous state, and enable interrupts back again. For these reasons, just calling `idt_set` with a C function to its argument isn't enough. We'll create a wrapper in assembly for that.

Note that in the previous section where we implemented `idt_zero`. Strictly speaking, this handler also needs to be defined in assembly as we implement `int21h` in this section. The reason we didn't was just for a quick demonstration purpose.

Also, remember that we need to respond with acknowledgment to IRQ from PIC. For now, we just call `outb(0x20, 0x20)`.

### Multi-tasking?

You can imagine handling IRQ 0 timer interrupts (mapped to INT 20h) will allow us to switch between tasks, which will create an illusion of a multi-tasking system even on a single-core processor.

[commit](https://github.com/taikiy/kernel/commit/8b01fa3cd90cb383c3861efd33f4d8b987b76945)

## The heap and memory allocation

Addressable memory in a 32-bit kernel is limited to 4,294,967,296 bytes or 4.29GB. When the system starts, the RAM is in an uninitialized state. BIOS will start initializing parts of RAM for video and other hardware. Unused parts of RAM which start from _0x01000000_ are available to us. However, since 0xC0000000 is reserved, the memory array starting from 0x01000000 can give us a maximum of 3.22GB for a machine with 4GB or higher RAM installed.

As a kernel, it is responsible for allocating heap memory and keeping track of which regions are being used, freed, and so on.

### The simplest possible heap implementation

1. `current_address` points to a start address that is free to use
2. a user calls `malloc` stores the address pointed by `current_address` to `tmp` variable
3. increment `current_address` by the allocated size
4. return `tmp`

Pro - Very easy to implement\
Con - No way to free memory so the system will run out of usable RAM at some point

### Our heap implementation

- We will have a table that describes which memory is taken, free, etc. We'll call it the "entry table"
- We will have a pointer to a piece of free memory, which will be allocated when `malloc` is called. We'll call it the "data pool".
- The heap will be allocated by blocks. Each block will be aligned to 4096 bytes = 4k bytes. If a user requests 50 bytes, 4096 bytes of memory will be returned.
- The entries for a single `malloc` call must be consecutive in the entry table.

Pro - Fast allocation/freeing of memory. Easy to implement.\
Con - Using blocks will result in wasted bytes. Memory fragmentation is possible. (we have enough free blocks, but since they are fragmented, they cannot be allocated.)

#### The Entry Table

- The entry table contains an array of 1-byte values that represent an entry (1 block) in the data pool.
- An array size will be the allocated heap size / 4096. If we allocate 100MB, 25600 bytes (entries = blocks).

#### The Entry structure

An entry is 1 byte and the structure is:

```
  +------------------------------------------------------+
  |          Flags           |         Entry Type        |
  |--------------------------+---------------------------|
7 | HAS_N | IS_FIRST | 0 | 0 | ET_3 | ET_2 | ET_1 | ET_0 | 0
  +------------------------------------------------------+
```

##### Flags

`HAS_N` - Set 1 if the entry to the right of us is part of our allocation\
`IS_FIRST` - Set 1 if this is the first entry of our allocation

##### Entry Types

`HEAP_BLOCK_TABLE_ENTRY_TAKEN` - The entry is taken and the address cannot be used\
`HEAP_BLOCK_TABLE_ENTRY_FREE` - The entry is free and may be used

#### The data pool

Simply a raw flat array of bytes that our heap implementation can give to users who need memory.

### Debugging the heap code

Use `qemu-system-i386` just so that we are truly in a 32-bit system. x86_64 is a 64-bit emulator.
