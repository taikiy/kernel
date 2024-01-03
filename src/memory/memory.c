#include "memory.h"

void*
memset(void* ptr, int c, size_t size)
{
    char* c_ptr = (char*)ptr;

    for (int i = 0; i < size; i++) {
        c_ptr[i] = (char)c;
    }

    return ptr;
}

/// @brief Compares byte string
/// @param ptr1 Byte string 1
/// @param ptr2 Byte string 2
/// @param size Both strings' size in bytes
/// @return Returns zero if the two strings are identical, otherwise returns the difference between the first two
/// differing bytes.
int
memcmp(const void* s1, const void* s2, size_t size)
{
    const char* c_ptr1 = s1;
    const char* c_ptr2 = s2;

    for (int i = 0; i < size; i++) {
        if (c_ptr1[i] != c_ptr2[i]) {
            return c_ptr1[i] - c_ptr2[i];
        }
    }

    return 0;
}

void*
memcpy(void* dest, const void* src, size_t size)
{
    char* c_dest = (char*)dest;
    const char* c_src = (const char*)src;

    for (int i = 0; i < size; i++) {
        c_dest[i] = c_src[i];
    }

    return dest;
}
