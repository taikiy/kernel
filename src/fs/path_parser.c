#include "path_parser.h"
#include "config.h"
#include "string/string.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "status.h"

const unsigned int MIN_PATH_LENGTH = 3;

/// @brief Validates the path format. Our path format will be: <drive_number>:/<path>
/// @param path
/// @return 1 if the path is valid, 0 otherwise
static int valid_format(const char *path)
{
    int len = strnlen(path, MAX_PATH_LENGTH);
    return (len >= MIN_PATH_LENGTH && isdigit(path[0]) && memcmp(&path[1], ":/", 2) == 0);
}

static int get_drive_by_path(const char **path)
{
    if (!valid_format(*path))
    {
        return -EINVPATH;
    }

    int drive_number = *path[0] - '0';

    // skip 3 bytes: <drive_number>:/ and set the path to the first path part
    *path += 3;

    return drive_number;
}

static struct path_root *create_root(int drive_numer)
{
    struct path_root *root = kzalloc(sizeof(struct path_root));
    root->drive_number = drive_numer;
    root->first = 0;
    return root;
}

static char *get_path_part(const char **path)
{
    char *part = kzalloc(MAX_PATH_LENGTH);

    int i = 0;
    while (**path && **path != '/')
    {
        part[i++] = **path;
        (*path)++;
    }
    if (**path == '/')
    {
        (*path)++;
    }

    if (i == 0)
    {
        kfree(part);
        return 0;
    }

    return part;
}

static struct path_part *parse_path_part(struct path_part *last, const char **path)
{
    char *part_str = get_path_part(path);
    if (!part_str)
    {
        return 0;
    }

    struct path_part *part = kzalloc(sizeof(struct path_part));
    part->name = part_str;
    part->next = 0;

    if (last)
    {
        last->next = part;
    }

    return part;
}

void path_free(struct path_root *root)
{
    if (!root)
    {
        return;
    }

    struct path_part *part = root->first;
    while (part)
    {
        struct path_part *next = part->next;
        kfree(part->name);
        kfree(part);
        part = next;
    }

    kfree(root);
}

struct path_root *path_parse(const char *path, const char *current_directory_path)
{
    const char *tmp_path = path;
    struct path_root *root = NULL;

    if (strlen(path) > MAX_PATH_LENGTH)
    {
        goto out;
    }

    int drive_number = get_drive_by_path(&tmp_path);
    if (drive_number < 0)
    {
        goto out;
    }

    root = create_root(drive_number);
    if (!root)
    {
        goto out;
    }

    struct path_part *first = parse_path_part(0, &tmp_path);
    if (!first)
    {
        goto out;
    }

    root->first = first;
    struct path_part *part = parse_path_part(first, &tmp_path);
    while (part)
    {
        part = parse_path_part(part, &tmp_path);
    }

out:
    return root;
}
