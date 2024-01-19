#ifndef SYSCALL_PROCESS_H
#define SYSCALL_PROCESS_H

#include "../task/process.h"

void* sys_exec(struct interrupt_frame* frame);

#endif
