#include "stream.h"
#include "config.h"
#include "memory/heap/kheap.h"

struct disk_stream*
disk_stream_open(int disk_number)
{
    struct disk* disk = get_disk(disk_number);
    if (!disk) {
        return 0;
    }

    struct disk_stream* stream = kzalloc(sizeof(struct disk_stream));
    stream->disk = disk;
    stream->sector = 0;
    stream->offset = 0;
    return stream;
}

status_t
disk_stream_seek(struct disk_stream* stream, unsigned int position)
{
    stream->sector = position / DISK_SECTOR_SIZE_BYTES;
    stream->offset = position % DISK_SECTOR_SIZE_BYTES;
    return ALL_OK;
}

status_t
disk_stream_read(struct disk_stream* stream, char* buf, unsigned int size)
{
    char block[DISK_SECTOR_SIZE_BYTES];
    status_t result = ERROR(EINVARG);

    for (int i = 0; i < size; i++) {
        if (stream->offset == 0 || i == 0) {
            result = disk_read_block(stream->disk, stream->sector, 1, block);
            if (result != ALL_OK) {
                goto out;
            }
        }

        buf[i] = block[stream->offset++];

        if (stream->offset == DISK_SECTOR_SIZE_BYTES) {
            stream->sector++;
            stream->offset = 0;
        }
    }

out:
    return result;
}

void
disk_stream_close(struct disk_stream* stream)
{
    kfree(stream);
}
