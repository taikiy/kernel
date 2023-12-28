# PIC (Programmable Interrupt Controller)

## Implement `in()` and `out()`

We create wrapper functions in C for assembler instructions to perform input from/output to a specified I/O port. This is a typical method to write a set of low-level instructions in assembly, export it, and use it from C.

[[commit](https://github.com/taikiy/kernel/commit/99a23c79d93c2fb05d8003ce4d91ab0957b91b2f)]

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

However, before handling the interrupt, we'll need to disable interrupts while in the handler block, store the register contents before handling the interrupt so that we can go back to the previous state, and enable interrupts again. For these reasons, just calling `idt_set` with a C function to its argument isn't enough. We'll create a wrapper in assembly for that.

Note that in the previous section where we implemented `idt_zero`. Strictly speaking, this handler also needs to be defined in assembly as we implement `int21h` in this section. The reason we didn't was just for a quick demonstration purpose.

Also, remember that we need to respond with acknowledgment to IRQ from PIC. For now, we just call `outb(0x20, 0x20)`.

### Multi-tasking?

Imagine handling IRQ 0 timer interrupts (mapped to INT 20h) will allow us to switch between tasks, which will create an illusion of a multi-tasking system even on a single-core processor.

[[commit](https://github.com/taikiy/kernel/commit/8b01fa3cd90cb383c3861efd33f4d8b987b76945)]

---

[previous](./5_interrupt_descriptor_table.md) | [next](./7_memory_management_heap.md)
