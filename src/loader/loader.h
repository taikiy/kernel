#ifndef LOADER_H
#define LOADER_H

#include "status.h"
#include <stddef.h>

status_t load_file(const char* file_path, void** out_ptr, size_t* out_size);

#endif
