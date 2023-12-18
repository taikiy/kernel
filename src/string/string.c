#include "string.h"

char tolower(char s1)
{
    if (s1 >= 65 && s1 <= 90)
    {
        s1 += 32;
    }

    return s1;
}

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
    unsigned char u1, u2;
    while (true)
    {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2)
        {
            return u1 - u2;
        }
        if (u1 == '\0')
        {
            return 0;
        }
    }
}

int strncmp(const char *str1, const char *str2, int n)
{
    unsigned char u1, u2;
    while (n-- > 0)
    {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2)
        {
            return u1 - u2;
        }
        if (u1 == '\0')
        {
            return 0;
        }
    }
    return 0;
}

int istrncmp(const char *str1, const char *str2, int n)
{
    unsigned char u1, u2;
    while (n-- > 0)
    {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2 && tolower(u1) != tolower(u2))
        {
            return u1 - u2;
        }
        if (u1 == '\0')
        {
            return 0;
        }
    }
    return 0;
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

char *strncpy(char *dest, const char *src, int n)
{
    int i = 0;
    while (src[i] && i < n)
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
    return dest;
}

char *strcat(char *dest, const char *src)
{
    int i = 0;
    int j = 0;
    while (dest[i])
    {
        i++;
    }
    while (src[j])
    {
        dest[i] = src[j];
        i++;
        j++;
    }
    dest[i] = 0;
    return dest;
}

char *strncat(char *dest, const char *src, int n)
{
    int i = 0;
    int j = 0;
    while (dest[i])
    {
        i++;
    }
    while (src[j] && j < n)
    {
        dest[i] = src[j];
        i++;
        j++;
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