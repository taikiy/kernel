#ifndef SYSCALL_H
#define SYSCALL_H

#include "idt/idt.h"
#include <stdint.h>

typedef INTERRUPT_HANDLER_CALLBACK SYSCALL_HANDLER_CALLBACK;

enum SYSCALL_COMMAND
{
    SYSCALL_COMMAND_0_SUM     = 0,
    SYSCALL_COMMAND_1_PRINT   = 1,
    SYSCALL_COMMAND_2_GET_KEY = 2,
};

void initialize_syscall_handlers();
void* syscall(int command, struct interrupt_frame* frame);

#endif
