#include "fat16.h"
#include "status.h"
#include <stdint.h>
#include "disk/disk.h"
#include "disk/stream.h"

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
    struct fat_header header;
    union
    {
        struct fat_header_extended extended;
    } shared;
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

    struct disk_stream *stream;     // for data clusters
    struct disk_stream *fat_stream; // for file allocation table
    struct disk_stream *dir_stream; // for directory entries
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
