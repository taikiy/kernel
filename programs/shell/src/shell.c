#include "shell.h"
#include "stdio.h"
#include "taios.h"

int
main(int argc, char* argv[])
{
    printf("\ntaiOS v0.1\n\n");
    char command[4098];
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
