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

    free(str);
    printf("free succeeded\n");

    while (1) {
        int c = getchar();
        if (c == 'Q') {
            break;
        }
        putchar(c);
    }
    while (1) {}
    return 0;
}
