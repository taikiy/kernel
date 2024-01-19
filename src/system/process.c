#include "process.h"
#include "../config.h"
#include "../memory/paging/paging.h"
#include "../status.h"
#include "./syscall.h"

void*
sys_exec(struct interrupt_frame* frame)
{
    status_t status = ALL_OK;

    char* arg = (char*)get_arg_from_task(get_current_task(), 0);
    char file_name[MAX_PATH_LENGTH];
    status = copy_data_from_user_space(get_current_task(), arg, file_name, MAX_PATH_LENGTH);
    if (status != ALL_OK) {
        return (void*)status;
    }

    struct process* proc = 0;
    status = create_process_and_switch(file_name, &proc);

    return (void*)status;
}
