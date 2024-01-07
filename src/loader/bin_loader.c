#include "bin_loader.h"
#include "config.h"
#include "fs/file.h"
#include "memory/heap/kheap.h"
#include <stdint.h>

status_t
load_binary_executable_file(void* file_ptr, size_t file_size, struct process_memory_map* out_mem_map)
{
    status_t result = ALL_OK;

    // The data base address of binary executable files starts from `&file_ptr[0]`
    out_mem_map->program_physical_address_start = file_ptr;
    out_mem_map->program_size = file_size;
    out_mem_map->program_virtual_address_start = (void*)USER_PROGRAM_VIRTUAL_ADDRESS_START; // default for plain binary

    // allocate the stack memory
    void* stack_ptr = kzalloc(USER_PROGRAM_STACK_SIZE);
    if (!stack_ptr) {
        result = ERROR(ENOMEM);
        goto out;
    }
    out_mem_map->stack_physical_address_start = stack_ptr;
    out_mem_map->stack_size = USER_PROGRAM_STACK_SIZE;                                        // default
    out_mem_map->stack_virtual_address_start = (void*)USER_PROGRAM_STACK_VIRTUAL_ADDRESS_END; // stack grows downwards

out:
    if (result != ALL_OK) {
        if (stack_ptr) {
            kfree(stack_ptr);
        }
    }
    return result;
}
