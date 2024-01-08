#include "fat16.h"
#include "../../config.h"
#include "../../disk/disk.h"
#include "../../disk/stream.h"
#include "../../memory/heap/kheap.h"
#include "../../memory/memory.h"
#include "../../status.h"
#include "../../string/string.h"
#include <stdint.h>

#define FAT16_SIGNATURE      0x29 // see boot.asm Extended BPB
#define FAT16_FAT_ENTRY_SIZE 2    // is this FATCopies?
#define FAT16_BAD_SECTOR     0xFFF7
#define FAT16_UNUSED_SECTOR  0x0000

typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_FILE      0
#define FAT_ITEM_TYPE_DIRECTORY 1

// FAT directory entry attributes
#define FAT16_ATTR_READ_ONLY    0x01
#define FAT16_ATTR_HIDDEN       0x02
#define FAT16_ATTR_SYSTEM       0x04
#define FAT16_ATTR_VOLUME_ID    0x08
#define FAT16_ATTR_SUBDIRECTORY 0x10
#define FAT16_ATTR_ARCHIVE      0x20
#define FAT16_ATTR_DEVICE       0x40
#define FAT16_ATTR_RESERVED     0x80

#define FAT16_FILE_NAME_LENGTH 8
#define FAT16_FILE_EXT_LENGTH  3

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
    uint8_t file_name[FAT16_FILE_NAME_LENGTH];
    uint8_t file_extension[FAT16_FILE_EXT_LENGTH];
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
    struct fat_directory_item* items;
    uint32_t count;
    int sector_position;
    int last_sector_position;
};

struct fat_item
{
    FAT_ITEM_TYPE type;
    union
    {
        struct fat_directory* directory;
        struct fat_directory_item* item;
    };
};

struct fat_private_data
{
    struct fat_header_universal header;
    struct fat_directory root_directory;

    struct disk_stream* data_stream; // for data clusters
    struct disk_stream* fat_stream;  // for file allocation table
    struct disk_stream* dir_stream;  // for directory entries
};

// Represents a file descriptor for an item in the FAT file system
struct fat_file_descriptor
{
    struct fat_item* item;
    uint32_t position;
};

status_t fat16_resolve(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode);
size_t fat16_read(struct disk* disk, void* fd, uint32_t size, uint32_t count, char* out);
status_t fat16_seek(void* private_data, uint32_t offset, FILE_SEEK_MODE mode);
status_t fat16_stat(void* private_data, struct file_stat* stat);
status_t fat16_close(void* private_data);

struct file_system fat16_fs = {
    .name = "FAT16",
    .resolve = fat16_resolve,
    .open = fat16_open,
    .read = fat16_read,
    .seek = fat16_seek,
    .stat = fat16_stat,
    .close = fat16_close,
};

struct file_system*
fat16_initialize()
{
    return &fat16_fs;
}

static void
fat16_private_initialize(struct disk* disk, struct fat_private_data* private_data)
{
    memset(private_data, 0, sizeof(struct fat_private_data));

    private_data->data_stream = disk_stream_open(disk->id);
    private_data->fat_stream = disk_stream_open(disk->id);
    private_data->dir_stream = disk_stream_open(disk->id);

    disk->private_data = private_data;
    disk->fs = &fat16_fs;
}

int
fat16_get_total_items_for_directory(
  struct disk_stream* stream,
  struct fat_private_data* private_data,
  uint32_t directory_sector_position
)
{
    struct fat_directory_item item;

    status_t result = 0;
    int count = 0;

    result = disk_stream_seek(stream, directory_sector_position * private_data->header.primary.bytes_per_sector);
    if (result != ALL_OK) {
        goto out;
    }

    while (1) {
        result = disk_stream_read(stream, (char*)&item, sizeof(struct fat_directory_item));
        if (result != ALL_OK) {
            goto out;
        }

        // End of the items
        if (item.file_name[0] == 0) {
            goto out;
        }

        // 0xE5 means the item is free (available)
        if (item.file_name[0] == 0xE5) {
            continue;
        }

        count++;
    }

out:
    if (stream) {
        disk_stream_close(stream);
    }
    if (result == ALL_OK) {
        return count;
    }
    return (int)result;
}

