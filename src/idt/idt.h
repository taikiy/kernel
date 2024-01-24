#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IRQ_0H  0x00
#define IRQ_0EH 0x0E
#define IRQ_20H 0x20
#define IRQ_21H 0x21
#define IRQ_80H 0x80

struct idt_desc
{
    uint16_t offset_low;   // offset bits 0..15
    uint16_t selector;     // a code segment selector in GDT or LDT
    uint8_t zero;          // unused bits, set to 0
    uint8_t type_attr;     // type and attributes
    uint16_t offset_high;  // offset bits 16..31
} __attribute__((packed)); // ensures there's no unexpected padding

struct idtr_desc
{
    uint16_t limit; // The length of the Interrupt Descriptor Table minus one
    uint32_t base;  // The address of the Interrupt Descriptor Table
} __attribute__((packed));

struct interrupt_frame
{
    // https://c9x.me/x86/html/file_module_x86_id_270.html
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t reserved; // esp, but we don't use the one set by `pushad`.
                       // We use the one below set by CPU when an interrupt occurs.
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed));

typedef void* (*INTERRUPT_HANDLER)(struct interrupt_frame*);

void initialize_idt();
void initialize_interrupt_handlers();
void enable_interrupts();
void disable_interrupts();

#endif
