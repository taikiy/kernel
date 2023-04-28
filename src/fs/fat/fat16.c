#include "fat16.h"
#include "status.h"

status_t fat16_resolve(struct disk *disk);
void *fat16_open(struct disk *disk, struct path_part *path, FILE_MODE mode);

struct file_system fat16_fs = {
    .name = "FAT16",
    .resolve = fat16_resolve,
    .open = fat16_open,
};

struct file_system *fat16_initialize()
{
    // strcpy(fat16_fs.name, "FAT16");
    return &fat16_fs;
}

status_t fat16_resolve(struct disk *disk)
{
    return -EIO;
}

void *fat16_open(struct disk *disk, struct path_part *path, FILE_MODE mode)
{
    return 0;
}
