#include "idt.h"
#include "config.h"
#include "io/io.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "system/sys.h"
#include "system/syscall.h"
#include "terminal/terminal.h"

extern void load_idt(struct idtr_desc* ptr);
extern void* isr_pointer_table[TOTAL_INTERRUPTS];

static struct idt_desc idt_descriptors[TOTAL_INTERRUPTS];
static struct idtr_desc idtr_descriptor;
static INTERRUPT_HANDLER_CALLBACK interrupt_handlers[TOTAL_INTERRUPTS];

/// @brief Saves the current task registers as it was when the interrupt 0x80 was made. This function must be called
/// while the kernel space paging is active.
/// @param frame The interrupt frame that contains the current task registers
static void
save_current_task_state(struct interrupt_frame* frame)
{
    struct task* current_task = get_current_task();
    if (!current_task) {
        panic("Cannot save the state of a null task!");
    }

    if (!frame) {
        panic("Cannot save the state of a task with a null frame!");
    }

    current_task->registers.ip = frame->ip;
    current_task->registers.cs = frame->cs;
    current_task->registers.flags = frame->flags;
    current_task->registers.esp = frame->esp;
    current_task->registers.ss = frame->ss;

    current_task->registers.eax = frame->eax;
    current_task->registers.ecx = frame->ecx;
    current_task->registers.edx = frame->edx;
    current_task->registers.ebp = frame->ebp;
    current_task->registers.esi = frame->esi;
    current_task->registers.edi = frame->edi;
    current_task->registers.ebx = frame->ebx;
}

void
default_interrupt_handler()
{
    // no-op
}

void*
int0h_handler(struct interrupt_frame* frame)
{
    print("Divide by zero error\n");
    return 0;
}

void*
int20h_handler(struct interrupt_frame* frame)
{
    print("Timer tick\n");
    return 0;
}

void*
int21h_handler(struct interrupt_frame* frame)
{
    print("Keyboard pressed\n");
    return 0;
}

void*
int80h_handler(struct interrupt_frame* frame)
{
    void* res = 0;

    int command = frame->eax;
    res = syscall(command, frame);

    return res;
}

void*
interrupt_handler_wrapper(int irq, struct interrupt_frame* frame)
{
    void* result = 0;

    if (interrupt_handlers[irq]) {
        switch_to_kernel_page();
        save_current_task_state(frame);
        result = interrupt_handlers[irq](frame);
        switch_to_user_page();
    } else {
        default_interrupt_handler();
    }

    // send ack
    outb(0x20, 0x20);

    return result;
}

static void
register_interrupt_handler(int irq, INTERRUPT_HANDLER_CALLBACK handler)
{
    if (irq < 0 || irq >= TOTAL_INTERRUPTS) {
        panic("Invalid interrupt number");
    }

    if (interrupt_handlers[irq]) {
        panic("Interrupt already registered");
    }

    interrupt_handlers[irq] = handler;
}

static void
idt_set(int irq, void* address)
{
    struct idt_desc* desc = &idt_descriptors[irq];

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
        idt_set(i, isr_pointer_table[i]);
    }

    load_idt(&idtr_descriptor);
}

void
initialize_interrupt_handlers()
{
    memset(interrupt_handlers, 0, sizeof(interrupt_handlers));

    register_interrupt_handler(IRQ_0H, int0h_handler);
    // register_interrupt_handler(0x20, int20h_handler);
    register_interrupt_handler(IRQ_21H, int21h_handler);
    register_interrupt_handler(IRQ_80H, int80h_handler);
}
