#include "loader.h"
#include "bin_loader.h"
#include "fs/file.h"
#include "memory/heap/kheap.h"
#include <stdint.h>

status_t
load_file(const char* file_path, void** out_ptr, size_t* out_size)
{
    status_t result = ALL_OK;

    if (!file_path) {
        return ERROR(EINVARG);
    }

    int fd = fopen(file_path, "r");
    if (!fd) {
        return ERROR(EIO);
    }

    // TODO: Check if the file is binary, ELF, etc.

    result = load_binary_executable_file(fd, out_ptr, out_size);
    if (result != ALL_OK) {
        goto out;
    }

out:
    // We can safely call fclose() even if fd is 0.
    fclose(fd);
    return result;
}