status_t
fat16_get_root_directory(
  struct disk_stream* stream,
  struct fat_private_data* private_data,
  struct fat_directory* directory
)
{
    status_t result = ALL_OK;

    struct fat_header* primary = &private_data->header.primary;

    // calculate the root directory sector
    // Note that `root_dir_entries` is the max entries this root directory can hold, not the actual
    // items in the root directory.
    uint32_t root_dir_size = primary->root_dir_entries * sizeof(struct fat_directory_item);
    uint32_t root_dir_sectors = (root_dir_size + (primary->bytes_per_sector - 1)) /
                                primary->bytes_per_sector; // add `bytes_per_sector - 1` to round up
    uint32_t root_dir_sector_pos = (primary->fat_copies * primary->sectors_per_fat) + primary->reserved_sectors;

    directory->items = (struct fat_directory_item*)kzalloc(root_dir_size);
    if (directory->items == 0) {
        result = ERROR(ENOMEM);
        goto out;
    }

    // read the root directory
    result = disk_stream_seek(stream, root_dir_sector_pos * primary->bytes_per_sector);
    if (result != ALL_OK) {
        goto out;
    }

    result = disk_stream_read(stream, (char*)directory->items, root_dir_size);
    if (result != ALL_OK) {
        goto out;
    }

    directory->count = fat16_get_total_items_for_directory(stream, private_data, root_dir_sector_pos);
    directory->sector_position = root_dir_sector_pos;
    directory->last_sector_position = root_dir_sector_pos + root_dir_sectors;

out:
    if (result != ALL_OK) {
        if (directory->items) {
            kfree(directory->items);
        }
    }
    return result;
}

status_t
fat16_resolve(struct disk* disk)
{
    status_t result = ALL_OK;
    struct disk_stream* stream = 0;

    struct fat_private_data* private_data = (struct fat_private_data*)kzalloc(sizeof(struct fat_private_data));
    fat16_private_initialize(disk, private_data);

    if (private_data->data_stream == 0 || private_data->fat_stream == 0 || private_data->dir_stream == 0) {
        result = ERROR(ENOMEM);
        goto out;
    }

    // We'll leave the streams in `private_data` untouched, and use new streams for reading the
    // header and root directory.
    stream = disk_stream_open(disk->id);

    result = disk_stream_read(stream, (char*)&private_data->header, sizeof(struct fat_header_universal));
    if (result != ALL_OK) {
        goto out;
    }

    // check if this is a FAT16 file system
    if (private_data->header.extended.signature != 0x29) {
        result = ERROR(EFSNOTSUPPORTED);
        goto out;
    }

    result = fat16_get_root_directory(stream, private_data, &private_data->root_directory);
    if (result != ALL_OK) {
        goto out;
    }

out:
    if (result != ALL_OK) {
        if (private_data->data_stream) {
            disk_stream_close(private_data->data_stream);
        }
        if (private_data->fat_stream) {
            disk_stream_close(private_data->fat_stream);
        }
        if (private_data->dir_stream) {
            disk_stream_close(private_data->dir_stream);
        }
        if (private_data) {
            kfree(private_data);
        }
        disk->private_data = 0;
    }
    if (stream) {
        disk_stream_close(stream);
    }
    return result;
}

void
fat16_free_directory(struct fat_directory* directory)
{
    if (!directory) {
        return;
    }
    if (directory->items) {
        kfree(directory->items);
    }
    kfree(directory);
}

void
fat16_free_item(struct fat_item* item)
{
    if (!item) {
        return;
    }
    if (item->type == FAT_ITEM_TYPE_DIRECTORY) {
        fat16_free_directory(item->directory);
    } else if (item->type == FAT_ITEM_TYPE_FILE) {
        kfree(item->item);
    }
    kfree(item);
}

