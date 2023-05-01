#ifndef FILE_H
#define FILE_H

#include "path_parser.h"
#include "disk/disk.h"
#include "status.h"

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
    FILE_MODE_READ = 1,
    FILE_MODE_WRITE = 2,
    FILE_MODE_APPEND = 4,
};

// forward declaration
struct disk;
typedef status_t (*FS_RESOLVE_FUNCTION)(struct disk *disk);
typedef void *(*FS_OPEN_FUNCTION)(struct disk *disk, struct path_part *path, FILE_MODE mode);

struct file_system
{
    char name[16];
    FS_OPEN_FUNCTION open;
    FS_RESOLVE_FUNCTION resolve;
};

struct file_descriptor
{
    int index;
    struct disk *disk;
    void *data; // file's content
};

void file_system_initialize();
void fs_insert_file_system(struct file_system *fs);
struct file_system *fs_resolve(struct disk *disk);
int fopen(const char *file_name, const char *mode);

#endif
