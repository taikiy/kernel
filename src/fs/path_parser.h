#ifndef PATH_PARSER_H
#define PATH_PARSER_H

struct path_root
{
    int drive_number;
    struct path_part *first;
};

struct path_part
{
    char *name;
    struct path_part *next;
};

struct path_root *path_parse(const char *path, const char *current_directory_path);
void path_free(struct path_root *root);

#endif
