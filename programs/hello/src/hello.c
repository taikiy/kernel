#include <stdlib.h>

int
main(int argc, char** argv)
{
    print("Hello, World from User Program Written in C!\n");

    char* str = malloc(100);
    if (!str) {
        print("malloc failed\n");
        return 1;
    }
    print("malloc succeeded\n");

    free(str);
    print("free succeeded\n");

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
