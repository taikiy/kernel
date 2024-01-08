#include <stdlib.h>

int
main(int argc, char** argv)
{
    print("Hello, World from User Program Written in C!\n");
    while (1) {
        int c = getchar();
        if (c == 'q') {
            break;
        }
        putchar(c);
    }
    while (1) {}
    return 0;
}
