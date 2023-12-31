# IDT - Interrupt Descriptor Table

IDT describes how interrupts are invoked in Protected Mode and can be mapped anywhere in memory (as opposed to IVT must be loaded at 0x00). This is equivalent to IVT (Interrupt Vector Table) in Real Mode, but a bit harder to set up. ([osdev wiki](https://wiki.osdev.org/Interrupt_descriptor_table))

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

## Gate Types

- 32-bit Task Gate (0x05 / 0b0101)
- 16-bit Interrupt Gate (0x06 / 0b0110)
- 16-bit Trap Gate (0x07 / 0b0111)
- 32-bit Interrupt Gate (0x0E / 0b1110)
- 32-bit Trap Gate (0x0F / 0b1111)

### Task Gate

Task gates reference TSS descriptors and can assist in multi-tasking when exceptions occur.

### Interrupt Gate

Interrupt gates are to be used for interrupts that we want to invoke ourselves in our code.

### Trap Gate

Trap gates are like interrupt gates, but used for exceptions. They disable interrupts on entry and re-enable on an `iret` instruction.

## Loading IDT

Use `lidt` instruction.

[[commit](https://github.com/taikiy/kernel/commit/e6abc5cf5b4a14e49d5b098d4d46bafc4e75f587)]

---

[Previous](./4_writing_kernel_in_C.md) | [Next](./6_programmable_interrupt_controller.md) | [Home](../README.md)
