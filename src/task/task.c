#include "task.h"
#include "../config.h"
#include "../idt/idt.h"
#include "../memory/heap/kheap.h"
#include "../memory/memory.h"
#include "../memory/paging/paging.h"
#include "../status.h"
#include "../system/sys.h"

struct task* current_task = 0;
struct task* head_task = 0;
struct task* tail_task = 0;

extern void jump_to_user_space(struct registers* registers);

static status_t
initialize_task(struct task* task, struct process* process)
{
    status_t result = ALL_OK;

    memset(task, 0, sizeof(struct task));
    // Map the entire 4GB address space to the task
    task->user_page = new_paging_map(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITABLE);
    if (!task->user_page) {
        return ERROR(EIO);
    }

    task->registers.eip = (uint32_t)process->program->entry_point_address;
    task->registers.esp = USER_PROGRAM_STACK_VIRTUAL_ADDRESS_START;
    task->registers.ss = USER_PROGRAM_DATA_SELECTOR;
    task->registers.cs = USER_PROGRAM_CODE_SELECTOR;

    task->process = process;

    return result;
}

/// @brief  Returns the next task in the queue.
/// @return The next task in the queue. This function returns 0 if there is no task in the queue.
static struct task*
get_next_task()
{
    if (!current_task) {
        panic("No task is running in the system!");
    }

    if (!current_task->next) // if current_task is the last task
    {
        return head_task;
    }

    return current_task->next;
}

static status_t
remove_task_from_queue(struct task* task)
{
    status_t result = ALL_OK;

    if (!task) {
        return ERROR(EINVARG);
    }

    // TODO: if there are no more tasks, set null to all task pointers

    if (task->prev) {
        task->prev->next = task->next;
    } else {
        head_task = task->next;
    }

    if (task->next) {
        task->next->prev = task->prev;
    } else {
        tail_task = task->prev;
    }

    if (current_task == task) {
        current_task = get_next_task();
    }

    return result;
}

struct task*
create_task(struct process* process)
{
    status_t result = ALL_OK;

    struct task* task = (struct task*)kzalloc(sizeof(struct task));
    if (!task) {
        result = ERROR(ENOMEM);
        goto out;
    }

    result = initialize_task(task, process);
    if (result != ALL_OK) {
        goto out;
    }

    if (!head_task) {
        head_task = task;
        tail_task = task;
        current_task = task;
    } else {
        tail_task->next = task;
        task->prev = tail_task;
        tail_task = task;
    }

out:
    if (result != ALL_OK) {
        if (task) {
            free_task(task);
        }
        return 0;
    }
    return task;
}

status_t
free_task(struct task* task)
{
    status_t result = ALL_OK;

    if (!task) {
        return ERROR(EINVARG);
    }

    if (task->user_page) {
        free_paging_map(task->user_page);
    }

    remove_task_from_queue(task);
    kfree(task);

    // Do not free the process here, because the process may be shared by other tasks

    return result;
}

struct task*
get_current_task()
{
    return current_task;
}

void
switch_task()
{
    struct task* next_task = get_next_task();
    if (!next_task) {
        panic("No task is running in the system!");
    }

    current_task = next_task;

    if (current_task->process->state == PROCESS_STATE_READY) {
        current_task->process->state = PROCESS_STATE_RUNNING;
    }

    switch_to_user_page(current_task);
    jump_to_user_space(&current_task->registers);
}
