#include "task.h"
#include "config.h"
#include "kernel.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"

struct task *current_task = 0;
struct task *head_task = 0;
struct task *tail_task = 0;

extern void return_task();

struct task *get_current_task()
{
    return current_task;
}

static status_t initialize_task(struct task *task, struct process *process)
{
    status_t result = ALL_OK;

    memset(task, 0, sizeof(struct task));
    // Map the entire 4GB address space to the task
    // We set `PAGING_ACCESS_FROM_ALL` flag to avoid any complications. In reality, we shouldn't set
    // this flag.
    task->page_directory = paging_new_4gb(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (!task->page_directory)
    {
        return ERROR(EIO);
    }

    task->registers.ip = USER_PROGRAM_VIRTUAL_ADDRESS_START;
    task->registers.esp = USER_PROGRAM_STACK_VIRTUAL_ADDRESS_START;
    task->registers.ss = USER_PROGRAM_DATA_SELECTOR;

    task->process = process;

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

    // TODO: if there are no more tasks, set null to all task pointers

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

    // Do not free the process here, because the process may be shared by other tasks

    return result;
}

struct task *create_task(struct process *process)
{
    status_t result = ALL_OK;

    struct task *task = (struct task *)kzalloc(sizeof(struct task));
    if (!task)
    {
        result = ERROR(ENOMEM);
        goto out;
    }

    result = initialize_task(task, process);
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

static status_t switch_task(struct task *task)
{
    status_t result = ALL_OK;

    if (!task)
    {
        return ERROR(EINVARG);
    }

    if (!task->page_directory)
    {
        return ERROR(EINVARG);
    }

    paging_switch(paging_4gb_chunk_get_directory(task->page_directory));
    current_task = task;

    return result;
}

// status_t
// task_page()
// {
//     status_t result = ALL_OK;

//     if (!get_next_task()) {
//         return ERROR(EINVARG);
//     }

//     set_segment_registers_for_userland();
//     result = switch_task(get_next_task());
//     if (result != ALL_OK) {
//         return result;
//     }

//     return result;
// }

status_t start_tasks()
{
    status_t result = ALL_OK;

    if (current_task) {
        panic("A task is already running!");
    }

    if (!head_task) {
        panic("There is no task to run!");
    }

    result = switch_task(head_task);
    if (result != ALL_OK) {
        return result;
    }
    return_task(&head_task->registers);

    return result;
}
