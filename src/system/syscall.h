#ifndef SYSCALL_H
#define SYSCALL_H

#include "../idt/idt.h"
#include "../task/task.h"
#include <stdint.h>

enum SYSCALL_COMMAND
{
    SYSCALL_COMMAND_EXEC = 0,
    SYSCALL_COMMAND_EXIT = 1,
    SYSCALL_COMMAND_GETCHAR = 2,
    SYSCALL_COMMAND_PUTCHAR = 3,
    SYSCALL_COMMAND_PUTS = 4,
    SYSCALL_COMMAND_MALLOC = 5,
    SYSCALL_COMMAND_FREE = 6,
};

void initialize_syscall_handlers();
void* syscall(int command, struct interrupt_frame* frame);
void* get_arg_from_task(struct task* task, int index);

#endif
