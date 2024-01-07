#include "process.h"

#include "config.h"
#include "fs/file.h"
#include "loader/loader.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "task.h"

static struct process* current_process = 0;
static struct process* processes[MAX_PROCESSES] = {};

static void
initialize_process(struct process* process)
{
    memset(process, 0, sizeof(struct process));
}

static status_t
load_program_for_process(const char* file_path, struct program** program)
{
    status_t result = ALL_OK;

    struct program* new_program = kzalloc(sizeof(struct program));
    if (!new_program) {
        return ERROR(ENOMEM);
    }

    new_program->file_type = PROGRAM_FILE_TYPE_UNKNOWN;
    result = load_file(file_path, new_program);
    if (result != ALL_OK) {
        goto out;
    }

    *program = new_program;

out:
    if (result != ALL_OK) {
        if (new_program) {
            kfree(new_program);
        }
    }
    return result;
}

struct process*
get_current_process()
{
    return current_process;
}

struct process*
get_process(uint16_t process_id)
{
    if (process_id < 0 || process_id >= MAX_PROCESSES) {
        return 0;
    }

    return processes[process_id];
}

status_t
free_process(struct process* process)
{
    status_t result = ALL_OK;

    if (!process) {
        return ERROR(EINVARG);
    }

    if (process->program->stack_physical_address_start) {
        kfree(process->program->stack_physical_address_start);
    }

    if (process->program->text_physical_address_start) {
        kfree(process->program->text_physical_address_start);
    }

    if (process->task) {
        free_task(process->task);
    }

    kfree(process);

    return result;
}

status_t
map_program_memory_space(struct process* process)
{
    return map_physical_address_to_pages(
      process->task->user_page,
      process->program->text_physical_address_start,
      process->program->text_virtual_address_start,
      process->program->text_size,
      PAGING_IS_PRESENT | PAGING_IS_WRITABLE | PAGING_ACCESS_FROM_ALL
    );
}

status_t
map_stack_memory_space(struct process* process)
{
    return map_physical_address_to_pages(
      process->task->user_page,
      process->program->stack_physical_address_start,
      process->program->stack_virtual_address_start,
      process->program->stack_size,
      PAGING_IS_PRESENT | PAGING_IS_WRITABLE | PAGING_ACCESS_FROM_ALL
    );
}

status_t
map_process_memory(struct process* process)
{
    status_t result = ALL_OK;

    result = map_stack_memory_space(process);
    if (result != ALL_OK) {
        goto out;
    }

    result = map_program_memory_space(process);
    if (result != ALL_OK) {
        goto out;
    }

out:
    return result;
}

/// @brief Loads data from `file_path`, creates a new process and saves it at `slot` of `processes`.
///        The newly created process' memory address will be stored in `*process`.
/// @param file_path The name of the file we want to load.
/// @param process An address of a pointer that holds the address of the process memory.
/// @param slot Process ID we want to assign to the process.
/// @return ALL_OK if the process is loaded successfully.
static status_t
load_process_to_slot(const char* file_path, struct process** process, int slot)
{
    status_t result = ALL_OK;

    // the slot is already taken by another process
    if (get_process(slot)) {
        return ERROR(EINVARG);
    }

    struct process* new_process = kzalloc(sizeof(struct process));
    if (!new_process) {
        return ERROR(ENOMEM);
    }

    initialize_process(new_process);

    // load the executable file and allocate data/stack memories
    result = load_program_for_process(file_path, &new_process->program);
    if (result != ALL_OK) {
        goto out;
    }

    // create the initial task
    struct task* new_task = create_task(new_process);
    if (!new_task) {
        result = ERROR(ENOMEM);
        goto out;
    }
    new_process->task = new_task;

    // map the data/stack memory to the process' virtual address space
    map_process_memory(new_process);

    // assign the process ID
    new_process->id = slot;
    processes[slot] = new_process;
    *process = new_process;

out:
    if (result != ALL_OK) {
        if (new_process) {
            free_process(new_process);
        }
    }
    return result;
}

status_t
switch_process(struct process* process)
{
    if (!process) {
        return ERROR(EINVARG);
    }

    current_process = process;
    return switch_task(process->task);
}

status_t
create_process(const char* file_path, struct process** process)
{
    if (!file_path) {
        return ERROR(EINVARG);
    }

    if (!process) {
        return ERROR(EINVARG);
    }

    int slot = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!processes[i]) {
            slot = i;
            break;
        }
        return ERROR(ETOOMANYPROCESSES);
    }

    return load_process_to_slot(file_path, process, slot);
}

status_t
create_process_and_switch(const char* file_path, struct process** process)
{
    status_t result = ALL_OK;

    result = create_process(file_path, process);
    if (result != ALL_OK) {
        return result;
    }

    return switch_process(*process);
}
