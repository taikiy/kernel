#include "syscall.h"
#include "../config.h"
#include "../memory/paging/paging.h"
#include "heap.h"
#include "io.h"
#include "sys.h"

// TODO: This file should be merged together with other ISR definitions in idt.c and placed in isr.c or something.

static INTERRUPT_HANDLER syscall_handlers[TOTAL_SYSCALL_COUNT];

/// @brief Returns an argument value at the given index passed to the syscall. This function returns a void pointer
/// because the argument can be of any type. The caller is responsible for casting the result to the correct type or
/// use `copy_data_from_user_space()` to copy the data pointed by the argument value.
/// @param task The task making the syscall.
/// @param index The index of the argument.
/// @return The argument value.
void*
get_arg_from_task(struct task* task, int index)
{
    switch_to_user_page();
    // `$esp + index` is `uint32_t`. This is the address of the argument value at `index`.
    // Then, we cast it to `uint32_t*` and dereference it to get the value.
    uint32_t arg = *(uint32_t*)(task->registers.esp + (index * sizeof(uint32_t)));
    switch_to_kernel_page();

    return (void*)arg;
}

static void
register_syscall_handler(int command, INTERRUPT_HANDLER handler)
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
    register_syscall_handler(SYSCALL_COMMAND_GETCHAR, sys_getchar);
    register_syscall_handler(SYSCALL_COMMAND_PUTCHAR, sys_putchar);
    register_syscall_handler(SYSCALL_COMMAND_MALLOC, sys_malloc);
    register_syscall_handler(SYSCALL_COMMAND_FREE, sys_free);
}

void*
syscall(int command, struct interrupt_frame* frame)
{
    void* result = 0;

    if (command < 0 || command >= TOTAL_SYSCALL_COUNT) {
        return result;
    }

    INTERRUPT_HANDLER handler = syscall_handlers[command];
    if (handler) {
        result = handler(frame);
    }

    return result;
}
