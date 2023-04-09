#include "stream.h"
#include "memory/heap/kheap.h"
#include "config.h"

struct disk_stream *disk_stream_new(int disk_number)
{
    struct disk *disk = get_disk(disk_number);
    if (!disk)
    {
        return 0;
    }

    struct disk_stream *stream = kzalloc(sizeof(struct disk_stream));
    stream->disk = disk;
    stream->sector = 0;
    stream->offset = 0;
    return stream;
}

int disk_stream_seek(struct disk_stream *stream, unsigned int position)
{
    stream->sector = position / DISK_SECTOR_SIZE_BYTES;
    stream->offset = position % DISK_SECTOR_SIZE_BYTES;
    return 0;
}

int disk_stream_read(struct disk_stream *stream, unsigned int size, char *buf)
{
    char block[DISK_SECTOR_SIZE_BYTES];
    status_t res = -EINVARG;

    for (int i = 0; i < size; i++)
    {
        if (stream->offset == 0 || i == 0)
        {
            res = disk_read_block(stream->disk, stream->sector, 1, block);
            if (res < 0)
            {
                goto out;
            }
        }

        buf[i] = block[stream->offset++];

        if (stream->offset == DISK_SECTOR_SIZE_BYTES)
        {
            stream->sector++;
            stream->offset = 0;
        }
    }

out:
    return res;
}

void disk_stream_close(struct disk_stream *stream)
{
    kfree(stream);
}
