#ifndef DISK_H
#define DISK_H

#include "fs/file.h"
#include "status.h"

#define DISK_TYPE_REAL 0;

typedef unsigned int DISK_TYPE;

struct disk
{
    DISK_TYPE type;
    unsigned int id;
    unsigned int sector_size;
    struct file_system* fs;
    void* private_data; // file system specific data i.e., fs::fat16::fat_private_data
};

void initialize_disks();
struct disk* get_disk(unsigned int disk_number);
status_t disk_read_block(struct disk* disk, unsigned int lba, unsigned int total, void* buf);

#endif
