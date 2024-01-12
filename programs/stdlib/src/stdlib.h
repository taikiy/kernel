#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

#define SYSCALL_MALLOC 4
#define SYSCALL_FREE   5

void* malloc(size_t size);
void free(void* ptr);

#endif
