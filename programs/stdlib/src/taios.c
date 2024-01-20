#include "taios.h"
#include "stdlib.h"
#include "string.h"
#include <stddef.h>
#include <stdint.h>

extern int make_syscall(uint32_t syscall_id, int argc, ...);

struct command_args*
parse_command(const char* command)
{
    char* command_copy = malloc(strlen(command) + 1);
    strcpy(command_copy, command);
    command_copy[strlen(command)] = '\0';

    struct command_args* head = NULL;
    struct command_args* tail = NULL;
    struct command_args* arg = NULL;
    char* token = strtok(command_copy, " ");
    while (token != NULL) {
        arg = malloc(sizeof(struct command_args));
        arg->value = token;
        arg->next = NULL;
        if (head == NULL) {
            head = arg;
        } else {
            tail->next = arg;
        }
        tail = arg;
        token = strtok(NULL, " ");
    }
    return head;
}

int
exec(const char* path)
{
    if (!path) {
        return -1;
    }

    struct command_args* command = parse_command(path);
    if (!command) {
        return -1;
    }

    int result = make_syscall(SYSCALL_EXEC, 1, (uint32_t)command);

    // free the command args
    while (command) {
        struct command_args* next = command->next;
        free(command);
        command = next;
    }

    return result;
}
