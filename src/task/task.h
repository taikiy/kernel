#ifndef TASK_H
#define TASK_H

#include "idt/idt.h"
#include "memory/paging/paging.h"
#include "process.h"
#include <stdint.h>

struct registers
{
    // General purpose registers that need to be saved/restored
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebx; // Bring ebx to the end to make it easier to save/restore. See
                  // `restore_general_purpose_registers` in `task.asm`

    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
};

struct task
{
    struct paging_map* user_page;
    struct registers registers;
    struct process* process;
    struct task* next;
    struct task* prev;
};

struct task* create_task(struct process* process);
status_t free_task(struct task* task);
struct task* get_current_task();
void save_current_task_state(struct interrupt_frame* frame);
void restore_current_task_state(struct interrupt_frame* frame);
void start_tasks();

#endif
