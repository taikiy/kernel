#ifndef FORMAT_BINARY_H
#define FORMAT_BINARY_H

#include "status.h"
#include <stddef.h>

status_t load_binary_executable_file(int fd, void** out_ptr, size_t* out_size);

#endif
