#ifndef PROCESS_H
#define PROCESS_H

#include "../config.h"
#include "../keyboard/keyboard.h"
#include "../status.h"
#include <stddef.h>
#include <stdint.h>

struct memory_layout
{
    void* physical_address_start;
    void* virtual_address_start;
    size_t size;
    uint32_t flags;
};

struct command_args
{
    char* value;
    struct command_args* next;
};

struct program
{
    char file_path[MAX_PATH_LENGTH];
    uint8_t file_type;

    void* entry_point_address;

    uint32_t program_section_count;
    struct memory_layout** program_sections;

    struct memory_layout* stack_section;

    struct command_args* command;
};

struct allocation
{
    void* ptr;
    size_t size;
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
    struct allocation* allocations[MAX_ALLOCATIONS_PER_PROCESS];

    // The program file that this process is running.
    struct program* program;

    struct keyboard_buffer keyboard_buffer;
};

status_t create_process(struct command_args* command, struct process** process);
void terminate_process(struct process* process, int status);
struct process* get_current_process();
void* process_malloc(struct process* process, size_t size);
void process_free(struct process* process, void* ptr);

#endif
