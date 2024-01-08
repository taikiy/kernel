#include "bin_loader.h"
#include "../config.h"
#include "../fs/file.h"
#include "../memory/heap/kheap.h"
#include "../memory/paging/paging.h"
#include <stdint.h>

status_t
load_program_section(void* file_ptr, size_t file_size, struct program* out_program)
{
    status_t result = ALL_OK;

    out_program->program_section_count = 1;
    out_program->program_sections = kzalloc(sizeof(struct memory_layout*) * out_program->program_section_count);
    if (!out_program->program_sections) {
        return ERROR(ENOMEM);
    }

    out_program->program_sections[0] = kzalloc(sizeof(struct memory_layout));
    if (!out_program->program_sections[0]) {
        result = ERROR(ENOMEM);
        goto out;
    }

    struct memory_layout* text_section = out_program->program_sections[0];

    // The text base address of binary executable files starts from `&file_ptr[0]`
    text_section->physical_address_start = file_ptr;
    text_section->size = file_size;
    text_section->virtual_address_start = (void*)USER_PROGRAM_VIRTUAL_ADDRESS_START; // default for plain binary
    // TODO: Do we need to set the writable flag for assembly files? Does .bss section need to be writable?
    text_section->flags = PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL;

out:
    if (result != ALL_OK) {
        if (out_program->program_sections[0]) {
            kfree(out_program->program_sections[0]);
        }
        if (out_program->program_sections) {
            kfree(out_program->program_sections);
        }
    }
    return result;
}

status_t
load_stack_section(struct program* out_program)
{
    status_t result = ALL_OK;

    // allocate the stack memory
    void* stack_ptr = kzalloc(USER_PROGRAM_STACK_SIZE);
    if (!stack_ptr) {
        result = ERROR(ENOMEM);
        goto out;
    }

    struct memory_layout* stack_section = kzalloc(sizeof(struct memory_layout));
    if (!stack_section) {
        result = ERROR(ENOMEM);
        goto out;
    }

    stack_section->physical_address_start = stack_ptr;
    stack_section->virtual_address_start = (void*)USER_PROGRAM_STACK_VIRTUAL_ADDRESS_END; // stack grows downwards
    stack_section->size = USER_PROGRAM_STACK_SIZE;                                        // default
    stack_section->flags = PAGING_IS_PRESENT | PAGING_IS_WRITABLE | PAGING_ACCESS_FROM_ALL;

    out_program->stack_section = stack_section;

out:
    if (result != ALL_OK) {
        if (stack_ptr) {
            kfree(stack_ptr);
        }
        if (stack_section) {
            kfree(stack_section);
        }
    }

    return result;
}

status_t
load_binary_executable_file(void* file_ptr, size_t file_size, struct program* out_program)
{
    status_t result = ALL_OK;

    result = load_program_section(file_ptr, file_size, out_program);
    if (result != ALL_OK) {
        return result;
    }

    result = load_stack_section(out_program);
    if (result != ALL_OK) {
        return result;
    }

    // The entry point of binary executable files is the beginning of the file.
    out_program->entry_point_address = (void*)USER_PROGRAM_VIRTUAL_ADDRESS_START; // default for plain binary

out:
    return result;
}
