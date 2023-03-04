#include "idt.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"
#include "io/io.h"

struct idt_desc idt_descriptors[TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void idt_load(struct idtr_desc *ptr);
extern void int21h();
extern void int_noop();

void idt_zero()
{
    print("Divide by zero error\n");
}

void int21h_handler()
{
    print("Keyboard pressed\n");
    outb(0x20, 0x20);
}

void int_noop_handler()
{
    outb(0x20, 0x20);
}

void idt_set(int interrupt_number, void *address)
{
    struct idt_desc *desc = &idt_descriptors[interrupt_number];

    desc->offset_1 = (uint32_t)address & 0xffff;
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;
    // Type:      0x0e/0b1110 = 32-bit interrupt gate
    // Attribute: Storage Segment = 0 (interrupt)
    //            DPL (Descriptor Privilege Level) = 0b11 or 3 (Ring 3)
    //            Present = 1 (0 for unused interrupt)
    desc->type_attr = 0x0e | 0xe0;
    desc->offset_2 = (uint32_t)address >> 16;
}

void idt_init()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    for (int i = 0; i < TOTAL_INTERRUPTS; i++)
    {
        idt_set(i, int_noop);
    }
    idt_set(0, idt_zero);
    idt_set(0x21, int21h);

    // Load the IDT
    idt_load(&idtr_descriptor);
}
