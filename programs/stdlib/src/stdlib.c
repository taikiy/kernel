#include "stdlib.h"
#include <stdint.h>

extern int make_syscall(uint32_t syscall_id, int argc, ...);

void*
malloc(size_t size)
{
    return (void*)make_syscall(SYSCALL_MALLOC, 1, (uint32_t)size);
}

void
free(void* ptr)
{
    make_syscall(SYSCALL_FREE, 1, (uint32_t)ptr);
    return;
}