// replace the first white space with null terminator
void
fat16_to_proper_string(char** out, const char* in, size_t length)
{
    while (*in != ' ' && *in != 0 && length > 0) {
        **out = *in;
        (*out)++;
        in++;
        length--;
    }
    **out = 0;
}

// join the file name and extension strings with "." in between, removing the white spaces
void
fat16_get_full_relative_filename(struct fat_directory_item* item, char* buffer, size_t buffer_size)
{
    memset(buffer, 0, buffer_size);
    char* out = buffer;
    fat16_to_proper_string(&out, (const char*)item->file_name, sizeof(item->file_name));
    if (item->file_extension[0] != ' ' && item->file_extension[0] != 0) {
        *out++ = '.';
        fat16_to_proper_string(&out, (const char*)item->file_extension, sizeof(item->file_extension));
    }
}

static uint32_t
fat16_get_first_cluster(struct fat_directory_item* item)
{
    return (item->first_cluster_high << 16) | item->first_cluster_low;
}

static uint32_t
fat16_get_first_fat_sector(struct fat_private_data* private_data)
{
    return private_data->header.primary.reserved_sectors;
}

static int
fat16_cluster_to_sector(struct fat_private_data* private_data, int cluster)
{
    return ((cluster - 2) * private_data->header.primary.sectors_per_cluster) +
           private_data->root_directory.last_sector_position;
}

static uint16_t
fat16_get_fat_entry(struct disk* disk, int cluster)
{
    status_t result = ALL_OK;

    struct fat_private_data* private_data = (struct fat_private_data*)disk->private_data;
    int fat_sector = fat16_get_first_fat_sector(private_data) * disk->sector_size;
    result = disk_stream_seek(private_data->fat_stream, fat_sector + (cluster * FAT16_FAT_ENTRY_SIZE));
    if (result != ALL_OK) {
        goto out;
    }

    uint16_t fat_entry;
    result = disk_stream_read(private_data->fat_stream, (char*)&fat_entry, sizeof(fat_entry));
    if (result != ALL_OK) {
        goto out;
    }

out:
    if (result != ALL_OK) {
        return 0;
    }
    return fat_entry;
}

// get the correct cluster to read based on the starting cluster and the offset
static int
fat16_get_cluster_for_offset(struct disk* disk, int starting_cluster, int offset)
{
    struct fat_private_data* private_data = (struct fat_private_data*)disk->private_data;

    int cluster_size = private_data->header.primary.sectors_per_cluster * disk->sector_size;
    int clusters_to_read = offset / cluster_size;
    int cluster_to_return = starting_cluster;

    // todo: why do we need to read each cluster? can't we just read the cluster we need?
    //       like this?
    //       int entry = fat16_get_fat_entry(private_data, starting_cluster * clusters_to_read);
    for (int _i = 0; _i < clusters_to_read; _i++) {
        int entry = (int)fat16_get_fat_entry(disk, cluster_to_return);
        if (entry == 0xFF8 || entry == -0xFFF) // todo: define constants
        {
            // end of the entries
            return -EIO;
        }

        // bad sector?
        if (entry == FAT16_BAD_SECTOR) {
            return -EIO;
        }

        // reserved sector?
        if (entry == 0xFF0 || entry == 0xFF6) {
            return -EIO;
        }

        // corrupted?
        if (entry == 0) {
            return -EIO;
        }
        cluster_to_return = entry;
    }

    return cluster_to_return;
}

