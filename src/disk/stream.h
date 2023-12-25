#ifndef STREAM_H
#define STREAM_H

#include "disk.h"
#include "status.h"

struct disk_stream
{
    struct disk* disk;
    unsigned int sector;
    unsigned int offset;
};

struct disk_stream* disk_stream_open(int disk_number);
status_t disk_stream_seek(struct disk_stream* stream, unsigned int position);
status_t disk_stream_read(struct disk_stream* stream, char* buf, unsigned int size);
void disk_stream_close(struct disk_stream* stream);

#endif
