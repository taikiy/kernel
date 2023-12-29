#include "syscall.h"
#include "config.h"
#include "sys.h"
#include "terminal/terminal.h"

static SYSCALL_HANDLER syscall_handlers[TOTAL_SYSCALL_COUNT];

void*
sys_print(struct interrupt_frame* frame)
{
    print("sys_print\n");
    return 0;
}

static void
register_syscall_handler(int command, SYSCALL_HANDLER handler)
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
    register_syscall_handler(SYSCALL_COMMAND_0_PRINT, sys_print);
}

void*
syscall(int command, struct interrupt_frame* frame)
{
    void* result = 0;

    if (command < 0 || command >= TOTAL_SYSCALL_COUNT) {
        return result;
    }

    SYSCALL_HANDLER handler = syscall_handlers[command];
    if (handler) {
        result = handler(frame);
    }

    return result;
}
