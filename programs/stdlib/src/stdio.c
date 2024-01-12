#include "stdio.h"
#include <stdint.h>

extern int make_syscall(uint32_t syscall_id, int argc, uint32_t argv[]);

void
print(const char* str)
{
    make_syscall(SYSCALL_PRINT, 1, (uint32_t[]){ (uint32_t)str });
    return;
}

int
getchar()
{
    int c = 0;
    while (1) {
        // argv array cannot be empty, so pass a dummy value
        c = make_syscall(SYSCALL_GETCHAR, 0, (uint32_t[]){ 0 });
        // blocks until a character is available
        if (c != 0) {
            break;
        }
    }
    return c;
}

int
putchar(int c)
{
    return make_syscall(SYSCALL_PUTCHAR, 1, (uint32_t[]){ (uint32_t)c });
}
