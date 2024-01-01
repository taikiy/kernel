#include "process.h"

#include "config.h"
#include "fs/file.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "string/string.h"
#include "task.h"

struct process* current_process = 0;
static struct process* processes[MAX_PROCESSES] = {};

static void
initialize_process(struct process* process)
{
    memset(process, 0, sizeof(struct process));
}

static status_t
load_binary_data(int fd, struct process* process)
{
    status_t result = ALL_OK;

    if (fd <= 0) {
        return ERROR(EINVARG);
    }

    if (!process) {
        return ERROR(EINVARG);
    }

    struct file_stat stat;
    result = fstat(fd, &stat);
    if (result != ALL_OK) {
        return ERROR(EIO);
    }

    uint32_t file_size = stat.size;
    void* program_data_ptr = kzalloc(file_size);
    if (!program_data_ptr) {
        return ERROR(ENOMEM);
    }

    // TODO: Read the file in chunks
    size_t read_items = fread(program_data_ptr, file_size, 1, fd);
    if (read_items != 1) {
        result = ERROR(EIO);
        goto out;
    }

    process->data = program_data_ptr;
    process->size = file_size;

out:
    if (result != ALL_OK) {
        if (program_data_ptr) {
            kfree(program_data_ptr);
        }
    }
    return result;
}

static status_t
load_data_for_process(const char* file_path, struct process* process)
{
    status_t result = ALL_OK;

    if (!file_path) {
        return ERROR(EINVARG);
    }

    if (!process) {
        return ERROR(EINVARG);
    }

    int fd = fopen(file_path, "r");
    if (!fd) {
        return ERROR(EIO);
    }

    // TODO: Check if the file is executable, binary, ELF, etc.

    result = load_binary_data(fd, process);
    if (result != ALL_OK) {
        goto out;
    }

    strncpy(process->file_path, file_path, sizeof(process->file_path) - 1);

out:
    // We can safely call fclose() even if fd is 0.
    fclose(fd);
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

    if (process->stack) {
        kfree(process->stack);
    }

    if (process->data) {
        kfree(process->data);
    }

    if (process->task) {
        free_task(process->task);
    }

    kfree(process);

    return result;
}

status_t
map_binary_data(struct process* process)
{
    return map_physical_address_to_pages(
      process->task->user_page,
      process->data,
      (void*)USER_PROGRAM_VIRTUAL_ADDRESS_START,
      process->size,
      PAGING_IS_PRESENT | PAGING_IS_WRITABLE | PAGING_ACCESS_FROM_ALL
    );
}

status_t
map_stack(struct process* process)
{
    return map_physical_address_to_pages(
      process->task->user_page,
      process->stack,
      (void*)USER_PROGRAM_STACK_VIRTUAL_ADDRESS_END, // END because the stack grows downwards
      USER_PROGRAM_STACK_SIZE,
      PAGING_IS_PRESENT | PAGING_IS_WRITABLE | PAGING_ACCESS_FROM_ALL
    );
}

status_t
map_process_memory(struct process* process)
{
    status_t result = ALL_OK;

    result = map_stack(process);
    if (result != ALL_OK) {
        goto out;
    }

    // TODO: implement for different file types
    result = map_binary_data(process);
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

    struct process* new_process = (struct process*)kzalloc(sizeof(struct process));
    if (!new_process) {
        return ERROR(ENOMEM);
    }

    initialize_process(new_process);

    // load the executable data from the file
    result = load_data_for_process(file_path, new_process);
    if (result != ALL_OK) {
        goto out;
    }

    // allocate the stack memory
    void* program_stack_ptr = kzalloc(USER_PROGRAM_STACK_SIZE);
    if (!program_stack_ptr) {
        result = ERROR(ENOMEM);
        goto out;
    }
    new_process->stack = program_stack_ptr;

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
