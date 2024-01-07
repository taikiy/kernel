#include "loader.h"
#include "bin_loader.h"
#include "elf_loader.h"
#include "fs/file.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "terminal/terminal.h"
#include <stdint.h>

static status_t
load_file_in_memory(const char* file_path, void** out_file_ptr, size_t* out_file_size)
{
    status_t result = ALL_OK;

    if (!file_path) {
        return ERROR(EINVARG);
    }

    int fd = fopen(file_path, "r");
    if (!fd) {
        return ERROR(EIO);
    }

    struct file_stat stat;
    result = fstat(fd, &stat);
    if (result != ALL_OK) {
        return ERROR(EIO);
    }

    void* file_ptr = kzalloc(stat.size);
    if (!file_ptr) {
        return ERROR(ENOMEM);
    }

    size_t read_items = fread(file_ptr, stat.size, 1, fd);
    if (read_items != 1) {
        result = ERROR(EIO);
        goto out;
    }

    *out_file_ptr = file_ptr;
    *out_file_size = stat.size;

out:
    if (result != ALL_OK) {
        if (file_ptr) {
            kfree(file_ptr);
        }
    }
    // We can safely call fclose() even if fd is 0.
    fclose(fd);
    return result;
}

status_t
load_file(const char* file_path, struct program* out_program)
{
    status_t result = ALL_OK;

    // load the entire file in memory
    void* file_ptr = 0;
    size_t file_size = 0;
    result = load_file_in_memory(file_path, &file_ptr, &file_size);
    if (result != ALL_OK) {
        return result;
    }

    if (is_elf_file(file_ptr)) {
        out_program->file_type = PROGRAM_FILE_TYPE_ELF;
        result = load_elf_executable_file(file_ptr, file_size, out_program);
    } else {
        out_program->file_type = PROGRAM_FILE_TYPE_BIN;
        result = load_binary_executable_file(file_ptr, file_size, out_program);
    }
    if (result != ALL_OK) {
        return result;
    }

    strncpy(out_program->file_path, file_path, sizeof(out_program->file_path) - 1);

    return result;
}
