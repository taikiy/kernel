#include "task.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "config.h"
#include "kernel.h"

struct task *current_task = 0;
struct task *head_task = 0;
struct task *tail_task = 0;

status_t initialize_task(struct task *task)
{
    status_t result = ALL_OK;

    memset(task, 0, sizeof(struct task));
    // Map the entire 4GB address space to the task
    // We set `PAGING_ACCESS_FROM_ALL` flag to avoid any complications. In reality, we shouldn't set this flag.
    task->page_directory = paging_new_4gb(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (!task->page_directory)
    {
        return ERROR(EIO);
    }

    task->registers.ip = USER_PROGRAM_VIRTUAL_ADDRESS;
    task->registers.esp = USER_PROGRAM_STACK_VIRTUAL_ADDRESS_START;
    task->registers.ss = USER_PROGRAM_DATA_SELECTOR;

    return result;
}

/// @brief  Returns the next task in the queue.
/// @return The next task in the queue. This function returns 0 if there is no task in the queue.
struct task *get_next_task()
{
    if (!current_task)
    {
        return 0;
    }

    if (!current_task->next) // if current_task is the last task
    {
        return head_task;
    }

    return current_task->next;
}

static status_t remove_task_from_queue(struct task *task)
{
    status_t result = ALL_OK;

    if (!task)
    {
        return ERROR(EINVARG);
    }

    if (task->prev)
    {
        task->prev->next = task->next;
    }
    else
    {
        head_task = task->next;
    }

    if (task->next)
    {
        task->next->prev = task->prev;
    }
    else
    {
        tail_task = task->prev;
    }

    if (current_task == task)
    {
        current_task = get_next_task();
    }

    return result;
}

status_t free_task(struct task *task)
{
    status_t result = ALL_OK;

    if (!task)
    {
        return ERROR(EINVARG);
    }

    if (task->page_directory)
    {
        paging_free_4gb(task->page_directory);
    }

    remove_task_from_queue(task);
    kfree(task);

    return result;
}

struct task *new_task()
{
    status_t result = ALL_OK;

    struct task *task = (struct task *)kzalloc(sizeof(struct task));
    if (!task)
    {
        result = ERROR(ENOMEM);
        goto out;
    }

    result = initialize_task(task);
    if (result != ALL_OK)
    {
        goto out;
    }

    if (!head_task)
    {
        head_task = task;
        tail_task = task;
    }
    else
    {
        tail_task->next = task;
        task->prev = tail_task;
        tail_task = task;
    }

out:
    if (result != ALL_OK)
    {
        if (task)
        {
            free_task(task);
        }
        return 0;
    }
    return task;
}
