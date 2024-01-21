#include "shell.h"
#include "stdio.h"
#include "string.h"
#include "taios.h"
#include <stddef.h>

int
main(int argc, char* argv[])
{
    printf("\ntaiOS v0.1\n\n");

    char command[MAX_COMMAND_LENGTH];
    while (1) {
        printf("> ");

        gets(command);
        if (*command == 'Q') {
            printf("Quit\n");
            break;
        }

        int result = exec(command);
        if (result != 0) {
            printf("exec failed: %d\n", result);
        }
    }
}
