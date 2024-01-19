#include "io.h"
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
    // the stack. We can get the value by simply casting the argument value to `char`.
    char c = (char)(uint32_t)get_arg_from_task(get_current_task(), 0);
    print(&c);
    return (void*)(int)c;
}
