#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include "config.h"
#include "format/elf.h"
#include "status.h"
#include "task/process.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    char name[MAX_PATH_LENGTH];

    /// @brief The size of the file in bytes.
    size_t size;
    /// @brief The physical address of the file in memory.
    void* address;

    void* program_virtual_address_start;
    void* program_virtual_address_stop;
    void* program_physical_address_start;
    void* program_physical_address_stop;
} elf_file;

bool is_elf_file(void* file_ptr);
status_t load_elf_executable_file(void* file_ptr, size_t file_size, struct program* out_program);

#endif
