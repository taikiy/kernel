#include "idt.h"
#include "config.h"
#include "io/io.h"
#include "kernel.h"
#include "memory/memory.h"
#include "task/task.h"
#include "terminal/terminal.h"

struct idt_desc idt_descriptors[TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

static SYSCALL_HANDLER syscall_handlers[TOTAL_SYSCALL_COUNT];

extern void load_idt(struct idtr_desc* ptr);
extern void int_noop();
extern void int21h();
extern void isr80h();

static void
int0h()
{
    print("Divide by zero error\n");
}

void
int_noop_handler()
{
    outb(0x20, 0x20);
}

void
int21h_handler()
{
    print("Keyboard pressed\n");
    outb(0x20, 0x20);
}

static void*
syscall(int command, struct interrupt_frame* frame)
{
    void* result = 0;

    if (command < 0 || command >= TOTAL_SYSCALL_COUNT) {
        return result;
    }

    SYSCALL_HANDLER handler = syscall_handlers[command];
    if (handler) {
        result = handler(frame);
    }

    return result;
}

void
register_syscall(int command, SYSCALL_HANDLER handler)
{
    if (command < 0 || command >= TOTAL_SYSCALL_COUNT) {
        panic("Invalid syscall command");
    }

    if (syscall_handlers[command]) {
        panic("Syscall already registered");
    }

    syscall_handlers[command] = handler;
}

void*
isr80h_handler(int command, struct interrupt_frame* frame)
{
    void* res = 0;

    switch_to_kernel_page();
    save_current_task_state(frame);

    res = syscall(command, frame);

    switch_to_user_page();

    return res;
}

static void
idt_set(int interrupt_number, void* address)
{
    struct idt_desc* desc = &idt_descriptors[interrupt_number];

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

void
initialize_idt()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    for (int i = 0; i < TOTAL_INTERRUPTS; i++) {
        idt_set(i, int_noop);
    }
    idt_set(INT_0H, int0h);
    idt_set(INT_21H, int21h);
    idt_set(INT_80H, isr80h);

    // Load the IDT
    load_idt(&idtr_descriptor);
}
