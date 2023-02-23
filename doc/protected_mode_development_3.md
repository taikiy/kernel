# Protected Mode Development - Part 3

## Writing C program in Protected Mode

Now, we start writing our kernel in C. [commit](https://github.com/taikiy/kernel/commit/2d58737d86668579690729b80f80fa2a9d01d422)

Debugging steps are the same as what we did in the [previous note](./protected_mode_development_2.md) "5. Build, run, and debug".

## Writing "Hello, World!" in Protected Mode

### What is Text Mode?

- Write ASCII to video memory
  - starting at address 0xB8000 for colors, or
  - starting at address 0xB0000 for monochrome
- Supports 16 unique colors, e.g.,
  - color number 1, Black, RGB = 0 0 0, HEX = 00 00 00
  - color number 2, Blue, RGB = 0 0 170, HEX = 00 00 AA
  - ...
  - color number 15, White, RGB = 255 255 255, HEX = FF FF FF
- No need to set individual screen pixels for printing characters
  - The video card will take the ascii value and automatically reference it in a font table to output pixels for the letter.

Each character will take 2 bytes.

- Byte 0 = ASCII character
- Byte 1 = Color

Example: Display "AB" in white

```x86asm
0xB8000 = 'A'
0xB8001 = 0x0F
0xB8002 = 'B'
0xB8003 = 0x0F
```

### Writing to video memory in C

```c
char *video_mem = (char *)0xB8000;
video_mem[0] = 'A';
video_mem[1] = '2'; // Green
```

but, we can optimize this by using `uint16_t`

```c
#include <stdint.h>

...

uint16_t *video_mem = (uint16_t *)0xB8000;
video_mem[0] = 0x0241; // 'A' (65 decimal or 0x41 hex) + Green (0x02) in the little-endian format
```

and with bit more helper functions, we can call `print` and write "Hello, World!".

[commit](https://github.com/taikiy/kernel/commit/fa0fbabaf9c9cd93bcaff966fb54164fa3da3df6)

### Interrupt Descriptor Table

IDT describes how interrupts are invoked in Protected Mode, and can be mapped anywhere in memory (as opposed to IVT must be loaded at 0x00). This is equivalent of IVT (Interrupt Vector Table) in Real Mode, but a bit harder to setup. ([osdev wiki](https://wiki.osdev.org/Interrupt_descriptor_table))

We'll use the C structure to represent IDT entries.

```c
struct idt_desc
{
  uint16_t offset_1;  // offset bits 0..15
  uint16_t selector;  // a code segment selector in GDT or LDT
  uint8_t  zero;      // unused bits, set to 0
  uint8_t  type_attr; // type and attributes
  uint16_t offset_2;  // offset bits 16..31
} __attribute__((packed));  // ensures there's no unexpected padding
```

We can create an array to store interrupt descriptors. Index 0 defines interrupt 0 `int 0`, index 1 defines `int 1`, and so on.

```c
struct idt_desc idt_descriptors[MAX_INTERRUPTS];
```

The location of the IDT is kept in the IDTR (IDT Register). This is loaded using the `lidt` assembly instruction, whose argument is a pointer to an IDTR.

```c
struct idtr_desc
{
  uint16_t limit; // The length of the Interrupt Descriptor Table minus one
  uint32_t base;  // The address of the Interrupt Descriptor Table
} __attribute__((packed));
```

![IDT](https://pdos.csail.mit.edu/6.828/2008/readings/i386/fig9-1.gif)

### Gate Types

- 32-bit Task Gate (0x05 / 0b0101)
- 16-bit Interrupt Gate (0x06 / 0b0110)
- 16-bit Trap Gate (0x07 / 0b0111)
- 32-bit Interrupt Gate (0x0E / 0b1110)
- 32-bit Trap Gate (0x0F / 0b1111)

#### Task Gate

Task gates reference TSS descriptors and can assist in multi-tasking when exceptions occur.

#### Interrupt Gate

Interrupt gates are to be used for interrupts that we want to invoke ourselves in our code.

#### Trap Gate

Trap gates are like interrupt gates, but used for exceptions. They disable interrupts on entry and re-enable on an `iret` instruction.

### Loading IDT

Use `lidt` instruction.

([commit]())
