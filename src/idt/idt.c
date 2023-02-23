#include "idt.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"

struct idt_desc idt_descriptors[TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void idt_load(struct idtr_desc *ptr);

void idt_zero()
{
    print("Divide by zero error\n");
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

    idt_set(0, idt_zero);

    // Load the IDT
    idt_load(&idtr_descriptor);
}
