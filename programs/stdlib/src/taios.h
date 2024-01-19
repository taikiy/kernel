#ifndef TAIOS_H
#define TAIOS_H

#include <stdbool.h>

#define SYSCALL_EXEC    1
#define SYSCALL_GETCHAR 2
#define SYSCALL_PUTCHAR 3
#define SYSCALL_MALLOC  4
#define SYSCALL_FREE    5

int exec(const char* path);

#endif
