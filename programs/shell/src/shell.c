#include "shell.h"
#include "stdio.h"
#include "string.h"
#include "taios.h"
#include <stddef.h>

int
main(int argc, char* argv[])
{
    printf("\ntaiOS v0.1\n");

    char command[MAX_COMMAND_LENGTH];
    while (1) {
        printf("\n> ");

        gets(command);

        char path[MAX_COMMAND_LENGTH] = "0:/";
        strcat(path, command);

        int result = exec(path);
        if (result != 0) {
            printf("exec failed: %d\n", result);
        }
    }

    return 0;
}
