#ifndef TAIOS_H
#define TAIOS_H

#include <stdbool.h>

#define MAX_COMMAND_LENGTH 1024

struct command_args
{
    char* value;
    struct command_args* next;
};

#define SYSCALL_EXEC    0
#define SYSCALL_EXIT    1
#define SYSCALL_GETCHAR 2
#define SYSCALL_PUTCHAR 3
#define SYSCALL_PUTS    4
#define SYSCALL_MALLOC  5
#define SYSCALL_FREE    6

int exec(const char* path);
void exit(int status);

#endif
