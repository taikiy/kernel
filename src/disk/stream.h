#ifndef DISK_STREAM_H
#define DISK_STREAM_H

#include "disk.h"

struct disk_stream
{
    struct disk *disk;
    unsigned int sector;
    unsigned int offset;
};

struct disk_stream *disk_stream_new(int disk_number);
int disk_stream_seek(struct disk_stream *stream, unsigned int position);
int disk_stream_read(struct disk_stream *stream, unsigned int size, char *buf);
void disk_stream_close(struct disk_stream *stream);

#endif
