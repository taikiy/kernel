#ifndef SYSCALL_IO_H
#define SYSCALL_IO_H

#include "../idt/idt.h"

void* sys_getchar(struct interrupt_frame* frame);
void* sys_putchar(struct interrupt_frame* frame);

#endif
