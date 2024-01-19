#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char** argv)
{
    printf("Hello, World from User Program Written in C!\n");

    const int mem_size = 100;
    char* str = malloc(mem_size);
    if (!str) {
        printf("malloc failed\n");
        return 1;
    }
    printf("malloc(%d) succeeded\n", mem_size);

    printf("Enter 'Q' to quit\n");
    while (1) {
        printf("Enter a string: ");
        gets(str);
        if (*str == 'Q') {
            break;
        }
        printf("You entered: ");
        printf("%s\n", str);
    }

    free(str);
    printf("free succeeded\n");

    return 0;
}