static status_t
fat16_read_internal(struct disk* disk, int cluster, int offset, int length, void* buffer)
{
    struct fat_private_data* private_data = (struct fat_private_data*)disk->private_data;
    struct disk_stream* stream = private_data->data_stream;

    int cluster_size = private_data->header.primary.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = fat16_get_cluster_for_offset(disk, cluster, offset);
    if (cluster_to_use < 0) {
        return ERROR(cluster_to_use);
    }

    int offset_from_cluster = offset % cluster_size;
    int sector_to_read =
      (fat16_cluster_to_sector(private_data, cluster_to_use) * disk->sector_size) + offset_from_cluster;
    int total_to_read = length > cluster_size ? cluster_size : length;

    status_t result = disk_stream_seek(stream, sector_to_read);
    if (result != ALL_OK) {
        return result;
    }
    result = disk_stream_read(stream, buffer, total_to_read);
    if (result != ALL_OK) {
        return result;
    }

    length -= total_to_read;
    if (length > 0) {
        // todo: avoid recursion in production
        result = fat16_read_internal(disk, cluster_to_use, offset + total_to_read, length, buffer + total_to_read);
    }

    return result;
}

struct fat_directory*
fat16_load_fat_directory(struct disk* disk, struct fat_directory_item* item)
{
    status_t result = ALL_OK;

    if (!(item->attributes & FAT16_ATTR_SUBDIRECTORY)) {
        return 0;
    }

    struct fat_directory* directory = (struct fat_directory*)kzalloc(sizeof(struct fat_directory));
    if (!directory) {
        return 0;
    }

    struct fat_private_data* private_data = (struct fat_private_data*)disk->private_data;
    int cluster = fat16_get_first_cluster(item);
    int directory_sector_pos = fat16_cluster_to_sector(private_data, cluster);

    directory->count =
      fat16_get_total_items_for_directory(private_data->dir_stream, private_data, directory_sector_pos);
    int directory_size = directory->count * sizeof(struct fat_directory_item);
    directory->items = kzalloc(directory_size);
    if (!directory->items) {
        result = ERROR(ENOMEM);
        goto out;
    }

    result = fat16_read_internal(disk, cluster, 0x00, directory_size, directory->items);
    if (result != ALL_OK) {
        goto out;
    }

out:
    if (result != ALL_OK) {
        fat16_free_directory(directory);
        return 0;
    }
    return directory;
}

struct fat_directory_item*
fat16_clone_directory_item(struct fat_directory_item* item, size_t size)
{
    if (size < sizeof(struct fat_directory_item)) {
        return 0;
    }
    struct fat_directory_item* clone = (struct fat_directory_item*)kzalloc(size);
    if (!clone) {
        return 0;
    }
    memcpy(clone, item, size);
    return clone;
}

struct fat_item*
fat16_new_fat_item_or_directory_item(struct disk* disk, struct fat_directory_item* item)
{
    struct fat_item* fat_item = (struct fat_item*)kzalloc(sizeof(struct fat_item));
    if (!fat_item) {
        return 0;
    }

    if (item->attributes & FAT16_ATTR_SUBDIRECTORY) {
        fat_item->directory = fat16_load_fat_directory(disk, item);
        fat_item->type = FAT_ITEM_TYPE_DIRECTORY;
    } else {
        // make a clone of the directory item here. `item` is a pointer that could be freed at any
        // time.
        fat_item->item = fat16_clone_directory_item(item, sizeof(struct fat_directory_item));
        fat_item->type = FAT_ITEM_TYPE_FILE;
    }

    return fat_item;
}

struct fat_item*
fat16_find_item_in_directory(struct disk* disk, struct fat_directory* directory, const char* name)
{
    struct fat_item* item = 0;
    char tmp_filename[MAX_PATH_LENGTH];
    for (int i = 0; i < directory->count; i++) {
        fat16_get_full_relative_filename(&directory->items[i], tmp_filename, sizeof(tmp_filename));
        if (istrncmp(tmp_filename, name, MAX_PATH_LENGTH) == 0) {
            item = fat16_new_fat_item_or_directory_item(disk, &directory->items[i]);
            break;
        }
    }
    return item;
}

