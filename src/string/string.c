#include "string.h"

int strlen(const char *ptr)
{
    int len = 0;
    while (ptr[len])
    {
        len++;
    }
    return len;
}

int strnlen(const char *ptr, int max)
{
    int len = 0;
    while (ptr[len] && len < max)
    {
        len++;
    }
    return len;
}

int strcmp(const char *str1, const char *str2)
{
    int i = 0;
    while (str1[i] && str2[i])
    {
        if (str1[i] != str2[i])
        {
            return 0;
        }
        i++;
    }
    return 1;
}

char *strcpy(char *dest, const char *src)
{
    int i = 0;
    while (src[i])
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
    return dest;
}

int atoi(const char *str)
{
    int i = 0;
    int result = 0;
    while (str[i])
    {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return result;
}

bool isdigit(char c)
{
    return c >= '0' && c <= '9';
}
