#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include "status.h"
#include <stddef.h>

status_t load_elf_executable_file(int fd, void** out_ptr, size_t* out_size);

#endif