struct fat_item*
fat16_get_directory_entry(struct disk* disk, struct path_part* path)
{
    struct fat_private_data* private_data = (struct fat_private_data*)disk->private_data;
    struct fat_item* current_item = 0;
    struct fat_item* root_item = fat16_find_item_in_directory(disk, &private_data->root_directory, path->name);
    if (!root_item) {
        goto out;
    }

    struct path_part* next_part = path->next;
    current_item = root_item;
    while (next_part) {
        if (current_item->type != FAT_ITEM_TYPE_DIRECTORY) {
            current_item = 0;
            goto out;
        }

        struct fat_item* tmp_item = fat16_find_item_in_directory(disk, current_item->directory, next_part->name);
        fat16_free_item(current_item);
        current_item = tmp_item;
        next_part = next_part->next;
    }

out:
    return current_item;
}

void*
fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode)
{
    status_t result = ALL_OK;

    if (mode != FILE_MODE_READ) {
        result = ERROR(EREADONLY);
        goto out;
    }

    struct fat_file_descriptor* descriptor = 0;
    descriptor = kzalloc(sizeof(struct fat_file_descriptor));
    if (!descriptor) {
        result = ERROR(ENOMEM);
        goto out;
    }

    descriptor->item = fat16_get_directory_entry(disk, path);
    if (!descriptor->item) {
        result = ERROR(EIO);
        goto out;
    }

    descriptor->position = 0;

out:
    if (result != ALL_OK) {
        kfree(descriptor);
        descriptor = 0;
    }
    return descriptor;
}

size_t
fat16_read(struct disk* disk, void* fd, size_t size, size_t count, char* out)
{
    status_t result = ALL_OK;

    struct fat_file_descriptor* descriptor = fd;
    struct fat_directory_item* item = descriptor->item->item;
    int offset = descriptor->position;

    size_t read_items = 0;
    for (uint32_t i = 0; i < count; i++) {
        result = fat16_read_internal(disk, fat16_get_first_cluster(item), offset, size, out);
        if (result != ALL_OK) {
            goto out;
        }
        offset += size;
        out += size;
        read_items++;
    }

out:
    return read_items;
}

status_t
fat16_seek(void* private_data, uint32_t offset, FILE_SEEK_MODE mode)
{
    status_t result = ALL_OK;

    struct fat_file_descriptor* descriptor = private_data;
    struct fat_item* item = descriptor->item;

    if (item->type != FAT_ITEM_TYPE_FILE) {
        result = ERROR(EINVARG);
        goto out;
    }

    struct fat_directory_item* file_item = item->item;
    if (offset > file_item->file_size) {
        result = ERROR(EIO);
        goto out;
    }

    switch (mode) {
        case FILE_SEEK_SET:
            descriptor->position = offset;
            break;
        case FILE_SEEK_CUR:
            descriptor->position += offset;
            break;
        case FILE_SEEK_END:
            descriptor->position = file_item->file_size + offset;
            break;
        default:
            result = ERROR(EINVARG);
            break;
    }

out:
    return result;
}

status_t
fat16_stat(void* private_data, struct file_stat* stat)
{
    status_t result = ALL_OK;

    struct fat_file_descriptor* descriptor = private_data;
    struct fat_item* item = descriptor->item;

    if (item->type != FAT_ITEM_TYPE_FILE) {
        result = ERROR(EINVARG);
        goto out;
    }

    struct fat_directory_item* file_item = item->item;
    stat->size = file_item->file_size;
    stat->flags = 0;
    if (file_item->attributes & FAT16_ATTR_HIDDEN) {
        stat->flags |= FILE_STAT_FLAG_HIDDEN;
    }
    if (file_item->attributes & FAT16_ATTR_SYSTEM) {
        stat->flags |= FILE_STAT_FLAG_SYSTEM;
    }
    if (file_item->attributes & FAT16_ATTR_READ_ONLY) {
        stat->flags |= FILE_STAT_FLAG_READONLY;
    }
    if (file_item->attributes & FAT16_ATTR_ARCHIVE) {
        stat->flags |= FILE_STAT_FLAG_ARCHIVE;
    }

out:
    return result;
}

status_t
fat16_close(void* private_data)
{
    struct fat_file_descriptor* descriptor = private_data;
    fat16_free_item(descriptor->item);
    kfree(descriptor);
    return ALL_OK;
}
