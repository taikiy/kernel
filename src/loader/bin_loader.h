#ifndef BIN_LOADER_H
#define BIN_LOADER_H

#include "../status.h"
#include "../task/process.h"
#include <stddef.h>

status_t load_binary_executable_file(void* file_ptr, size_t file_size, struct program* out_program);

#endif
