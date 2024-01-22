#include "process.h"
#include "../config.h"
#include "../memory/heap/kheap.h"
#include "../memory/paging/paging.h"
#include "../status.h"
#include "../string/string.h"
#include "./syscall.h"

static void
free_command_args(struct command_args* command)
{
    struct command_args* next = command;
    // Note that we just need to free the first value pointer, because everything else is just an offset from it.
    // We could call kfree() for every command->value, but freeing the memory needs to scan the whole heap table,
    // which is expensive. So we just free the first one.
    kfree(next->value);
    while (next) {
        struct command_args* current = next;
        next = current->next;
        kfree(current);
    }
}

static struct command_args*
copy_command_args_from_user_space(char* arg)
{
    status_t status = ALL_OK;

    struct command_args* head = 0;
    struct command_args* tail = 0;
    struct command_args* next = (struct command_args*)arg;
    do {
        struct command_args* command = kzalloc(sizeof(struct command_args));
        status = copy_data_from_user_space(get_current_task(), next, command, sizeof(struct command_args));
        if (status != ALL_OK) {
            if (command) {
                kfree(command);
            }
            goto out;
        }

        if (!head) {
            // this is the first command arg and the `value` is the whole argument string, so we malloc once
            char* value = kzalloc(MAX_COMMAND_LENGTH);
            status = copy_data_from_user_space(get_current_task(), command->value, value, MAX_COMMAND_LENGTH);
            if (status != ALL_OK) {
                goto out;
            }
            command->value = value;
            head = command;
        } else {
            // this is not the first command arg and the `value` is just an offset from the first command arg's `value`
            command->value = tail->value + strlen(tail->value) + 1;
            tail->next = command;
        }

        tail = command;
        next = command->next;
    } while (next);

out:
    if (status != ALL_OK) {
        if (head) {
            free_command_args(head);
        }
        head = 0;
    }

    return head;
}

void*
sys_exec(struct interrupt_frame* frame)
{
    status_t status = ALL_OK;

    char* arg = (char*)get_arg_from_task(get_current_task(), 0);
    struct command_args* command = copy_command_args_from_user_space(arg);
    if (!command) {
        return (void*)ERROR(EINVARG);
    }

    struct process* proc = 0;
    status = create_process_and_switch(command, &proc);

    return (void*)status;
}
