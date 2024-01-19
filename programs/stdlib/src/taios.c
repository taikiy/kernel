#include "taios.h"
#include <stdint.h>

extern int make_syscall(uint32_t syscall_id, int argc, ...);

int
exec(const char* path)
{
    return make_syscall(SYSCALL_EXEC, 1, (uint32_t)path);
}
