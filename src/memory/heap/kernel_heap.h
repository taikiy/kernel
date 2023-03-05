#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

#include <stdint.h>
#include <stddef.h>

void kernel_heap_initialize();
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif
