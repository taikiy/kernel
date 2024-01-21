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
    struct command_args* command = 0;
    void* next = arg;
    do {
        command = kzalloc(sizeof(struct command_args)); // no error checking here. if it fails, next line fails too
        status = copy_data_from_user_space(get_current_task(), next, command, sizeof(struct command_args));
        if (status != ALL_OK) {
            goto out;
        }

        if (!head) {
            // head's `value` is the user space malloc'ed string ptr
            char* value = kzalloc(MAX_COMMAND_LENGTH);
            status = copy_data_from_user_space(get_current_task(), command->value, value, MAX_COMMAND_LENGTH);
            if (status != ALL_OK) {
                goto out;
            }
            command->value = value;
            head = command;
        } else {
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
