#include "task.h"
#include "config.h"
#include "idt/idt.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "status.h"
#include "system/sys.h"

struct task* current_task = 0;
struct task* head_task = 0;
struct task* tail_task = 0;

extern void return_to_user_space();

static status_t
initialize_task(struct task* task, struct process* process)
{
    status_t result = ALL_OK;

    memset(task, 0, sizeof(struct task));
    // Map the entire 4GB address space to the task
    // We set `PAGING_ACCESS_FROM_ALL` flag to avoid any complications. In reality, we shouldn't set
    // this flag.
    task->user_page = new_paging_map(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (!task->user_page) {
        return ERROR(EIO);
    }

    task->registers.ip = USER_PROGRAM_VIRTUAL_ADDRESS_START;
    task->registers.esp = USER_PROGRAM_STACK_VIRTUAL_ADDRESS_START;
    task->registers.ss = USER_PROGRAM_DATA_SELECTOR;
    task->registers.cs = USER_PROGRAM_CODE_SELECTOR;

    task->process = process;

    return result;
}

/// @brief  Returns the next task in the queue.
/// @return The next task in the queue. This function returns 0 if there is no task in the queue.
struct task*
get_next_task()
{
    if (!current_task) {
        return 0;
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

/// @brief Saves the current task registers. This function must be called while the kernel space paging is active.
/// @param frame The interrupt frame that contains the current task registers
void
save_current_task_state(struct interrupt_frame* frame)
{
    if (!current_task) {
        panic("Cannot save the state of a null task!");
    }

    if (!frame) {
        panic("Cannot save the state of a task with a null frame!");
    }

    current_task->registers.ip = frame->ip;
    current_task->registers.cs = frame->cs;
    current_task->registers.flags = frame->flags;
    current_task->registers.esp = frame->esp;
    current_task->registers.ss = frame->ss;

    current_task->registers.eax = frame->eax;
    current_task->registers.ecx = frame->ecx;
    current_task->registers.edx = frame->edx;
    current_task->registers.ebp = frame->ebp;
    current_task->registers.esi = frame->esi;
    current_task->registers.edi = frame->edi;
    current_task->registers.ebx = frame->ebx;
}

void
start_tasks()
{
    if (current_task) {
        panic("A task is already running!");
    }

    if (!head_task) {
        panic("There is no task to run!");
    }

    current_task = head_task;
    switch_to_user_page();
    return_to_user_space(&current_task->registers);
}
