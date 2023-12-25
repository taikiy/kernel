#ifndef FILE_H
#define FILE_H

#include "disk/disk.h"
#include "path_parser.h"
#include "status.h"
#include <stdint.h>

typedef unsigned int FILE_SEEK_MODE;
enum FILE_SEEK_MODES
{
    FILE_SEEK_SET,
    FILE_SEEK_CUR,
    FILE_SEEK_END
};

typedef unsigned int FILE_MODE;
enum FILE_MODES
{
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID,
};

typedef unsigned int FILE_STAT_FLAGS;
enum
{
    FILE_STAT_FLAG_HIDDEN = 1,
    FILE_STAT_FLAG_SYSTEM = 2,
    FILE_STAT_FLAG_READONLY = 4,
    FILE_STAT_FLAG_DIRECTORY = 8,
    FILE_STAT_FLAG_ARCHIVE = 16,
};

// forward declaration
struct disk;
struct file_stat;
typedef status_t (*FS_RESOLVE_FUNCTION)(struct disk* disk);
typedef void* (*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
typedef status_t (*FS_READ_FUNCTION)(struct disk* disk, void* private_data, uint32_t size, uint32_t count, char* out);
typedef status_t (*FS_SEEK_FUNCTION)(void* private_data, uint32_t offset, FILE_SEEK_MODE mode);
typedef status_t (*FS_STAT_FUNCTION)(void* private_data, struct file_stat* stat);
typedef status_t (*FS_CLOSE_FUNCTION)(void* private_data);

struct file_system
{
    char name[16];
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
    FS_READ_FUNCTION read;
    FS_SEEK_FUNCTION seek;
    FS_STAT_FUNCTION stat;
    FS_CLOSE_FUNCTION close;
};

struct file_descriptor
{
    int index;
    struct disk* disk;
    void* data; // file's content
};

struct file_stat
{
    uint32_t size;         /* file size, in bytes */
    FILE_STAT_FLAGS flags; /* user defined flags for file */
};

void initialize_file_systems();
void insert_file_system(struct file_system* fs);
struct file_system* fs_resolve(struct disk* disk);
int fopen(const char* file_name, const char* mode);
status_t fread(void* ptr, uint32_t size, uint32_t count, int fd);
status_t fseek(int fd, uint32_t offset, FILE_SEEK_MODE mode);
status_t fstat(int fd, struct file_stat* stat);
status_t fclose(int fd);

#endif
