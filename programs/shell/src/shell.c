#include "shell.h"
#include "stdio.h"

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
    }
}
