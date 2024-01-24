#include "idt.h"
#include "../config.h"
#include "../io/io.h"
#include "../keyboard/keyboard.h"
#include "../memory/memory.h"
#include "../memory/paging/paging.h"
#include "../system/sys.h"
#include "../system/syscall.h"
#include "../task/task.h"
#include "../terminal/terminal.h"

extern void load_idt(struct idtr_desc* ptr);
extern void* isr_table[TOTAL_INTERRUPTS];

static struct idt_desc idt_descriptors[TOTAL_INTERRUPTS];
static struct idtr_desc idtr_descriptor;
static INTERRUPT_HANDLER interrupt_handlers[TOTAL_INTERRUPTS];

/// @brief Saves the current task registers as it was when the interrupt 0x80 was made. This function must be called
/// while the kernel space paging is active.
/// @param frame The interrupt frame that contains the current task registers
static void
save_task_state(struct task* task, struct interrupt_frame* frame)
{
    if (!task) {
        panic("Cannot save the state of a null task!");
    }

    if (!frame) {
        panic("Cannot save the state of a task with a null frame!");
    }

    // When a new process is created, we set the EIP to the address of the entry point of the process. We don't switch
    // to the new process until the first time the clock interrupt is triggered. So, when the clock interrupt is called
    // for the first time, we have to make sure that we don't overwrite the EIP of the new process with some garbage
    // value. Once the process is switched to, we change the process state to RUNNING.
    if (task->process->state != PROCESS_STATE_RUNNING) {
        return;
    }

    task->registers.eip = frame->eip;
    task->registers.cs = frame->cs;
    task->registers.eflags = frame->eflags;
    task->registers.esp = frame->esp;
    task->registers.ss = frame->ss;

    task->registers.eax = frame->eax;
    task->registers.ecx = frame->ecx;
    task->registers.edx = frame->edx;
    task->registers.ebp = frame->ebp;
    task->registers.esi = frame->esi;
    task->registers.edi = frame->edi;
    task->registers.ebx = frame->ebx;
}

void
default_interrupt_handler()
{
    // no-op
}

void*
exception_handler(struct interrupt_frame* frame)
{
    print("Exception occurred\n");

    disable_interrupts();
    terminate_process(get_current_task()->process, -1);
    enable_interrupts();

    // Send ack. Since we've already freed the process so there's nothing to return to, so call switch_task() to switch
    // to another process' task.
    outb(0x20, 0x20);
    switch_task();
    return 0;
}

void*
clock()
{
    // Send ack before switching tasks. Once we call switch_task(), we will not return to this function.
    outb(0x20, 0x20);
    switch_task();
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
interrupt_handle_wrapper(int irq, struct interrupt_frame* frame)
{
    void* result = 0;

    if (interrupt_handlers[irq]) {
        struct task* current_task = get_current_task();
        switch_to_kernel_page();
        save_task_state(current_task, frame);
        result = interrupt_handlers[irq](frame);
        switch_to_user_page(current_task);
    } else {
        default_interrupt_handler();
    }

    // send ack
    outb(0x20, 0x20);

    return result;
}

static void
register_interrupt_handler(int irq, INTERRUPT_HANDLER handler)
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

    desc->offset_low = (uint32_t)address & 0xffff;
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;
    // Type:      0x0e/0b1110 = 32-bit interrupt gate
    // Attribute: Storage Segment = 0 (interrupt)
    //            DPL (Descriptor Privilege Level) = 0b11 or 3 (Ring 3)
    //            Present = 1 (0 for unused interrupt)
    desc->type_attr = 0x0e | 0xe0;
    desc->offset_high = (uint32_t)address >> 16;
}

void
initialize_idt()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    for (int i = 0; i < TOTAL_INTERRUPTS; i++) {
        idt_set(i, isr_table[i]);
    }

    load_idt(&idtr_descriptor);
}

void
initialize_interrupt_handlers()
{
    memset(interrupt_handlers, 0, sizeof(interrupt_handlers));

    register_interrupt_handler(IRQ_0H, exception_handler);
    register_interrupt_handler(IRQ_0EH, exception_handler);
    register_interrupt_handler(IRQ_20H, clock);
    register_interrupt_handler(IRQ_21H, keyboard_interrupt_handler);
    register_interrupt_handler(IRQ_80H, int80h_handler);

    initialize_syscall_handlers();
}
