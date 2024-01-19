#include "process.h"
#include "../config.h"
#include "../fs/file.h"
#include "../loader/loader.h"
#include "../memory/heap/kheap.h"
#include "../memory/memory.h"
#include "../memory/paging/paging.h"
#include "../terminal/terminal.h"
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

    if (process->program) {
        struct program* program = process->program;

        if (program->program_sections) {
            for (int i = 0; i < program->program_section_count; i++) {
                if (program->program_sections[i]) {
                    kfree(program->program_sections[i]);
                }
            }
            kfree(program->program_sections);
        }

        if (program->stack_section) {
            kfree(program->stack_section);
        }

        if (program->entry_point_address) {
            kfree(program->entry_point_address);
        }

        kfree(program);
    }

    if (process->task) {
        free_task(process->task);
    }

    for (int i = 0; i < MAX_ALLOCATIONS_PER_PROCESS; i++) {
        if (process->allocations[i]) {
            kfree(process->allocations[i]);
        }
    }

    kfree(process);

    return result;
}

status_t
map_program_memory_space(struct process* process)
{
    struct program* program = process->program;

    for (int i = 0; i < program->program_section_count; i++) {
        struct memory_layout* section = program->program_sections[i];

        status_t result = map_physical_address_to_pages(
          process->task->user_page,
          section->physical_address_start,
          section->virtual_address_start,
          section->size,
          section->flags
        );
        if (result != ALL_OK) {
            return result;
        }
    }

    return ALL_OK;
}

status_t
map_stack_memory_space(struct process* process)
{
    struct program* program = process->program;

    return map_physical_address_to_pages(
      process->task->user_page,
      program->stack_section->physical_address_start,
      program->stack_section->virtual_address_start,
      program->stack_section->size,
      program->stack_section->flags
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

static int
find_empty_slot()
{
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!processes[i]) {
            return i;
        }
    }
    return -1;
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

    int slot = find_empty_slot();
    if (slot < 0) {
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

void*
process_malloc(struct process* process, size_t size)
{
    void* ptr = kmalloc(size);
    if (!ptr) {
        return 0;
    }

    for (int i = 0; i < MAX_ALLOCATIONS_PER_PROCESS; i++) {
        if (!process->allocations[i]) {
            process->allocations[i] = ptr;
            return ptr;
        }
    }

    // no empty slot found
    return 0;
}

void
process_free(struct process* process, void* ptr)
{
    for (int i = 0; i < MAX_ALLOCATIONS_PER_PROCESS; i++) {
        if (process->allocations[i] == ptr) {
            kfree(ptr);
            process->allocations[i] = 0;
            return;
        }
    }
}
