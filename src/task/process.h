#ifndef PROCESS_H
#define PROCESS_H

#include "config.h"
#include <stdint.h>

struct process
{
    // Process ID
    uint16_t id;

    // Executable file name
    char file_path[MAX_PATH_LENGTH];

    // The main process task
    // It's possible for a process to have multiple tasks (threads). For now, we only have 1.
    struct task* task;

    // The main process heap allocations. This is used to free the heap when the process exits.
    // TODO: This should be a pointer and malloced in the heap when we create a process.
    void* allocations[MAX_ALLOCATIONS_PER_PROCESS];

    // The pointer to the process data loaded onto memory.
    // A process data could be of a binary file, ELF file, etc. For now, we assume that this is a
    // binary file. (no header, no sections, etc.)
    void* data;

    // The pointer to the stack memory
    void* stack;

    // The size of the process memory pointed by `ptr`
    uint32_t size;
};

#endif
