#include "bin_loader.h"
#include "fs/file.h"
#include "memory/heap/kheap.h"
#include <stdint.h>

status_t
load_binary_executable_file(int fd, void** out_ptr, size_t* out_size)
{
    status_t result = ALL_OK;

    if (fd <= 0) {
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

    *out_ptr = program_data_ptr;
    *out_size = file_size;

out:
    if (result != ALL_OK) {
        if (program_data_ptr) {
            kfree(program_data_ptr);
        }
    }
    return result;
}
