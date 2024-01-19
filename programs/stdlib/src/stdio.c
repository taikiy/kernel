#include "stdio.h"
#include "taios.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

extern int make_syscall(uint32_t syscall_id, int argc, ...);

static bool
is_newline(char c)
{
    return c == '\n' || c == '\r';
}

void
printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    for (const char* c = format; *c; c++) {
        if (*c != '%') {
            putchar(*c);
            continue;
        }

        c++;
        switch (*c) {
            case 'c': {
                char ch = (char)va_arg(args, int);
                putchar(ch);
                break;
            }
            case 's': {
                char* str = va_arg(args, char*);
                for (char* c = str; *c; c++) {
                    putchar(*c);
                }
                break;
            }
            case 'd': {
                int num = va_arg(args, int);
                if (num < 0) {
                    putchar('-');
                    num = -num;
                }
                int divisor = 1;
                while (num / divisor > 9) {
                    divisor *= 10;
                }
                while (divisor > 0) {
                    int digit = num / divisor;
                    putchar('0' + digit);
                    num -= digit * divisor;
                    divisor /= 10;
                }
                break;
            }
            case 'x': {
                int num = va_arg(args, int);
                int divisor = 1;
                while (num / divisor > 15) {
                    divisor *= 16;
                }
                while (divisor > 0) {
                    int digit = num / divisor;
                    if (digit < 10) {
                        putchar('0' + digit);
                    } else {
                        putchar('a' + digit - 10);
                    }
                    num -= digit * divisor;
                    divisor /= 16;
                }
                break;
            }
            default: {
                putchar(*c);
                break;
            }
        }
    }

    va_end(args);
    return;
}

int
getchar()
{
    int c = 0;
    while (1) {
        c = make_syscall(SYSCALL_GETCHAR, 0);
        // blocks until a character is available
        if (c) {
            break;
        }
    }
    return c;
}

char*
gets(char* str)
{
    char* c = str;
    while (1) {
        *c = getchar();
        if (is_newline(*c)) {
            putchar(*c);
            break;
        }
        if (*c == '\b') {
            if (c > str) {
                putchar(*c);
                c--;
            }
            continue;
        }
        putchar(*c);
        c++;
    }
    *c = '\0';
    return str;
}

int
putchar(int c)
{
    return make_syscall(SYSCALL_PUTCHAR, 1, (uint32_t)c);
}
