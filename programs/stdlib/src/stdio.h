#ifndef STDIO_H
#define STDIO_H

#define SYSCALL_PRINT   1
#define SYSCALL_GETCHAR 2
#define SYSCALL_PUTCHAR 3

void print(const char* str);
int getchar();
int putchar(int c);

#endif
