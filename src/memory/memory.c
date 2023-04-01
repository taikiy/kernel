#include "memory.h"

void *memset(void *ptr, int c, size_t size)
{
    char *c_ptr = (char *)ptr;

    for (int i = 0; i < size; i++)
    {
        c_ptr[i] = (char)c;
    }

    return ptr;
}

int memcmp(const void *ptr1, const void *ptr2, size_t size)
{
    const char *c_ptr1 = ptr1;
    const char *c_ptr2 = (const char *)ptr2;

    for (int i = 0; i < size; i++)
    {
        if (c_ptr1[i] != c_ptr2[i])
        {
            return c_ptr1[i] - c_ptr2[i];
        }
    }

    return 0;
}
