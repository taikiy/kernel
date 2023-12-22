#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_desc
{
    uint16_t offset_1;     // offset bits 0..15
    uint16_t selector;     // a code segment selector in GDT or LDT
    uint8_t zero;          // unused bits, set to 0
    uint8_t type_attr;     // type and attributes
    uint16_t offset_2;     // offset bits 16..31
} __attribute__((packed)); // ensures there's no unexpected padding

struct idtr_desc
{
    uint16_t limit; // The length of the Interrupt Descriptor Table minus one
    uint32_t base;  // The address of the Interrupt Descriptor Table
} __attribute__((packed));

void initialize_idt();
void enable_interrupts();
void disable_interrupts();

#endif
