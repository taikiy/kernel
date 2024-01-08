#ifndef SYSCALL_H
#define SYSCALL_H

#include "../idt/idt.h"
#include "../task/task.h"
#include <stdint.h>

enum SYSCALL_COMMAND
{
    SYSCALL_COMMAND_0_SUM = 0,
    SYSCALL_COMMAND_1_PRINT = 1,
    SYSCALL_COMMAND_2_GETCHAR = 2,
    SYSCALL_COMMAND_3_PUTCHAR = 3,
    SYSCALL_COMMAND_4_MALLOC = 4,
    SYSCALL_COMMAND_5_FREE = 5,
};

void initialize_syscall_handlers();
void* syscall(int command, struct interrupt_frame* frame);
void* get_arg_from_task(struct task* task, int index);

#endif
