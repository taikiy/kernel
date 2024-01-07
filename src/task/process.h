#ifndef PROCESS_H
#define PROCESS_H

#include "config.h"
#include "keyboard/keyboard.h"
#include "status.h"
#include <stddef.h>
#include <stdint.h>

struct program
{
    char file_path[MAX_PATH_LENGTH];

    uint8_t file_type;

    void* text_physical_address_start;
    void* text_virtual_address_start;
    size_t text_size;

    void* stack_physical_address_start;
    void* stack_virtual_address_start;
    size_t stack_size;
};

struct process
{
    // Process ID
    uint16_t id;

    // The main process task
    // It's possible for a process to have multiple tasks (threads). For now, we only have 1.
    struct task* task;

    // The main process heap allocations. This is used to free the heap when the process exits.
    // TODO: This should be a pointer and malloced in the heap when we create a process.
    void* allocations[MAX_ALLOCATIONS_PER_PROCESS];

    // The program file that this process is running.
    struct program* program;

    struct keyboard_buffer keyboard_buffer;
};

status_t create_process(const char* file_path, struct process** process);
status_t create_process_and_switch(const char* file_path, struct process** process);
struct process* get_current_process();
status_t switch_process(struct process* process);

#endif
