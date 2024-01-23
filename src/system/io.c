#include "io.h"
#include "../memory/heap/kheap.h"
#include "../memory/paging/paging.h"
#include "../terminal/terminal.h"
#include "syscall.h"
#include <stdint.h>

// int getchar();
void*
sys_getchar(struct interrupt_frame* frame)
{
    // `sys_getchar` takes no argument from the user program. We can simply return the first key in the keyboard buffer.
    uint8_t key = pop_key();
    return (void*)(int)key;
}

// int putchar(int c);
void*
sys_putchar(struct interrupt_frame* frame)
{
    // `sys_putchar` takes one `char` argument. Since `char` is a 1-byte primitive type, the value is directly stored in
    // the stack. We can get the value by simply casting the argument value to `char`. However, we need to make sure
    // that the next memory on the stack is null-terminated.
    char c[2];
    c[0] = (char)(uint32_t)get_arg_from_task(get_current_task(), 0);
    c[1] = '\0';
    print(c);
    return (void*)(int)*c;
}

// int puts(const char* str);
void*
sys_puts(struct interrupt_frame* frame)
{
    char* arg = (char*)get_arg_from_task(get_current_task(), 0);
    int len = (int)get_arg_from_task(get_current_task(), 1);
    char* str = kzalloc(len + 1);
    copy_data_from_user_space(get_current_task(), arg, str, len);
    print(str);
    kfree(str);
    return (void*)(int)len;
}
