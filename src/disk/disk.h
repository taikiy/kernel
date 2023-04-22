#ifndef DISK_H
#define DISK_H

#include "status.h"
#include "fs/file.h"

#define DISK_TYPE_REAL 0;

typedef unsigned int DISK_TYPE;

struct disk
{
    DISK_TYPE type;
    unsigned int id;
    unsigned int sector_size;
    struct file_system *fs;
};

void disk_search_and_initialize();
struct disk *get_disk(unsigned int disk_number);
status_t disk_read_block(struct disk *disk, unsigned int lba, unsigned int total, void *buf);

#endif
