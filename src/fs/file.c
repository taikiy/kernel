#include "file.h"
#include "config.h"
#include "terminal/terminal.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"

struct file_system *file_systems[MAX_FILE_SYSTEM_COUNT];
struct file_descriptor *file_descriptors[MAX_FILE_DESCRIPTOR_COUNT];

static struct file_system **fs_get_free_file_system()
{
    for (int i = 0; i < MAX_FILE_SYSTEM_COUNT; i++)
    {
        if (file_systems[i] == 0)
        {
            return &file_systems[i];
        }
    }
    return 0;
}

void fs_insert_file_system(struct file_system *fs)
{
    struct file_system **free_fs = fs_get_free_file_system();
    if (free_fs == 0)
    {
        // TODO: Error handling
        print("No free file system slots\n");
        while (1)
        {
        };
    }
    *free_fs = fs;
}

static struct file_system *fat16_initialize()
{
    // struct file_system *fat16 = (struct file_system *)kzalloc(sizeof(struct file_system));
    // fat16->name = "fat16";
    // fat16->open = fat16_open;
    // fat16->resolve = fat16_resolve;

    // return fat16;
    return 0;
}

static void fs_load_static_file_systems()
{
    fs_insert_file_system(fat16_initialize());
}

static void fs_load_file_systems()
{
    fs_load_static_file_systems();
}

void file_system_initialize()
{
    memset(file_systems, 0, sizeof(file_systems));
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load_file_systems();
}

static status_t new_file_descriptor(struct file_descriptor **fd)
{
    int res = -ENOMEM;

    for (int i = 0; i < MAX_FILE_DESCRIPTOR_COUNT; i++)
    {
        if (file_descriptors[i] == 0)
        {
            struct file_descriptor *new_fd = (struct file_descriptor *)kzalloc(sizeof(struct file_descriptor));
            // File descriptors start at 1
            new_fd->index = i + 1;
            file_descriptors[i] = new_fd;
            *fd = new_fd;
            res = 0;
            break;
        }
    }
    return res;
}

static struct file_descriptor *get_file_descriptor(int fd)
{
    if (fd < 1 || fd > MAX_FILE_DESCRIPTOR_COUNT)
    {
        return 0;
    }
    return file_descriptors[fd - 1];
}

struct file_system *fs_resolve(struct disk *disk)
{
    for (int i = 0; i < MAX_FILE_SYSTEM_COUNT; i++)
    {
        if (file_systems[i] != 0 && file_systems[i]->resolve(disk) == 0)
        {
            return file_systems[i];
        }
    }
    return 0;
}

status_t fopen(const char *file_name, const char *mode)
{
    return -EIO;
}
