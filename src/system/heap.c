#include "heap.h"
#include "../memory/heap/kheap.h"
#include "../memory/memory.h"
#include "../task/process.h"
#include "../task/task.h"
#include "syscall.h"

// void* malloc(size_t size);
void*
sys_malloc(struct interrupt_frame* frame)
{
    struct task* current_task = get_current_task();
    size_t size = (size_t)get_arg_from_task(current_task, 0);
    return process_malloc(current_task->process, size);
}

// void free(void* ptr);
void*
sys_free(struct interrupt_frame* frame)
{
    void* ptr = (void*)get_arg_from_task(get_current_task(), 0);
    process_free(get_current_task()->process, ptr);
    return 0;
}
