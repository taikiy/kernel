#ifndef LOADER_H
#define LOADER_H

#include "status.h"
#include "task/process.h"
#include <stddef.h>

typedef uint8_t PROGRAM_FILE_TYPE;
enum PROGRAM_FILE_TYPE
{
    PROGRAM_FILE_TYPE_UNKNOWN,
    PROGRAM_FILE_TYPE_ELF,
    PROGRAM_FILE_TYPE_BIN,
};

status_t load_file(const char* file_path, struct program* out_program);

#endif
