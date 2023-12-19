#include "file.h"
#include "config.h"
#include "terminal/terminal.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "fat/fat16.h"
#include "status.h"
#include "string/string.h"

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

static status_t put_file_descriptor(struct file_descriptor **fd)
{
    status_t result = ERROR(ENOMEM);

    for (int i = 0; i < MAX_FILE_DESCRIPTOR_COUNT; i++)
    {
        if (file_descriptors[i] == 0)
        {
            struct file_descriptor *new_fd = (struct file_descriptor *)kzalloc(sizeof(struct file_descriptor));
            // File descriptors start at 1
            new_fd->index = i + 1;
            file_descriptors[i] = new_fd;
            *fd = new_fd;
            result = 0;
            break;
        }
    }
    return result;
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
        if (file_systems[i] != 0 && file_systems[i]->resolve(disk) == ALL_OK)
        {
            return file_systems[i];
        }
    }
    return 0;
}

static FILE_MODE file_mode_str_to_enum(const char *mode_str)
{
    if (strcmp(mode_str, "r") == 0)
    {
        return FILE_MODE_READ;
    }
    else if (strcmp(mode_str, "w") == 0)
    {
        return FILE_MODE_WRITE;
    }
    else if (strcmp(mode_str, "a") == 0)
    {
        return FILE_MODE_APPEND;
    }
    else
    {
        return FILE_MODE_INVALID;
    }
}

int fopen(const char *file_name, const char *mode)
{
    status_t result = ALL_OK;

    struct path_root *path = path_parse(file_name, NULL);

    // invalid path, or just the root directory
    if (!path || !path->first)
    {
        result = ERROR(EINVPATH);
        goto out;
    }

    struct disk *disk = get_disk(path->drive_number);

    // invalid disk number, or no file system
    if (!disk || !disk->fs)
    {
        result = ERROR(EIO);
        goto out;
    }

    FILE_MODE file_mode = file_mode_str_to_enum(mode);

    // invalid file mode
    if (file_mode == FILE_MODE_INVALID)
    {
        result = ERROR(EINVARG);
        goto out;
    }

    void *private_data = disk->fs->open(disk, path->first, file_mode);
    if (!private_data)
    {
        result = ERROR(EIO);
        goto out;
    }

    struct file_descriptor *fd = 0;
    result = put_file_descriptor(&fd);
    if (result != ALL_OK)
    {
        goto out;
    }
    fd->data = private_data;
    fd->disk = disk;

out:
    // fopen returns 0 on error
    if (result != ALL_OK)
        fd->index = 0;

    return fd->index;
}

status_t fread(void *ptr, uint32_t size, uint32_t count, int fd)
{
    status_t result = ALL_OK;
    if (size == 0 || count == 0 || fd < 1)
    {
        result = ERROR(EINVARG);
        goto out;
    }

    struct file_descriptor *descriptor = get_file_descriptor(fd);
    if (!descriptor)
    {
        result = ERROR(EINVARG);
        goto out;
    }

out:
    if (result != ALL_OK)
        return 0;

    return descriptor->disk->fs->read(descriptor->disk, descriptor->data, size, count, (char *)ptr);
}

status_t fseek(int fd, uint32_t offset, FILE_SEEK_MODE mode)
{
    status_t result = ALL_OK;

    struct file_descriptor *descriptor = get_file_descriptor(fd);
    if (!descriptor)
    {
        result = ERROR(EINVARG);
        goto out;
    }

    result = descriptor->disk->fs->seek(descriptor->data, offset, mode);

out:
    return result;
}
