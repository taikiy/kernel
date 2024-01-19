#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
test_strtok()
{
    printf("strtok()\n");

    // NOTE: strtok modifies the string it is given, so we can't do `char* str = ...`.
    // In C, string literals are immutable and stored in `.rodata`. `char* str = ...` would
    // create a pointer to `.rodata`, which is immutable, so we would get a segfault when
    // strtok tries to modify it.
    char str[] = "This .is a test string. It has many words.";
    char* expected[] = { "This", "is", "a", "test", "string", "It", "has", "many", "words" };

    int i = 0;
    for (char* word = strtok(str, " ."); word; word = strtok(NULL, " .")) {
        if (strcmp(word, expected[i++])) {
            printf("FAIL: expected \"%s\", got \"%s\"\n", expected[i - 1], word);
            return;
        }
    }
    printf("OK");
}

int
main(int argc, char** argv)
{
    printf("Standard library tests\n\n");

    test_strtok();

    while (1) {}
    return 0;
}
