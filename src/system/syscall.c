#include "syscall.h"
#include "config.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "sys.h"
#include "task/task.h"
#include "terminal/terminal.h"

static SYSCALL_HANDLER_CALLBACK syscall_handlers[TOTAL_SYSCALL_COUNT];

/// @brief Returns an argument value at the given index passed to the syscall. This function returns a void pointer
/// because the argument can be of any type. The caller is responsible for casting the result to the correct type or
/// use `copy_data_from_user_space()` to copy the data pointed by the argument value.
/// @param task The task making the syscall.
/// @param index The index of the argument.
/// @return The argument value.
static void*
get_arg_from_task(struct task* task, int index)
{
    switch_to_user_page();
    // `$esp + index` is `uint32_t`. This is the address of the argument value at `index`.
    // Then, we cast it to `uint32_t*` and dereference it to get the value.
    uint32_t arg = *(uint32_t*)(task->registers.esp + (index * sizeof(uint32_t)));
    switch_to_kernel_page();

    return (void*)arg;
}

void*
sys_sum(struct interrupt_frame* frame)
{
    print("sys_sum\n");

    struct task* current_task = get_current_task();

    // `sys_sum` takes two `int` arguments. Since `int` is a 4-byte primitive type, the value is directly stored in
    // the stack. We can get the value by simply casting the argument value to `int`.
    int arg1 = (int)get_arg_from_task(current_task, 0);
    int arg2 = (int)get_arg_from_task(current_task, 1);

    print("arg1: ");
    print_int(arg1);
    print("\narg2: ");
    print_int(arg2);
    print("\n");

    return (void*)arg1 + arg2;
}

void*
sys_print(struct interrupt_frame* frame)
{
    print("sys_print\n");

    struct task* current_task = get_current_task();

    // `sys_print` takes one `char*` argument. Since `char*` is a pointer type, the value is the address of the string
    // in the user space. We can get the string from the address by using `copy_data_from_user_space()`.
    char* arg1 = (char*)get_arg_from_task(current_task, 0);
    char buf[4096];
    copy_data_from_user_space(current_task, arg1, &buf, sizeof(buf));

    print(buf);
    print("\n");

    return 0;
}

void*
sys_getchar(struct interrupt_frame* frame)
{
    // `sys_getchar` takes no argument from the user program. We can simply return the first key in the keyboard buffer.
    uint8_t key = pop_key();
    return (void*)(uint32_t)key;
}

static void
register_syscall_handler(int command, SYSCALL_HANDLER_CALLBACK handler)
{
    if (command < 0 || command >= TOTAL_SYSCALL_COUNT) {
        panic("Invalid syscall command");
    }

    if (syscall_handlers[command]) {
        panic("Syscall already registered");
    }

    syscall_handlers[command] = handler;
}

void
initialize_syscall_handlers()
{
    register_syscall_handler(SYSCALL_COMMAND_0_SUM, sys_sum);
    register_syscall_handler(SYSCALL_COMMAND_1_PRINT, sys_print);
    register_syscall_handler(SYSCALL_COMMAND_2_GET_KEY, sys_getchar);
}

void*
syscall(int command, struct interrupt_frame* frame)
{
    void* result = 0;

    if (command < 0 || command >= TOTAL_SYSCALL_COUNT) {
        return result;
    }

    SYSCALL_HANDLER_CALLBACK handler = syscall_handlers[command];
    if (handler) {
        result = handler(frame);
    }

    return result;
}
