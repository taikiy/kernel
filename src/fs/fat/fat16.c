#include "fat16.h"
#include "status.h"
#include <stdint.h>
#include "disk/disk.h"
#include "disk/stream.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"

#define FAT16_SIGNATURE 0x29   // see boot.asm Extended BPB
#define FAT16_FAT_ENTRY_SIZE 2 // is this FATCopies?
#define FAT16_BAD_SECTOR 0xFFF7
#define FAT16_UNUSED_SECTOR 0x0000

typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_FILE 0
#define FAT_ITEM_TYPE_DIRECTORY 1

// FAT directory entry attributes
#define FAT16_ATTR_READ_ONLY 0x01
#define FAT16_ATTR_HIDDEN 0x02
#define FAT16_ATTR_SYSTEM 0x04
#define FAT16_ATTR_VOLUME_ID 0x08
#define FAT16_ATTR_DIRECTORY 0x10
#define FAT16_ATTR_ARCHIVE 0x20
#define FAT16_ATTR_DEVICE 0x40
#define FAT16_ATTR_RESERVED 0x80

struct fat_header
{
    uint8_t jump[3];
    uint8_t oem_id[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t total_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;
} __attribute__((packed));

struct fat_header_extended
{
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t system_id[8];
} __attribute__((packed));

struct fat_header_universal
{
    struct fat_header primary;
    union
    {
        struct fat_header_extended extended;
    };
} __attribute__((packed));

struct fat_directory_item
{
    uint8_t file_name[8];
    uint8_t file_extension[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_ms;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t last_modification_time;
    uint16_t last_modification_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed));

struct fat_directory
{
    struct fat_directory_item *items;
    uint32_t count;
    int sector_position;
    int last_sector_position;
};

struct fat_item
{
    FAT_ITEM_TYPE type;
    union
    {
        struct fat_directory directory;
        struct fat_directory_item *item;
    } shared;
};

struct fat_private_data
{
    struct fat_header_universal header;
    struct fat_directory root_directory;

    struct disk_stream *data_stream; // for data clusters
    struct disk_stream *fat_stream;  // for file allocation table
    struct disk_stream *dir_stream;  // for directory entries
};

// Represents a file descriptor for an item in the FAT file system
struct fat_item_descriptor
{
    struct fat_item *item;
    uint32_t position;
};

status_t fat16_resolve(struct disk *disk);
void *fat16_open(struct disk *disk, struct path_part *path, FILE_MODE mode);

struct file_system fat16_fs = {
    .name = "FAT16",
    .resolve = fat16_resolve,
    .open = fat16_open,
};

struct file_system *fat16_initialize()
{
    return &fat16_fs;
}

static void fat16_private_initialize(struct disk *disk, struct fat_private_data *private_data)
{
    memset(private_data, 0, sizeof(struct fat_private_data));

    private_data->data_stream = disk_stream_open(disk->id);
    private_data->fat_stream = disk_stream_open(disk->id);
    private_data->dir_stream = disk_stream_open(disk->id);

    disk->private_data = private_data;
    disk->fs = &fat16_fs;
}

int fat16_get_total_items_for_directory(struct disk_stream *stream, struct fat_private_data *private_data, uint32_t directory_sector_position)
{
    struct fat_directory_item item;

    int result = 0;
    int count = 0;

    result = disk_stream_seek(stream, directory_sector_position * private_data->header.primary.bytes_per_sector);
    if (result != ALL_OK)
    {
        goto out;
    }

    while (1)
    {
        result = disk_stream_read(stream, (char *)&item, sizeof(struct fat_directory_item));
        if (result != ALL_OK)
        {
            goto out;
        }

        // End of the items
        if (item.file_name[0] == 0)
        {
            goto out;
        }

        // 0xE5 means the item is free (available)
        if (item.file_name[0] == 0xE5)
        {
            continue;
        }

        count++;
    }

out:
    if (stream)
    {
        disk_stream_close(stream);
    }
    if (result == ALL_OK)
    {
        result = count;
    }
    return result;
}

status_t fat16_get_root_directory(struct disk_stream *stream, struct fat_private_data *private_data, struct fat_directory *directory)
{
    status_t result = ALL_OK;

    struct fat_header *primary = &private_data->header.primary;

    // calculate the root directory sector
    // Note that `root_dir_entries` is the max entries this root directory can hold, not the actual items in the root directory.
    uint32_t root_dir_size = primary->root_dir_entries * sizeof(struct fat_directory_item);
    uint32_t root_dir_sectors = (root_dir_size + (primary->bytes_per_sector - 1)) / primary->bytes_per_sector; // add `bytes_per_sector - 1` to round up
    uint32_t root_dir_sector_pos = (primary->fat_copies * primary->sectors_per_fat) + primary->reserved_sectors;

    directory->items = (struct fat_directory_item *)kzalloc(root_dir_size);
    if (directory->items == 0)
    {
        result = -ENOMEM;
        goto out;
    }

    // read the root directory
    result = disk_stream_seek(stream, root_dir_sector_pos * primary->bytes_per_sector);
    if (result != ALL_OK)
    {
        goto out;
    }

    result = disk_stream_read(stream, (char *)directory->items, root_dir_size);
    if (result != ALL_OK)
    {
        goto out;
    }

    directory->count = fat16_get_total_items_for_directory(stream, private_data, root_dir_sector_pos);
    directory->sector_position = root_dir_sector_pos;
    directory->last_sector_position = root_dir_sector_pos + root_dir_sectors;

out:
    if (result != ALL_OK)
    {
        if (directory->items)
        {
            kfree(directory->items);
        }
    }
    return result;
}

status_t fat16_resolve(struct disk *disk)
{
    status_t result = ALL_OK;
    struct disk_stream *stream = 0;

    struct fat_private_data *private_data = (struct fat_private_data *)kzalloc(sizeof(struct fat_private_data));
    fat16_private_initialize(disk, private_data);

    if (private_data->data_stream == 0 || private_data->fat_stream == 0 || private_data->dir_stream == 0)
    {
        result = -ENOMEM;
        goto out;
    }

    // We'll leave the streams in `private_data` untouched, and use new streams for reading the header and root directory.
    stream = disk_stream_open(disk->id);

    result = disk_stream_read(stream, (char *)&private_data->header, sizeof(struct fat_header_universal));
    if (result != ALL_OK)
    {
        goto out;
    }

    // check if this is a FAT16 file system
    if (private_data->header.extended.signature != 0x29)
    {
        result = -EFSNOTSUPPORTED;
        goto out;
    }

    result = fat16_get_root_directory(stream, private_data, &private_data->root_directory);
    if (result != ALL_OK)
    {
        goto out;
    }

out:
    if (result != ALL_OK)
    {
        if (private_data->data_stream)
        {
            disk_stream_close(private_data->data_stream);
        }
        if (private_data->fat_stream)
        {
            disk_stream_close(private_data->fat_stream);
        }
        if (private_data->dir_stream)
        {
            disk_stream_close(private_data->dir_stream);
        }
        if (private_data)
        {
            kfree(private_data);
        }
        disk->private_data = 0;
    }
    if (stream)
    {
        disk_stream_close(stream);
    }
    return result;
}

void *fat16_open(struct disk *disk, struct path_part *path, FILE_MODE mode)
{
    return 0;
}
