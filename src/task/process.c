#include "process.h"
#include "../config.h"
#include "../fs/file.h"
#include "../loader/loader.h"
#include "../memory/heap/kheap.h"
#include "../memory/memory.h"
#include "../memory/paging/paging.h"
#include "../string/string.h"
#include "../system/sys.h"
#include "task.h"

extern uint32_t inject_process_arguments_to_stack(struct registers* registers, int argc, char* argv[]);

// The current process is the one that is currently running in the foreground. That means, it's the process that the
// user is interacting with. In CUI, it's the one currently active in the terminal. In GUI, it's the window that is
// currently above all others.
static struct process* current_process = 0;
static struct process* processes[MAX_PROCESSES] = {};

static struct process* root_process = 0;

static void
initialize_process(struct process* process)
{
    memset(process, 0, sizeof(struct process));
}

static status_t
load_program_for_process(struct command_args* command, struct program** program)
{
    status_t result = ALL_OK;

    struct program* new_program = kzalloc(sizeof(struct program));
    if (!new_program) {
        return ERROR(ENOMEM);
    }

    new_program->file_type = PROGRAM_FILE_TYPE_UNKNOWN;
    const char* file_path = command->value;
    result = load_file(file_path, new_program);
    if (result != ALL_OK) {
        goto out;
    }

    new_program->command = command;

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

static status_t
free_process(struct process* process)
{
    status_t result = ALL_OK;

    if (!process) {
        return ERROR(EINVARG);
    }

    // before anything, free the task so that the scheduler doesn't pick it up
    if (process->task) {
        free_task(process->task);
    }

    if (process->program) {
        struct program* program = process->program;

        if (program->program_sections) {
            for (int i = 0; i < program->program_section_count; i++) {
                if (program->program_sections[i]) {
                    kfree(program->program_sections[i]->physical_address_start);
                    kfree(program->program_sections[i]);
                }
            }
            kfree(program->program_sections);
        }

        if (program->stack_section) {
            kfree(program->stack_section->physical_address_start);
            kfree(program->stack_section);
        }

        if (program->entry_point_address) {
            kfree(program->entry_point_address);
        }

        if (program->command) {
            struct command_args* command = program->command;
            while (command) {
                struct command_args* next = command->next;
                kfree(command);
                command = next;
            }
        }

        kfree(program);
    }

    for (int i = 0; i < MAX_ALLOCATIONS_PER_PROCESS; i++) {
        if (process->allocations[i]) {
            struct allocation* mem = process->allocations[i];
            kfree(mem->ptr);
            kfree(mem);
        }
    }

    processes[process->id] = 0;
    kfree(process);

    // We don't change the current process here. The caller needs to do that.

    return result;
}

status_t
map_program_memory_space(struct process* process)
{
    struct program* program = process->program;

    for (int i = 0; i < program->program_section_count; i++) {
        struct memory_layout* section = program->program_sections[i];

        status_t result = map_paging_addresses(
          process->task->user_page,
          section->virtual_address_start,
          section->physical_address_start,
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

    return map_paging_addresses(
      process->task->user_page,
      program->stack_section->virtual_address_start,
      program->stack_section->physical_address_start,
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
load_process(struct command_args* command, struct process** process, int slot)
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
    result = load_program_for_process(command, &new_process->program);
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

static int
find_empty_process_slot()
{
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!processes[i]) {
            return i;
        }
    }
    return -1;
}

static int
count_args(struct command_args* args)
{
    int count = 0;
    struct command_args* command = args;
    while (command) {
        count++;
        command = command->next;
    }
    return count;
}

static status_t
set_command_line_arguments(struct process* process)
{
    if (!process) {
        return ERROR(EINVARG);
    }

    // get all the command arguments and join them into an array
    struct command_args* next = process->program->command->next; // skip the program path
    int argc = count_args(next);
    if (argc > MAX_COMMAND_ARGS) {
        return ERROR(ETOOMANYARGS);
    }
    char** argv = 0;
    if (argc > 0) {
        argv = process_malloc(process, argc * sizeof(char*));
        if (!argv) {
            panic("process_malloc failed");
        }
        for (int i = 0; i < argc; i++) {
            struct command_args* current = next;
            char* value = process_malloc(process, strlen(current->value) + 1);
            if (!value) {
                panic("process_malloc failed");
            }
            strcpy(value, current->value);
            argv[i] = value;
            next = current->next;
        }
    }

    switch_to_user_page(process->task);
    uint32_t new_stack_pointer = inject_process_arguments_to_stack(&process->task->registers, argc, argv);
    switch_to_kernel_page();
    process->task->registers.esp = new_stack_pointer;

    return ALL_OK;
}

status_t
create_process(struct command_args* command, struct process** process)
{
    status_t status = ALL_OK;

    if (!command || !command->value) {
        return ERROR(EINVARG);
    }

    if (!process) {
        return ERROR(EINVARG);
    }

    int slot = find_empty_process_slot();
    if (slot < 0) {
        return ERROR(ETOOMANYPROCESSES);
    }

    status = load_process(command, process, slot);
    if (status != ALL_OK) {
        return status;
    }

    current_process = *process;
    // TODO: We shouldn't call this here. We should let the scheduler do it.
    if (!root_process) {
        root_process = current_process;
    }

    set_command_line_arguments(current_process);

    // TODO: We shouldn't call this here. We should let the scheduler do it.
    switch_task();

    return status;
}

void
terminate_process(struct process* process, int status)
{
    if (!process) {
        return;
    }

    free_process(process);

    // TODO: need a better design
    current_process = root_process;
    switch_task();
}

int
find_empty_malloc_slot(struct process* process)
{
    for (int i = 0; i < MAX_ALLOCATIONS_PER_PROCESS; i++) {
        if (!process->allocations[i]) {
            return i;
        }
    }
    return -1;
}

void*
process_malloc(struct process* process, size_t size)
{
    status_t result = ALL_OK;

    void* ptr = kmalloc(size);
    if (!ptr) {
        return 0;
    }

    int slot = find_empty_malloc_slot(process);
    if (slot < 0) {
        result = ERROR(ETOOMANYPROCMALLOCS);
        goto out;
    }

    struct allocation* allocation = kmalloc(sizeof(struct allocation));
    if (!allocation) {
        result = ERROR(ENOMEM);
        goto out;
    }
    allocation->ptr = ptr;
    allocation->size = size;
    process->allocations[slot] = allocation;

    // map the physical address to the process' virtual address space
    // TODO: if the system has more than one task per process, we need to loop through all tasks and map the address to
    // each task's virtual address space.
    result = map_paging_addresses(
      process->task->user_page, ptr, ptr, size, PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITABLE
    );

out:
    if (result != ALL_OK) {
        if (ptr) {
            kfree(ptr);
        }
        if (allocation) {
            kfree(allocation);
        }
        ptr = 0;
    }
    return ptr;
}

void
process_free(struct process* process, void* ptr)
{
    struct allocation* mem = 0;
    for (int i = 0; i < MAX_ALLOCATIONS_PER_PROCESS; i++) {
        if (process->allocations[i] && process->allocations[i]->ptr == ptr) {
            mem = process->allocations[i];
            process->allocations[i] = 0;
            break;
        }
    }

    if (!mem) {
        // `ptr` doesn't belong to this process
        return;
    }

    // Unlink the virtual address from the process' paging table. If we don't do this, any process that has access to
    // the same virtual address can access the physical address. This is a security issue. Unlinking is done by setting
    // the table_entry's flag to 0.
    // TODO: if the system has more than one task per process, we need to loop through all tasks and unmap the address
    // to each task's virtual address space.
    status_t result = map_paging_addresses(process->task->user_page, ptr, ptr, mem->size, 0);
    if (result != ALL_OK) {
        panic("Failed to unmap virtual address!");
    }

    kfree(mem->ptr);
    kfree(mem);
    return;
}
