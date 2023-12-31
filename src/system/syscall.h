#ifndef SYSCALL_H
#define SYSCALL_H

#include "idt/idt.h"
#include <stdint.h>

typedef void* (*SYSCALL_HANDLER)(struct interrupt_frame*);

enum SYSCALL_COMMAND
{
    SYSCALL_COMMAND_0_SUM = 0,
    SYSCALL_COMMAND_1_PRINT = 1,
};

void initialize_syscall_handlers();
void* syscall(int command, struct interrupt_frame* frame);

#endif
