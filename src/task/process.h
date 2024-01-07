#ifndef PROCESS_H
#define PROCESS_H

#include "config.h"
#include "keyboard/keyboard.h"
#include "status.h"
#include <stddef.h>
#include <stdint.h>

struct process_memory_map
{
    void* program_physical_address_start;
    void* program_virtual_address_start;
    size_t program_size;

    void* stack_physical_address_start;
    void* stack_virtual_address_start;
    size_t stack_size;
};

struct process
{
    // Process ID
    uint16_t id;

    // Executable file name
    char file_path[MAX_PATH_LENGTH];

    uint8_t file_type;

    // The main process task
    // It's possible for a process to have multiple tasks (threads). For now, we only have 1.
    struct task* task;

    // The main process heap allocations. This is used to free the heap when the process exits.
    // TODO: This should be a pointer and malloced in the heap when we create a process.
    void* allocations[MAX_ALLOCATIONS_PER_PROCESS];

    // Virtual-to-physical memory mappings for program and stack memory
    struct process_memory_map mem_map;

    struct keyboard_buffer keyboard_buffer;
};

status_t create_process(const char* file_path, struct process** process);
status_t create_process_and_switch(const char* file_path, struct process** process);
struct process* get_current_process();
status_t switch_process(struct process* process);

#endif
