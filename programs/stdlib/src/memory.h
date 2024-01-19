#ifndef STD_MEMORY_H
#define STD_MEMORY_H

#include <stddef.h>

void* memset(void* ptr, int c, size_t size);
int memcmp(const void* ptr1, const void* ptr2, size_t size);
void* memcpy(void* dest, const void* src, size_t size);

#endif
