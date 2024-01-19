#include "string.h"

char
tolower(char s1)
{
    if (s1 >= 65 && s1 <= 90) {
        s1 += 32;
    }

    return s1;
}

char*
strrev(char* str)
{
    int i = 0;
    int j = strlen(str) - 1;
    while (i < j) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
    return str;
}

int
strlen(const char* ptr)
{
    int len = 0;
    while (ptr[len]) {
        len++;
    }
    return len;
}

int
strnlen(const char* ptr, int max)
{
    int len = 0;
    while (ptr[len] && len < max) {
        len++;
    }
    return len;
}

int
strcmp(const char* str1, const char* str2)
{
    unsigned char u1, u2;
    while (true) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2) {
            return u1 - u2;
        }
        if (u1 == '\0') {
            return 0;
        }
    }
}

int
strncmp(const char* str1, const char* str2, int n)
{
    unsigned char u1, u2;
    while (n-- > 0) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2) {
            return u1 - u2;
        }
        if (u1 == '\0') {
            return 0;
        }
    }
    return 0;
}

int
istrncmp(const char* str1, const char* str2, int n)
{
    unsigned char u1, u2;
    while (n-- > 0) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2 && tolower(u1) != tolower(u2)) {
            return u1 - u2;
        }
        if (u1 == '\0') {
            return 0;
        }
    }
    return 0;
}

char*
strcpy(char* dest, const char* src)
{
    int i = 0;
    while (src[i]) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
    return dest;
}

char*
strncpy(char* dest, const char* src, int n)
{
    int i = 0;
    while (src[i] && i < n) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
    return dest;
}

char*
strcat(char* dest, const char* src)
{
    int i = 0;
    int j = 0;
    while (dest[i]) {
        i++;
    }
    while (src[j]) {
        dest[i] = src[j];
        i++;
        j++;
    }
    dest[i] = 0;
    return dest;
}

char*
strncat(char* dest, const char* src, int n)
{
    int i = 0;
    int j = 0;
    while (dest[i]) {
        i++;
    }
    while (src[j] && j < n) {
        dest[i] = src[j];
        i++;
        j++;
    }
    dest[i] = 0;
    return dest;
}

int
atoi(const char* str)
{
    int i = 0;
    int result = 0;
    while (str[i]) {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return result;
}

char*
itoa(int n, char* str, int base)
{
    int i = 0;
    bool is_negative = false;
    if (n == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    if (n < 0 && base == 10) {
        is_negative = true;
        n = -n;
    }
    while (n != 0) {
        int rem = n % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        n = n / base;
    }
    if (is_negative) {
        str[i++] = '-';
    }
    str[i] = '\0';
    strrev(str);
    return str;
}

bool
isdigit(char c)
{
    return c >= '0' && c <= '9';
}

char*
strchr(const char* s, int c)
{
    while (*s != (char)c) {
        if (!*s++) {
            return 0;
        }
    }
    return (char*)s;
}

char* sp = 0;
char*
strtok(char* str, const char* delim)
{
    if (str) {
        sp = str;
    }
    if (!sp) {
        return 0;
    }

    char* token_start = sp;
    char* token_end = 0;

    while (*sp) {
        if (!strchr(delim, *sp)) {
            sp++;
            continue;
        }

        do {
            *sp = 0; // null terminate the token
            token_end = sp;
            sp++;
        } while (*sp && strchr(delim, *sp));
        break;
    }

    if (!token_end) {
        token_end = sp;
    }
    if (token_start == token_end) {
        return 0;
    }
    return token_start;
}
