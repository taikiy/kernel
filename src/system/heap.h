#ifndef SYSCALL_HEAP_H
#define SYSCALL_HEAP_H

#include "../idt/idt.h"

void* sys_malloc(struct interrupt_frame* frame);
void* sys_free(struct interrupt_frame* frame);

#endif
